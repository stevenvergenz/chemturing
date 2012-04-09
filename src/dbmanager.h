#ifndef __DBMANAGER_H
#define __DBMANAGER_H

#include <QObject>
#include <QString>
#include <QDebug>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include "simulation.h"

namespace DBManager
{
	bool prepareDatabase(QString host, QString dbname, QString user, QString password);
	bool commitRun( Simulation* s );
}

#endif

