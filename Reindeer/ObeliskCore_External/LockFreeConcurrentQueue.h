#pragma once

#include <concurrent_queue.h>

namespace obelisk
{
	// Wrapper for concurrency::concurrent_queue
	// Can be changed easily if we want to not rely on ppl
	// Details of current implementation https://msdn.microsoft.com/en-us/library/ee355358.aspx
	// Another potential option can be found at:
	// https://github.com/cameron314/concurrentqueue/blob/master/concurrentqueue.h
	template <typename T>
	class LockFreeConcurrentQueue : public concurrency::concurrent_queue<T>
	{

	};
}