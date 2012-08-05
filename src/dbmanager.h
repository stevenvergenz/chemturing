#ifndef __DBMANAGER_H
#define __DBMANAGER_H

#include <QObject>
#include <QString>
#include <QDebug>

#include <QtSql>

#include "simulation.h"

namespace DBManager
{
	bool prepareDatabase(QString host, QString dbname, QString user, QString password);
	bool commitSimulation( Simulation* s );
}

#endif

