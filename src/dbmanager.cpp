#include "dbmanager.h"

namespace DB
{


/*******************************************************************
  Ensures that all tables exist and are properly formatted.
  Fails otherwise.
*******************************************************************/
bool prepareDatabase( ConnectionInfo xInfo )
{
	DB::connectionInfo = xInfo;

	db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName(xInfo.host);
	db.setDatabaseName(xInfo.dbname);
	db.setUserName(xInfo.user);
	db.setPassword(xInfo.password);

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
		"sim_id INT UNSIGNED NOT NULL, "
		"final_state INT UNSIGNED NOT NULL, "
		"length INT UNSIGNED, "
		"term_loop_id INT UNSIGNED, "
			
		"PRIMARY KEY (sim_id)"
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
	
	db.close();

	return true;
}


/*******************************************************************
  Submits all relevant run information to the run table
*******************************************************************/
bool commitSimulation( Simulation* s )
{
	// set up thread-dependent db connection
	QThreadStorage<QSqlDatabase*> storage;
	QSqlDatabase* db;
	if( !storage.hasLocalData() ){
		db = new QSqlDatabase();
		storage.setLocalData(db);
		*db = QSqlDatabase::cloneDatabase(DB::db, QThread::currentThreadId());

		// make sure the database exists and the credentials are good
		if( !db->open() ){
			qCritical() << "Could not open threaded database" << endl;
			return false;
		}
	}
	else {
		db = storage.localData();
	}

	QSqlQuery query(*db);
	State* state = s->getStates();

	// check to see if the sim's initial state has already been run
	query.prepare("SELECT COUNT(state_def) FROM states WHERE state_def=:id;");
	query.bindValue( ":id", state->pack() );
	if( !query.exec() ){
		qCritical() << "Cannot query state table!" << endl;
		return false;
	}

	// consider the commit done if the initial state has already been tested
	query.first();
	if( query.record().value(0).toInt() != 0 ){
		return true;
	}

	// otherwise

	return true;
}


} // end namespace DBManager
