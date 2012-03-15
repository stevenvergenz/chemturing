#ifndef __DBMANAGER_H
#define __DBMANAGER_H

#include <QString>
#include <QtSql>

namespace DBManager
{
	QSqlDatabase db;

	void prepareDatabase(QString host, QString dbname, QString user, QString password);
}

#endif

