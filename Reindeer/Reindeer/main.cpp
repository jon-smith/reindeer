#include "stdafx.h"
#include "QtReindeerMainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtReindeerMainWindow w;
    w.show();
    return a.exec();
}
