#include "MessageQueue.h"

#define NOMINMAX
#include "zmq\zmq.hpp"

#include "PlatformSpecific.h"

#include <atomic>
#include <future>
#include <iostream>
#include <deque>
#include <random>

using namespace reindeer;

namespace
{
	void log(const std::string &s)
	{
		std::cout << s << "\n";
		obelisk::platform_utilities::outputDebugString(s);
	}

	zmq::message_t messageFromString(const std::string &str)
	{
		return zmq::message_t(std::begin(str), std::end(str));
	}

	std::string msgDataAsString(const zmq::message_t &msg)
	{
		return std::string(static_cast<const char*>(msg.data()), msg.size());
	}

	void setRandomSocketID(zmq::socket_t &socket)
	{
		unsigned char id[16];
		id[0] = 255;
		std::mt19937 gen(std::random_device{}());
		for (int i = 1; i < 15; ++i)
		{
			id[i] = std::max<unsigned char>(static_cast<unsigned char>(gen()), 1);
		}
		id[15] = '\0';
		socket.setsockopt(ZMQ_IDENTITY, id, 16);
	}

	enum class SocketConnectionType
	{
		BIND, CONNECT
	};

	template <int SocketType, SocketConnectionType ConnectionType>
	struct ContextSocket
	{
		ContextSocket(const std::string &address) :
			socket(context, SocketType)
		{
			setRandomSocketID(socket);

			if constexpr (ConnectionType == SocketConnectionType::BIND)
				socket.bind(address);
			else if constexpr (ConnectionType == SocketConnectionType::CONNECT)
				socket.connect(address);
			else
				static_assert(false);
		}

	private:
		zmq::context_t context{};
	public:
		// Socket is below context to ensure correct order of initialisation
		zmq::socket_t socket;
	};
}

ConsumeReplyServer::ConsumeReplyServer(const std::string &bindAddress,
	std::function<std::string(const std::string &)> processMessageReturnReply,
	std::chrono::milliseconds pollInterval) :
	processMessageReturnReply(processMessageReturnReply),
	pollInterval(pollInterval)
{
	serverTask = std::async(std::launch::async, [this, bindAddress]() {
		serverThread(bindAddress);
	});
}

ConsumeReplyServer::~ConsumeReplyServer()
{
	kill();
	serverTask.wait();
}

unsigned ConsumeReplyServer::messagesReceived() const
{
	return nMessagesReceived;
}

unsigned ConsumeReplyServer::messagesProcessed() const
{
	return nMessagesProcessed;
}

void ConsumeReplyServer::kill()
{
	killFlag = true;
}

void ConsumeReplyServer::serverThread(const std::string &bindAddress)
{
	ContextSocket<ZMQ_REP, SocketConnectionType::BIND> context(bindAddress);

	hasConnected = true;

	while (!killFlag)
	{
		zmq::message_t request;

		// Poll for requests
		while (!killFlag)
		{
			if (context.socket.recv(&request, ZMQ_DONTWAIT))
			{
				++nMessagesReceived;
				break;
			}
			else
			{
				std::this_thread::sleep_for(pollInterval);
			}
		}

		if (!killFlag)
		{
			const auto requestAsStr = msgDataAsString(request);
			const auto result = processMessageReturnReply(requestAsStr);

			++nMessagesProcessed;

			context.socket.send(messageFromString(result));
		}
	}
}

struct RequestClient::Impl : public ContextSocket<ZMQ_REQ, SocketConnectionType::CONNECT>
{
	using ContextSocket::ContextSocket;
};

RequestClient::RequestClient(const std::string &connectionAddress) :
	impl(std::make_unique<Impl>(connectionAddress))
{
}

RequestClient::~RequestClient() = default;

std::string RequestClient::sendMessageAndWaitForReply(const std::string &msg)
{
	impl->socket.send(messageFromString(msg));

	zmq::message_t request;
	impl->socket.recv(&request);

	return msgDataAsString(request);
}

struct PublishServer::Impl : public ContextSocket<ZMQ_PUB, SocketConnectionType::BIND>
{
	using ContextSocket::ContextSocket;
};

PublishServer::PublishServer(const std::string &bindAddress) :
	impl(std::make_unique<Impl>(bindAddress))
{
}

PublishServer::~PublishServer() = default;

void PublishServer::publish(const std::string &message)
{
	impl->socket.send(messageFromString(message));
}

SubscriberClient::SubscriberClient(const std::string &connectionAddress,
	std::function<void(const std::string &)> processMessage,
	std::chrono::milliseconds pollInterval) :
	processMessage(processMessage),
	pollInterval(pollInterval)
{
	threadTask = std::async(std::launch::async, [this, connectionAddress]() {
		threadFunction(connectionAddress);
	});
}

SubscriberClient::~SubscriberClient()
{
	kill();
}

void SubscriberClient::kill()
{
	killFlag = true;
	threadTask.wait();
}

void SubscriberClient::threadFunction(const std::string &connectionAddress)
{
	ContextSocket<ZMQ_SUB, SocketConnectionType::CONNECT> subscriber(connectionAddress);

	// Subscribe to all messages
	subscriber.socket.setsockopt(ZMQ_SUBSCRIBE, "", 0);

	hasConnected = true;

	while (!killFlag)
	{
		zmq::message_t receivedMessage;

		// Poll for messages
		while (!killFlag)
		{
			if (subscriber.socket.recv(&receivedMessage, ZMQ_DONTWAIT))
			{
				const auto messageAsString = msgDataAsString(receivedMessage);
				processMessage(messageAsString);
				break;
			}

			std::this_thread::sleep_for(pollInterval);
		}
	}
}

LoadBalancingBroker::LoadBalancingBroker(const std::string &clientAddress,
	const std::string &serverAddress)
{
	threadTask = std::async(std::launch::async, [this, clientAddress, serverAddress]() {
		threadFunction(clientAddress, serverAddress);
	});
}

LoadBalancingBroker::~LoadBalancingBroker()
{
	killFlag = true;
	threadTask.wait();
}

void LoadBalancingBroker::threadFunction(const std::string &clientAddress,
	const std::string &serverAddress)
{
	zmq::context_t context{};

	zmq::socket_t clientSocket(context, ZMQ_ROUTER);
	zmq::socket_t serverSocket(context, ZMQ_ROUTER);

	clientSocket.bind(clientAddress);
	serverSocket.bind(serverAddress);

	const auto serverPollItemPrototype = zmq::pollitem_t{ serverSocket, 0, ZMQ_POLLIN, 0 };
	const auto clientPollItemPrototype = zmq::pollitem_t{ clientSocket, 0, ZMQ_POLLIN, 0 };

	std::deque<std::string> availableWorkerIds;
	while (!killFlag)
	{
		// Poll for messages from clients and servers(workers)
		// If we don't have available workers, don't poll the clients
		std::vector<zmq::pollitem_t> pollItems = { serverPollItemPrototype };
		if (!availableWorkerIds.empty())
			pollItems.push_back(clientPollItemPrototype);

		// Poll, with an abitrary timeout (so we can kill)
		const auto pollRV = zmq::poll(pollItems, std::chrono::milliseconds(30));
		if (pollRV == -1)
			continue;

		if (pollItems[0].revents & ZMQ_POLLIN)
		{
			zmq::message_t workerID;
			if (serverSocket.recv(&workerID))
			{
				// The worker will always send two extra blank messages
				zmq::message_t empty;
				serverSocket.recv(&empty);
				serverSocket.recv(&empty);

				// We have a new worker ready
				availableWorkerIds.push_back(msgDataAsString(workerID));
			}
		}

		if (pollItems.size() > 1)
		{
			if (pollItems[1].revents & ZMQ_POLLIN)
			{
				// We should receive three messages - the ID, blank and the actual request
				zmq::message_t clientID;
				zmq::message_t empty;
				zmq::message_t clientRequest;

				if (clientSocket.recv(&clientID) &&
					clientSocket.recv(&empty) &&
					clientSocket.recv(&clientRequest))
				{
					const auto workerToUse = availableWorkerIds.front();
					availableWorkerIds.pop_front();

					const auto clientIDString = msgDataAsString(clientID);

					// Send the request to the worker
					const auto sendSuccess =
						// WorkerID + "" is necessary to get to the right worker
						serverSocket.send(messageFromString(workerToUse), ZMQ_SNDMORE) &&
						serverSocket.send(messageFromString(""), ZMQ_SNDMORE) &&
						// We then send the client ID and the request, as the RequestWorker expects
						serverSocket.send(messageFromString(clientIDString), ZMQ_SNDMORE) &&
						serverSocket.send(clientRequest);

					// Start by sending back the clientID and blank for the router to send to the correct place
					clientSocket.send(messageFromString(clientIDString), ZMQ_SNDMORE);
					clientSocket.send(messageFromString(""), ZMQ_SNDMORE);
					// Then send the client id to be used (blank if it failed)
					clientSocket.send(messageFromString(sendSuccess ? workerToUse : ""));
				}
			}
		}
	}
}

RequestWorker::RequestWorker(const std::string &bindAddress,
	std::function<void(const std::string &)> processRequest,
	std::chrono::milliseconds pollInterval) :
	processRequest(processRequest),
	pollInterval(pollInterval)
{
	serverTask = std::async(std::launch::async, [this, bindAddress]() {
		serverThread(bindAddress);
	});
}

RequestWorker::~RequestWorker()
{
	kill();
	serverTask.wait();
}

void RequestWorker::kill()
{
	killFlag = true;
}

void RequestWorker::serverThread(const std::string &connectAddress)
{
	ContextSocket<ZMQ_REQ, SocketConnectionType::CONNECT> context(connectAddress);

	hasConnected = true;	

	while (!killFlag)
	{
		// Send a message to say we're ready
		context.socket.send(messageFromString(""));

		zmq::message_t clientID;
		zmq::message_t request;

		// Poll for requests
		while (!killFlag)
		{
			if (context.socket.recv(&clientID, ZMQ_DONTWAIT))
			{
				context.socket.recv(&request);
				break;
			}
			else
			{
				std::this_thread::sleep_for(pollInterval);
			}
		}

		if (!killFlag)
		{
			const auto requestAsStr = msgDataAsString(request);
			processRequest(requestAsStr);
		}
	}
}