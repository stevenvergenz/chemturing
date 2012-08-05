#ifndef __SIMULATION_H
#define __SIMULATION_H

#include <QFile>
#include <QTextStream>
#include <QRunnable>

#include "state.h"

class Simulation : public QRunnable
{
private:
	State* initial;
	int loopStep;
	float execPct;
	float rewritePct;
	QString outfile;
	
public:
	Simulation( State* s, QString out );
	~Simulation();
	void run();
	void print();
	State* getStates();
};

#endif
