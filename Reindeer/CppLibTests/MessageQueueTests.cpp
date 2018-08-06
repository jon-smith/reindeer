#include "stdafx.h"
#include "CppUnitTest.h"

#include "ReindeerLib\MessageQueue.h"
#include "FormatString.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace reindeer;

namespace CppLibTests
{
	TEST_CLASS(MessageQueueTests)
	{
		const std::string serverAddress = "tcp://*:5555";
		const std::string clientAddress = "tcp://localhost:5555";

	public:

		TEST_METHOD(ServerSmokeTest)
		{
			SimpleServer server(serverAddress, {}, std::chrono::milliseconds(30));
		}

		TEST_METHOD(ClientSmokeTest)
		{
			SimpleClient client(clientAddress);
		}

		TEST_METHOD(MessageQueueSmokeTest)
		{
			const auto serverFn = [](const std::string &msg)
			{
				Logger::WriteMessage(std::string("Received: " + msg).c_str());
				std::this_thread::sleep_for(std::chrono::milliseconds(100));
				return ("Message");
			};

			SimpleServer server(serverAddress, serverFn, std::chrono::milliseconds(30));
			SimpleClient client(clientAddress);

			const std::vector<std::string> toSend = {
				"one",
				"two",
				"three"
			};

			for (const auto &msg : toSend)
			{
				client.sendMessageAndWaitForReply(msg);
			}
			server.kill();
		}
	};
}