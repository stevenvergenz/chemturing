#include "dbmanager.h"

namespace DBManager
{


bool prepareDatabase(QString host, QString dbname, QString user, QString password)
{
	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName(host);
	db.setDatabaseName(dbname);
	db.setUserName(user);
	db.setPassword(password);

	if( !db.open() ){
		qCritical("Could not open database");
		return false;
	}

	QSqlQuery query;

	// create the "states" table to contain the discrete states
	query.prepare("CREATE TABLE IF NOT EXISTS states ("
		"state_def INT UNSIGNED, "
		"stepnum SMALLINT UNSIGNED, "
		"run_id INT UNSIGNED NOT NULL, "
		"loop_id INT, "
		"PRIMARY KEY (state_def), "
		"FOREIGN KEY (run_id) REFERENCES runs, "
		"FOREIGN KEY (loop_id) REFERENCES loops"
	") ENGINE=InnoDB;" );

	query.prepare("CREATE TABLE IF NOT EXISTS merges ("
		"merge_id INT UNSIGNED, "
		"run_id INT UNSIGNED, "
		"merge_point INT UNSIGNED, "
	");");

	query.prepare("CREATE TABLE IF NOT EXISTS loops ("
		
	");");

	query.prepare("CREATE TABLE IF NOT EXISTS runs ("
		"run_id INT UNSIGNED, "
		"length INT UNSIGNED, "
		"term_type ENUM(merge, loop), "
		"term_id INT UNSIGNED, "
		"exec_pct FLOAT, "
		"rewrite_pct FLOAT, "
		"PRIMARY KEY (run_id)"
	");");
}


} // end namespace DBManager
