#include <QtCore/QCoreApplication>

#include "simulation.h"

int main(int argc, char *argv[])
{
	//QCoreApplication a(argc, argv);
	
	Simulation sim;
	sim.run();
	sim.print();
	
	return 0;
	//return a.exec();
}
