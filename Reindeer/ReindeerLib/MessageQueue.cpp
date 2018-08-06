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

}

unsigned SimpleServer::messagesProcessed() const
{

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
			const auto requestAsStr = std::string(request.str());
			const auto result = processMessageReturnReply(requestAsStr);

			++nMessagesProcessed;

			if (!result.empty())
			{
				zmq::message_t reply(std::begin(result), std::end(result));
				socket.send(reply);
			}
		}
	}
}

void MessageQueue::test()
{
	const auto serverFn = [](const std::string &msg)
	{
		log("Received: " + msg);
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		return ("Message");
	};

	SimpleServer server("tcp://*:5555", serverFn, std::chrono::milliseconds(30));	

	auto client = std::async(std::launch::async, []() {
		zmq::context_t context{};
		zmq::socket_t socket(context, ZMQ_REQ);
		socket.connect("tcp://localhost:5555");

		const std::vector<std::string> toSend = {
			"one",
			"two",
			"three"
		};

		for (const auto &msg : toSend)
		{
			zmq::message_t zMsg(msg.size());
			memcpy((void*)zMsg.data(), msg.data(), msg.size());
			socket.send(zMsg);

			zmq::message_t request;
			socket.recv(&request);
		}
	});
	
	client.wait();

	server.kill();

	return;
}