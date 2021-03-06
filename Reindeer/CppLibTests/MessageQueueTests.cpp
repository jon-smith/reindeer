#include "stdafx.h"
#include "CppUnitTest.h"

#include "ReindeerLib\MessageQueue.h"
#include "FormatString.hpp"
#include "StdLockUtilsT.h"
#include "ContainerMaker.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace reindeer;

namespace CppLibTests
{
	TEST_CLASS(MessageQueueTests)
	{
		const std::string serverAddress = "tcp://*:5555";
		const std::string clientAddress = "tcp://localhost:5555";

		const std::string serverAddress2 = "tcp://*:5556";
		const std::string clientAddress2 = "tcp://localhost:5556";

	public:

		TEST_METHOD(ConsumeReplyServerInitState)
		{
			ConsumeReplyServer server(serverAddress, {}, std::chrono::milliseconds(30));
			Assert::IsTrue(server.messagesProcessed() == 0, L"Initial message processed count non-zero");
			Assert::IsTrue(server.messagesReceived() == 0, L"Initial message received count non-zero");
		}

		TEST_METHOD(RequestClientSmokeTest)
		{
			RequestClient client(clientAddress);
		}

		TEST_METHOD(ServerClientCommEmptyReplies)
		{
			const auto serverFn = [](const std::string &msg)
			{
				return std::string{};
			};

			ConsumeReplyServer server(serverAddress, serverFn, std::chrono::milliseconds(30));
			RequestClient client(clientAddress);

			const auto reply = client.sendMessageAndWaitForReply("Message");

			Assert::IsTrue(reply.empty(), L"Reply is not empty");

			Assert::IsTrue(server.messagesProcessed() == 1, L"Message processed count incorrect");
			Assert::IsTrue(server.messagesReceived() == 1, L"Message received count incorrect");
		}

		TEST_METHOD(ServerClientCommWithReplies)
		{
			// Simple reverse function
			const auto serverFn = [](const std::string &msg)
			{
				return std::string(rbegin(msg), rend(msg));
			};

			ConsumeReplyServer server(serverAddress, serverFn, std::chrono::milliseconds(30));
			RequestClient client(clientAddress);

			const auto message = "Message";
			const auto reply = client.sendMessageAndWaitForReply("Message");

			Assert::AreEqual(reply, serverFn(message), L"Reply is not as expected");

			Assert::IsTrue(server.messagesProcessed() == 1, L"Message processed count incorrect");
			Assert::IsTrue(server.messagesReceived() == 1, L"Message received count incorrect");
		}

		TEST_METHOD(ServerClientCommWithLogOutput)
		{
			const auto serverFn = [](const std::string &msg)
			{
				Logger::WriteMessage(std::string("Received: " + msg).c_str());
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				return ("Message");
			};

			ConsumeReplyServer server(serverAddress, serverFn, std::chrono::milliseconds(30));
			RequestClient client(clientAddress);

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

		TEST_METHOD(PublisherSubscribeTest)
		{
			std::vector<std::string> received;
			std::mutex m;

			const auto messageFn = [&received, &m](const std::string &msg)
			{
				obelisk::lockAndCall(m, [&received, &msg]() {
					received.push_back(msg);
				});

				Logger::WriteMessage(std::string("Received: " + msg).c_str());
				std::this_thread::sleep_for(std::chrono::milliseconds(30));
			};

			PublishServer server(serverAddress);
			SubscriberClient client(clientAddress, messageFn, std::chrono::milliseconds(30));

			while (true)
			{
				if (obelisk::lockCallAndReturn(m, [&received]() {
					return received.size() > 3;
				}))
				{
					break;
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(30));

				server.publish("Message");
			}
		}

		TEST_METHOD(LoadBalancer)
		{
			LoadBalancingBroker broker(serverAddress, serverAddress2);

			constexpr auto N_CLIENTS = 3;
			constexpr auto N_WORKERS = 2;
			constexpr auto N_REQ_PER_CLIENT = 5;

			auto clients = obelisk::generateVector<std::unique_ptr<RequestClient>>(
				[this](size_t i) {
				return std::make_unique<RequestClient>(clientAddress);
			}, 0, N_CLIENTS);

			std::atomic<unsigned> workCount(0);
			const auto poll = std::chrono::milliseconds(2);
			const auto workerFunc = [&workCount](const std::string &msg)
			{
				Logger::WriteMessage(std::string("Received: " + msg).c_str());
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
				++workCount;
			};

			auto workers = obelisk::generateVector<std::unique_ptr<RequestWorker>>(
				[this, workerFunc, poll](size_t i) {
				return std::make_unique<RequestWorker>(clientAddress2, workerFunc, poll);
			}, 0, N_WORKERS);

			
			for (size_t i = 0; i < N_REQ_PER_CLIENT; ++i)
			{
				for (auto &client : clients)
				{
					const auto reply = client->sendMessageAndWaitForReply("Running request index " + std::to_string(i));
					Logger::WriteMessage(std::string("Client received: " + reply).c_str());
				}
			}

			// Let it have its time
			while (workCount < N_REQ_PER_CLIENT*N_CLIENTS)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(10));
			}

			Assert::IsTrue(workCount == N_REQ_PER_CLIENT*N_CLIENTS);
		}
	};
}