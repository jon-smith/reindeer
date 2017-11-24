#pragma once

#include <mutex>
#include <QWidget>

#include "EventObserver.h"

// Basic Qt view which can be updated on a timer or on calls to requestUpdate
// It will not block to Qt UI queue with update requests no matter how many times requestUpdate is called
// Derived classes should override doUpdateView for to do drawing
class QtBaseView : public QWidget, protected obelisk::EventObserver
{
	Q_OBJECT

public:
	QtBaseView(QWidget *parent = nullptr);
	~QtBaseView();

	// request a one-off update (calls update() asychronously, which in turn calls paintGL)
	// can be called from any thread
	void requestUpdate();

protected:

	// Start a timer which calls requests an update after each interval
	// Must be called in the Qt UI thread
	void startUpdateTimer(std::chrono::milliseconds time);
	void stopUpdateTimer();

	void showEvent(QShowEvent*) override;
	void hideEvent(QHideEvent*) override;

signals:
	void signalUpdateView();

private:
	virtual void doUpdateView() = 0;

	void emitSignalUpdateView();
	void updateViewCaller();
	bool updateStartedSinceLastEmit = true;
	mutable std::mutex emitUpdateMutex;
	const std::unique_ptr<QTimer> updateTimer;
	std::chrono::milliseconds updateTimerTime;
	bool reactivateUpdateTimerOnShow = false;
};