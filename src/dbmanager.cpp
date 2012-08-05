#include "dbmanager.h"

namespace DBManager
{
QSqlDatabase db;


/*******************************************************************
  Ensures that all tables exist and are properly formatted.
  Fails otherwise.
*******************************************************************/
bool prepareDatabase(QString host, QString dbname, QString user, QString password)
{
	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName(host);
	db.setDatabaseName(dbname);
	db.setUserName(user);
	db.setPassword(password);

	if( !db.open() ){
		qCritical() << "Could not open database";
		return false;
	}


	/*QString createMerges = 
		"CREATE TABLE IF NOT EXISTS merges ("
		"merge_id INT UNSIGNED, "
		"run_id INT UNSIGNED, "
		"merge_point INT UNSIGNED, "
		");";
	*/

	QString createLoops = 
		"CREATE TABLE IF NOT EXISTS loops ("
		"loop_id INT UNSIGNED NOT NULL, "
		"length INT UNSIGNED, "
		"instance_cnt INT UNSIGNED, "
		"PRIMARY KEY (loop_id)"
		") ENGINE=InnoDB;";

	QString createRuns = 
		"CREATE TABLE IF NOT EXISTS runs ("
		"run_id INT UNSIGNED NOT NULL, "
		"final_state INT UNSIGNED NOT NULL, "
		"length INT UNSIGNED, "
		"term_loop_id INT UNSIGNED, "
		"PRIMARY KEY (run_id)"
		") ENGINE=InnoDB;";

	// create the "states" table to contain the discrete states
	QString createStates = 
		"CREATE TABLE IF NOT EXISTS states ("
		"state_def INT UNSIGNED, "
		"next_state INT UNSIGNED, "
		"stepnum SMALLINT UNSIGNED, "
		"run_id INT UNSIGNED NOT NULL, "
		"loop_id INT UNSIGNED, "
		"PRIMARY KEY (state_def), "
		"FOREIGN KEY (run_id) REFERENCES runs, "
		"FOREIGN KEY (loop_id) REFERENCES loops"
		") ENGINE=InnoDB;";
		
		
	// create tables
	QSqlQuery query;
	if( !query.exec(createStates) || !query.exec(createLoops) || !query.exec(createRuns) ){
		qCritical() << "Could not verify critical table: " << db.lastError().text();
		return false;
	}
	
	return true;
}


/*******************************************************************
  Submits all relevant run information to the run table
*******************************************************************/
bool commitRun( Simulation* s )
{
	return true;
}

bool testState( State* s )
{
	
}

} // end namespace DBManager
