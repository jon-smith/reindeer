#pragma once

class QPen;
class QPainter;
class QString;
class QFont;
class QPoint;
class QBrush;

namespace qt_drawing_helpers
{
	// RAII class to save the QPainter state and then restore when it goes out of scope
	class ScopedQPainterState
	{
	public:
		ScopedQPainterState(QPainter &painter);
		~ScopedQPainterState();

	private:
		QPainter &painter;
	};

	void drawTextBox(QPainter &painter,
		const QString &text,
		const QFont &font,
		const QPoint &topLeft,
		const QBrush &fillBrush,
		const QPen &textPen);
}
