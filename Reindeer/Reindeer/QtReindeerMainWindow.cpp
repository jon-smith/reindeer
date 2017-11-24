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

	ui->mainView->initPoints();
}

QtReindeerMainWindow::~QtReindeerMainWindow()
{

}

void QtReindeerMainWindow::onResetButton()
{
  ui->mainView->initPoints();
}

void QtReindeerMainWindow::onCheckDiffusePoints()
{
	ui->mainView->setDiffusePoints(ui->diffuseCheck->isChecked());
}