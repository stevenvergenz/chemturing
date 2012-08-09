#ifndef __SIMULATION_H
#define __SIMULATION_H

#include <QFile>
#include <QTextStream>
#include <QRunnable>

#include "state.h"

class Simulation;

#include "dbmanager.h"

class Simulation : public QRunnable
{
private:
	State* initial;
	State* loopState;
	int length, loopLength;
	QString outfile;

public:
	Simulation( State* s, QString out );
	~Simulation();
	void run();
	void print();
	State* getInitialState();
	State* getLoopState();
	int getTotalLength();
	int getLoopLength();

	// handling abnormal exits
	static bool abort;
	static QMutex abortLock;
};

#endif
