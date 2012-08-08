#include <QtCore/QCoreApplication>
#include <QStringList>
#include <QMap>
#include <QDir>
#include <iostream>

#include "simulation.h"
#include "dispatcher.h"
#include "dbmanager.h"

using namespace std;

void printUsage()
{
	cout    << "CHEMTURING v1.0, written by Steven Vergenz" << endl
		<< "Usage:" << endl
		<< "  chemecher [--help] --mode <mode> [--count <c>] [--threads <tc>]" << endl
		<< "      [--file-path <dir>] [--database <host> <db> <user> <pass>]" << endl
		<< "Arguments:" << endl
		<< "  --help       : Print this usage information and exit." << endl
		<< "  --mode       : REQUIRED - Sets state generation mode. RANDOM or SEQUENCE valid." << endl
		<< "  --count      : Sets the number of simulations to run. Defaults to 1." << endl
		<< "  --threads    : Sets the number of worker threads. Defaults to the number of processors." << endl
		<< "  --output-dir : Write simulations to files in the given directory. Defaults to current directory." << endl
		<< "  --database   : Use the given database for state comparisons." << endl
		<< "  NOTE: if neither output-dir nor database is specified, chemturing will default to stdout." << endl
		<< endl;
}


QMap<QString,QVariant>* parseArguments( QStringList arglist )
{
	// default no-arguments call prints usage
	if( arglist.size() == 1 ){
		printUsage();
		return NULL;
	}
	
	// initialize all arguments to defaults
	QMap<QString,QVariant> *map = new QMap<QString,QVariant>();
	//map->insert("mode", "random");
	map->insert("count", 1);
	map->insert("threads", QThread::idealThreadCount());
	//map->insert("output-dir", QDir::current().absolutePath());

	// loop over arguments, populate map
	QStringList::const_iterator i;
	for( i=arglist.constBegin()+1; i!=arglist.constEnd(); i++ )
	{
		//cout << (*i).toStdString() << endl;

		// parse mode
		if( *i == "--mode" )
		{
			i++;
			if( QString::compare( *i, "random", Qt::CaseInsensitive ) == 0 ){
				(*map)["mode"] = "random";
			}
			else if( QString::compare( *i, "sequence", Qt::CaseInsensitive ) == 0 ){
				(*map)["mode"] = "sequence";
			}
			else {
				cout << "Invalid mode, defaulting to RANDOM." << endl;
				(*map)["mode"] = "random";
			}
		}
		
		// parse simulation count
		else if( *i == "--count" )
		{
			i++;
			bool ok;
			int val = (*i).toInt( &ok );
			if( !ok ){
				cout << "Invalid simulation count, defaulting to 1." << endl;
				(*map)["count"] = 1;
			}
			else
				(*map)["count"] = val;
		}
		
		// parse thread count
		else if( *i == "--threads" )
		{
			i++;
			bool ok;
			int val = (*i).toInt( &ok );
			if( !ok ){
				cout << "Invalid thread count, defaulting to the number of processors." << endl;
			}
			else
				(*map)["threads"] = val;
		}
		
		// parse database information
		else if( *i == "--database" )
		{
			i++;
			QString val;
			for(int c=0; c<4; c++){
				if( i == arglist.constEnd() ){
					cout << "Not enough arguments to --database" << endl;
					delete map;
					return NULL;
				}
				val = val + *i + " ";
				i++;
			}

			(*map)["database"] = val;

			// make sure we haven't run past the end
			if( i == arglist.constEnd() )
				break;
		}
		
		// parse file output directory
		else if( *i == "--output-dir" )
		{
			i++;
			QDir dir(*i);
			if( !dir.exists() ){
				cout << "Specified directory does not exist, or is a file. Defaulting to current directory." << endl;
			}
			else {
				(*map)["output-dir"] = dir.absolutePath();
			}
		}
		
		// parse help flag
		else if( *i == "--help" )
		{
			printUsage();
			delete map;
			return NULL;
		}
		
		// fail on all other arguments
		else 
		{
			cout << QString("Invalid argument: %1").arg(*i).toStdString() << endl;
			delete map;
			return NULL;
		}
	}

	if( !map->contains("mode") ){
		cout << "The --mode flag is NOT optional. Please specify a state generation mode." << endl;
		delete map;
		return NULL;
	}
	
	return map;
}


int main(int argc, char** argv)
{
	QCoreApplication a(argc, argv);

	// pull out arguments
	QMap<QString,QVariant>* options = parseArguments( a.arguments() );

	// make sure parse succeeded
	if( options == NULL ){
		cout << "No simulations run." << endl;
		return 1;
	}
	
	Dispatcher dispatch(options);
	
	delete options;
	
	return a.exec();
}


