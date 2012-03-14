/*
 * state.h
 * This class contains the definition of a State
 * That is, bits of storage, a mode, a mem bit, and pointers.
 */

#ifndef __STATE_H
#define __STATE_H

#include <QTextStream>
#include <QString>

// will work as-is up to 31 bits
#define NUM_BITS 20
#define MULT_SKIP 5

// bool would work just as well, same size, but char has int operators
typedef unsigned char Bit;
// guaranteed 64 bits
typedef unsigned long long ull;

using namespace std;

struct State
{
	typedef enum {RPL=0, SYN=1} Mode;

	// state information
	Bit bit[NUM_BITS];
	Mode mode;
	Bit prep;
	Bit mem;
	unsigned char progPtr;
	unsigned char dataPtr;
	QString note;

	// linked list pointers
	State* next;
	State* prev;

	// constructors
	State();
	State( ull packed );
	State( State* other );

	// packing data
	ull pack();
	void unpack(ull val);
	void packRots();
	ull packedRot[NUM_BITS];

	// calculation
	State* calcNextState();

	bool equals(State* b);
	QString toString(QString format = "%1 %2 %3 %4");
};

bool operator==(State a, State b);

#endif
