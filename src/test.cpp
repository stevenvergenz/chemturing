#include <cstdio>

#include "state.h"

int main()
{
	// initialize start state
	State* head = new State();
	head->mode = State::SYN;
	head->mem = 1;
	head->progPtr = 0;
	head->dataPtr = 18;
	int array[20] = {1,1,0,0,1,0,0,1,1,0,1,0,0,1,0,0,1,1,1,1};
	for(int i=0; i<20; i++)
		head->bit[i] = array[i];

	// print first 31 states
	State* cur = head;
	for(int i=0; i<33; i++){
		cur->next = cur->calcNextState();
		cur->next->prev = cur;
		cur->print();
		cur = cur->next;
	}

	cur = head;
	while(cur!=NULL){
		State* temp = cur->next;
		delete cur;
		cur = temp;
	}
	
	return 0;
}
