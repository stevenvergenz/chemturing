#include <QtCore/QCoreApplication>

#include "simulation.h"
#include "dispatcher.h"

int main(int argc, char *argv[])
{
	QCoreApplication a(argc, argv);

	Dispatcher::init();
	Simulation sim( Dispatcher::genRandomState() );

	a.connect( &sim, SIGNAL(finished()), SLOT(quit()) );
	sim.start();
	
	//return 0;
	return a.exec();
}
