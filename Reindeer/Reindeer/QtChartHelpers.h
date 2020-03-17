#pragma once

#include <qcolor.h>

#include "ColourStructs.h"
#include "VectorT.h"

namespace reindeer
{
	struct ChartXY;
	struct ContinuousAxisParameters;
}

class QPainter;

namespace qt_chart
{
	struct DrawingParameters
	{
		int width = 0;
		int height = 0;

		int leftBorder = 10;
		int rightBorder = 50;

		int horizontalBorder = 10;

		int xAxisHeight = 30;
		int yAxisWidth = 55;

		double majorTickLength = 5.0;
		double minTickLength = 2.5;

		obelisk::ColourRGBA axisColour = obelisk::ColourRGBA(0.5, 0.5, 0.5);
		QColor labelColor = QColor(255, 255, 255);
	};

	// Is the mouse point within the plot region (excluding borders and axes)
	bool isMouseInPlotRegion(const DrawingParameters &params, const obelisk::Vector2i &localMousePos);

	obelisk::Vector2d calculateChartCoordinatesFromMousePosition(const DrawingParameters &params,
		const reindeer::ContinuousAxisParameters &xAxis,
		const reindeer::ContinuousAxisParameters &yAxis,
		const obelisk::Vector2i &localMousePos);

	struct Marker
	{
		double position = 0.0;
		obelisk::ColourRGBA colour;
		double drawSize = 1.0;
	};

	void drawAxesGL(const reindeer::ChartXY &chart, const DrawingParameters &params);
	void drawChartGL(const reindeer::ChartXY &chart, const DrawingParameters &params, const std::vector<Marker> &verticalMarkers);
	void drawChartLabels(QPainter &painter, const reindeer::ChartXY &chart, const DrawingParameters &params);
}