#include "state.h"

/*
 * Constructors
 */

State::State()
 : mode(RPL), prep(0), mem(0), progPtrOffset(0), dataPtr(0), next(NULL), prev(NULL)
{
	for(int i=0; i<NUM_BITS; i++)
		bit[i] = 0;
}

State::State( ull packed )
 : next(NULL), prev(NULL)
{
	unpack( packed );
}

State::State( State* other, int offset )
 : next(NULL), prev(NULL)
{
	mode = other->mode;
	prep = other->prep;
	mem = other->mem;
	dataPtr = (other->dataPtr - offset + NUM_BITS) % NUM_BITS;
	progPtrOffset = (other->progPtrOffset + offset) % NUM_BITS;
	for( int i=0; i<NUM_BITS; i++ ){
		bit[i] = other->bit[ (i+offset)%NUM_BITS ];
	}
}

/*
 * Comparison operator
 */

bool operator==(State a, State b){
	return a.equals(&b);
}

bool State::equals(State* b)
{
	return pack() == b->pack();
}

/*
 * Pack all state data into an integer for fast comparisons
 * Bit 0: mode
 * Bit 1: prep
 * Bit 2: mem
 * Bits 3-7: dataPtr
 * Bits 8-28 : bits, w/ progPtr at bit 0
 */
ull State::pack()
{
	ull packed = mode&1;
	packed <<= 1; packed |= (prep&1);
	packed <<= 1; packed |= (mem&1);
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
	mem = val&1; val >>= 1;
	prep = val&1; val >>= 1;
	mode = val&1;
}

/*
 * Calculate the next step, and add to linked list
 */
State* State::calcNextState()
{
	// pack state bits into a switchable int
	unsigned int varyState = 0;
	varyState |= mode; varyState <<= 1;
	varyState |= prep; varyState <<= 1;
	varyState |= bit[0]; varyState <<= 1;
	varyState |= bit[1];

	// prepare new state for returning
	State* following;
	if( varyState == 0 || varyState == 4 )
		following = new State(this, 2*MULT_SKIP);
	else
		following = new State(this, 2);

	// pre-operation global items of business
	following->prep = 0;

	// handle different modes on a case-by-case basis
	switch( varyState ){
	
	// multi-scroll
	case 0: case 4:
		note = "MULT";
		following->dataPtr = (following->dataPtr+MULT_SKIP-1)%NUM_BITS;
		break;

	// scroll
	case 1: case 5:
		note = "SCROLL";
		break;

	// read
	case 2:
		if( following->mem != following->bit[ following->dataPtr ] ){
			note = "READ!!!";
		}
		else note = "READ";
		following->mem = following->bit[ following->dataPtr ];
		following->prep = 1;
		break;
	case 11:
		note = "Product3";
		following->prep = 1;
		break;

	// write
	case 3: case 7:
		if( following->mem != following->bit[ following->dataPtr ] ){
			note = "WRITE!!!";
		}
		else note = "WRITE";
		following->bit[following->dataPtr] = following->mem;
		break;

	// to SYN
	case 6:
		note = "TO SYN";
		following->mode = SYN;
		break;

	// to RPL
	case 15:
		note = "TO RPL";
		following->mode = RPL;
		break;

	// products and whatnot
	default:
		note = "Product"+QString::number(varyState-8);
		break;
	};

	// post-operation global items of business
	following->dataPtr = (following->dataPtr+1)%NUM_BITS;

	return following;
}

QString State::toString(QString format)
{
	QChar progsym('\\'), datasym('/');
	QString bits;

	// print bit field
	for(int i=-(progPtrOffset); i<NUM_BITS-progPtrOffset; i++){
		if( i==0 )
			bits += progsym;
		else
			bits += " ";
		bits += QString::number(bit[(i+NUM_BITS)%NUM_BITS]);
		if( (i+NUM_BITS)%NUM_BITS == dataPtr )
			bits += datasym;
		else
			bits += " ";
	}

	return QString(format)
		.arg(bits)
	        .arg(mode==0?"RPL":"SYN")
	        .arg(mem)
	        .arg(note);
}
