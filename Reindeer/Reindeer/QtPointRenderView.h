#pragma once

#include "QtBaseOpenGLView.h"

#include <array>
#include <chrono>

#include "ReindeerLib/XYZ.hpp"

namespace reindeer
{
	class DiffusionSimulator;
}

class QtPointRenderView : public QtBaseOpenGLView
{
public:
	QtPointRenderView(QWidget *parent = nullptr);
	~QtPointRenderView();

	// Resets all point positions
	void initPoints();
	void setDiffusePoints(bool diffuse);

private:

	// Mouse funcs
	void mousePressEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

	void onMouseDownTimer();

	// GL funcs
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void doOpenGLDrawing() override;
	void doQPainterDrawing() override;

	// Update timings
	std::chrono::steady_clock::time_point lastPaintClockTime;
	std::chrono::nanoseconds lastOpenGLDrawTimeTaken = {};
	std::chrono::nanoseconds lastUpdatePositionTimeTaken = {};
	std::chrono::nanoseconds lastUpdateColourTimeTaken = {};
	float smoothedDrawInterval_s = {};

	// View info
	std::array<float, 4 * 4> currentRotationMatrix = {
		1.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	float zoom = 1.f;
	XYZ<float> cameraPosition = { 0.f,0.f,0.f };

	// Mouse click + drag tracking variables
	bool lButtonDown = false;
	QPointF lastLButtonClickPoint;
	std::array<float, 4 * 4> lastLButtonClickRotationMatrix = {};
	void updateRotation(QPointF mousePos);

	// Timer to update on mouse position during click and drag
	const std::unique_ptr<QTimer> mouseDownTimer;

	// Options
	bool doPositionAndColourUpdates = false;

	const std::unique_ptr<reindeer::DiffusionSimulator> simulator;
};