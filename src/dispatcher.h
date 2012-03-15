#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#include <ctime>
#include <QMutex>
#include <QMutexLocker>

#include "state.h"


namespace Dispatcher
{
	void init();
	State* genRandomState();
	State* genSequentialState();
}

#endif
