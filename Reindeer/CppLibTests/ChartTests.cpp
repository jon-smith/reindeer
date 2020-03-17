#include "stdafx.h"
#include "CppUnitTest.h"

#include "FormatString.hpp"
#include "ReindeerLib/TickHelpers.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace reindeer;

namespace CppLibTests
{
	TEST_CLASS(ChartTests)
	{
		TEST_METHOD(NiceTickIntervals_MinMaxAdjust)
		{
			// 0.0, 1.0, 2.0 ... 10.0
			const auto tickInc = calculateIntervalForNiceTicks(0.0, 10.0, 11, false);
			Assert::AreEqual(1.0, tickInc, L"0-10, 11 int");

			// 0.0, 0.2, ... 1.0
			const auto tickInc2 = calculateIntervalForNiceTicks(0.0, 1.0, 7, false);
			Assert::AreEqual(0.2, tickInc2, L"0-1 7int");

			// 0.0, 0.2, ... 1.0
			const auto tickInc3 = calculateIntervalForNiceTicks(0.0, 2.5, 7, false);
			Assert::AreEqual(0.5, tickInc3, L"0-2.5 7int");

		}

		TEST_METHOD(NiceTickIntervals_FixMinMax)
		{
			// 0.0, 1.0, 2.0 ... 10.0
			const auto tickInc = calculateIntervalForNiceTicks(0.1, 9.9, 11, true);
			Assert::AreEqual(1.0, tickInc, L"0-10, 11 int");

			// 0.0, 0.2, ... 1.0
			const auto tickInc2 = calculateIntervalForNiceTicks(0.1, 0.9, 7, true);
			Assert::AreEqual(0.2, tickInc2, L"0-1 7int");

			// 0.0, 0.5 ...
			const auto tickInc3 = calculateIntervalForNiceTicks(0.0, 2.6, 7, true);
			Assert::AreEqual(0.5, tickInc3, L"0-2.6 7int");

			// 0.0, 1.0, 2.0 ... 10.0
			const auto tickInc4 = calculateIntervalForNiceTicks(0.1, 10.1, 11, true);
			Assert::AreEqual(1.0, tickInc4, L"0.1-10.1, 11 int");
		}
	};
}