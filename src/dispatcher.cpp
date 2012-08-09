#include "dispatcher.h"


Dispatcher::Dispatcher( QMap<QString,QVariant>* options )
: stateHash(0), dataPointer(0)
{
	// seed random numbers
	qsrand( time(0) );
	
	// initialize database if specified and if it doesn't exist
	// localhost, ChemTuring, ctuser, T)^x83$PFhsS:1i
	if( options->contains("database") )
	{
		QStringList info = options->value("database").toString().split(" ", QString::SkipEmptyParts);
		DB::ConnectionInfo cnxInfo;
		cnxInfo.host     = info[0];
		cnxInfo.dbname   = info[1];
		cnxInfo.user     = info[2];
		cnxInfo.password = info[3];

		std::cout << "Connecting to database with " << cnxInfo.toString().toStdString() << endl;
		if( ! DB::prepareDatabase(cnxInfo) ){
			qFatal("Could not verify database!");
		}
		else std::cout << "Connected" << endl;
	}

	if( options->contains("output-dir") ){
		outputDir = options->value("output-dir").toString();
	}

	// no other output mode was specified, default to stdout
	if( !options->contains("database") && !options->contains("output-dir") ){
		outputDir = "stdout";
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
	
	// queue start/end events
	connect( this, SIGNAL(readyToCalculate()), this, SLOT(startCalculation()) );
	connect( QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanUp()) );
	connect( this, SIGNAL(done()), QCoreApplication::instance(), SLOT(quit()) );
	
	emit readyToCalculate();
}


void Dispatcher::startCalculation()
{
	int cid;
	QDir dir(outputDir);

	for( cid=0; cid<runcount; cid++ )
	{
		// only run unique simulations
		State* init = genState();
		while( DB::stateAlreadyRun(init) ){
			delete init;
			init = genState();
		}

		QString file;
		if( outputDir == "stdout" ){
			file = "stdout";
		}
		else if( outputDir == "" ){
			file = "";
		}
		else {
			file = dir.absoluteFilePath(
				QString::number( init->pack(), 16 ).rightJustified(7, '0') + ".txt"
			);
		}

		Simulation* s = new Simulation( init, file );
		s->setAutoDelete(true);
		threadpool.start(s);
	}
	
	while( !threadpool.waitForDone(1000) )
		std::cout << ".";

	if( Simulation::abort ){
		qCritical() << "There was a problem with the simulation! They have been aborted.";
	}
	else {
		std::cout << endl << "Simulations completed successfully." << endl;
	}
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
	s->dataPtr = Dispatcher::random(6) % NUM_BITS;

	// randomize bits and properties
	unsigned long r = Dispatcher::random(NUM_BITS+3) % (1<<(NUM_BITS+3));
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

// produces at least n bits of entropy
ull Dispatcher::random(int bits)
{
	// make sure the generator is random enough
	const int maxbits = log2( (double)(RAND_MAX+1) );
	ull val = qrand();
	for(int i=1; i<(bits/maxbits)+1; i++){
		val <<= maxbits;
		val |= qrand();
	}
	return val;

	/*if( RAND_MAX < (1<<bits) ){
		qFatal("Random number generator is not random enough!");
	}*/
}
