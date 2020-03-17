#include "QtChartHelpers.h"

#include <QtGui/qopengl.h>
#include <QPainter>

#include "QtDrawingHelpers.h"

#include "ReindeerLib/ChartStructures.h"
#include "ReindeerLib/SeriesHelpers.h"
#include "ReindeerLib/TickHelpers.h"

using namespace obelisk;
using namespace reindeer;
using namespace qt_chart;

namespace
{
	int calcPlotRegionWidth(const DrawingParameters &params)
	{
		return params.width - params.leftBorder - params.rightBorder - params.yAxisWidth;
	}

	int calcPlotRegionHeight(const DrawingParameters &params)
	{
		return params.height - params.horizontalBorder * 2 - params.xAxisHeight;
	}

	QRect calcPlotRegion(const DrawingParameters &params)
	{
		return QRect(params.leftBorder + params.yAxisWidth, params.horizontalBorder + params.xAxisHeight, calcPlotRegionWidth(params), calcPlotRegionHeight(params));
	}

	double calcXScaleFactor(const DrawingParameters &params, const reindeer::ContinuousAxisParameters &xAxis)
	{
		const auto plotWidth = calcPlotRegionWidth(params);
		const auto xRange = xAxis.calcRange();
		return static_cast<double>(plotWidth) / xRange;
	}

	double calcYScaleFactor(const DrawingParameters &params, const reindeer::ContinuousAxisParameters &yAxis)
	{
		const auto plotHeight = calcPlotRegionHeight(params);
		const auto yRange = yAxis.calcRange();
		return static_cast<double>(plotHeight) / yRange;
	}

	void drawXAxisLabel(QPainter &painter, const ChartXY &chart, const DrawingParameters &params)
	{
		const auto xAxisLabelRegion = QRect(params.leftBorder + params.yAxisWidth,
			params.height - params.horizontalBorder - params.xAxisHeight,
			calcPlotRegionWidth(params),
			params.xAxisHeight);

		painter.drawText(xAxisLabelRegion, Qt::AlignBottom | Qt::AlignHCenter, QString::fromStdWString(chart.xAxis.label));
	}

	void drawYAxisLabel(QPainter &painter, const ChartXY &chart, const DrawingParameters &params)
	{
		// We need to do transformations at the y axis is rotated
		qt_drawing_helpers::ScopedQPainterState ts(painter);

		// Find centre of y-axis label
		const auto height = static_cast<double>(calcPlotRegionHeight(params));
		const auto centreX = static_cast<double>(params.leftBorder) + static_cast<double>(params.yAxisWidth) * 0.5;
		const auto centreY = static_cast<double>(params.horizontalBorder) + height * 0.5;

		// Rotate around centre of axis
		// Round translation to improve antialiasing
		painter.translate(QPointF(static_cast<float>(round(centreX)), static_cast<float>(round(centreY))));
		painter.rotate(-90);

		const auto drawRegion = QRectF(-height * 0.5, -static_cast<double>(params.yAxisWidth) / 2.0, height, params.yAxisWidth);
		painter.drawText(drawRegion, Qt::AlignTop | Qt::AlignHCenter, QString::fromStdWString(chart.yAxis.label));
	}

	void drawAxisLabels(QPainter &painter, const ChartXY &chart, const DrawingParameters &params)
	{
		drawXAxisLabel(painter, chart, params);
		drawYAxisLabel(painter, chart, params);
	}

	void drawXTickLabels(QPainter &painter, const ChartXY &chart, const DrawingParameters &params)
	{
		const auto offsetX = chart.xAxis.parameters.min;
		const auto scalingX = calcXScaleFactor(params, chart.xAxis.parameters);
		const auto tickValues = getTickPositions(chart.xAxis.parameters);
		const auto tickLabels = getTickLabels(chart.xAxis.parameters);

		assert(tickValues.size() == tickLabels.labels.size());

		for (size_t i = 0; i < std::min(tickValues.size(), tickLabels.labels.size()); ++i)
		{
			const auto xPos = (tickValues[i] - offsetX) * scalingX;
			painter.drawText(xPos, params.majorTickLength, 50, params.xAxisHeight, Qt::AlignLeft | Qt::AlignTop, QString::fromStdWString(tickLabels.labels[i]));
		}

		if (!tickLabels.labelAnnotation.empty())
		{
			const auto xPos = calcPlotRegionWidth(params);
			const auto yPos = params.majorTickLength + static_cast<double>(params.xAxisHeight) / 2.0;
			painter.drawText(xPos, yPos, 50, params.xAxisHeight, Qt::AlignLeft | Qt::AlignTop, QString::fromStdWString(tickLabels.labelAnnotation));
		}
	}

	void drawYTickLabels(QPainter &painter, const ChartXY &chart, const DrawingParameters &params)
	{
		const auto offsetY = chart.yAxis.parameters.min;
		const auto scalingY = calcYScaleFactor(params, chart.yAxis.parameters);
		const auto tickValues = getTickPositions(chart.yAxis.parameters);
		const auto tickLabels = getTickLabels(chart.yAxis.parameters);

		assert(tickValues.size() == tickLabels.labels.size());

		const auto xPos = -params.yAxisWidth - params.majorTickLength;
		const auto labelHeight = params.xAxisHeight;
		for (size_t i = 0; i < std::min(tickValues.size(), tickLabels.labels.size()); ++i)
		{
			const auto yPos = (tickValues[i] - offsetY) * scalingY;
			painter.drawText(xPos, -yPos - labelHeight / 2.0, 50, labelHeight, Qt::AlignRight | Qt::AlignVCenter, QString::fromStdWString(tickLabels.labels[i]));
		}

		if (!tickLabels.labelAnnotation.empty())
		{
			const auto yPos = calcPlotRegionHeight(params);
			painter.drawText(xPos, -yPos - labelHeight / 2.0, 50, labelHeight, Qt::AlignLeft | Qt::AlignVCenter, QString::fromStdWString(tickLabels.labelAnnotation));
		}
	}

	void drawTickLabels(QPainter &painter, const ChartXY &chart, const DrawingParameters &params)
	{
		qt_drawing_helpers::ScopedQPainterState ts(painter);

		// Start at the bottom left of chart
		painter.translate(params.leftBorder + params.yAxisWidth, params.horizontalBorder + calcPlotRegionHeight(params));

		drawXTickLabels(painter, chart, params);
		drawYTickLabels(painter, chart, params);
	}	
}

bool qt_chart::isMouseInPlotRegion(const DrawingParameters &params, const obelisk::Vector2i &localMousePos)
{
	const auto plotRegion = calcPlotRegion(params);
	return plotRegion.contains(localMousePos.x, localMousePos.y);
}

obelisk::Vector2d qt_chart::calculateChartCoordinatesFromMousePosition(const DrawingParameters &params,
	const ContinuousAxisParameters &xAxis, const ContinuousAxisParameters &yAxis,
	const obelisk::Vector2i &localMousePos)
{
	const auto plotRegion = calcPlotRegion(params);

	const auto xScaleFactor = calcXScaleFactor(params, xAxis);
	const auto yScaleFactor = calcYScaleFactor(params, yAxis);

	return {
		xScaleFactor != 0.0 ? static_cast<double>(localMousePos.x - plotRegion.left()) / xScaleFactor + xAxis.min : 0.0,
		yScaleFactor != 0.0 ? static_cast<double>(localMousePos.y - plotRegion.top()) / yScaleFactor + yAxis.min : 0.0
	};
}

void qt_chart::drawAxesGL(const ChartXY &chart, const DrawingParameters &params)
{
	glPushMatrix();

	// Set line format
	glColor3f(params.axisColour.red, params.axisColour.green, params.axisColour.blue);
	glLineWidth(1.0);
	glPointSize(1.0);

	// Move to bottom left of plot
	glTranslated(static_cast<GLdouble>(params.leftBorder + params.yAxisWidth),
		static_cast<GLdouble>(params.horizontalBorder + params.xAxisHeight),
		0.0);

	// Scale so we can use raw x/y values
	const auto scalingX = calcXScaleFactor(params, chart.xAxis.parameters);
	const auto scalingY = calcYScaleFactor(params, chart.yAxis.parameters);
	const auto xMin = chart.xAxis.parameters.min;
	const auto yMin = chart.yAxis.parameters.min;

	// Offsets to make the axes and ticks line up at edges rather than centre
	const auto yAxisLineWidthOffset = 0.5 / scalingY;
	const auto xAxisLineWidthOffset = 0.5 / scalingX;

	// Scale and translate so we can just use raw chart points
	glScaled(scalingX, scalingY, 1.0);
	glTranslated(-xMin, -yMin, 0.0);

	// Factor to darken gridlines compared to axes
	constexpr auto gridlineColourFactor = 0.5f;

	glColor3f(params.axisColour.red * gridlineColourFactor,
		params.axisColour.green * gridlineColourFactor,
		params.axisColour.blue * gridlineColourFactor);

	const auto xTickStart = chart.xAxis.parameters.min;
	const auto xTickEnd = chart.xAxis.parameters.min - params.majorTickLength / scalingX;
	const auto yTickPositions = getTickPositions(chart.yAxis.parameters);

	// Grid lines
	glBegin(GL_LINES);
	for (const auto &y : yTickPositions)
	{
		glVertex2d(chart.xAxis.parameters.calcMax(), y);
		glVertex2d(chart.xAxis.parameters.min, y);
	}
	glEnd();

	glColor3f(params.axisColour.red, params.axisColour.green, params.axisColour.blue);
	glBegin(GL_LINES);

	// Axis ticks
	for (const auto &y : yTickPositions)
	{
		glVertex2d(xTickStart, y);
		glVertex2d(xTickEnd, y);
	}
	glEnd();

	// Y-axis line
	glBegin(GL_LINES);
	glVertex2d(chart.xAxis.parameters.min - xAxisLineWidthOffset, chart.yAxis.parameters.min);
	glVertex2d(chart.xAxis.parameters.min - xAxisLineWidthOffset, chart.yAxis.parameters.calcMax());
	glEnd();

	glColor3f(params.axisColour.red * gridlineColourFactor,
		params.axisColour.green * gridlineColourFactor,
		params.axisColour.blue * gridlineColourFactor);

	const auto yTickStart = chart.yAxis.parameters.min;
	const auto yTickEnd = chart.yAxis.parameters.min - params.majorTickLength / scalingY;
	const auto xTickPositions = getTickPositions(chart.xAxis.parameters);

	// X grid lines
	glBegin(GL_LINES);
	for (const auto &x : xTickPositions)
	{
		glVertex2d(x, chart.yAxis.parameters.calcMax());
		glVertex2d(x, chart.yAxis.parameters.min);
	}
	glEnd();

	// X ticks
	glColor3f(params.axisColour.red, params.axisColour.green, params.axisColour.blue);
	glBegin(GL_LINES);
	for (const auto &x : xTickPositions)
	{
		glVertex2d(x, yTickStart);
		glVertex2d(x, yTickEnd);
	}
	glEnd();

	// X-axis line
	glBegin(GL_LINES);
	glVertex2d(chart.xAxis.parameters.min, chart.yAxis.parameters.min - yAxisLineWidthOffset);
	glVertex2d(chart.xAxis.parameters.calcMax(), chart.yAxis.parameters.min - yAxisLineWidthOffset);
	glEnd();

	glPopMatrix();
}

void qt_chart::drawChartGL(const ChartXY &chart, const DrawingParameters &params, const std::vector<Marker> &verticalMarkers)
{
	drawAxesGL(chart, params);

	glPushMatrix();
	glTranslated(static_cast<GLdouble>(params.leftBorder + params.yAxisWidth),
		static_cast<GLdouble>(params.horizontalBorder + params.xAxisHeight),
		0.0);

	// Scale and translate so we can just use raw chart points
	const auto scalingX = calcXScaleFactor(params, chart.xAxis.parameters);
	const auto scalingY = calcYScaleFactor(params, chart.yAxis.parameters);
	const auto xMin = chart.xAxis.parameters.min;
	const auto yMin = chart.yAxis.parameters.min;

	// Offsets to make the axes and ticks line up at edges rather than centre
	const auto yAxisLineWidthOffset = 0.5 / scalingY;
	const auto xAxisLineWidthOffset = 0.5 / scalingX;

	// Scale and translate so we can just use raw chart points
	glScaled(scalingX, scalingY, 1.0);
	glTranslated(-xMin, -yMin, 0.0);

	// Scissor region for the actual plot region to avoid displaying points
	// This means we don't have to cull ourselves
	glScissor(params.leftBorder + params.yAxisWidth,
		params.horizontalBorder + params.xAxisHeight,
		calcPlotRegionWidth(params),
		calcPlotRegionHeight(params));

	glEnable(GL_SCISSOR_TEST);

	for (auto &series : chart.series)
	{
		// Set series format
		glColor4f(series.format.colour.red, series.format.colour.green, series.format.colour.blue, series.format.colour.alpha);
		glLineWidth(series.format.size);
		glPointSize(series.format.size);

		const auto drawSizeScaledX = series.format.size / scalingX;
		const auto drawSizeScaledY = series.format.size / scalingY;

		// Line plots
		switch (series.format.type)
		{
			case SeriesType::SCATTER:

				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(2, GL_DOUBLE, 0, getGlVertexPointer2D(series));
				glDrawArrays(GL_POINTS, 0, series.data.size());
				glDisableClientState(GL_VERTEX_ARRAY);

				break;
			
			case SeriesType::LINE:

				glBegin(GL_LINE_STRIP);
				for (size_t i = 0; i < series.data.size(); ++i)
				{
					// Normal data, add to line strip
					if (series.data[i].y != XYSeries::ABSENT_VALUE)
					{
						glVertex2d(series.data[i].x, series.data[i].y);

						// Early exit if this point out past the max
						if (series.data[i].x > chart.xAxis.parameters.calcMax())
							break;
					}
					// If we have empty data, end this line strip and begin a new one
					else // (series.data[i].y == XYSeries::ABSENT_VALUE)
					{
						glEnd();

						// If the current line strip only had a single point
						// It won't display, so display as a dot
						if (i > 1 &&
							series.data[i - 1].y != XYSeries::ABSENT_VALUE &&
							series.data[i - 2].y == XYSeries::ABSENT_VALUE)
						{
							glBegin(GL_POINTS);
							glVertex2d(series.data[i - 1].x, series.data[i - 1].y);
							glEnd();
						}

						glBegin(GL_LINE_STRIP);
					}
				}
				glEnd();
				break;

			default:
				assert(false);
				break;
		}
	}

	// Draw vertical markers
	for (auto const &m : verticalMarkers)
	{
		// Set format
		glColor4f(m.colour.red, m.colour.green, m.colour.blue, m.colour.alpha);
		glLineWidth(m.drawSize);

		glBegin(GL_LINE_STRIP);
		glVertex2d(m.position, chart.yAxis.parameters.min - yAxisLineWidthOffset);
		glVertex2d(m.position, chart.yAxis.parameters.calcMax() - yAxisLineWidthOffset);
		glEnd();
	}

	glDisable(GL_SCISSOR_TEST);

	glPopMatrix();
}

void qt_chart::drawChartLabels(QPainter &painter, const ChartXY &chart, const DrawingParameters &params)
{
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setRenderHint(QPainter::TextAntialiasing);
	painter.setPen(params.labelColor);

	drawAxisLabels(painter, chart, params);
	drawTickLabels(painter, chart, params);
}