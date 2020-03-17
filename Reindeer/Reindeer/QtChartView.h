#pragma once

#include "QtBaseOpenGLView.h"

#include "VectorT.h"

namespace reindeer
{
	struct ContinuousAxisParameters;
	struct ChartXY;
	struct XYSeries;
}

class QTimer;

class QtChartView : public QtBaseOpenGLView
{
	Q_OBJECT

public:
	QtChartView(QWidget *parent);
	~QtChartView();

	void setChartSeries(const std::vector<reindeer::XYSeries> &series);

	reindeer::ContinuousAxisParameters getCurrentXAxis() const;
	reindeer::ContinuousAxisParameters getCurrentYAxis() const;

	void setFixedXAxis(const reindeer::ContinuousAxisParameters &axis);
	void setAutoXAxis(bool useAuto);

	void setFixedYAxis(const reindeer::ContinuousAxisParameters &axis);
	void setAutoYAxis(bool useAuto);

	void setAllowAutoAxisOffsets(bool allowOffsets);

private:

	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void doOpenGLDrawing() override;
	void doQPainterDrawing() override;

	void mouseMoveEvent(QMouseEvent *e) override;
	void leaveEvent(QEvent * e) override;

	void drawChartArea(const reindeer::ChartXY &chartData);

private:

	struct Impl;
	const std::unique_ptr<Impl> impl;
};
