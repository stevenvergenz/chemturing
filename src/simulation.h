#ifndef __SIMULATION_H
#define __SIMULATION_H

#include <QTextStream>
#include <QThread>

#include "state.h"

class Simulation : public QThread
{
private:
	State* initial;

public:
	Simulation( State* s );
	~Simulation();
	void run();
	void print(FILE* fio = stdout);
};

#endif
