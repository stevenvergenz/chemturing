#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#include <ctime>
#include <cmath>
#include <iostream>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QThreadPool>
#include <QStringList>
#include <QDebug>
#include <QMap>
#include <QDir>

#include "simulation.h"
#include "state.h"
#include "dbmanager.h"


class Dispatcher : public QObject
{
Q_OBJECT
public:
	enum GenMode {SEQUENTIAL, RANDOM};
	
	Dispatcher( QMap<QString,QVariant>* options );
	static ull random( int bits );

signals:
	void readyToCalculate();
	void done();

public slots:
	void startCalculation();
	void cleanUp();
	
private:
	// sequential state information
	unsigned long stateHash;
	int dataPointer;
	
	QThreadPool threadpool;
	QMutex mutex;
	
	// run parameters
	GenMode mode;
	int runcount;
	QString outputDir;
	
	State* genState();
	State* genRandomState();
	State* genSequentialState();
};

#endif
