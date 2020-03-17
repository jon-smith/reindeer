#include "TickHelpers.h"

#include <array>
#include <cassert>

#include "ChartStructures.h"

#include "StringFuncs.h"
#include "FormatString.hpp"

using namespace reindeer;

namespace
{
	std::vector<int> splitNumberIntoDigits(int number)
	{
		if (number == 0)
		{
			return { 0 };
		}

		std::vector<int> digits;
		// Build number in reverse
		while (number != 0)
		{
			int last = number % 10;
			digits.push_back(last);
			number = (number - last) / 10;
		}

		std::reverse(std::begin(digits), std::begin(digits));

		return digits;
	}

	// Get number from 0-9 as super script character
	wchar_t getNumberAsSuperScript(unsigned n)
	{
		switch (n)
		{
		case 0:
			return L'⁰';
		case 1:
			return L'¹';
		case 2:
			return L'²';
		case 3:
			return L'³';
		case 4:
			return L'⁴';
		case 5:
			return L'⁵';
		case 6:
			return L'⁶';
		case 7:
			return L'⁷';
		case 8:
			return L'⁸';
		case 9:
			return L'⁹';
		}

		assert(false);
		return L'?';
	}

	std::pair<double, int> getStandardForm(double val)
	{
		if (val == 0.0)
			return {};

		auto numberOfZeroes = static_cast<int>(log10(std::abs(val)));
		return std::make_pair(val / static_cast<double>(pow(10, numberOfZeroes)), numberOfZeroes);
	}

	std::wstring getStandardFormExponentPartString(int exponent)
	{
		std::wstring str;
		if (exponent != 0)
		{
			str += L"x10";
			if (exponent < 0)
				str += L"⁻";

			const auto digits = splitNumberIntoDigits(std::abs(exponent));
			// Only need to write the 1 if negative (10¹ = 10)
			if (exponent < 0 || digits.size() == 1)
			{
				str += L"¹";
			}
			else
			{
				for (const auto d : digits)
				{
					str += getNumberAsSuperScript(static_cast<unsigned>(std::abs(d)));
				}
			}
		}
		return str;
	}
}

double reindeer::calculateIntervalForNiceTicks(const double min, const double max, const size_t maxNumberOfIntervals, const bool allowAxisEndOffsets)
{
	const std::array<double, 6> tickSpacingsToTry{ 0.5, 0.2, 0.1, 0.05, 0.02, 0.01 };

	// Calculate a start spacing that will be too large and then look for smaller spaces until it fits the max intervals
	const auto maxTickSpacing = (max - min) / std::max(1, static_cast<int>(maxNumberOfIntervals) - 2);

	// Get the magnitude of the spacing relative to the spacings to try
	const auto power = std::ceil(std::log10(maxTickSpacing));

	double currentBestTickSpacing = std::pow(10.0, power);

	for (const auto &tickStartDigit : tickSpacingsToTry)
	{
		const auto tickSpacing = tickStartDigit * std::pow(10.0, power);

		if (!allowAxisEndOffsets)
		{
			if ((std::ceil(max / tickSpacing) - std::floor(min / tickSpacing)) > maxNumberOfIntervals)
				return currentBestTickSpacing;
		}
		else
		{
			if ((std::floor(max / tickSpacing) - std::ceil(min / tickSpacing)) > maxNumberOfIntervals)
				return currentBestTickSpacing;
		}

		currentBestTickSpacing = tickSpacing;
	}

	// Should never get here
	assert(L"Nice ticks not found");
	return currentBestTickSpacing;
}

ContinuousAxisParameters reindeer::calculateAxisParamsForNiceTicks(const double min, const double max, const size_t maxNumberOfIntervals, const bool allowAxisEndOffsets)
{
	const auto interval = calculateIntervalForNiceTicks(min, max, maxNumberOfIntervals, allowAxisEndOffsets);
	const auto minToUse = std::floor(min / interval) * interval;

	ContinuousAxisParameters axis;
	axis.min = minToUse;
	axis.inc = interval;

	if (allowAxisEndOffsets)
	{
		const auto lowestTickVal = std::ceil(minToUse / axis.inc) * axis.inc;
		const auto highestTickVal = std::floor(max / axis.inc) * axis.inc;

		axis.nInc = static_cast<size_t>((highestTickVal - lowestTickVal) / axis.inc);
		axis.firstOffset = lowestTickVal - minToUse;
		axis.finalOffset = max - highestTickVal;
	}
	else
	{
		// Find a number of increments that fits in the range
		axis.firstOffset = 0.0;
		axis.finalOffset = 0.0;
		axis.nInc = static_cast<size_t>(std::ceil(max / axis.inc) - std::floor(axis.min / axis.inc));
	}

	return axis;
}

std::vector<double> reindeer::getTickPositions(const ContinuousAxisParameters &axis)
{
	std::vector<double> tickPositions;

	auto nextTick = axis.min + axis.firstOffset;
	for (size_t i = 0; i < axis.nInc + 1; ++i)
	{
		tickPositions.push_back(nextTick);
		nextTick += axis.inc;
	}

	return tickPositions;
}

AxisTickLabels reindeer::getTickLabels(const ContinuousAxisParameters &axis)
{
	const auto tickVals = getTickPositions(axis);

	auto maximumExponent = tickVals.empty() ? 0 : std::numeric_limits<int>::lowest();
	for (auto &t : tickVals)
	{
		auto exponent = getStandardForm(t).second;
		if (exponent > maximumExponent)
			maximumExponent = exponent;
	}

	// If exponent is between -3 and 3, don't make any adjustment
	// e.g. display 0.01 and 100
	if (maximumExponent > -3 && maximumExponent < 3)
		maximumExponent = 0;

	const auto exponentScaling = 1.0 / pow(10, maximumExponent);
	std::vector<std::wstring> labels;
	for (auto &t : tickVals)
	{
		const auto scaledT = t * exponentScaling;

		labels.push_back(obelisk::formatString(L"%.3f", scaledT));
	}

	const auto standardFormPostFix = getStandardFormExponentPartString(maximumExponent);

	AxisTickLabels result;
	result.labels = labels;
	result.labelAnnotation = standardFormPostFix;
	return result;
}
