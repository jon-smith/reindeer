#pragma once

#include <vector>

#include "VectorT.h"
#include "ColourStructs.h"

namespace reindeer
{
	enum class SeriesType
	{	
		SCATTER,
		LINE
	};

	struct SeriesFormat
	{
		std::wstring name;
		obelisk::ColourRGBA colour;
		float size = 1.0;
		SeriesType type = SeriesType::SCATTER;
	};

	struct XYSeries
	{
		std::vector<obelisk::Vector2<double>> data;
		SeriesFormat format;

		// Value to use to indicate empty/absent/null
		static constexpr double ABSENT_VALUE = std::numeric_limits<double>::lowest();
	};

	struct ContinuousAxisParameters
	{
		double min = 0.0;
		double inc = 0.0;
		size_t nInc = 0;

		double firstOffset = 0.0;
		double finalOffset = 0.0;

		double calcMax() const { return min + inc * static_cast<double>(nInc) + finalOffset; }
		double calcRange() const { return calcMax() - min; }
	};

	struct ContinuousAxis
	{
		std::wstring label;
		ContinuousAxisParameters parameters;
	};

	struct ChartXY
	{
		ContinuousAxis xAxis;
		ContinuousAxis yAxis;
		std::vector<XYSeries> series;
	};

	ContinuousAxisParameters createAutoscaledAxis(const double min, const double max, size_t maxNInc, bool allowMinMaxOffsets = false);
	ContinuousAxisParameters createSimpleAxis(double min, double inc, size_t nInc);

	obelisk::Vector2<ContinuousAxisParameters> createAutoscaledAxes(const ChartXY &chart, size_t xMaxNInc, size_t yMaxNInc, bool allowMinMaxOffsets);
	obelisk::Vector2<ContinuousAxisParameters> createAutoscaledAxes(const std::vector<XYSeries> &series, size_t xMaxNInc, size_t yMaxNInc, bool allowMinMaxOffsets);
}
