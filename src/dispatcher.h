#ifndef __DISPATCHER_H
#define __DISPATCHER_H

#include <ctime>

#include "state.h"


namespace Dispatcher
{
	void init();
	State* genRandomState();
	State* genSequentialState();
}

#endif
