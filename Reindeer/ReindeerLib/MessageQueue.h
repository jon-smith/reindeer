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
			std::chrono::milliseconds pollInterval);

		~ConsumeReplyServer();

		unsigned messagesReceived() const;
		unsigned messagesProcessed() const;

		void kill();

	private:

		void serverThread(const std::string &bindAddress);

		std::future<void> serverTask;
		const std::function<std::string(const std::string &)> processMessageReturnReply;
		const std::chrono::milliseconds pollInterval;

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

	class PublishServer
	{
	public:
		PublishServer(const std::string &bindAddress);
		~PublishServer();

		void publish(const std::string &message);

	private:
		struct Impl;
		const std::unique_ptr<Impl> impl;
	};

	class SubscriberClient
	{
	public:
		SubscriberClient(const std::string &connectionAddress,
			std::function<void(const std::string &)> processMessage,
			std::chrono::milliseconds pollInterval);

		~SubscriberClient();

		void kill();

	private:

		void threadFunction(const std::string &connectionAddress);

		std::future<void> threadTask;
		const std::function<void(const std::string &)> processMessage;
		const std::chrono::milliseconds pollInterval;

		std::atomic_bool hasConnected{ false };
		std::atomic_bool killFlag{ false };
	};

	// Use RequestWorker to define worker task
	// RequestClients can send a string request and will receive a reply when started with the id of the worker
	class LoadBalancingBroker
	{
	public:
		LoadBalancingBroker(const std::string &clientAddress,
			const std::string &serverAddress);

		~LoadBalancingBroker();

	private:

		void threadFunction(const std::string &clientAddress,
			const std::string &serverAddress);

		std::atomic_bool killFlag{ false };
		std::future<void> threadTask;
	};

	// Worker class to use with LoadBalancingBroker
	class RequestWorker
	{
	public:
		RequestWorker(const std::string &connectAddress,
			std::function<void(const std::string &)> processRequest,
			std::chrono::milliseconds pollInterval);

		~RequestWorker();

		void kill();

	private:

		void serverThread(const std::string &connectAddress);

		std::future<void> serverTask;
		const std::function<void(const std::string &)> processRequest;
		const std::chrono::milliseconds pollInterval;

		std::atomic_bool hasConnected{ false };
		std::atomic_bool killFlag{ false };
	};
}