#include "ChartStructures.h"

#include <algorithm>

#include "SeriesHelpers.h"
#include "TickHelpers.h"
#include "StringFuncs.h"
#include "FormatString.hpp"

using namespace obelisk;
using namespace reindeer;

ContinuousAxisParameters reindeer::createAutoscaledAxis(const double min, const double max, size_t maxNInc, bool allowMinMaxOffsets)
{
	// If min and max are the same, just create an abitrary range
	if (min == max)
	{
		ContinuousAxisParameters axis;
		axis.nInc = 3;
		if (axis.min == 0)
		{
			axis.min = 0;
			axis.inc = 1;
		}
		else if (axis.min < 0.0)
		{
			axis.inc = std::abs(min) * 0.2;
			axis.min = min * 1.2;
		}
		else // min > 0.0
		{
			axis.min = min * 0.8;
			axis.inc = min * 0.2;
		}
		return axis;
	}

	const auto minToUse = std::min(max, min);
	const auto maxToUse = std::max(max, min);

	return calculateAxisParamsForNiceTicks(minToUse, maxToUse, maxNInc, allowMinMaxOffsets);
}

ContinuousAxisParameters reindeer::createSimpleAxis(double min, double inc, size_t nInc)
{
	ContinuousAxisParameters axis;
	axis.min = min;
	axis.inc = inc;
	axis.nInc = nInc;
	return axis;
}

Vector2<ContinuousAxisParameters> reindeer::createAutoscaledAxes(const ChartXY &chart, size_t xMaxNInc, size_t yMaxNInc, bool allowMinMaxOffsets)
{
	return createAutoscaledAxes(chart.series, xMaxNInc, yMaxNInc, allowMinMaxOffsets);
}


obelisk::Vector2<ContinuousAxisParameters> reindeer::createAutoscaledAxes(const std::vector<XYSeries> &series, size_t xMaxNInc, size_t yMaxNInc, bool allowMinMaxOffsets)
{
	auto seriesRange = getRange(series);

	// If no min has been set, set to 0
	if (seriesRange.first.x == NULL_MIN_MAX.first.x)
		seriesRange.first.x = 0.0;

	if (seriesRange.first.y == NULL_MIN_MAX.first.y)
		seriesRange.first.y = 0.0;

	// If no max has been set, set to 0
	if (seriesRange.second.x == NULL_MIN_MAX.second.x)
		seriesRange.second.x = 0.0;

	if (seriesRange.second.y == NULL_MIN_MAX.second.y)
		seriesRange.second.y = 0.0;

	const auto xAxis = createAutoscaledAxis(seriesRange.first.x, seriesRange.second.x, xMaxNInc, allowMinMaxOffsets);
	const auto yAxis = createAutoscaledAxis(seriesRange.first.y, seriesRange.second.y, yMaxNInc, allowMinMaxOffsets);

	return { xAxis, yAxis };
}