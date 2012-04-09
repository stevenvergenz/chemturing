#include <QtCore/QCoreApplication>
#include <QThreadPool>

#include "simulation.h"
#include "dispatcher.h"
#include "dbmanager.h"

int main(int argc, char** argv)
{
	QCoreApplication a(argc, argv);

	Dispatcher dispatch;
	dispatch.startCalculation();
	
	//return 0;
	return a.exec();
}
