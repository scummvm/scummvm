/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 * Virtual processor.
 */

#include "tinsel/dw.h"
#include "tinsel/drives.h"
#include "tinsel/events.h"	// 'POINTED' etc.
#include "tinsel/handle.h"	// LockMem()
#include "tinsel/dialogs.h"	// for inventory id's
#include "tinsel/pcode.h"	// opcodes etc.
#include "tinsel/scn.h"	// FindChunk()
#include "common/serializer.h"
#include "tinsel/timers.h"
#include "tinsel/tinlib.h"	// Library routines
#include "tinsel/tinsel.h"

#include "common/util.h"

namespace Tinsel {

//----------------- EXTERN FUNCTIONS --------------------

extern int CallLibraryRoutine(CORO_PARAM, int operand, int32 *pp, const INT_CONTEXT *pic, RESUME_STATE *pResumeState);

//----------------- LOCAL DEFINES --------------------

/** list of all opcodes */
enum OPCODE {
	OP_HALT = 0,	//!< end of program
	OP_IMM = 1,		//!< loads signed immediate onto stack
	OP_ZERO = 2,	//!< loads zero onto stack
	OP_ONE = 3,		//!< loads one onto stack
	OP_MINUSONE = 4,	//!< loads minus one onto stack
	OP_STR = 5,		//!< loads string offset onto stack
	OP_FILM = 6,	//!< loads film offset onto stack
	OP_FONT = 7,	//!< loads font offset onto stack
	OP_PAL = 8,		//!< loads palette offset onto stack
	OP_LOAD = 9,	//!< loads local variable onto stack
	OP_GLOAD = 10,	//!< loads global variable onto stack - long offset to variable
	OP_STORE = 11,	//!< pops stack and stores in local variable - long offset to variable
	OP_GSTORE = 12,	//!< pops stack and stores in global variable - long offset to variable
	OP_CALL = 13,	//!< procedure call
	OP_LIBCALL = 14,	//!< library procedure call - long offset to procedure
	OP_RET = 15,		//!< procedure return
	OP_ALLOC = 16,	//!< allocate storage on stack
	OP_JUMP = 17,	//!< unconditional jump	- signed word offset
	OP_JMPFALSE = 18,	//!< conditional jump	- signed word offset
	OP_JMPTRUE = 19,	//!< conditional jump	- signed word offset
	OP_EQUAL = 20,	//!< tests top two items on stack for equality
	OP_LESS,	//!< tests top two items on stack
	OP_LEQUAL,	//!< tests top two items on stack
	OP_NEQUAL,	//!< tests top two items on stack
	OP_GEQUAL,	//!< tests top two items on stack
	OP_GREAT = 25,	//!< tests top two items on stack
	OP_PLUS,	//!< adds top two items on stack and replaces with result
	OP_MINUS,	//!< subs top two items on stack and replaces with result
	OP_LOR,		//!< logical or of top two items on stack and replaces with result
	OP_MULT,	//!< multiplies top two items on stack and replaces with result
	OP_DIV = 30,		//!< divides top two items on stack and replaces with result
	OP_MOD,		//!< divides top two items on stack and replaces with modulus
	OP_AND,		//!< bitwise ands top two items on stack and replaces with result
	OP_OR,		//!< bitwise ors top two items on stack and replaces with result
	OP_EOR,		//!< bitwise exclusive ors top two items on stack and replaces with result
	OP_LAND = 35,	//!< logical ands top two items on stack and replaces with result
	OP_NOT,		//!< logical nots top item on stack
	OP_COMP,	//!< complements top item on stack
	OP_NEG,		//!< negates top item on stack
	OP_DUP,		//!< duplicates top item on stack
	OP_ESCON = 40,	//!< start of escapable sequence
	OP_ESCOFF = 41,	//!< end of escapable sequence
	OP_CIMM,	//!< loads signed immediate onto stack (special to case statements)
	OP_CDFILM	//!< loads film offset onto stack but not in current scene
};

// modifiers for the above opcodes
#define	OPSIZE8		0x40	//!< when this bit is set - the operand size is 8 bits
#define	OPSIZE16	0x80	//!< when this bit is set - the operand size is 16 bits

#define	OPMASK		0x3F	//!< mask to isolate the opcode

bool bNoPause = false;

//----------------- LOCAL GLOBAL DATA --------------------

static int32 *pGlobals = 0;		// global vars

static int numGlobals = 0;		// How many global variables to save/restore

static INT_CONTEXT *icList = 0;

static uint32 hMasterScript;

//----------------- SCRIPT BUGS WORKAROUNDS --------------

const byte fragment1[] = {OP_ZERO, OP_GSTORE | OPSIZE16, 206, 0};
const int fragment1_size = 4;
const byte fragment2[] = {OP_LIBCALL | OPSIZE8, 110};
const int fragment2_size = 2;
const byte fragment3[] = {OP_ZERO, OP_GSTORE | OPSIZE16, 490 % 256, 490 / 256};
const int fragment3_size = 4;
const byte fragment4[] = {OP_IMM | OPSIZE16, 900 % 256, 900 / 256, OP_JUMP | OPSIZE16, 466 % 256, 466 / 256};
const int fragment4_size = 6;
const byte fragment5[] = {OP_IMM | OPSIZE16, 901 % 256, 901 / 256, OP_JUMP | OPSIZE16, 488 % 256, 488 / 256};
const int fragment5_size = 6;
const byte fragment6[] = {OP_IMM | OPSIZE16, 903 % 256, 903 / 256, OP_JUMP | OPSIZE16, 516 % 256, 516 / 256};
const int fragment6_size = 6;
const byte fragment7[] = {OP_IMM | OPSIZE16, 908 % 256, 908 / 256, OP_JUMP | OPSIZE16, 616 % 256, 616 / 256};
const int fragment7_size = 6;
const byte fragment8[] = {OP_IMM | OPSIZE16, 910 % 256, 910 / 256, OP_JUMP | OPSIZE16, 644 % 256, 644 / 256};
const int fragment8_size = 6;
const byte fragment9[] = {OP_JUMP | OPSIZE8, 123};
const int fragment9_size = 2;
const byte fragment10[] = {OP_IMM | OPSIZE16, 160 % 256, 160 / 256, OP_JUMP | OPSIZE16, 136 % 256, 136 / 256};
const int fragment10_size = 6;

const WorkaroundEntry workaroundList[] = {
	// DW1-SCN: Global 206 is whether Rincewind is trying to take the book back to the present.
	// In the GRA version, it was global 373, and was reset when he is returned to the past, but 
	// was forgotten in the SCN version, so this ensures the flag is properly reset
	{TINSEL_V1, true, 427942095, 1, fragment1_size, fragment1},

	// DW1-GRA: Rincewind exiting the Inn is blocked by the luggage. Whilst you can then move
	// into walkable areas, saving and restoring the game, it will error if you try to move. 
	// This fragment turns off NPC blocking for the Outside Inn rooms so that the luggage won't block
	// Past Outside Inn
	{TINSEL_V1, false, 444622076, 0,  fragment2_size, fragment2},
	// Present Outside Inn
	{TINSEL_V1, false, 352600876, 0,  fragment2_size, fragment2},

	// DW1-GRA: Talking to palace guards in Act 2 gives !!!HIGH STRING||| - this happens if you initiate dialog with
	// one of the guards, but not the other. So these fragments provide the correct talk parameters where needed
	{TINSEL_V1, false, 310506872, 463, fragment4_size, fragment4},
	{TINSEL_V1, false, 310506872, 485, fragment5_size, fragment5},
	{TINSEL_V1, false, 310506872, 513, fragment6_size, fragment6},
	{TINSEL_V1, false, 310506872, 613, fragment7_size, fragment7},
	{TINSEL_V1, false, 310506872, 641, fragment8_size, fragment8},

	// DW1-SCN: The script for the lovable street-Starfish does a 'StopSample' after flicking the coin to ensure it's
	// sound is stopped, but which also accidentally can stop any active conversation with the Amazon
	{TINSEL_V1, true, 394640351, 121, fragment9_size, fragment9},

	// DW2: In the garden, global #490 is set when the bees begin their 'out of hive' animation, and reset when done.
	// But if the game is saved/restored during it, the animation sequence is reset without the global being cleared.
	// This causes bugs in several actions which try to disable the bees animation, since they wait indefinitely for
	// the global to be cleared, incorrectly believing the animation is currently playing. This includes
	// * Giving the brochure to the beekeeper
	// * Stealing the mallets from the wizards
	// This fix ensures that the global is reset when the Garden scene is loaded (both entering and restoring a game)
	{TINSEL_V2, true, 2888147476U, 0, fragment3_size, fragment3},

	// DW1-GRA: Corrects text being drawn partially off-screen during the blackboard description of the Librarian
	{TINSEL_V1,false, 293831402, 133, fragment10_size, fragment10},

	{TINSEL_V0, false, 0, 0, 0, NULL}
};

//----------------- LOCAL GLOBAL DATA --------------------

/**
 * Keeps the code array pointer up to date.
 */
void LockCode(INT_CONTEXT *ic) {
	if (ic->GSort == GS_MASTER) {
		if (TinselV2)
			// Get the srcipt handle from a specific global chunk
			ic->code = (byte *)LockMem(hMasterScript);
		else
			ic->code = (byte *)FindChunk(MASTER_SCNHANDLE, CHUNK_PCODE);
	} else
		ic->code = (byte *)LockMem(ic->hCode);
}

/**
 * Find a free interpret context and allocate it to the calling process.
 */
static INT_CONTEXT *AllocateInterpretContext(GSORT gsort) {
	INT_CONTEXT *pic;
	int	i;

	for (i = 0, pic = icList; i < NUM_INTERPRET; i++, pic++) {
		if (pic->GSort == GS_NONE) {
			pic->pProc = g_scheduler->getCurrentProcess();
			pic->GSort = gsort;
			return pic;
		}
#ifdef DEBUG
		else {
			if (pic->pProc == g_scheduler->getCurrentProcess())
				error("Found unreleased interpret context");
		}
#endif
	}

	error("Out of interpret contexts");
}

static void FreeWaitCheck(PINT_CONTEXT pic, bool bVoluntary) {
	int i;

	// Is this waiting for something?
	if (pic->waitNumber1) {
		for (i = 0; i < NUM_INTERPRET; i++) {
			if ((icList + i)->waitNumber2 == pic->waitNumber1) {
				(icList + i)->waitNumber2 = 0;
				break;
			}
		}
	}

	// Is someone waiting for this?
	if (pic->waitNumber2) {
		for (i = 0; i < NUM_INTERPRET; i++) {
			if ((icList + i)->waitNumber1 == pic->waitNumber2) {
				(icList + i)->waitNumber1 = 0;
				(icList + i)->resumeCode = bVoluntary ? RES_FINISHED : RES_CUTSHORT;
				g_scheduler->reschedule((icList + i)->pProc);
				break;
			}
		}
		assert(i < NUM_INTERPRET);
	}
}

/**
 * Normal release of an interpret context.
 * Called from the end of Interpret().
 */
static void FreeInterpretContextPi(INT_CONTEXT *pic) {
	FreeWaitCheck(pic, true);
	if (TinselV2)
		memset(pic, 0, sizeof(INT_CONTEXT));
	pic->GSort = GS_NONE;
}

/**
 * Free interpret context owned by a dying process.
 * Ensures that interpret contexts don't get lost when an Interpret()
 * call doesn't complete.
 */
void FreeInterpretContextPr(PROCESS *pProc) {
	INT_CONTEXT *pic;
	int	i;

	for (i = 0, pic = icList; i < NUM_INTERPRET; i++, pic++) {
		if (pic->GSort != GS_NONE && pic->pProc == pProc) {
			FreeWaitCheck(pic, false);
			if (TinselV2)
				memset(pic, 0, sizeof(INT_CONTEXT));
			pic->GSort = GS_NONE;
			break;
		}
	}
}

/**
 * Free all interpret contexts except for the master script's
 */
void FreeMostInterpretContexts(void) {
	INT_CONTEXT *pic;
	int	i;

	for (i = 0, pic = icList; i < NUM_INTERPRET; i++, pic++) {
		if ((pic->GSort != GS_MASTER) && (pic->GSort != GS_GPROCESS)) {
			memset(pic, 0, sizeof(INT_CONTEXT));
			pic->GSort = GS_NONE;
		}
	}
}

/**
 * Free the master script's interpret context.
 */
void FreeMasterInterpretContext(void) {
	INT_CONTEXT *pic;
	int	i;

	for (i = 0, pic = icList; i < NUM_INTERPRET; i++, pic++)	{
		if ((pic->GSort == GS_MASTER) || (pic->GSort == GS_GPROCESS)) {
			memset(pic, 0, sizeof(INT_CONTEXT));
			pic->GSort = GS_NONE;
			return;
		}
	}
}

/**
 * Allocate and initialise an interpret context.
 * Called from a process prior to Interpret().
 * @param gsort			which sort of code
 * @param hCode			Handle to code to execute
 * @param event			Causal event
 * @param hpoly			Associated polygon (if any)
 * @param actorId		Associated actor (if any)
 * @param pinvo			Associated inventory object
 */
INT_CONTEXT *InitInterpretContext(GSORT gsort, SCNHANDLE hCode,	TINSEL_EVENT event,
		HPOLYGON hpoly, int actorid, INV_OBJECT *pinvo, int myEscape) {
	INT_CONTEXT *ic;

	ic = AllocateInterpretContext(gsort);

	// Previously parameters to Interpret()
	ic->hCode = hCode;
	LockCode(ic);
	ic->event = event;
	ic->hPoly = hpoly;
	ic->idActor = actorid;
	ic->pinvo = pinvo;

	// Previously local variables in Interpret()
	ic->bHalt = false;		// set to exit interpeter
	ic->escOn = myEscape > 0;
	ic->myEscape = myEscape;
	ic->sp = 0;
	ic->bp = ic->sp + 1;
	ic->ip = 0;			// start of code

	ic->resumeState = RES_NOT;

	return ic;
}

/**
 * Allocate and initialise an interpret context with restored data.
 */
INT_CONTEXT *RestoreInterpretContext(INT_CONTEXT *ric) {
	INT_CONTEXT *ic;

	ic = AllocateInterpretContext(GS_NONE);	// Sort will soon be overridden

	memcpy(ic, ric, sizeof(INT_CONTEXT));
	ic->pProc = g_scheduler->getCurrentProcess();
	ic->resumeState = RES_1;

	LockCode(ic);

	return ic;
}

/**
 * Allocates enough RAM to hold the global Glitter variables.
 */
void RegisterGlobals(int num) {
	if (pGlobals == NULL) {
		numGlobals = num;

		hMasterScript = !TinselV2 ? 0 :
			READ_LE_UINT32(FindChunk(MASTER_SCNHANDLE, CHUNK_MASTER_SCRIPT));

		// Allocate RAM for pGlobals and make sure it's allocated
		pGlobals = (int32 *)calloc(numGlobals, sizeof(int32));
		if (pGlobals == NULL) {
			error("Cannot allocate memory for global data");
		}

		// Allocate RAM for interpret contexts and make sure it's allocated
		icList = (INT_CONTEXT *)calloc(NUM_INTERPRET, sizeof(INT_CONTEXT));
		if (icList == NULL) {
			error("Cannot allocate memory for interpret contexts");
		}
		g_scheduler->setResourceCallback(FreeInterpretContextPr);
	} else {
		// Check size is still the same
		assert(numGlobals == num);

		memset(pGlobals, 0, numGlobals * sizeof(int32));
		memset(icList, 0, NUM_INTERPRET * sizeof(INT_CONTEXT));
	}

	if (TinselV2) {
		// read initial values
		CdCD(nullContext);

		Common::File f;
		if (!f.open(GLOBALS_FILENAME))
			error(CANNOT_FIND_FILE, GLOBALS_FILENAME);

		int32 length = f.readSint32LE();
		if (length != num)
			error(FILE_IS_CORRUPT, GLOBALS_FILENAME);

		for (int i = 0; i < length; ++i)
			pGlobals[i] = f.readSint32LE();

		if (f.ioFailed())
			error(FILE_IS_CORRUPT, GLOBALS_FILENAME);

		f.close();
	}
}

void FreeGlobals(void) {
	free(pGlobals);
	pGlobals = NULL;

	free(icList);
	icList = NULL;
}

/**
 * (Un)serialize the global data for save/restore game.
 */
void syncGlobInfo(Common::Serializer &s) {
	for (int i = 0; i < numGlobals; i++) {
		s.syncAsSint32LE(pGlobals[i]);
	}
}

/**
 * (Un)serialize an interpreter context for save/restore game.
 */
void INT_CONTEXT::syncWithSerializer(Common::Serializer &s) {
	if (s.isLoading()) {
		// Null out the pointer fields
		pProc = NULL;
		code = NULL;
		pinvo = NULL;
	}
	// Write out used fields
	s.syncAsUint32LE(GSort);
	s.syncAsUint32LE(hCode);
	s.syncAsUint32LE(event);
	s.syncAsSint32LE(hPoly);
	s.syncAsSint32LE(idActor);

	for (int i = 0; i < PCODE_STACK_SIZE; ++i)
		s.syncAsSint32LE(stack[i]);

	s.syncAsSint32LE(sp);
	s.syncAsSint32LE(bp);
	s.syncAsSint32LE(ip);
	s.syncAsUint32LE(bHalt);
	s.syncAsUint32LE(escOn);
	s.syncAsSint32LE(myEscape);
}

/**
 * Return pointer to and size of global data for save/restore game.
 */
void SaveInterpretContexts(INT_CONTEXT *sICInfo) {
	memcpy(sICInfo, icList, NUM_INTERPRET * sizeof(INT_CONTEXT));
}

/**
 * Fetches up to 4 bytes from the code script
 */
static int32 GetBytes(const byte *scriptCode, const WorkaroundEntry* &wkEntry, int &ip, uint numBytes) {
	assert(numBytes <= 4 && numBytes != 3);
	const byte *code = scriptCode;

	if (wkEntry != NULL) {
		if (ip >= wkEntry->numBytes) {
			// Finished the workaround
			ip = wkEntry->ip;
			wkEntry = NULL;
		} else {
			code = wkEntry->script;
		}
	}

	uint32 tmp;
	switch (numBytes) {
	case 0:
		// Instruction byte
		tmp = code[ip++ * (TinselV0 ? 4 : 1)];
		break;
	case 1:
		// Fetch and sign extend a 8 bit value to 32 bits.
		tmp = (int8)code[ip++];
		break;
	case 2:
		// Fetch and sign extend a 16 bit value to 32 bits.
		tmp = (int16)READ_LE_UINT16(code + ip);
		ip += 2;
		break;
	default:
		if (TinselV0)
			tmp = (int32)READ_LE_UINT32(code + ip++ * 4);
		else {
			tmp = (int32)READ_LE_UINT32(code + ip);
			ip += 4;
		}
		break;
	}

	return tmp;
}

/**
 * Fetch (and sign extend, if necessary) a 8/16/32 bit value from the code
 * stream and advance the instruction pointer accordingly.
 */
static int32 Fetch(byte opcode, const byte *code, const WorkaroundEntry* &wkEntry, int &ip) {
	if (TinselV0)
		// Fetch a 32 bit value.
		return GetBytes(code, wkEntry, ip, 4);
	else if (opcode & OPSIZE8)
		// Fetch and sign extend a 8 bit value to 32 bits.
		return GetBytes(code, wkEntry, ip, 1);
	else if (opcode & OPSIZE16)
		return GetBytes(code, wkEntry, ip, 2);
	
	return GetBytes(code, wkEntry, ip, 4);
}

/**
 * Interprets the PCODE instructions in the code array.
 */
void Interpret(CORO_PARAM, INT_CONTEXT *ic) {
	do {
		int tmp, tmp2;
		int ip = ic->ip;
		const WorkaroundEntry *wkEntry = ic->fragmentPtr;

		if (wkEntry == NULL) {
			// Check to see if a workaround fragment needs to be executed
			for (wkEntry = workaroundList; wkEntry->script != NULL; ++wkEntry) {
				if ((wkEntry->version == TinselVersion) && 
					(wkEntry->hCode == ic->hCode) &&
					(wkEntry->ip == ip) &&
					(!TinselV1 || (wkEntry->scnFlag == ((_vm->getFeatures() & GF_SCNFILES) != 0)))) {
					// Point to start of workaround fragment
					ip = 0;
					break;
				}
			}
			if (wkEntry->script == NULL)
				wkEntry = NULL;
		}

		byte opcode = (byte)GetBytes(ic->code, wkEntry, ip, 0);
		if (TinselV0 && ((opcode & OPMASK) > OP_IMM))
			opcode += 3;

		debug(7, "ip=%d  Opcode %d (-> %d)", ic->ip, opcode, opcode & OPMASK);
		switch (opcode & OPMASK) {
		case OP_HALT:			// end of program

			ic->bHalt = true;
			break;

		case OP_IMM:			// loads immediate data onto stack
		case OP_STR:			// loads string handle onto stack
		case OP_FILM:			// loads film handle onto stack
		case OP_CDFILM:			// loads film handle onto stack
		case OP_FONT:			// loads font handle onto stack
		case OP_PAL:			// loads palette handle onto stack

			ic->stack[++ic->sp] = Fetch(opcode, ic->code, wkEntry, ip);
			break;

		case OP_ZERO:			// loads zero onto stack
			ic->stack[++ic->sp] = 0;
			break;

		case OP_ONE:			// loads one onto stack
			ic->stack[++ic->sp] = 1;
			break;

		case OP_MINUSONE:		// loads minus one onto stack
			ic->stack[++ic->sp] = -1;
			break;

		case OP_LOAD:			// loads local variable onto stack

			ic->stack[++ic->sp] = ic->stack[ic->bp + Fetch(opcode, ic->code, wkEntry, ip)];
			break;

		case OP_GLOAD:				// loads global variable onto stack

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			assert(0 <= tmp && tmp < numGlobals);
			ic->stack[++ic->sp] = pGlobals[tmp];
			break;

		case OP_STORE:				// pops stack and stores in local variable

			ic->stack[ic->bp + Fetch(opcode, ic->code, wkEntry, ip)] = ic->stack[ic->sp--];
			break;

		case OP_GSTORE:				// pops stack and stores in global variable

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			assert(0 <= tmp && tmp < numGlobals);
			pGlobals[tmp] = ic->stack[ic->sp--];
			break;

		case OP_CALL:				// procedure call

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			//assert(0 <= tmp && tmp < codeSize);	// TODO: Verify jumps are not out of bounds
			ic->stack[ic->sp + 1] = 0;	// static link
			ic->stack[ic->sp + 2] = ic->bp;	// dynamic link
			ic->stack[ic->sp + 3] = ip;	// return address
			ic->bp = ic->sp + 1;		// set new base pointer
			ip = tmp;	// set ip to procedure address
			break;

		case OP_LIBCALL:		// library procedure or function call

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			// NOTE: Interpret() itself is not using the coroutine facilities,
			// but still accepts a CORO_PARAM, so from the outside it looks
			// like a coroutine. In fact it may still acts as a kind of "proxy"
			// for some underlying coroutine. To enable this, we just pass on
			// 'coroParam' to CallLibraryRoutine(). If we then detect that
			// coroParam was set to a non-zero value, this means that some
			// coroutine code did run at some point, and we are now supposed
			// to sleep or die -- hence, we 'return' if coroParam != 0.
			//
			// This works because Interpret() is fully re-entrant: If we return
			// now and are later called again, then we will end up in the very
			// same spot (i.e. here).
			//
			// The reasons we do it this way, instead of turning Interpret into
			// a 'proper' coroutine are (1) we avoid implementation problems
			// (CORO_INVOKE involves adding 'case' statements, but Interpret
			// already has a huge switch/case, so that would not work out of the
			// box), (2) we incurr less overhead, (3) it's easier to debug,
			// (4) it's simply cool ;).
			tmp2 = CallLibraryRoutine(coroParam, tmp, &ic->stack[ic->sp], ic, &ic->resumeState);
			if (coroParam)
				return;
			if (!TinselV0)
				ic->sp += tmp2;
			LockCode(ic);
			if (TinselV2 && (ic->resumeState == RES_1))
				ic->resumeState = RES_NOT;
			break;

		case OP_RET:			// procedure return

			ic->sp = ic->bp - 1;		// restore stack
			ip = ic->stack[ic->sp + 3];	// return address
			ic->bp = ic->stack[ic->sp + 2];	// restore previous base pointer
			break;

		case OP_ALLOC:			// allocate storage on stack

			ic->sp += (int32)Fetch(opcode, ic->code, wkEntry, ip);
			break;

		case OP_JUMP:	// unconditional jump

			ip = Fetch(opcode, ic->code, wkEntry, ip);
			wkEntry = NULL;					// In case a jump occurs from a workaround
			break;

		case OP_JMPFALSE:	// conditional jump

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			if (ic->stack[ic->sp--] == 0) {
				// condition satisfied - do the jump
				ip = tmp;
				wkEntry = NULL;					// In case a jump occurs from a workaround
			}
			break;

		case OP_JMPTRUE:	// conditional jump

			tmp = Fetch(opcode, ic->code, wkEntry, ip);
			if (ic->stack[ic->sp--] != 0) {
				// condition satisfied - do the jump
				ip = tmp;
				wkEntry = NULL;					// In case a jump occurs from a workaround
			}
			break;

		case OP_EQUAL:			// tests top two items on stack for equality
		case OP_LESS:			// tests top two items on stack
		case OP_LEQUAL:			// tests top two items on stack
		case OP_NEQUAL:			// tests top two items on stack
		case OP_GEQUAL:			// tests top two items on stack
		case OP_GREAT:			// tests top two items on stack
		case OP_LOR:			// logical or of top two items on stack and replaces with result
		case OP_LAND:			// logical ands top two items on stack and replaces with result

			// pop one operand
			ic->sp--;
			assert(ic->sp >= 0);
			tmp = ic->stack[ic->sp];
			tmp2 = ic->stack[ic->sp + 1];

			// replace other operand with result of operation
			switch (opcode) {
			case OP_EQUAL:  tmp = (tmp == tmp2); break;
			case OP_LESS:   tmp = (tmp <  tmp2); break;
			case OP_LEQUAL: tmp = (tmp <= tmp2); break;
			case OP_NEQUAL: tmp = (tmp != tmp2); break;
			case OP_GEQUAL: tmp = (tmp >= tmp2); break;
			case OP_GREAT:  tmp = (tmp >  tmp2); break;

			case OP_LOR:    tmp = (tmp || tmp2); break;
			case OP_LAND:   tmp = (tmp && tmp2); break;
			}

			ic->stack[ic->sp] = tmp;
			break;

		case OP_PLUS:			// adds top two items on stack and replaces with result
		case OP_MINUS:			// subs top two items on stack and replaces with result
		case OP_MULT:			// multiplies top two items on stack and replaces with result
		case OP_DIV:			// divides top two items on stack and replaces with result
		case OP_MOD:			// divides top two items on stack and replaces with modulus
		case OP_AND:			// bitwise ands top two items on stack and replaces with result
		case OP_OR:				// bitwise ors top two items on stack and replaces with result
		case OP_EOR:			// bitwise exclusive ors top two items on stack and replaces with result

			// pop one operand
			ic->sp--;
			assert(ic->sp >= 0);
			tmp = ic->stack[ic->sp];
			tmp2 = ic->stack[ic->sp + 1];

			// replace other operand with result of operation
			switch (opcode) {
			case OP_PLUS:   tmp += tmp2; break;
			case OP_MINUS:  tmp -= tmp2; break;
			case OP_MULT:   tmp *= tmp2; break;
			case OP_DIV:    tmp /= tmp2; break;
			case OP_MOD:    tmp %= tmp2; break;
			case OP_AND:    tmp &= tmp2; break;
			case OP_OR:     tmp |= tmp2; break;
			case OP_EOR:    tmp ^= tmp2; break;
			}
			ic->stack[ic->sp] = tmp;
			break;

		case OP_NOT:			// logical nots top item on stack

			ic->stack[ic->sp] = !ic->stack[ic->sp];
			break;

		case OP_COMP:			// complements top item on stack
			ic->stack[ic->sp] = ~ic->stack[ic->sp];
			break;

		case OP_NEG:			// negates top item on stack
			ic->stack[ic->sp] = -ic->stack[ic->sp];
			break;

		case OP_DUP:			// duplicates top item on stack
			ic->stack[ic->sp + 1] = ic->stack[ic->sp];
			ic->sp++;
			break;

		case OP_ESCON:
			bNoPause = true;
			ic->escOn = true;
			ic->myEscape = GetEscEvents();
			break;

		case OP_ESCOFF:
			ic->escOn = false;
			ic->myEscape = 0;
			break;

		default:
			error("Interpret() - Unknown opcode");
		}

		// check for stack under-overflow
		assert(ic->sp >= 0 && ic->sp < PCODE_STACK_SIZE);
		ic->ip = ip;
		ic->fragmentPtr = wkEntry;
	} while (!ic->bHalt);

	// make sure stack is unwound
	assert(ic->sp == 0);

	FreeInterpretContextPi(ic);
}

/**
 * Associates an interpret context with the
 * process that will run it.
 */
void AttachInterpret(INT_CONTEXT *pic, PROCESS *pProc) {
	// Attach the process which is using this context
	pic->pProc = pProc;
}

/**
 * Generate a number that isn't being used.
 */
static uint32 UniqueWaitNumber(void) {
	uint32 retval;
	int i;

	for (retval = DwGetCurrentTime(); 1; retval--) {
		if (retval == 0)
			retval = (uint32)-1;

		for (i = 0; i < NUM_INTERPRET; i++) {
			if ((icList+i)->waitNumber1 == retval
			 || (icList+i)->waitNumber2 == retval)
				break;
		}

		if (i == NUM_INTERPRET)
			return retval;
	}
}

/**
 * WaitInterpret
 */
void WaitInterpret(CORO_PARAM, PPROCESS pWaitProc, bool *result) {
	int i;
	PPROCESS currentProcess = g_scheduler->getCurrentProcess();
	assert(currentProcess);
	assert(currentProcess != pWaitProc);
	if (result) *result = false;

	/*
	 * Calling process is the waiter, find its interpret context.
	 */

	CORO_BEGIN_CONTEXT;
		PINT_CONTEXT picWaiter, picWaitee;
	CORO_END_CONTEXT(_ctx);


	CORO_BEGIN_CODE(_ctx);

	for (i = 0, _ctx->picWaiter = icList; i < NUM_INTERPRET; i++, _ctx->picWaiter++) {
		if (_ctx->picWaiter->GSort != GS_NONE && _ctx->picWaiter->pProc == currentProcess) {
			break;
		}
	}

	/*
	 * Find the interpret context of the process we're waiting for
	 */
	for (i = 0, _ctx->picWaitee = icList; i < NUM_INTERPRET; i++, _ctx->picWaitee++) {
		if (_ctx->picWaitee->GSort != GS_NONE && _ctx->picWaitee->pProc == pWaitProc) {
			break;
		}
	}

	/*
	 * Set the first as waiting for the second
	 */
	assert(_ctx->picWaitee->waitNumber2 == 0);
	_ctx->picWaiter->waitNumber1 = _ctx->picWaitee->waitNumber2 = UniqueWaitNumber();
	_ctx->picWaiter->resumeCode = RES_WAITING;

	/*
	 * Wait for it
	 */
	CORO_GIVE_WAY;
	while (_ctx->picWaiter->resumeCode == RES_WAITING) {
		CORO_SLEEP(1);
	}

	if (result)
		*result = (_ctx->picWaiter->resumeCode == RES_FINISHED);
	CORO_END_CODE;
}

/**
 * CheckOutWaiters
 */
void CheckOutWaiters(void) {
	int i, j;

	// Check all waited for have someone waiting
	for (i = 0; i < NUM_INTERPRET; i++)	{
		// If someone is supposedly waiting for this one
		if ((icList + i)->GSort != GS_NONE && (icList + i)->waitNumber2) {
			// Someone really must be waiting for this one
			for (j = 0; j < NUM_INTERPRET; j++) {
				if ((icList + j)->GSort != GS_NONE
				 && (icList + j)->waitNumber1 == (icList + i)->waitNumber2) {
					break;
				}
			}
			assert(j < NUM_INTERPRET);
		}
	}

	// Check waiting for someone to wait for
	for (i = 0; i < NUM_INTERPRET; i++) {
		// If someone is supposedly waiting for this one
		if ((icList + i)->GSort != GS_NONE && (icList + i)->waitNumber1) {
			// Someone really must be waiting for this one
			for (j = 0; j < NUM_INTERPRET; j++) {
				if ((icList + j)->GSort != GS_NONE
				 && (icList + j)->waitNumber2 == (icList + i)->waitNumber1) {
					break;
				}
			}
			assert(j < NUM_INTERPRET);
		}
	}
}

} // End of namespace Tinsel
