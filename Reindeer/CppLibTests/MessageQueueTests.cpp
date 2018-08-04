#include "stdafx.h"
#include "CppUnitTest.h"

#include "ReindeerLib\MessageQueue.h"
#include "FormatString.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CppLibTests
{
	TEST_CLASS(MessageQueueTests)
	{
	public:

		TEST_METHOD(MessageQueueSmokeTest)
		{
			reindeer::MessageQueue::test();
		}
	};
}