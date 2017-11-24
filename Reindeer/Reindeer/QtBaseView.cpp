#include "Stdafx.h"
#include "QtBaseView.h"

#include <qtimer>
#include <QApplication>

QtBaseView::QtBaseView(QWidget *parent) :
	QWidget(parent),
	updateTimer(std::make_unique<QTimer>(this))
{
	// Disconnect everything from update controls
	disconnect(this, &QtBaseView::signalUpdateView, nullptr, nullptr);

	// Connect the timer to the update function
	connect(updateTimer.get(), &QTimer::timeout, this, &QtBaseView::emitSignalUpdateView);

	// Connect as a QueuedConnection so that it is called in the UI thread (which will crash if signalUpdatView is called from another thread)
	connect(this, &QtBaseView::signalUpdateView, this, &QtBaseView::updateViewCaller, Qt::QueuedConnection);
}

QtBaseView::~QtBaseView()
{
}

void QtBaseView::requestUpdate()
{
	// Only request an update if the timer isn't active
	if (!updateTimer->isActive())
		emitSignalUpdateView();
}

void QtBaseView::startUpdateTimer(std::chrono::milliseconds time)
{
	// Stop the update timer if the time is different
	if (updateTimerTime != time)
		updateTimer->stop();

	if (!updateTimer->isActive())
		updateTimer->start(time.count());

	updateTimerTime = time;
}

void QtBaseView::stopUpdateTimer()
{
	if (updateTimer->isActive())
		updateTimer->stop();

	reactivateUpdateTimerOnShow = false;
}

void QtBaseView::showEvent(QShowEvent*e)
{
	QWidget::showEvent(e);
	signalUpdateView();

	if (reactivateUpdateTimerOnShow)
		startUpdateTimer(updateTimerTime);
}

void QtBaseView::hideEvent(QHideEvent*e)
{
	if (updateTimer->isActive())
	{
		reactivateUpdateTimerOnShow = true;
		updateTimer->stop();
	}

	QWidget::hideEvent(e);
}

void QtBaseView::emitSignalUpdateView()
{
	std::lock_guard<std::mutex> lk(emitUpdateMutex);
	// This stops multiple signalUpdateScene being emitted in succession
	if (updateStartedSinceLastEmit)
	{
		updateStartedSinceLastEmit = false;
		emit signalUpdateView();
	}
}

void QtBaseView::updateViewCaller()
{
	{
		std::lock_guard<std::mutex> lk(emitUpdateMutex);
		updateStartedSinceLastEmit = true;
	}

	if (isVisible())
		doUpdateView();
}