#include "dbmanager.h"

namespace DB
{

// namespace variables
QSqlDatabase db;
ConnectionInfo connectionInfo;
QMutex simLock;

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
		"loop_id BIGINT UNSIGNED NOT NULL, "
		"length INT UNSIGNED, "
		"instance_cnt INT UNSIGNED, "
			
		"PRIMARY KEY (loop_id)"
		") ENGINE=InnoDB;";

	QString createSimulations = 
		"CREATE TABLE IF NOT EXISTS simulations ("
		"sim_id BIGINT UNSIGNED NOT NULL, "
		"final_state BIGINT UNSIGNED, "
		"length INT UNSIGNED, "
		"term_loop_id BIGINT UNSIGNED, "
			
		"PRIMARY KEY (sim_id)"
		") ENGINE=InnoDB;";

	// create the "states" table to contain the discrete states
	QString createStates = 
		"CREATE TABLE IF NOT EXISTS states ("
		"state_def BIGINT UNSIGNED NOT NULL, "
		"next_state BIGINT UNSIGNED, "
		"stepnum SMALLINT UNSIGNED, "
		"sim_id BIGINT UNSIGNED NOT NULL, "
		"loop_id BIGINT UNSIGNED, "
			
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
		*db = QSqlDatabase::cloneDatabase(DB::db, QString::number(QThread::currentThreadId()) );

		// make sure the database exists and the credentials are good
		if( !db->open() ){
			qCritical() << "Could not open threaded database" << endl;
			return false;
		}
	}
	else {
		db = storage.localData();
	}

	// prepare queries
	QSqlQuery testState(*db), addState(*db), addSim(*db), updateSim(*db);
	testState.prepare("SELECT COUNT(state_def)as count, loop_id, sim_id FROM states "
		"WHERE state_def=:id;");
	addState.prepare("INSERT INTO states (state_def, next_state, stepnum, sim_id, loop_id) "
		"VALUES (:statedef, :nextstate, :stepnum, :simid, :loopid);");
	addSim.prepare("INSERT INTO simulations (sim_id, length) VALUES (:simid, :length);");
	updateSim.prepare("UPDATE simulations SET final_state=:final, term_loop_id=:termloopid "
		"WHERE sim_id=:id;");

	State* i;
	State* initial = s->getInitialState();
	ull loopFlag = 0;

	// make sure the simulation has been run
	if( initial == NULL || initial->next == NULL ){
		qCritical() << "Cannot commit an incomplete simulation" << endl;
		return false;
	}

	// lock simulation
	QMutexLocker locker( &DB::simLock );

	// start transaction
	db->transaction();

	// loop over states
	for( i = initial; i->next!=NULL; i = i->next )
	{
		// check if current state is in the db
		testState.bindValue( ":id", QVariant(i->pack()) );
		if( !testState.exec() ){
			qCritical() << "Cannot query state table!" << endl;
			db->rollback();
			return false;
		}
		testState.first();

		// if state IS NOT in db
		if( testState.record().value("count").toInt() == 0 )
		{
			// initial state is fresh
			if( i == initial )
			{
				// commit new simulation entry
				addSim.bindValue(":simid", QVariant(initial->pack()));
				addSim.bindValue(":length", QVariant(s->getLength()));
				if( !addSim.exec() ){
					qCritical() << "Cannot add new simulation to database!" << endl;
					db->rollback();
					return false;
				}
			}

			// state is the start of the loop
			if( i == s->getLoopState() )
			{
				// set the loop id flag for future commits
				loopFlag = i->pack();

				// create new entry in the loop table

				// update sim table with term loop id
			}

			// commit state to db
		}

		// state IS in db
		else
		{
			// initial state has already been checked
			if( i == initial )
			{
				// our work here is done
				db->rollback();
				return false;
			}

			// update simulation table with merger point and term loop id

			// increment term loop instance count

			// default case: break, complete the transaction, return true
			// includes self-loop case
			break;

		} // end state presence branch

	} // end state loop


	// commit transaction
	db->commit();

	// unlock simulation (implied)

	return true;
}


} // end namespace DBManager
