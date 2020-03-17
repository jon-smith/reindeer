#include "QtDrawingHelpers.h"

#include <QPainter>

using namespace qt_drawing_helpers;

ScopedQPainterState::ScopedQPainterState(QPainter &painter) : painter(painter)
{
	painter.save();
}

ScopedQPainterState::~ScopedQPainterState()
{
	painter.restore();
}

void qt_drawing_helpers::drawTextBox(QPainter &painter,
	const QString &text,
	const QFont &font,
	const QPoint &topLeft,
	const QBrush &fillBrush,
	const QPen &textPen)
{
	ScopedQPainterState scopedPainter(painter);

	QFontMetrics fm(font, painter.device());

	// Create a default bounding rectangle
	auto textBoxRect = fm.boundingRect(text);

	// Adjust to have a border
	textBoxRect.adjust(0, 0, 5, 2);

	textBoxRect.moveTopLeft(topLeft);
	painter.fillRect(textBoxRect, fillBrush);

	painter.setPen(textPen);
	painter.drawText(textBoxRect, text, QTextOption(Qt::AlignCenter | Qt::AlignHCenter));
}