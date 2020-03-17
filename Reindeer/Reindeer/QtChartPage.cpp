#include "stdafx.h"
#include "QtChartPage.h"

#include "ui_chartPage.h"

#include "ReindeerLib/ChartStructures.h"

using namespace reindeer;

class QtChartPage::ChartDataGenerator
{
public:

	ChartDataGenerator() = default;

	std::vector<XYSeries> generateRandom(unsigned nSeries, unsigned nPoints)
	{
		std::vector<XYSeries> seriesData;
		for (unsigned s = 0; s < nSeries; ++s)
		{
			XYSeries series;
			series.format.type = SeriesType::LINE;

			// Generate some different colours
			const auto colourFactor = 0.5f + static_cast<float>(s) / (2.f*static_cast<float>(nSeries));
			if (s % 2)
			{
				series.format.colour = obelisk::ColourRGBA(colourFactor, 0.1f, 0.1f);
			}
			else
			{
				series.format.colour = obelisk::ColourRGBA(0.1f, 0.1f, colourFactor);
			}

			auto lastPoint = 0.0;

			for (unsigned p = 0; p < nPoints; ++p)
			{
				const auto x = static_cast<double>(p);
				const auto y = lastPoint + randomValue(randomEng);
				lastPoint = y;
				series.data.push_back({ x, y });
			}
			seriesData.push_back(series);
		}

		return seriesData;
	}

	std::vector<XYSeries> currentSeries;

private:

	// Random number generators to reuse
	std::mt19937 randomEng = std::mt19937(std::random_device()());
	std::normal_distribution<double> randomValue = std::normal_distribution<double>(0.f, 1.f);
};


QtChartPage::QtChartPage(QWidget *parent)
	: QWidget(parent),
	ui(std::make_unique<Ui::ChartPage>()),
	chartGen(std::make_unique<ChartDataGenerator>())
{
	ui->setupUi(this);

	connect(ui->fixYAxisCheck, &QPushButton::clicked, [this] {updateAxisSettingsAndControls(); });
	connect(ui->yMaxSpin, &QtDoubleSpinBox2::signalOnChangeComplete, [this] {updateAxisSettingsAndControls(); });
	connect(ui->axisOffsetCheck, &QPushButton::clicked, [this] {updateAxisSettingsAndControls(); });
	connect(ui->drawLinesCheck, &QPushButton::clicked, [this] {updateChartWithSeries(); });
	connect(ui->nSeriesSpin, &QtSpinBox2::signalOnChangeComplete, [this] {generateRandomChartData(); });
	connect(ui->nPointsSpin, &QtSpinBox2::signalOnChangeComplete, [this] {generateRandomChartData(); });
	connect(ui->generateCheckBox, &QPushButton::clicked, [this] {onStartStopDataGeneration(); });

	ui->mainChart->setChartSeries({});

	updateAxisSettingsAndControls();
	generateRandomChartData();
}

QtChartPage::~QtChartPage()
{

}

void QtChartPage::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == chartDataGenerateTimerId)
	{
		generateRandomChartData();
	}
}

void QtChartPage::updateAxisSettingsAndControls()
{
	const auto fixedYAxis = ui->fixYAxisCheck->isChecked();
	const auto allowOffset = ui->axisOffsetCheck->isChecked();
	const auto yMax = ui->yMaxSpin->value();

	ui->yMaxSpin->setEnabled(fixedYAxis);

	const auto fixedAxis = reindeer::createAutoscaledAxis(0.0, yMax, 5, allowOffset);
	ui->mainChart->setFixedYAxis(fixedAxis);
	ui->mainChart->setAutoYAxis(!fixedYAxis);
	ui->mainChart->setAllowAutoAxisOffsets(allowOffset);
}

void QtChartPage::updateChartWithSeries()
{
	const auto type = ui->drawLinesCheck->isChecked() ?
		SeriesType::LINE : SeriesType::SCATTER;

	for (auto &s : chartGen->currentSeries)
		s.format.type = type;

	ui->mainChart->setChartSeries(chartGen->currentSeries);
}

void QtChartPage::onStartStopDataGeneration()
{
	// Kill any existing timer
	if (chartDataGenerateTimerId > 0)
	{
		killTimer(chartDataGenerateTimerId);
		chartDataGenerateTimerId = 0;
	}

	const auto doGenerate = ui->generateCheckBox->isChecked();
	if (doGenerate)
	{
		chartDataGenerateTimerId = startTimer(std::chrono::milliseconds(100));
		generateRandomChartData();
	}
}

void QtChartPage::generateRandomChartData()
{
	const unsigned nSeries = ui->nSeriesSpin->value();
	const unsigned nPoints = ui->nPointsSpin->value();

	chartGen->currentSeries = chartGen->generateRandom(nSeries, nPoints);

	updateChartWithSeries();
}