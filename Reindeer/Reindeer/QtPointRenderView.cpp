#include "stdafx.h"
#include "QtPointRenderView.h"

#include <array>
#include <random>
#include <cassert>
#include <atomic>

#include "FormatString.hpp"
#include "GeneralHelpers.h"

#include "ReindeerLib/DiffusionSimulator.h"
#include "ReindeerLib/MatrixUtils.hpp"

namespace
{
	constexpr auto simulationWidth = 500.f;
	constexpr auto simulationHeight = 500.f;
	constexpr size_t nPoints = 1'000'000;
}

QtPointRenderView::QtPointRenderView(QWidget *parent) :
QtBaseOpenGLView(parent),
mouseDownTimer(std::make_unique<QTimer>(this)),
lastPaintClockTime(std::chrono::steady_clock::now()),
simulator(std::make_unique<reindeer::DiffusionSimulator>())
{
	startUpdateTimer(std::chrono::milliseconds(10));

	// When the mouse is down do updates on a timer so we follow it as quickly as possible
	connect(mouseDownTimer.get(), &QTimer::timeout, this, &QtPointRenderView::onMouseDownTimer);
}

QtPointRenderView::~QtPointRenderView() = default;

void QtPointRenderView::initPoints()
{
	simulator->initialise(nPoints, simulationWidth, simulationHeight);
}

void QtPointRenderView::setDiffusePoints(bool diffuse)
{
	doPositionAndColourUpdates = diffuse;
}

void QtPointRenderView::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		lButtonDown = true;
		lastLButtonClickPoint = event->localPos();
		lastLButtonClickRotationMatrix = currentRotationMatrix;
		// Start a timer to follow mouse movements (quicker than mouseMove event)
		mouseDownTimer->start(std::chrono::milliseconds(20).count());
	}
}

void QtPointRenderView::mouseReleaseEvent(QMouseEvent* event)
{
	if (lButtonDown && event->button() == Qt::LeftButton)
	{
		// Stop l button down functionality
		lButtonDown = false;
		mouseDownTimer->stop();
		updateRotation(QPointF(event->pos()));
	}
}

void QtPointRenderView::wheelEvent(QWheelEvent* event)
{
	auto const scaledDelta = event->delta() / 240.f;
	zoom *= powf(1.1f, scaledDelta * 2);
}

void QtPointRenderView::onMouseDownTimer()
{
	if (lButtonDown)
		updateRotation(QPointF(mapFromGlobal(QCursor::pos())));
}

void QtPointRenderView::initializeGL()
{
	glEnable(GL_POINT_SMOOTH);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glPointSize(1);
}

void QtPointRenderView::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, h, 0, -100.f, 100.f);
}

void QtPointRenderView::doOpenGLDrawing()
{
	if (doPositionAndColourUpdates)
	{
		auto const timings = simulator->update();
		lastUpdatePositionTimeTaken = timings.updatePositionTime;
		lastUpdateColourTimeTaken = timings.updateColourTime;
	}
	else
	{
		lastUpdatePositionTimeTaken = {};
		lastUpdateColourTimeTaken = {};
	}

	auto const preOpenGL = std::chrono::steady_clock::now();

	// Start drawing
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();

	// Small border
	glTranslatef(50.f, 130.f, 0);

	// Apply zoom
	glScaled(zoom, zoom, zoom);

	// Apply camera position
	glTranslatef(cameraPosition.x, cameraPosition.y, cameraPosition.z);
		
	// Apply rotation to mid point
	glTranslatef(simulationWidth*0.5, simulationHeight*0.5, 0.f);
	glMultMatrixf(currentRotationMatrix.data());
	glTranslatef(-simulationWidth*0.5, -simulationHeight*0.5, 0.f);

	// Draw points using basic VAs
	// Todo...improve with VBOs
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	simulator->data.lockedModify([](reindeer::DiffusionSimulator::DataT &data)
	{
		for (auto &d : data)
		{
			assert(d.colours.size() == d.positions.size() * 3);

			glVertexPointer(3, GL_FLOAT, 0, d.positions.data());
			glColorPointer(3, GL_UNSIGNED_BYTE, 0, d.colours.data());
			glDrawArrays(GL_POINTS, 0, d.positions.size());
		}
	});

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);

	glPopMatrix();

	glFlush();

	lastOpenGLDrawTimeTaken = std::chrono::steady_clock::now() - preOpenGL;
}

void QtPointRenderView::doQPainterDrawing()
{
	auto const now = std::chrono::steady_clock::now();
	auto const timeSinceLastPaint_s = std::chrono::duration<double>(now - lastPaintClockTime).count();

	// Calculate exponentially smoothed drawing interval
	constexpr auto smoothLevel = 0.80;
	smoothedDrawInterval_s = smoothedDrawInterval_s * smoothLevel + timeSinceLastPaint_s * (1.0 - smoothLevel);

	auto const fps = smoothedDrawInterval_s != 0.0 ? 1.0 / smoothedDrawInterval_s : 0.0;
	lastPaintClockTime = now;

	QPainter painter(this);

	auto const timeString = obelisk::formatString(
		L"Steady clock count: %lld\n"
		L"Position update time: %.3f ms\n"
		L"Color update time: %.3f ms\n"
		L"OpenGL draw time: %.3f ms\n"
		L"FPS: %.2f",
		std::chrono::steady_clock::now().time_since_epoch().count(),
		static_cast<float>(lastUpdatePositionTimeTaken.count()) / 1e6,
		static_cast<float>(lastUpdateColourTimeTaken.count()) / 1e6,
		static_cast<float>(lastOpenGLDrawTimeTaken.count()) / 1e6,
		fps);

	painter.setPen(Qt::green);
	painter.setFont(QFont("consolas"));
	painter.drawText(10, 10, 300, 70, Qt::TextWordWrap, QString::fromStdWString(timeString));

	painter.end();
}

void QtPointRenderView::updateRotation(QPointF mousePos)
{
	auto const currentMousePos = mousePos;
	auto const mouseMoveDelta = currentMousePos - lastLButtonClickPoint;

	// Get rotation in radians from mouse movement
	auto constexpr rotationScaleForMouse = 600.f; // Increase to make less sensitive to mouse
	auto const xRotationRad = static_cast<float>(mouseMoveDelta.y()) / rotationScaleForMouse;
	auto const yRotationRad = static_cast<float>(mouseMoveDelta.x()) / rotationScaleForMouse;

	std::array<float, 4 * 4> const xMatrix = {
		1.f, 0.f, 0.f, 0.f,
		0.f, cos(xRotationRad), -sin(xRotationRad), 0.f,
		0.f, sin(xRotationRad), cos(xRotationRad), 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	std::array<float, 4 * 4> const yMatrix = {
		cos(yRotationRad), 0.f, sin(yRotationRad), 0.f,
		0.f, 1.f, 0.f, 0.f,
		-sin(yRotationRad), 0.f, cos(yRotationRad), 0.f,
		0.f, 0.f, 0.f, 1.f
	};

	// Update rotation relative to what it was when we first clicked the mouse
	// First y and then x
	currentRotationMatrix = reindeer::squareMatrixMultiply<4>(reindeer::squareMatrixMultiply<4>(lastLButtonClickRotationMatrix, yMatrix), xMatrix);
}