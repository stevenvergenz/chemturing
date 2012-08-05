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

	// make sure the database exists and the credentials are good
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

	QString createSimulations = 
		"CREATE TABLE IF NOT EXISTS simulations ("
		"emu_id INT UNSIGNED NOT NULL, "
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
		"sim_id INT UNSIGNED NOT NULL, "
		"loop_id INT UNSIGNED, "
			
		"PRIMARY KEY (state_def), "
		"FOREIGN KEY (sim_id) REFERENCES simulations, "
		"FOREIGN KEY (loop_id) REFERENCES loops"
		") ENGINE=InnoDB;";
		
		
	// ensure that the tables exist (does not guarantee layout)
	QSqlQuery query;
	if( !query.exec(createStates) || !query.exec(createLoops) || !query.exec(createSimulations) ){
		qCritical() << "Could not create critical table: " << db.lastError().text();
		return false;
	}
	
	// confirm the layout of the critical tables
	QSqlRecord table = db.record("loops");
	if( !table.contains("loop_id")
			|| !table.contains("length")
			|| !table.contains("instance_cnt")
	){
		qCritical() << "Loop table is of the wrong format!";
		return false;
	}
	
	table = db.record("simulations");
	if( !table.contains("sim_id")
			|| !table.contains("final_state")
			|| !table.contains("length")
			|| !table.contains("term_loop_id")
	){
		qCritical() << "Simulations table is of the wrong format!";
		return false;
	}
	
	table = db.record("states");
	if( !table.contains("state_def")
			|| !table.contains("next_state")
			|| !table.contains("stepnum")
			|| !table.contains("sim_id")
			|| !table.contains("loop_id")
	){
		qCritical() << "State table is of the wrong format!";
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


} // end namespace DBManager
