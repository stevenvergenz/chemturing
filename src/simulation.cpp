#include "simulation.h"

Simulation::Simulation()
{
	initial = new State();
	initial->mode = State::SYN;
	initial->mem = 1;
	initial->dataPtr = 18;
	int array[20] = {1,1,0,0,1,0,0,1,1,0,1,0,0,1,0,0,1,1,1,1};
	for(int i=0; i<20; i++)
		initial->bit[i] = array[i];
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
	State* cur = initial;

	while(true)
	{
		// check for loops
		State* temp = initial;
		int stepnum = 0;
		bool match = false;
		while( temp != cur && !match ){
			if( temp->equals(cur) )
				match = true;
			temp = temp->next;
			stepnum++;
		}

		// break out of sim loop if match found
		if( match ){
			cur->note = QString("LOOP TO %1").arg(stepnum-1);
			break;
		}

		// iterate
		cur->next = cur->calcNextState();
		cur->next->prev = cur;
		cur = cur->next;
	}

	// calculate simulation properties: mode cycling, dynamism
	
}

void Simulation::print(FILE* fio)
{
	QTextStream qout(fio);
	qout << "Step Bits (prog=\\ data=/)    Mode Mem Command" << endl;
	State* cur = initial;
	int stepcount = 0;
	while( cur != NULL ){
		qout << QString("%1 ").arg(stepcount++, 4)
			<< cur->toString("%1  %2  %3   %4") << endl;
		cur = cur->next;
	}
}

