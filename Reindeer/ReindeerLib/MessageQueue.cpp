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

void MessageQueue::test()
{
	std::atomic_bool kill{ false };

	auto server = std::async(std::launch::async, [&kill]() {

		zmq::context_t context{};
		zmq::socket_t socket(context, ZMQ_REP);
		socket.bind("tcp://*:5555");

		while (!kill)
		{
			zmq::message_t request;

			// Wait for the request from the client
			while (!kill && !socket.recv(&request, ZMQ_DONTWAIT))
			{				
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			}

			if (!kill)
			{
				log("Received: " + std::string((char*)request.data()));

				std::this_thread::sleep_for(std::chrono::milliseconds(500));

				// Send back
				const std::string rawMessage("Message");
				zmq::message_t reply(rawMessage.size());
				memcpy((void*)reply.data(), rawMessage.data(), rawMessage.size());
				socket.send(reply);
			}
		}
	});

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
	kill = true;

	server.wait();


	return;
}