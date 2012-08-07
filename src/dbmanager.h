#ifndef __DBMANAGER_H
#define __DBMANAGER_H

#include <QObject>
#include <QString>
#include <QDebug>

#include <QtSql>

#include "simulation.h"

namespace DB
{
	struct ConnectionInfo {
		QString host;
		QString dbname;
		QString user;
		QString password;
	};
	extern ConnectionInfo connectionInfo;
	extern QSqlDatabase db;

	bool prepareDatabase( ConnectionInfo xInfo );
	bool commitSimulation( Simulation* s );
}

#endif

