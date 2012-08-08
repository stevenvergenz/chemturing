#include "simulation.h"

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
	qDebug() << "Writing to " << outfile << endl;

	State* cur = initial;
	length = 0;
	int stepnum = 0;

	while(true)
	{
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

		cur->stepNum = stepnum++;

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
	DB::commitSimulation(this);
	print();
}


void Simulation::print()
{
	QFile f(outfile);
	f.open(QIODevice::WriteOnly);
	QTextStream qout( &f );
	
	qout << "Step Bits (prog=\\ data=/)    Mode Mem Command" << endl;
	State* cur = initial;
	while( cur != NULL ){
		qout << QString("%1 ").arg(cur->stepNum, 4)
			<< cur->toString("%1  %2  %3   %4") << endl;
		cur = cur->next;
	}
	
	f.close();
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
