#ifndef REINDEER_H
#define REINDEER_H

#include <memory>

#include <QtWidgets/QMainWindow>

namespace Ui
{
  class ReindeerClass;
}

class QtReindeerMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit QtReindeerMainWindow(QWidget *parent = nullptr);
  ~QtReindeerMainWindow();

private:
	void timerEvent(QTimerEvent *event) override;
	void doNextLoopView();
	void onResetButton();
	void onCheckDiffusePoints();
	void onLoopViewClick();

	enum class ViewType{SINGLE, LEFT_RIGHT, TOP_BOTTOM, QUAD};
	void setViewType(ViewType type);

	void updateAppPalette();

	const std::unique_ptr<Ui::ReindeerClass> ui;
	ViewType currentViewType = ViewType::SINGLE;
	int currentLoopViewIndex = 0;
	int loopViewTimerID = 0;
};

#endif // REINDEER_H
