#include "stdafx.h"
#include "QtReindeerMainWindow.h"

#include "ui_reindeer.h"

QtReindeerMainWindow::QtReindeerMainWindow(QWidget *parent)
  : QMainWindow(parent),
  ui(std::make_unique<Ui::ReindeerClass>())
{
  ui->setupUi(this);

  connect(ui->resetButton, &QPushButton::clicked, [this] {onResetButton(); });
	connect(ui->diffuseCheck, &QCheckBox::clicked, [this] {onCheckDiffusePoints(); });
	connect(ui->singleViewButton, &QPushButton::clicked, [this] {setViewType(ViewType::SINGLE); });
	connect(ui->leftRightViewButton, &QPushButton::clicked, [this] {setViewType(ViewType::LEFT_RIGHT); });
	connect(ui->topBottomViewButton, &QPushButton::clicked, [this] {setViewType(ViewType::TOP_BOTTOM); });
	connect(ui->quadViewButton, &QPushButton::clicked, [this] {setViewType(ViewType::QUAD); });
	connect(ui->loopViewButton, &QPushButton::clicked, [this] {onLoopViewClick(); });

	onResetButton();
}

QtReindeerMainWindow::~QtReindeerMainWindow()
{

}

void QtReindeerMainWindow::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == loopViewTimerID)
	{
		doNextLoopView();
	}
}

void QtReindeerMainWindow::doNextLoopView()
{
	switch (currentViewType)
	{
		// Single does nothing
	case ViewType::SINGLE:
		currentLoopViewIndex = 2;
		break;
		// 0-2-0-2
	case ViewType::TOP_BOTTOM:
		currentLoopViewIndex = currentLoopViewIndex == 2 ? 0 : 2;
		break;
		// 2-3-2-3
	case ViewType::LEFT_RIGHT:
		currentLoopViewIndex = currentLoopViewIndex == 3 ? 2 : 3;
		break;
		// 0-1-2-3-0-1-2-3
	case ViewType::QUAD:
		++currentLoopViewIndex;
		if (currentLoopViewIndex > 3)
			currentLoopViewIndex = 0;
		break;
	}

	ui->mainView0->setVisible(currentLoopViewIndex == 0);
	ui->mainView1->setVisible(currentLoopViewIndex == 1);
	ui->mainView2->setVisible(currentLoopViewIndex == 2);
	ui->mainView3->setVisible(currentLoopViewIndex == 3);
}

void QtReindeerMainWindow::onResetButton()
{
  ui->mainView0->initPoints();
	ui->mainView1->initPoints();
	ui->mainView2->initPoints();
	ui->mainView3->initPoints();
}

void QtReindeerMainWindow::onCheckDiffusePoints()
{
	ui->mainView0->setDiffusePoints(ui->diffuseCheck->isChecked());
	ui->mainView1->setDiffusePoints(ui->diffuseCheck->isChecked());
	ui->mainView2->setDiffusePoints(ui->diffuseCheck->isChecked());
	ui->mainView3->setDiffusePoints(ui->diffuseCheck->isChecked());
}

void QtReindeerMainWindow::onLoopViewClick()
{
	const auto toLoop = ui->loopViewButton->isChecked();

	ui->singleViewButton->setEnabled(!toLoop);
	ui->leftRightViewButton->setEnabled(!toLoop);
	ui->topBottomViewButton->setEnabled(!toLoop);
	ui->quadViewButton->setEnabled(!toLoop);

	if (toLoop)
	{
		currentLoopViewIndex = -1;
		loopViewTimerID = startTimer(std::chrono::milliseconds(1000));
		doNextLoopView();
	}
	else
	{
		killTimer(loopViewTimerID);
		loopViewTimerID = 0;
		setViewType(currentViewType);
	}
}

void QtReindeerMainWindow::setViewType(ViewType type)
{
	currentViewType = type;

	ui->mainView0->setVisible(currentViewType == ViewType::TOP_BOTTOM || currentViewType == ViewType::QUAD);
	ui->mainView1->setVisible(currentViewType == ViewType::QUAD);
	ui->mainView2->show();
	ui->mainView3->setVisible(currentViewType == ViewType::LEFT_RIGHT || currentViewType == ViewType::QUAD);
}