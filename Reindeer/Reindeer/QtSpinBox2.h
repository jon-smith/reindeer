#pragma once

#include <QDoubleSpinBox>

// Subclass of QDoubleSpinBox and QSpinBox with custom signals
// void signalFinishedEditing() when editing is finished (enter/return or lose focus)
// void signalStepChange() when stepped up/down

namespace impl
{
	class QtSpinAltStepBase
	{
	public:
		// Ctrl+click step size
		void setCtrlStepSize(double stepSize);
		double getCtrlStepSize() const;
		// Alt+click step size
		void setAltStepSize(double stepSize);
		double getAltStepSize() const;

	protected:

		bool ctrlStepActive = false;
		double ctrlStepSize = 1.0;
		bool altStepActive = false;
		double altStepSize = 1.0;
	};
}

class QtDoubleSpinBox2 : public QDoubleSpinBox, public impl::QtSpinAltStepBase
{
	Q_OBJECT;

public:
	QtDoubleSpinBox2(QWidget *parent = nullptr);
	~QtDoubleSpinBox2();

	void stepBy(int step) override;

signals:
	void signalFinishedEditing();
	void signalStepChange();
	void signalOnChangeComplete();  // Either step change or finish edit, but not during typing
	void signalEnterPressed();  // fired only when Enter (carriage return) is pressed

private:
	void focusOutEvent(QFocusEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;
	void keyPressEvent(QKeyEvent *e) override;
};

class QtSpinBox2 : public QSpinBox, public impl::QtSpinAltStepBase
{
	Q_OBJECT;

public:
	QtSpinBox2(QWidget *parent = nullptr);
	~QtSpinBox2();

	void stepBy(int step) override;

signals:
	void signalFinishedEditing();
	void signalStepChange();
	void signalOnChangeComplete();  // Either step change or finish edit, but not during typing
	void signalEnterPressed();  // fired only when Enter (carriage return) is pressed

private:
	void focusOutEvent(QFocusEvent *e) override;
	void wheelEvent(QWheelEvent *e) override;
	void keyPressEvent(QKeyEvent *e) override;
};