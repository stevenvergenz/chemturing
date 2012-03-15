#include "dispatcher.h"

namespace Dispatcher {

// private variables
unsigned long stateSeed;
int dataPointer;
QMutex mutex;

void init()
{
	// make sure the generator is random enough
	if( RAND_MAX < (1<<NUM_BITS) ){
		qCritical("Random number generator is not random enough!");
		return;
	}

	// initialize sequential parameters
	stateSeed = 0;
	dataPointer = 0;

	// seed random numbers
	qsrand( time(0) );
}

State* genRandomState()
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

State* genSequentialState()
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


} // end namespace Dispatcher
