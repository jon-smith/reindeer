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
  void onResetButton();
	void onCheckDiffusePoints();

  const std::unique_ptr<Ui::ReindeerClass> ui;
};

#endif // REINDEER_H
