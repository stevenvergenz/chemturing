#include "dispatcher.h"


Dispatcher::Dispatcher()
: stateSeed(0), dataPointer(0)
{
	// make sure the generator is random enough
	if( RAND_MAX < (1<<NUM_BITS) ){
		qCritical("Random number generator is not random enough!");
		return;
	}

	// seed random numbers
	qsrand( time(0) );
	
	// initialize database, if it doesn't exist
	if( !DBManager::prepareDatabase("localhost", "ChemTuring", "ctuser", "T)^x83$PFhsS:1i") ){
		qCritical("Could not verify database!");
	}
	
	// queue cleanup event
	connect( QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanUp()) );
}

void Dispatcher::startCalculation()
{

}

State* Dispatcher::genRandomState()
{
	State* s = new State();

	// randomize data pointer
	s->dataPtr = qrand() % NUM_BITS;

	// randomize bits and properties
	unsigned long r = qrand() % (1<<(NUM_BITS+3));
	s->mem = r&1;  r>>= 1;
	s->mode = r&1; r>>= 1;
	s->prep = r&1; r>>= 1;
	for( int i=0; i<NUM_BITS; i++ ){
		s->bit[i] = r&1;
		r >>= 1;
	}

	return s;
}

State* Dispatcher::genSequentialState()
{
	if( stateSeed > 1<<(NUM_BITS+3) ){
		return NULL;
	}

	unsigned long seed = stateSeed;
	State* s = new State();
	QMutexLocker lock(&mutex);

	// populate new state with contents of seed
	s->mem = seed&1;  seed >>=1;
	s->mode = seed&1; seed >>=1;
	s->prep = seed&1; seed >>=1;
	s->dataPtr = dataPointer;

	for( int i=0; i<NUM_BITS; i++ ){
		s->bit[i] = seed&1; seed >>=1;
	}

	// increment seed values
	dataPointer = (dataPointer+1) % NUM_BITS;
	if( dataPointer == 0 ){
		stateSeed++;
	}

	return s;
}

void Dispatcher::cleanUp()
{

}
