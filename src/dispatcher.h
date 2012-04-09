#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#include <ctime>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>

#include "state.h"
#include "dbmanager.h"


class Dispatcher : public QObject
{
Q_OBJECT
public:
	enum {SEQUENTIAL, RANDOM} Mode;
	
	Dispatcher();

public slots:
	void startCalculation();
	void cleanUp();
	
private:
	unsigned long stateSeed;
	int dataPointer;
	QMutex mutex;

	State* genRandomState();
	State* genSequentialState();
};

#endif
