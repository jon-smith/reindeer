#include "stdafx.h"
#include "CppUnitTest.h"

#include <algorithm>
#include <vector>

#include "../PointGenLib_Rust/PointGenLib.h"
#include "FormatString.hpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace CppLibTests
{
	TEST_CLASS(PointGenLib)
	{
	public:

		// Test the quality of our random number generator using some basic metrics
		// This isn't very statistical, just something to give me an idea of if it's working or not
		TEST_METHOD(TestCMWC)
		{
			// Decide how many random numbers we want to test
			constexpr auto nNumbers = 100000000;

			// Define min and max so update as we get new random numbesr
			uint32_t minR = std::numeric_limits<uint32_t>::max();
			uint32_t maxR = 0;

			// Create a histogram to bin random number to investigate uniformity
			constexpr auto binWidth = uint32_t(2) << 20;
			constexpr auto nBins = 1 + std::numeric_limits<uint32_t>::max() / binWidth;
			auto bins = std::vector<uint32_t>(nBins);
			
			// Calculate average and average squared
			double average = 0.0;
			double averageSq = 0.0;
			constexpr auto multiplier = 1.0/static_cast<double>(nNumbers);

			// Create a number of random numbers, update histogram and min/max
			for (auto i = 0; i < nNumbers; ++i)
			{
				auto const rand = pointgen_random_cmwc();

				auto const dRand = static_cast<double>(rand);
				average += dRand*multiplier;
				averageSq += dRand*dRand*multiplier;

				++bins[rand / binWidth];
				
				minR = std::min(minR, rand);
				maxR = std::max(maxR, rand);
			}

			// Confirm that histogram bins are within a sensible tolerance
			for (auto const b : bins)
			{
				auto const proportionInBin = static_cast<double>(b*bins.size()) / static_cast<double>(nNumbers);
				Assert::AreEqual(1.0, proportionInBin, 0.15, L"Bin outside of tolerance");
			}

			// Print mean and variance and assert if not within sensible tolerance
			auto const dUint32Max = static_cast<double>(std::numeric_limits<uint32_t>::max());
			auto const expectedMean = dUint32Max*0.5;
			auto const expectedVariance = dUint32Max*dUint32Max / 12.0;
			auto const variance = averageSq - average*average;
			Logger::WriteMessage(obelisk::formatString(L"Mean %f", average).c_str());
			Logger::WriteMessage(obelisk::formatString(L"Expected mean %f", expectedMean).c_str());
			Logger::WriteMessage(obelisk::formatString(L"Var %f", variance).c_str());
			Logger::WriteMessage(obelisk::formatString(L"Expected var %f", expectedVariance).c_str());
			Assert::AreEqual(expectedMean, average, expectedMean / 1000.0, L"Mean outside of tolerance");
			Assert::AreEqual(expectedVariance, variance, expectedVariance / 1000.0, L"Variance outside of tolerance");

			// Print min and max and assert if not within a sensible tolerance
			Logger::WriteMessage(obelisk::formatString(L"Min %u", minR).c_str());
			Logger::WriteMessage(obelisk::formatString(L"Max %u", maxR).c_str());
			Assert::IsTrue(minR < 1000, L"Min outside of tolerance");
			Assert::IsTrue(maxR > std::numeric_limits<uint32_t>::max() - 1000, L"Max outside of tolerance");
		}

		TEST_METHOD(TestUniformRandom)
		{	
			// Decide how many random numbers we want to test
			constexpr auto nNumbers = 100000000;

			// Calculate average and average squared
			double average = 0.0;
			double averageSq = 0.0;
			constexpr auto multiplier = 1.0 / static_cast<double>(nNumbers);

			// Create a number of random numbers, update histogram and min/max
			for (auto i = 0; i < nNumbers; ++i)
			{
				auto const rand = pointgen_random_uniform_double();
				average += rand*multiplier;
				averageSq += rand*rand*multiplier;
			}

			// Print mean and variance and assert if not within sensible tolerance
			auto const expectedMean = 0.5;
			auto const expectedVariance = 1.0 / 12.0;
			auto const variance = averageSq - average*average;
			Logger::WriteMessage(obelisk::formatString(L"Mean %f", average).c_str());
			Logger::WriteMessage(obelisk::formatString(L"Expected mean %f", expectedMean).c_str());
			Logger::WriteMessage(obelisk::formatString(L"Var %f", variance).c_str());
			Logger::WriteMessage(obelisk::formatString(L"Expected var %f", expectedVariance).c_str());
			Assert::AreEqual(expectedMean, average, expectedMean / 1000.0, L"Mean outside of tolerance");
			Assert::AreEqual(expectedVariance, variance, expectedVariance / 1000.0, L"Variance outside of tolerance");
		}

		TEST_METHOD(TestCMWCSeeds)
		{
			auto const defaultFirstRand = pointgen_random_cmwc();
			// Try a different seed
			pointgen_set_cmwc_seed(1);
			auto const randAfterSeed1 = pointgen_random_cmwc();
			// And another seed
			pointgen_set_cmwc_seed(20);
			auto const randAfterSeed20 = pointgen_random_cmwc();
			// Check that seed 1 is consistent
			pointgen_set_cmwc_seed(1);
			auto const randAfterSeed1Again = pointgen_random_cmwc();

			// Check different seeds are different (of course, if they were equal it wouldn't mean it isn't working, as they are /random/)
			Assert::AreNotEqual(defaultFirstRand, randAfterSeed1, L"Default and seed 1 should not be equal");
			Assert::AreNotEqual(defaultFirstRand, randAfterSeed20, L"Default and seed 20 should not be equal");
			Assert::AreEqual(randAfterSeed1, randAfterSeed1Again, L"First random after seed 1 should be equal");
		}
	};
}