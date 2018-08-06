#include "MessageQueue.h"

#include "zmq\zmq.hpp"

#include "PlatformSpecific.h"

#include <atomic>
#include <future>
#include <iostream>

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
			const auto requestAsStr = std::string(static_cast<const char*>(request.data()), request.size());
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

	return std::string(static_cast<const char*>(request.data()), request.size());
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
				const auto messageAsString = std::string(static_cast<const char*>(receivedMessage.data()), receivedMessage.size());
				processMessage(messageAsString);
				break;
			}

			std::this_thread::sleep_for(pollInterval);
		}
	}
}