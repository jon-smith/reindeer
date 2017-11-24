#include "stdafx.h"
#include "QtSpinBox2.h"

#include <QKeyEvent>
#include <qapplication.h>

void impl::QtSpinAltStepBase::setCtrlStepSize(double stepSize)
{
	ctrlStepSize = stepSize;
	ctrlStepActive = true;
}

double impl::QtSpinAltStepBase::getCtrlStepSize() const
{
	return ctrlStepSize;
}

void impl::QtSpinAltStepBase::setAltStepSize(double stepSize)
{
	altStepSize = stepSize;
	altStepActive = true;
}

double impl::QtSpinAltStepBase::getAltStepSize() const
{
	return altStepSize;
}

QtDoubleSpinBox2::QtDoubleSpinBox2(QWidget *parent) : QDoubleSpinBox(parent)
{
	setFocusPolicy(Qt::StrongFocus);
}

QtDoubleSpinBox2::~QtDoubleSpinBox2()
{
}

void QtDoubleSpinBox2::stepBy(int step)
{
	auto const keyMod = QApplication::queryKeyboardModifiers();

	// If alt down, use the alt step
	auto const altDown = altStepActive && keyMod.testFlag(Qt::AltModifier);
	// If alt not down, and ctrl down, use the ctrl step
	auto const ctrlDown = !altDown && ctrlStepActive && keyMod.testFlag(Qt::ControlModifier);

	auto const currentStep = singleStep();

	if (altDown)
		setSingleStep(altStepSize);
	else if (ctrlDown)
		setSingleStep(ctrlStepSize);

	QDoubleSpinBox::stepBy(step);

	if (altDown || ctrlDown)
		setSingleStep(currentStep);

	emit signalStepChange();
	emit signalOnChangeComplete();
}

void QtDoubleSpinBox2::focusOutEvent(QFocusEvent *e)
{
	QDoubleSpinBox::focusOutEvent(e);
	emit signalFinishedEditing();
	emit signalOnChangeComplete();
}

void QtDoubleSpinBox2::wheelEvent(QWheelEvent *e)
{
	if (hasFocus())
	{
		if (e->delta() > 0)
			stepBy(1);
		else
			stepBy(-1);

		e->accept();
	}
	else
		e->ignore();
}

void QtDoubleSpinBox2::keyPressEvent(QKeyEvent *e)
{
	QDoubleSpinBox::keyPressEvent(e);

	switch (e->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
			emit signalFinishedEditing();
			emit signalOnChangeComplete();
			emit signalEnterPressed();
			break;
	}
}

QtSpinBox2::QtSpinBox2(QWidget *parent) : QSpinBox(parent)
{
	setFocusPolicy(Qt::StrongFocus);
}

QtSpinBox2::~QtSpinBox2()
{
}

void QtSpinBox2::stepBy(int step)
{
	auto const keyMod = QApplication::queryKeyboardModifiers();

	// If alt down, use the alt step
	auto const altDown = altStepActive && keyMod.testFlag(Qt::AltModifier);
	// If alt not down, and ctrl down, use the ctrl step
	auto const ctrlDown = !altDown && ctrlStepActive && keyMod.testFlag(Qt::ControlModifier);

	auto const currentStep = singleStep();

	if (altDown)
		setSingleStep(altStepSize);
	else if (ctrlDown)
		setSingleStep(ctrlStepSize);

	QSpinBox::stepBy(step);

	if (altDown || ctrlDown)
		setSingleStep(currentStep);

	emit signalStepChange();
	emit signalOnChangeComplete();
}

void QtSpinBox2::focusOutEvent(QFocusEvent *e)
{
	QSpinBox::focusOutEvent(e);
	emit signalFinishedEditing();
	emit signalOnChangeComplete();
}

void QtSpinBox2::wheelEvent(QWheelEvent *e)
{
	if (hasFocus())
	{
		if (e->delta() > 0)
			stepBy(1);
		else
			stepBy(-1);

		e->accept();
	}
	else
		e->ignore();
}

void QtSpinBox2::keyPressEvent(QKeyEvent *e)
{
	QSpinBox::keyPressEvent(e);

	switch (e->key())
	{
		case Qt::Key_Return:
		case Qt::Key_Enter:
			emit signalFinishedEditing();
			emit signalOnChangeComplete();
			emit signalEnterPressed();
			break;
	}
}