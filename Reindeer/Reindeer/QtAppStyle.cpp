#include "QtAppStyle.h"

#include "FormatString.hpp"

namespace
{
	// Based on https://gist.github.com/QuantumCD/6245215
	// Colours similar to dark VS theme
	const auto darkBackground = QColor(45, 45, 48);
	const auto darkerBackground = QColor(37, 37, 39);
	const auto darkestBackground = QColor(30, 30, 31);

	const auto lightBackground = QColor(240, 240, 240);

	const auto darkButton = QColor(63, 63, 70);
	const auto lightButton = QColor(199, 199, 199);

	const auto tooltipBorder = QColor(63, 63, 70);

	const auto brightBlue = QColor(51, 153, 255);
	const auto purple = QColor(185, 117, 182);

	const auto disabledText = QColor(Qt::white).darker(300);

	QString tooltipCSS()
	{
		return QString::fromStdString(
			obelisk::formatString( "QToolTip { color: #000000; background-color: %s; border: 1px %s;}",
				lightBackground.name(QColor::HexRgb).toStdString().c_str(),
				tooltipBorder.name(QColor::HexRgb).toStdString().c_str()));
	}

	QPalette buildDarkPalette()
	{
		QPalette palette;

		palette.setColor(QPalette::Window, darkBackground);
		palette.setColor(QPalette::WindowText, Qt::white);
		palette.setColor(QPalette::Base, darkerBackground);
		palette.setColor(QPalette::AlternateBase, darkestBackground);
		palette.setColor(QPalette::ToolTipBase, Qt::white);
		palette.setColor(QPalette::ToolTipText, Qt::black);
		palette.setColor(QPalette::Text, Qt::white);
		palette.setColor(QPalette::Button, darkButton);
		palette.setColor(QPalette::ButtonText, Qt::white);
		palette.setColor(QPalette::BrightText, Qt::red);
		palette.setColor(QPalette::Link, brightBlue);
		palette.setColor(QPalette::LinkVisited, purple);
		palette.setColor(QPalette::Highlight, brightBlue);
		palette.setColor(QPalette::HighlightedText, Qt::black);

		palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
		palette.setColor(QPalette::Disabled, QPalette::Base, darkBackground);
		palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
		palette.setColor(QPalette::Disabled, QPalette::Button, darkerBackground);
		palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);

		return palette;
	}

	QPalette buildLightPalette()
	{
		QPalette palette;

		palette.setColor(QPalette::Window, lightBackground);
		palette.setColor(QPalette::WindowText, Qt::black);
		palette.setColor(QPalette::Base, Qt::white);
		palette.setColor(QPalette::AlternateBase, Qt::white);
		palette.setColor(QPalette::ToolTipBase, Qt::white);
		palette.setColor(QPalette::ToolTipText, Qt::black);
		palette.setColor(QPalette::Text, Qt::black);
		palette.setColor(QPalette::Button, lightButton);
		palette.setColor(QPalette::ButtonText, Qt::black);
		palette.setColor(QPalette::BrightText, Qt::red);
		palette.setColor(QPalette::Link, brightBlue);
		palette.setColor(QPalette::LinkVisited, purple);
		palette.setColor(QPalette::Highlight, brightBlue);
		palette.setColor(QPalette::HighlightedText, Qt::black);

		palette.setColor(QPalette::Disabled, QPalette::WindowText, disabledText);
		palette.setColor(QPalette::Disabled, QPalette::Base, lightBackground);
		palette.setColor(QPalette::Disabled, QPalette::Text, disabledText);
		palette.setColor(QPalette::Disabled, QPalette::Button, lightBackground);
		palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText);

		return palette;
	}
}

void qt_app_style::setDarkTheme(QApplication &app)
{
	app.setStyle(QStyleFactory::create("Fusion"));
	app.setPalette(buildDarkPalette());
	app.setStyleSheet(tooltipCSS());
}

void qt_app_style::setLightTheme(QApplication &app)
{
	app.setStyle(QStyleFactory::create("Fusion"));
	app.setPalette(buildLightPalette());
	app.setStyleSheet(tooltipCSS());
}