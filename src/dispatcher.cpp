#include "dispatcher.h"


Dispatcher::Dispatcher( QMap<QString,QVariant>* options )
: stateHash(0), dataPointer(0)
{
	// make sure the generator is random enough
	if( RAND_MAX < (1<<NUM_BITS) ){
		qFatal("Random number generator is not random enough!");
	}

	// seed random numbers
	qsrand( time(0) );
	
	// initialize database if specified and if it doesn't exist
	// localhost, ChemTuring, ctuser, T)^x83$PFhsS:1i
	if( options->contains("database") )
	{
		QStringList info = options->value("database").toString().split(" ", QString::SkipEmptyParts);
		qDebug() << "Connecting to database with (" << info << ")";
		if( ! DBManager::prepareDatabase(info[0], info[1], info[2], info[3]) ){
			qFatal("Could not verify database!");
		}
	}
	
	// initialize manditory run parameters
	if( options->value("mode").toString() == "random" ){
		mode = RANDOM;
	}
	else {
		mode = SEQUENTIAL;
	}
	runcount = options->value("count").toInt();
	threadpool.setMaxThreadCount( options->value("threads").toInt() );
	outputDir = QDir( options->value("output-dir").toString() );
	
	// queue start/end events
	connect( this, SIGNAL(readyToCalculate()), this, SLOT(startCalculation()) );
	connect( QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanUp()) );
	connect( this, SIGNAL(done()), QCoreApplication::instance(), SLOT(quit()) );
	
	emit readyToCalculate();
}


void Dispatcher::startCalculation()
{
	int cid;
	
	for( cid=0; cid<runcount; cid++ )
	{
		State* init = genState();
		QString file = QString::number( init->pack(), 16 ).rightJustified(7, '0') + ".txt";
		Simulation* s = new Simulation( init, outputDir.absoluteFilePath(file) );
		s->setAutoDelete(true);
		threadpool.start(s);
	}
	
	threadpool.waitForDone();
	emit done();
	exit(0);
}


State* Dispatcher::genState()
{
	if( mode == RANDOM )
		return genRandomState();
	else
		return genSequentialState();
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
	if( stateHash > 1<<(NUM_BITS+3) ){
		return NULL;
	}

	unsigned long seed = stateHash;
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
		stateHash++;
	}

	return s;
}

void Dispatcher::cleanUp()
{

}
