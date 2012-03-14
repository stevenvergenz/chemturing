#include "state.h"

/*
 * Constructors
 */

State::State()
 : mode(RPL), prep(0), mem(0), progPtr(0), dataPtr(0), next(NULL), prev(NULL)
{
	for(int i=0; i<NUM_BITS; i++)
		bit[i] = 0;
}

State::State( ull packed )
 : next(NULL), prev(NULL)
{
	unpack( packed );
}

State::State( State* other )
 : next(NULL), prev(NULL)
{
	unpack( other->pack() );
}

/*
 * Comparison operator
 */

bool operator==(State a, State b){
	return a.equals(&b);
}

bool State::equals(State* b)
{
	ull other = b->pack();

	for(int i=0; i<NUM_BITS; i++){
		if( packedRot[i] == other ) return true;
	}

	return false;
}

/*
 * Pack all state data into an integer for fast comparisons
 * Bit 0: mode
 * Bit 1: prep
 * Bit 2: mem
 * Bits 3-7: progPtr
 * Bits 8-12: dataPtr
 * Bits 13- : bits
 */
ull State::pack()
{
	ull packed = mode&1;
	packed <<= 1; packed |= (prep&1);
	packed <<= 1; packed |= (mem&1);
	packed <<= 5; packed |= (progPtr&0x1f);
	packed <<= 5; packed |= (dataPtr&0x1f);
	for(int i=0; i<NUM_BITS; i++){
		packed <<= 1; packed |= (bit[i]&1);
	}

	return packed;
}

void State::unpack(ull val)
{
	for(int i=NUM_BITS-1; i>=0; i--){
		bit[i] = val&1; val >>= 1;
	}
	dataPtr = val&0x1f; val >>= 5;
	progPtr = val&0x1f; val >>= 5;
	mem = val&1; val >>= 1;
	prep = val&1; val >>= 1;
	mode = val&1;
}

void State::packRots()
{
	// perform rotations
	for( int i=0; i<NUM_BITS; i++ )
	{
		// save current rotation
		packedRot[i] = pack();

		// rotate bitfield one to the right
		progPtr = (progPtr+1) % NUM_BITS;
		dataPtr = (dataPtr+1) % NUM_BITS;
		Bit temp = bit[NUM_BITS-1];
		for( int j=NUM_BITS-1; j>0; j-- ){
			bit[j] = bit[j-1];
		}
		bit[0] = temp;
	}
}


/*
 * Calculate the next step, and add to linked list
 */
State* State::calcNextState()
{
	// prepare new state for returning
	State* next = new State(this);

	// pack state bits into a switchable int
	unsigned int varyState = 0;
	varyState |= mode; varyState <<= 1;
	varyState |= prep; varyState <<= 1;
	varyState |= bit[progPtr]; varyState <<= 1;
	varyState |= bit[(progPtr+1)%NUM_BITS];

	// handle different modes on a case-by-case basis
	switch( varyState ){
	
	// multi-scroll
	case 0: case 4:
		note = "MULT";
		next->progPtr = (next->progPtr+2*MULT_SKIP)%NUM_BITS;
		next->dataPtr = (next->dataPtr+1*MULT_SKIP)%NUM_BITS;
		next->prep = 0;
		break;

	// scroll
	case 1: case 5:
		note = "SCROLL";
		next->progPtr = (next->progPtr+2)%NUM_BITS;
		next->dataPtr = (next->dataPtr+1)%NUM_BITS;
		next->prep = 0;
		break;

	// read
	case 2:
		next->mem = next->bit[ next->dataPtr ];
	case 11:
		note = varyState==2?"READ":"Product3";
		next->prep = 1;
		next->progPtr = (next->progPtr+2)%NUM_BITS;
		next->dataPtr = (next->dataPtr+1)%NUM_BITS;
		break;

	// write
	case 3: case 7:
		note = "WRITE";
		next->bit[dataPtr] = next->mem;
		next->prep = 0;
		next->progPtr = (next->progPtr+2)%NUM_BITS;
		next->dataPtr = (next->dataPtr+1)%NUM_BITS;
		break;

	// to SYN
	case 6:
		note = "TO SYN";
		next->mode = SYN;
		next->prep = 0;
		next->progPtr = (next->progPtr+2)%NUM_BITS;
		next->dataPtr = (next->dataPtr+1)%NUM_BITS;
		break;

	// to RPL
	case 15:
		note = "TO RPL";
		next->mode = RPL;
		next->prep = 0;
		next->progPtr = (next->progPtr+2)%NUM_BITS;
		next->dataPtr = (next->dataPtr+1)%NUM_BITS;
		break;

	// products and whatnot
	default:
		note = "Product"+QString::number(varyState-8);
		next->prep = 0;
		next->progPtr = (next->progPtr+2)%NUM_BITS;
		next->dataPtr = (next->dataPtr+1)%NUM_BITS;
		break;
	};

	next->packRots();

	return next;
}

QString State::toString(QString format)
{
	QChar progsym('\\'), datasym('/');
	QString bits;

	// print bit field
	for(int i=0; i<NUM_BITS; i++){
		if( i==progPtr )
			bits += progsym;
		bits += QString::number(bit[i]);
		if( i==dataPtr )
			bits += datasym;
	}

	return QString(format)
		.arg(bits)
	        .arg(mode==0?"RPL":"SYN")
	        .arg(mem)
	        .arg(note);
}
