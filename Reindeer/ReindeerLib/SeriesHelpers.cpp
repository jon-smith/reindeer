#include "SeriesHelpers.h"

#include <algorithm>

using namespace obelisk;

namespace
{
	void updateMinMax(std::pair <Vector2d, Vector2d> &current, Vector2d value)
	{
		// Update min
		if (value.x < current.first.x)
		{
			current.first.x = value.x;
		}

		if (value.y < current.first.y)
		{
			current.first.y = value.y;
		}

		// Update max
		if (value.x > current.second.x)
		{
			current.second.x = value.x;
		}

		if (value.y > current.second.y)
		{
			current.second.y = value.y;
		}
	}

	void updateMinMax(std::pair <Vector2d, Vector2d> &current, const std::pair <Vector2d, Vector2d> &value)
	{
		// Update min
		if (value.first.x < current.first.x)
		{
			current.first.x = value.first.x;
		}

		if (value.first.y < current.first.y)
		{
			current.first.y = value.first.y;
		}

		// Update max
		if (value.second.x > current.second.x)
		{
			current.second.x = value.second.x;
		}

		if (value.second.y > current.second.y)
		{
			current.second.y = value.second.y;
		}
	}
}

namespace reindeer
{
	void sortDataOnX(XYSeries &series)
	{
		std::sort(series.data.begin(), series.data.end(), [](const Vector2d &l, const Vector2d &r) { return l.x < r.x; });
	}

	std::pair<Vector2d, Vector2d> getRange(const XYSeries &series)
	{
		auto minMax = NULL_MIN_MAX;

		for (const auto &d : series.data)
		{
			if (d.y != XYSeries::ABSENT_VALUE)
			{
				updateMinMax(minMax, d);
			}
		}

		return minMax;
	}

	std::pair<obelisk::Vector2d, obelisk::Vector2d> getRange(const std::vector<XYSeries> &series)
	{
		auto minMax = NULL_MIN_MAX;

		for (auto const &s : series)
		{
			auto seriesMinMax = getRange(s);

			// No min/max has been set, continue
			if (seriesMinMax == NULL_MIN_MAX)
				continue;

			updateMinMax(minMax, seriesMinMax);
		}

		return minMax;
	}
}