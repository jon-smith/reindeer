#include "Stdafx.h"
#include "QtBaseOpenGLView.h"

#include <qtimer>
#include <Qmenu>
#include <QApplication>
#include <QClipboard>
#include <QScreen>
#include <QDesktopWidget>

QtBaseOpenGLView::QtBaseOpenGLView(QWidget *parent) :
	QOpenGLWidget(parent),
	updateTimer(std::make_unique<QTimer>(this))
{
	// Disconnect everything from update controls
	disconnect(this, &QtBaseOpenGLView::signalUpdateView, nullptr, nullptr);

	// Connect the timer to the update function
	connect(updateTimer.get(), &QTimer::timeout, this, &QtBaseOpenGLView::emitSignalUpdateView);

	// Connect as a QueuedConnection so that it is called in the UI thread (which will crash if signalUpdatView is called from another thread)
	connect(this, &QtBaseOpenGLView::signalUpdateView, this, &QtBaseOpenGLView::updateViewCaller, Qt::QueuedConnection);

	// Set up context menu
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &QtBaseOpenGLView::customContextMenuRequested, this, &QtBaseOpenGLView::showContextMenu);
}

QtBaseOpenGLView::~QtBaseOpenGLView()
{
}

void QtBaseOpenGLView::requestUpdate()
{
	// Only request an update if the timer isn't active
	if (!updateTimer->isActive())
		emitSignalUpdateView();
}

void QtBaseOpenGLView::startUpdateTimer(std::chrono::milliseconds time)
{
	// Stop the update timer if the time is different
	if (updateTimerTime != time)
		updateTimer->stop();

	if (!updateTimer->isActive())
		updateTimer->start(time.count());

	updateTimerTime = time;
}

void QtBaseOpenGLView::stopUpdateTimer()
{
	if (updateTimer->isActive())
		updateTimer->stop();

	reactivateUpdateTimerOnShow = false;
}

void QtBaseOpenGLView::showEvent(QShowEvent*e)
{
	QWidget::showEvent(e);
	signalUpdateView();

	if (reactivateUpdateTimerOnShow)
		startUpdateTimer(updateTimerTime);
}

void QtBaseOpenGLView::hideEvent(QHideEvent*e)
{
	if (updateTimer->isActive())
	{
		reactivateUpdateTimerOnShow = true;
		updateTimer->stop();
	}

	QWidget::hideEvent(e);
}

void QtBaseOpenGLView::showContextMenu(const QPoint &pos)
{
	QMenu contextMenu(tr("Context menu"), this);

	QAction copyToClipboard("Copy to clipboard", this);
	connect(&copyToClipboard, &QAction::triggered, this, &QtBaseOpenGLView::onCopyToClipboard);
	contextMenu.addAction(&copyToClipboard);

	contextMenu.exec(mapToGlobal(pos));
}

void QtBaseOpenGLView::onCopyToClipboard() const
{
	auto const desktop = QApplication::desktop();
	auto const wid = desktop->winId();
	auto const screenCount = QApplication::screens().size();

	// Hack fix for QPixmap::grabWindow(winId()) not working on some PCs
	// Loop through all screens and find which screen we're on
	for (auto i = 0; i<screenCount; ++i)
	{
		auto const screenGeometry = desktop->screenGeometry(i);
		auto const widgetGeometryGlobalCoords = QRect(mapToGlobal(QPoint(0, 0)), size());

		// Check if the widget is on this screen
		if (screenGeometry.intersects(widgetGeometryGlobalCoords))
		{
			auto const widgetGeometryScreenCoords = QRect(mapToGlobal(QPoint(0, 0)) - screenGeometry.topLeft(), size());
			auto const desktopPixmap = QPixmap::grabWindow(wid, screenGeometry.left(), screenGeometry.top(), screenGeometry.width(), screenGeometry.height());
			auto const widgetCrop = desktopPixmap.copy(widgetGeometryScreenCoords);
			QApplication::clipboard()->setPixmap(widgetCrop);
		}
	}
}

void QtBaseOpenGLView::paintGL()
{
	doOpenGLDrawing();
}

void QtBaseOpenGLView::paintEvent(QPaintEvent *event)
{
	QOpenGLWidget::paintEvent(event);
	doQPainterDrawing();
}

void QtBaseOpenGLView::emitSignalUpdateView()
{
	std::lock_guard<std::mutex> lk(emitUpdateMutex);
	// This stops multiple signalUpdateScene being emitted in succession
	if (updateStartedSinceLastEmit)
	{
		updateStartedSinceLastEmit = false;
		emit signalUpdateView();
	}
}

void QtBaseOpenGLView::updateViewCaller()
{
	{
		std::lock_guard<std::mutex> lk(emitUpdateMutex);
		updateStartedSinceLastEmit = true;
	}

	if (isVisible())
		update();
}