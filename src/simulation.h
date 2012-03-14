#ifndef __SIMULATION_H
#define __SIMULATION_H

#include <QTextStream>

#include "state.h"

class Simulation
{
private:
	State* initial;

public:
	Simulation();
	~Simulation();
	void run();
	void print(FILE* fio = stdout);
};

#endif
