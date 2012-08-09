#ifndef __DBMANAGER_H
#define __DBMANAGER_H

#include <QObject>
#include <QString>
#include <QDebug>
#include <QMutex>
#include <QtSql>

#include "simulation.h"

namespace DB
{
	struct ConnectionInfo {
		QString host;
		QString dbname;
		QString user;
		QString password;
		QString toString(){
			return QString("(%1 %2 %3 %4)").arg(host,dbname,user,password);
		}
	};
	extern ConnectionInfo connectionInfo;
	extern QSqlDatabase db;
	extern QMutex simLock;

	bool prepareDatabase( ConnectionInfo xInfo );
	bool commitSimulation( Simulation* s );
	bool stateAlreadyRun( State* s );
}

#endif

