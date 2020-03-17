#pragma once

#include <memory>

#include <QtWidgets/QMainWindow>

namespace Ui
{
	class ChartPage;
}

class QtChartPage : public QWidget
{
	Q_OBJECT

public:
	explicit QtChartPage(QWidget *parent = nullptr);
	~QtChartPage();

private:

	void timerEvent(QTimerEvent *event) override;

	void updateAxisSettingsAndControls();
	void updateChartWithSeries();
	void onStartStopDataGeneration();

	void generateRandomChartData();

	int chartDataGenerateTimerId = 0;

	const std::unique_ptr<Ui::ChartPage> ui;

	class ChartDataGenerator;
	const std::unique_ptr<ChartDataGenerator> chartGen;
};