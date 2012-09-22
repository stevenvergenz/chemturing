#include "simulation.h"

bool Simulation::abort = false;
QMutex Simulation::abortLock;

Simulation::Simulation( State* s, QString out ) : initial(s), outfile(out)
{

	/*initial = new State();
	initial->mode = State::SYN;
	initial->mem = 1;
	initial->dataPtr = 18;
	int array[20] = {1,1,0,0,1,0,0,1,1,0,1,0,0,1,0,0,1,1,1,1};
	for(int i=0; i<20; i++)
		initial->bit[i] = array[i];
	*/
}


Simulation::~Simulation()
{
	State* cur = initial;
	while(cur!=NULL){
		State* temp = cur->next;
		delete cur;
		cur = temp;
	}
}


void Simulation::run()
{
	//qDebug() << "Running " << QString::number( initial->pack(), 16 ).rightJustified(7, '0') << endl;

	// do not run the simulation if there is a problem
	if( Simulation::abort ) return;

	State* cur = initial;
	length = 0;
	int stepnum = 0;

	while(true)
	{
		cur->stepNum = stepnum++;

		// check for loops
		State* temp = initial;
		bool match = false;
		while( temp != cur && !match ){
			if( temp->equals(cur) ){
				match = true;
				cur->note = QString("%1 LOOP").arg(temp->stepNum);
				loopLength = cur->stepNum - temp->stepNum;
				loopState = temp;
			}
			temp = temp->next;
		}

		// break out of sim loop if match found
		if( match ) break;

		// iterate
		cur->next = cur->calcNextState();
		cur->next->prev = cur;
		cur = cur->next;
		length++;
	}

	// calculate simulation properties: mode cycling, dynamism, etc.


	// commit new run to the database and/or file
	if( !DB::commitSimulation(this) ){
		qCritical() << "Failed to commit simulation to database!";
		Simulation::abortLock.lock();
		Simulation::abort = true;
		Simulation::abortLock.unlock();
	}
	print();
}


void Simulation::print()
{
	QTextStream qout(stdout);
	QFile f;
	if( outfile == "" ){
		return;
	}
	else if( outfile != "stdout" ){
		f.setFileName(outfile);
		f.open(QIODevice::WriteOnly);
		qout.setDevice( &f );
		//qDebug() << "Running " << QString::number( initial->pack(), 16 ).rightJustified(7, '0') << endl;
	}
	
	qout << "Step Bits (prog=\\ data=/)    Mode Mem Command" << endl;
	State* cur = initial;
	while( cur != NULL ){
		qout << QString("%1 ").arg(cur->stepNum, 4)
			<< cur->toString("%1  %2  %3   %4") << endl;
		cur = cur->next;
	}
	qout << "Loop length: " << loopLength << endl;
	
	if( f.isOpen() ) f.close();
}

State* Simulation::getInitialState(){
	return initial;
}

State* Simulation::getLoopState(){
	return loopState;
}

int Simulation::getTotalLength(){
	return length;
}

int Simulation::getLoopLength(){
	return loopLength;
}
