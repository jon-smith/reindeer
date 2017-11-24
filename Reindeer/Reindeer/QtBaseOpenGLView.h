#pragma once

#include <mutex>
#include <QOpenGLWidget>

// Basic open GL view which can be updated on a timer or on calls to requestUpdate
// It will not block to Qt UI queue with update requests no matter how many times requestUpdate is called
// Derived classes should override the following functions to do drawing:
// void initializeGL();
// void resizeGL(int w, int h);
// void doQPainterDrawing();
// void doOpenGLDrawing();
class QtBaseOpenGLView : public QOpenGLWidget
{
	Q_OBJECT

public:
	QtBaseOpenGLView(QWidget *parent = nullptr);
	~QtBaseOpenGLView();

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

protected:
	virtual void showContextMenu(const QPoint &pos);
	void onCopyToClipboard() const;

private:
	virtual void doQPainterDrawing() = 0;
	virtual void doOpenGLDrawing() = 0;

	void paintGL() override;
	void paintEvent(QPaintEvent *event) override;

	void emitSignalUpdateView();
	void updateViewCaller();
	bool updateStartedSinceLastEmit = true;
	mutable std::mutex emitUpdateMutex;
	const std::unique_ptr<QTimer> updateTimer;
	std::chrono::milliseconds updateTimerTime;
	bool reactivateUpdateTimerOnShow = false;
};