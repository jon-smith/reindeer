#pragma once

#include <vector>

namespace reindeer
{
	struct ContinuousAxisParameters;

	struct AxisTickLabels
	{
		std::vector<std::wstring> labels;
		std::wstring labelAnnotation;
	};

	double calculateIntervalForNiceTicks(const double min, const double max, const size_t maxNumberOfIntervals, const bool allowAxisEndOffsets);
	ContinuousAxisParameters calculateAxisParamsForNiceTicks(const double min, const double max, const size_t maxNumberOfIntervals, const bool allowAxisEndOffsets);

	std::vector<double> getTickPositions(const ContinuousAxisParameters &axis);

	AxisTickLabels getTickLabels(const ContinuousAxisParameters &axis);
}