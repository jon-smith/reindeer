#include "QtChartView.h"

#include <QTimer>
#include <QPainter>
#include <QLabel>
#include <QMovie>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>

#include "QtChartHelpers.h"
#include "QtDrawingHelpers.h"

#include "ReindeerLib/SeriesHelpers.h"
#include "ReindeerLib/ChartStructures.h"

#include "Optional.h"
#include "ColourStructs.h"
#include "StringFuncs.h"
#include "FormatString.hpp"

using namespace reindeer;
using namespace obelisk;
using namespace qt_chart;

struct QtChartView::Impl
{
	std::vector<XYSeries> series;
	DrawingParameters chartParams;

	Vector2<ContinuousAxisParameters> fixedAxes;

	Vector2<ContinuousAxisParameters> autoAxesWithOffset;
	Vector2<ContinuousAxisParameters> autoAxesNoOffset;

	Vector2<bool> useAutoAxis;

	bool allowAxisOffset = false;

	void recalculateAutoAxes()
	{
		autoAxesWithOffset = createAutoscaledAxes(series, 5, 5, true);
		autoAxesNoOffset = createAutoscaledAxes(series, 5, 5, false);
	}

	ContinuousAxisParameters getXAxisToUse() const
	{
		return !useAutoAxis.x && fixedAxes.x.nInc > 0 ? fixedAxes.x : 
			(allowAxisOffset ? autoAxesWithOffset.x : autoAxesNoOffset.x);
	}

	ContinuousAxisParameters getYAxisToUse() const
	{
		return !useAutoAxis.y && fixedAxes.y.nInc > 0 ? fixedAxes.y :
			(allowAxisOffset ? autoAxesWithOffset.y : autoAxesNoOffset.y);
	}

	ChartXY getChartDataToUse() const
	{
		ChartXY chart;
		chart.series = series;
		chart.xAxis.parameters = getXAxisToUse();
		chart.yAxis.parameters = getYAxisToUse();
		return chart;
	}
};

QtChartView::QtChartView(QWidget *parent)
  : QtBaseOpenGLView(parent),
	impl(std::make_unique<Impl>())
{
	// antialiasing
	QSurfaceFormat format;
	format.setSamples(4);
	setFormat(format);

	// Call onMouseMove even if button isn't down
	setMouseTracking(true);
}

QtChartView::~QtChartView() = default;

void QtChartView::setChartSeries(const std::vector<XYSeries> &series)
{
	impl->series = series;

	impl->recalculateAutoAxes();

	requestUpdate();
}

ContinuousAxisParameters QtChartView::getCurrentXAxis() const
{
	return impl->getXAxisToUse();
}

ContinuousAxisParameters QtChartView::getCurrentYAxis() const
{
	return impl->getYAxisToUse();
}

void QtChartView::setFixedXAxis(const reindeer::ContinuousAxisParameters &axis)
{
	impl->fixedAxes.x = axis;
	requestUpdate();
}

void QtChartView::setAutoXAxis(bool useAuto)
{
	impl->useAutoAxis.x = useAuto;
	requestUpdate();
}

void QtChartView::setFixedYAxis(const reindeer::ContinuousAxisParameters &axis)
{
	impl->fixedAxes.y = axis;
	requestUpdate();
}

void QtChartView::setAutoYAxis(bool useAuto)
{
	impl->useAutoAxis.y = useAuto;
	requestUpdate();
}

void QtChartView::setAllowAutoAxisOffsets(bool allowOffsets)
{
	impl->allowAxisOffset = allowOffsets;
	requestUpdate();
}

void QtChartView::initializeGL()
{
	glClearColor(0.3f, 0.3f, 0.3f, 1.f);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
}

void QtChartView::resizeGL(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, static_cast<double>(w), 0.0, static_cast<double>(h), 1.0, -1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void QtChartView::mouseMoveEvent(QMouseEvent *e)
{
	requestUpdate();
	QtBaseOpenGLView::mouseMoveEvent(e);
}

void QtChartView::leaveEvent(QEvent * e)
{
	requestUpdate();
	QtBaseOpenGLView::leaveEvent(e);
}

void QtChartView::drawChartArea(const ChartXY &chartData)
{
	drawChartGL(chartData, impl->chartParams, {});
}

void QtChartView::doOpenGLDrawing()
{
	impl->chartParams.height = height();
	impl->chartParams.width = width();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (impl->chartParams.height <= 0.0 || impl->chartParams.width <= 0.0)
		return;

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	drawChartArea(impl->getChartDataToUse());

	glDisable(GL_BLEND);
}

void QtChartView::doQPainterDrawing()
{
	const auto chartData = impl->getChartDataToUse();

	QPainter painter(this);
	drawChartLabels(painter, chartData, impl->chartParams);

	const auto localMousePos = mapFromGlobal(QCursor::pos());
	const auto localMousePosV = Vector2i(localMousePos.x(), localMousePos.y());

	if (isMouseInPlotRegion(impl->chartParams, localMousePosV))
	{
		const auto chartCoords = calculateChartCoordinatesFromMousePosition(impl->chartParams, chartData.xAxis.parameters, chartData.yAxis.parameters, localMousePosV);
		const auto text = obelisk::formatString("(%.2f, %.2f)", chartCoords.x, chartCoords.y);

		qt_drawing_helpers::drawTextBox(painter, QString::fromStdString(text), font(), QPoint(width() - 100, 10), painter.brush(), QPen(Qt::white));
	}

	painter.end();
}
