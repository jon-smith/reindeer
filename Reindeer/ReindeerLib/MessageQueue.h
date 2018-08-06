#pragma once

#include <future>
#include <atomic>

namespace reindeer
{
	class ConsumeReplyServer
	{
	public:
		ConsumeReplyServer(const std::string &bindAddress,
			std::function<std::string(const std::string &)> processMessageReturnReply,
			std::chrono::milliseconds maxWaitTime);

		~ConsumeReplyServer();

		unsigned messagesReceived() const;
		unsigned messagesProcessed() const;

		void kill();

	private:

		void serverThread(const std::string &bindAddress);

		std::future<void> serverTask;
		const std::function<std::string(const std::string &)> processMessageReturnReply;
		const std::chrono::milliseconds maxWaitTime;

		std::atomic<unsigned> nMessagesReceived{ 0 };
		std::atomic<unsigned> nMessagesProcessed{ 0 };
		std::atomic_bool hasConnected{ false };
		std::atomic_bool killFlag{ false };
	};

	class RequestClient
	{
	public:
		RequestClient(const std::string &connectionAddress);
		~RequestClient();

		std::string sendMessageAndWaitForReply(const std::string &msg);

	private:
		struct Impl;
		const std::unique_ptr<Impl> impl;
	};
}