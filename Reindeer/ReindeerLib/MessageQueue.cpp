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
}

SimpleServer::SimpleServer(const std::string &bindAddress,
	std::function<std::string(const std::string &)> processMessageReturnReply,
	std::chrono::milliseconds maxWaitTime) :
	processMessageReturnReply(processMessageReturnReply),
	maxWaitTime(maxWaitTime)
{
	serverTask = std::async(std::launch::async, [this, bindAddress]() {
		serverThread(bindAddress);
	});
}

SimpleServer::~SimpleServer()
{
	kill();
	serverTask.wait();
}

unsigned SimpleServer::messagesReceived() const
{
	return nMessagesReceived;
}

unsigned SimpleServer::messagesProcessed() const
{
	return nMessagesProcessed;
}

void SimpleServer::kill()
{
	killFlag = true;
}

void SimpleServer::serverThread(const std::string &bindAddress)
{
	zmq::context_t context{};
	zmq::socket_t socket(context, ZMQ_REP);
	socket.bind(bindAddress);

	hasConnected = true;

	while (!killFlag)
	{
		zmq::message_t request;

		// Poll for requests
		while (!killFlag)
		{
			if (socket.recv(&request, ZMQ_DONTWAIT))
			{
				++nMessagesReceived;
				break;
			}
			else
			{
				std::this_thread::sleep_for(maxWaitTime);
			}
		}

		if (!killFlag)
		{
			const auto requestAsStr = std::string(static_cast<const char*>(request.data()), request.size());
			const auto result = processMessageReturnReply(requestAsStr);

			++nMessagesProcessed;

			zmq::message_t reply(std::begin(result), std::end(result));
			socket.send(reply);
		}
	}
}

struct SimpleClient::Impl
{
	Impl() : socket(context, ZMQ_REQ)
	{

	}

	zmq::context_t context{};
	zmq::socket_t socket;
};

SimpleClient::SimpleClient(const std::string &connectionAddress) :
	impl(std::make_unique<Impl>())
{
	impl->socket.connect(connectionAddress);
}

SimpleClient::~SimpleClient() = default;

std::string SimpleClient::sendMessageAndWaitForReply(const std::string &msg)
{
	zmq::message_t zMsg(msg.size());
	memcpy((void*)zMsg.data(), msg.data(), msg.size());
	impl->socket.send(zMsg);

	zmq::message_t request;
	impl->socket.recv(&request);

	return std::string(static_cast<const char*>(request.data()), request.size());
}