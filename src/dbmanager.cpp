#include "dbmanager.h"

namespace DB
{

// namespace variables
QSqlDatabase db;
ConnectionInfo connectionInfo = {"","","",""};
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
		"FOREIGN KEY (sim_id)  REFERENCES simulations(sim_id), "
		"FOREIGN KEY (loop_id) REFERENCES loops(loop_id)"
		") ENGINE=InnoDB;";
		
		
	// ensure that the tables exist (does not guarantee layout)
	QSqlQuery query(DB::db);
	if( !query.exec(createLoops) || !query.exec(createSimulations) || !query.exec(createStates) ){
		qCritical() << "Could not create critical table: " << query.lastError().text() << query.lastError().type();
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
	
	// leave open for main-thread db calls
	//db.close();

	return true;
}


/*******************************************************************
  Submits all relevant run information to the run table
*******************************************************************/
bool commitSimulation( Simulation* s )
{
	// do nothing if database is not configured
	if( DB::connectionInfo.dbname == "" ){
		return true;
	}

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
	QSqlQuery addLoop(*db), incrementLoop(*db), termLoopFromState(*db);

	testState.prepare(
		"SELECT COUNT(state_def) AS count, loop_id, sim_id FROM states "
		"WHERE state_def=:id;");
	addState.prepare(
		"INSERT INTO states (state_def, next_state, stepnum, sim_id, loop_id) "
		"VALUES (:statedef, :nextstate, :stepnum, :simid, :loopid);");
	addSim.prepare(
		"INSERT INTO simulations (sim_id, length) VALUES (:simid, :length);");
	updateSim.prepare(
		"UPDATE simulations SET final_state=:final, term_loop_id=:termloopid "
		"WHERE sim_id=:id;");
	addLoop.prepare(
		"INSERT INTO loops (loop_id, length, instance_cnt) "
		"VALUES (:loopid, :length, 0);");
	incrementLoop.prepare(
		"UPDATE loops SET instance_cnt=instance_cnt+1 WHERE loop_id=:id;");
	termLoopFromState.prepare(
		"SELECT simulations.term_loop_id AS termloop FROM states JOIN simulations "
		"ON states.sim_id=simulations.sim_id WHERE states.state_def=:id;");

	State* i;
	State* initial = s->getInitialState();
	ull loopState = 0;
	bool loopFlag = false;

	// make sure the simulation has been run
	if( initial == NULL || initial->next == NULL ){
		qCritical() << "Cannot commit an incomplete simulation." << endl;
		return false;
	}

	// lock simulation
	QMutexLocker locker( &DB::simLock );

	// start transaction
	db->transaction();

	// loop over states
	for( i = initial; i!=NULL; i = i->next )
	{
		// check if current state is in the db
		testState.bindValue( ":id", QVariant(i->pack()) );
		if( !testState.exec() ){
			qCritical() << "Cannot query state table!" << endl
				<< testState.lastError().text();
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
				addSim.bindValue(":length", QVariant(s->getTotalLength()));
				if( !addSim.exec() ){
					qCritical() << "Cannot add new simulation to database!" << endl
						<< addSim.lastError().text();
					db->rollback();
					return false;
				}
			}

			// state is the start of the loop
			if( *i == *(s->getLoopState()) )
			{
				// set the loop id flag for future commits
				loopFlag = true;
				loopState = i->pack();

				// create new entry in the loop table
				addLoop.bindValue(":id", QVariant(loopState));
				addLoop.bindValue(":length", QVariant(s->getLoopLength()));
				if( !addLoop.exec() ){
					qCritical() << "Cannot add new loop to database!" << endl
						<< addLoop.lastError().text();
					db->rollback();
					return false;
				}

			}

			// commit state to db
			addState.bindValue(":statedef", QVariant(i->pack()));
			addState.bindValue(":nextstate", QVariant(i->next->pack()));
			addState.bindValue(":stepnum", QVariant(i->stepNum));
			addState.bindValue(":simid", QVariant(initial->pack()));
			addState.bindValue(":loopid", loopFlag ? QVariant(loopState) : QVariant() );
			if( !addState.exec() ){
				qCritical() << "Cannot add new state to database!" << endl
					<< addState.lastError().text();
				db->rollback();
				return false;
			}
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

			// get the db state's loop id
			ull termLoop = 0;
			if( testState.record().value("loop_id").isNull() )
			{
				// join a non-loop state
				termLoopFromState.bindValue(":id", QVariant(i->pack()));
				if( !termLoopFromState.exec() ){
					qCritical() << "Cannot retrieve terminal loop from state!" << endl
						<< termLoopFromState.lastError().text();
					db->rollback();
					return false;
				}
				termLoop = termLoopFromState.record().value("termloop").toLongLong();
			}
			else
			{
				// join a loop state
				termLoop = testState.record().value("loop_id").toLongLong();
			}

			// update simulation table with merger point and term loop id
			updateSim.bindValue(":final", QVariant(i->pack()));
			updateSim.bindValue(":termloopid", QVariant(termLoop));
			updateSim.bindValue(":id", QVariant(initial->pack()));
			if( !updateSim.exec() ){
				qCritical() << "Cannot update sim with terminal info!" << endl
					<< updateSim.lastError().text();
				db->rollback();
				return false;
			}

			// increment term loop instance count
			incrementLoop.bindValue(":id", termLoop);
			if( !incrementLoop.exec() ){
				qCritical() << "Cannot increment loop instance count!" << endl
					<< incrementLoop.lastError().text();
				db->rollback();
				return false;
			}

			// default case: break, complete the transaction, return true
			// includes self-looping state case
			break;

		} // end state presence branch

	} // end state loop


	// commit transaction
	db->commit();

	// unlock simulation
	locker.unlock();

	return true;
}


bool stateAlreadyRun(State *s)
{
	QSqlQuery query(DB::db);
	query.prepare("SELECT COUNT(state_def) AS count FROM states WHERE state_def=:id;");
	query.bindValue(":id", QVariant(s->pack()) );
	query.exec();
	return query.record().value("count").toInt() != 0;
}

} // end namespace DBManager
