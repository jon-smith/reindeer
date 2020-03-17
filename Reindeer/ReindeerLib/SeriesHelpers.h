#pragma once

#include "ChartStructures.h"

namespace reindeer
{
	// Get a pointer to use with glVertexPointer
	// glVertexPointer(2, GL_DOUBLE, 0, series.getGlVertexPointer2D());
	inline const double *getGlVertexPointer2D(const XYSeries &series)
	{
		if (series.data.empty())
			return nullptr;

		return reinterpret_cast<const double *>(series.data.data());
	}

	void sortDataOnX(XYSeries &series);

	constexpr obelisk::Vector2d MAX_VEC_2D(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());
	constexpr obelisk::Vector2d MIN_VEC_2D(-std::numeric_limits<double>::max(), -std::numeric_limits<double>::max());
	constexpr auto NULL_MIN_MAX = std::pair(MAX_VEC_2D, MIN_VEC_2D);

	std::pair<obelisk::Vector2d, obelisk::Vector2d> getRange(const XYSeries &series);
	std::pair<obelisk::Vector2d, obelisk::Vector2d> getRange(const std::vector<XYSeries> &series);
}