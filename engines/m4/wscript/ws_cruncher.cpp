/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/wscript/ws_cruncher.h"
#include "m4/wscript/wscript.h"
#include "m4/wscript/wst_regs.h"
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/dbg/debug.h"
#include "m4/mem/mem.h"
#include "m4/vars.h"

namespace M4 {

#define VERIFY_INTIALIZED(s) if (!_GWS(cruncherInitialized)) error_show(FL, 'WSCI', "%s failed.", s);

static int32 dataFormats[] = { 0, 5, 8, 12, 16 };

static const frac16 sinCosTable[320] = {
	(frac16)0,		(frac16)1608,	(frac16)3215,	(frac16)4821,
	(frac16)6423,	(frac16)8022,	(frac16)9616,	(frac16)11204,
	(frac16)12785,	(frac16)14359,	(frac16)15923,	(frac16)17479,
	(frac16)19024,	(frac16)20557,	(frac16)22078,	(frac16)23586,
	(frac16)25079,	(frac16)26557,	(frac16)28020,	(frac16)29465,
	(frac16)30893,	(frac16)32302,	(frac16)33692,	(frac16)35061,
	(frac16)36409,	(frac16)37736,	(frac16)39039,	(frac16)40319,
	(frac16)41575,	(frac16)42806,	(frac16)44011,	(frac16)45189,
	(frac16)46340,	(frac16)47464,	(frac16)48558,	(frac16)49624,
	(frac16)50660,	(frac16)51665,	(frac16)52639,	(frac16)53581,
	(frac16)54491,	(frac16)55368,	(frac16)56212,	(frac16)57022,
	(frac16)57797,	(frac16)58538,	(frac16)59243,	(frac16)59913,
	(frac16)60547,	(frac16)61144,	(frac16)61705,	(frac16)62228,
	(frac16)62714,	(frac16)63162,	(frac16)63571,	(frac16)63943,
	(frac16)64276,	(frac16)64571,	(frac16)64826,	(frac16)65043,
	(frac16)65220,	(frac16)65358,	(frac16)65457,	(frac16)65516,
	(frac16)65536,	(frac16)65516,	(frac16)65457,	(frac16)65358,
	(frac16)65220,	(frac16)65043,	(frac16)64826,	(frac16)64571,
	(frac16)64276,	(frac16)63943,	(frac16)63571,	(frac16)63162,
	(frac16)62714,	(frac16)62228,	(frac16)61705,	(frac16)61144,
	(frac16)60547,	(frac16)59913,	(frac16)59243,	(frac16)58538,
	(frac16)57797,	(frac16)57022,	(frac16)56212,	(frac16)55368,
	(frac16)54491,	(frac16)53581,	(frac16)52639,	(frac16)51665,
	(frac16)50660,	(frac16)49624,	(frac16)48558,	(frac16)47464,
	(frac16)46340,	(frac16)45189,	(frac16)44011,	(frac16)42806,
	(frac16)41575,	(frac16)40319,	(frac16)39039,	(frac16)37736,
	(frac16)36409,	(frac16)35061,	(frac16)33692,	(frac16)32302,
	(frac16)30893,	(frac16)29465,	(frac16)28020,	(frac16)26557,
	(frac16)25079,	(frac16)23586,	(frac16)22078,	(frac16)20557,
	(frac16)19024,	(frac16)17479,	(frac16)15923,	(frac16)14359,
	(frac16)12785,	(frac16)11204,	(frac16)9616,	(frac16)8022,
	(frac16)6423,	(frac16)4821,	(frac16)3215,	(frac16)1608,
	(frac16)0,		(frac16)-1608,	(frac16)-3215,	(frac16)-4821,
	(frac16)-6423,	(frac16)-8022,	(frac16)-9616,	(frac16)-11204,
	(frac16)-12785,	(frac16)-14359,	(frac16)-15923,	(frac16)-17479,
	(frac16)-19024,	(frac16)-20557,	(frac16)-22078,	(frac16)-23586,
	(frac16)-25079,	(frac16)-26557,	(frac16)-28020,	(frac16)-29465,
	(frac16)-30893,	(frac16)-32302,	(frac16)-33692,	(frac16)-35061,
	(frac16)-36409,	(frac16)-37736,	(frac16)-39039,	(frac16)-40319,
	(frac16)-41575,	(frac16)-42806,	(frac16)-44011,	(frac16)-45189,
	(frac16)-46340,	(frac16)-47464,	(frac16)-48558,	(frac16)-49624,
	(frac16)-50660,	(frac16)-51665,	(frac16)-52639,	(frac16)-53581,
	(frac16)-54491,	(frac16)-55368,	(frac16)-56212,	(frac16)-57022,
	(frac16)-57797,	(frac16)-58538,	(frac16)-59243,	(frac16)-59913,
	(frac16)-60547,	(frac16)-61144,	(frac16)-61705,	(frac16)-62228,
	(frac16)-62714,	(frac16)-63162,	(frac16)-63571,	(frac16)-63943,
	(frac16)-64276,	(frac16)-64571,	(frac16)-64826,	(frac16)-65043,
	(frac16)-65220,	(frac16)-65358,	(frac16)-65457,	(frac16)-65516,
	(frac16)-65536,	(frac16)-65516,	(frac16)-65457,	(frac16)-65358,
	(frac16)-65220,	(frac16)-65043,	(frac16)-64826,	(frac16)-64571,
	(frac16)-64276,	(frac16)-63943,	(frac16)-63571,	(frac16)-63162,
	(frac16)-62714,	(frac16)-62228,	(frac16)-61705,	(frac16)-61144,
	(frac16)-60547,	(frac16)-59913,	(frac16)-59243,	(frac16)-58538,
	(frac16)-57797,	(frac16)-57022,	(frac16)-56212,	(frac16)-55368,
	(frac16)-54491,	(frac16)-53581,	(frac16)-52639,	(frac16)-51665,
	(frac16)-50660,	(frac16)-49624,	(frac16)-48558,	(frac16)-47464,
	(frac16)-46340,	(frac16)-45189,	(frac16)-44011,	(frac16)-42806,
	(frac16)-41575,	(frac16)-40319,	(frac16)-39039,	(frac16)-37736,
	(frac16)-36409,	(frac16)-35061,	(frac16)-33692,	(frac16)-32302,
	(frac16)-30893,	(frac16)-29465,	(frac16)-28020,	(frac16)-26557,
	(frac16)-25079,	(frac16)-23586,	(frac16)-22078,	(frac16)-20557,
	(frac16)-19024,	(frac16)-17479,	(frac16)-15923,	(frac16)-14359,
	(frac16)-12785,	(frac16)-11204,	(frac16)-9616,	(frac16)-8022,
	(frac16)-6423,	(frac16)-4821,	(frac16)-3215,	(frac16)-1608,
	(frac16)0,		(frac16)1608,	(frac16)3215,	(frac16)4821,
	(frac16)6423,	(frac16)8022,	(frac16)9616,	(frac16)11204,
	(frac16)12785,	(frac16)14359,	(frac16)15923,	(frac16)17479,
	(frac16)19024,	(frac16)20557,	(frac16)22078,	(frac16)23586,
	(frac16)25079,	(frac16)26557,	(frac16)28020,	(frac16)29465,
	(frac16)30893,	(frac16)32302,	(frac16)33692,	(frac16)35061,
	(frac16)36409,	(frac16)37736,	(frac16)39039,	(frac16)40319,
	(frac16)41575,	(frac16)42806,	(frac16)44011,	(frac16)45189,
	(frac16)46340,	(frac16)47464,	(frac16)48558,	(frac16)49624,
	(frac16)50660,	(frac16)51665,	(frac16)52639,	(frac16)53581,
	(frac16)54491,	(frac16)55368,	(frac16)56212,	(frac16)57022,
	(frac16)57797,	(frac16)58538,	(frac16)59243,	(frac16)59913,
	(frac16)60547,	(frac16)61144,	(frac16)61705,	(frac16)62228,
	(frac16)62714,	(frac16)63162,	(frac16)63571,	(frac16)63943,
	(frac16)64276,	(frac16)64571,	(frac16)64826,	(frac16)65043,
	(frac16)65220,	(frac16)65358,	(frac16)65457,	(frac16)65516
};

static const frac16 *sinTable = &(sinCosTable[0]);
static const frac16 *cosTable = &(sinCosTable[64]);

int32 *ws_GetDataFormats() {
	return &dataFormats[0];
}

bool ws_InitCruncher(void) {
	// Make sure the cruncher has not been initialized
	if (_GWS(cruncherInitialized))
		error_show(FL, 'WSCR');

	// Register the end of sequence struct with the stash manager
	mem_register_stash_type(&_GWS(memtypeEOS), sizeof(EOSreq), 32, "+EOS");
	if (_GWS(memtypeEOS) < 0)
		error_show(FL, 'WSCE');

	if ((_GWS(myCruncher) = (cruncher *)mem_alloc(sizeof(cruncher), "cruncher")) == nullptr)
		error_show(FL, 'OOM!', "%d bytes.", sizeof(cruncher));

	_GWS(myCruncher)->backLayerAnim8 = nullptr;
	_GWS(myCruncher)->frontLayerAnim8 = nullptr;
	_GWS(myCruncher)->firstAnim8ToCrunch = nullptr;
	_GWS(myCruncher)->lastAnim8ToCrunch = nullptr;

	// Set up stack
	_GWS(stackSize) = 2048;
	if ((_GWS(stackBase) = (uint32 *)mem_alloc(_GWS(stackSize), "crunchstack")) == nullptr) {
		error_show(FL, 'OOM!', "%d bytes.", _GWS(stackSize));
	}
	_GWS(stackTop) = _GWS(stackBase);
	_GWS(stackLimit) = (uint32 *)((byte *)_GWS(stackBase) + (uint32)_GWS(stackSize));

	_GWS(cruncherInitialized) = true;

	return true;
}

void ws_KillCruncher() {
	Anim8 *myAnim8;

	if (_GWS(cruncherInitialized)) {
		myAnim8 = _GWS(myCruncher)->firstAnim8ToCrunch;
		while (myAnim8) {
			_GWS(myCruncher)->firstAnim8ToCrunch = myAnim8->next;
			if (myAnim8->myCCB) {
				KillCCB(myAnim8->myCCB, false);
			}
			mem_free(myAnim8->myRegs);
			myAnim8 = _GWS(myCruncher)->firstAnim8ToCrunch;
		}
		mem_free(_GWS(myCruncher));
		if (_GWS(stackBase)) {
			mem_free(_GWS(stackBase));
		}

		_GWS(cruncherInitialized) = false;
	}
}

Anim8 *ws_AddAnim8ToCruncher(machine *m, int32 sequHash) {
	Anim8 *myAnim8;
	frac16 *myRegs;
	int32 numLocalVars;
	int32 i;

	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_AddAnim8ToCruncher()");

	// Allocate an anim8 structure
	if ((myAnim8 = (Anim8 *)mem_alloc(sizeof(Anim8), "Anim8")) == nullptr) {
		ws_LogErrorMsg(FL, "Out of memory - mem requested: %d.", sizeof(Anim8));
		return nullptr;
	}

	// Find the sequence
	if ((myAnim8->sequHandle = ws_GetSEQU((uint32)sequHash, &numLocalVars, &myAnim8->pcOffset)) == nullptr) {
		return nullptr;
	}

	// Allocate an array of registers
	if ((myRegs = (frac16 *)mem_alloc(sizeof(frac16) * (IDX_COUNT + numLocalVars), "Anim8 regs")) == nullptr) {
		ws_LogErrorMsg(FL, "Out of memory - mem requested: %d.", sizeof(frac16) * (IDX_COUNT + numLocalVars));
		return nullptr;
	}

	// Initialize the Anim8 structure
	myAnim8->active = true;
	myAnim8->sequHash = sequHash;
	myAnim8->myMachine = m;								// Pointer back to myMachine
	myAnim8->eosReqOffset = -1;
	myAnim8->myParent = m->parentAnim8;					// The parent anim8
	myAnim8->myCCB = nullptr;
	myAnim8->dataHash = m->dataHash;					// The array of data
	myAnim8->dataHandle = m->dataHandle;
	myAnim8->dataOffset = m->dataOffset;
	myAnim8->startTime = 0;
	myAnim8->switchTime = 0;
	myAnim8->flags = 0;
	myAnim8->numLocalVars = numLocalVars;
	myAnim8->myRegs = myRegs;
	myAnim8->returnStackIndex = 0;

	// Link it into the execution list
	myAnim8->next = nullptr;
	myAnim8->prev = _GWS(myCruncher)->lastAnim8ToCrunch;
	if (_GWS(myCruncher)->lastAnim8ToCrunch) {
		_GWS(myCruncher)->lastAnim8ToCrunch->next = myAnim8;
	} else {
		_GWS(myCruncher)->firstAnim8ToCrunch = myAnim8;
	}
	_GWS(myCruncher)->lastAnim8ToCrunch = myAnim8;

	// Now link it into the layering list
	myAnim8->myLayer = 0;
	myAnim8->infront = nullptr;
	myAnim8->behind = _GWS(myCruncher)->frontLayerAnim8;
	if (_GWS(myCruncher)->frontLayerAnim8) {
		_GWS(myCruncher)->frontLayerAnim8->infront = myAnim8;
	} else {
		_GWS(myCruncher)->backLayerAnim8 = myAnim8;
	}
	_GWS(myCruncher)->frontLayerAnim8 = myAnim8;

	// Now clear the registers, but set the scale = 100%
	for (i = 0; i < IDX_COUNT + numLocalVars; i++) {
		myAnim8->myRegs[i] = 0;
	}
	myAnim8->myRegs[IDX_S] = 0x10000;
	myAnim8->myRegs[IDX_MACH_ID] = m->machID;

	return myAnim8;
}

bool ws_ChangeAnim8Program(machine *m, int32 newSequHash) {
	Anim8 *myAnim8;
	int32 numLocalVars;

	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_ChangeAnim8Program()");

	// Parameter verification
	if ((!m) || (!m->myAnim8)) {
		error_show(FL, 'WSMI');
	}

	myAnim8 = m->myAnim8;

	// Find the sequence
	if ((myAnim8->sequHandle = ws_GetSEQU((uint32)newSequHash, &numLocalVars, &myAnim8->pcOffset)) == nullptr) {
		return false;
	}

	// Now see if we've started a sequence requiring more vars than the prev
	if (myAnim8->numLocalVars < numLocalVars) {
		ws_LogErrorMsg(FL, "Can't launch a sequence with more local vars than the previous sequence.");
		return false;
	}

	// Intialize the Anim8
	myAnim8->switchTime = 0;
	myAnim8->active = 1;
	myAnim8->eosReqOffset = -1;
	myAnim8->sequHash = newSequHash;
	myAnim8->returnStackIndex = 0;

	return true;
}

void ws_RemoveAnim8FromCruncher(Anim8 *myAnim8) {
	EOSreq *tempEOSreq;

	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_RemoveAnim8FromCruncher()");

	if (!myAnim8)
		error_show(FL, 'WSAI');

	// In case we are crunching the current list of EOS requests, remove any for this machine
	tempEOSreq = _GWS(EOSreqList);
	while (tempEOSreq && (tempEOSreq->myAnim8 != myAnim8)) {
		tempEOSreq = tempEOSreq->next;
	}

	if (tempEOSreq) {
		if (tempEOSreq->next) {
			tempEOSreq->next->prev = tempEOSreq->prev;
		}
		if (tempEOSreq->prev) {
			tempEOSreq->prev->next = tempEOSreq->next;
		} else {
			_GWS(EOSreqList) = tempEOSreq->next;
		}

		mem_free_to_stash((void *)tempEOSreq, _GWS(memtypeEOS));
	}

	// Incase we are in the middle of crunching
	if (myAnim8 == _GWS(crunchNext)) {
		_GWS(crunchNext) = myAnim8->next;
	}

	// Remove myAnim8 from the crunch list
	if (myAnim8->prev) {
		myAnim8->prev->next = myAnim8->next;
	} else {
		_GWS(myCruncher)->firstAnim8ToCrunch = myAnim8->next;
	}

	if (myAnim8->next) {
		myAnim8->next->prev = myAnim8->prev;
	} else {
		_GWS(myCruncher)->lastAnim8ToCrunch = myAnim8->prev;
	}

	// Now remove it from the anim8 layer
	if (myAnim8->infront) {
		myAnim8->infront->behind = myAnim8->behind;
	} else {
		_GWS(myCruncher)->frontLayerAnim8 = myAnim8->behind;
	}
	if (myAnim8->behind) {
		myAnim8->behind->infront = myAnim8->infront;
	} else {
		_GWS(myCruncher)->backLayerAnim8 = myAnim8->infront;
	}

	// Clean up and free the CCB for this anim8
	if (myAnim8->myCCB) {
		KillCCB(myAnim8->myCCB, true);
	}

	// Deallocate the register list
	mem_free(myAnim8->myRegs);

	// Finally, turf myAnim8
	mem_free(myAnim8);
}


// This procedure flags the anim8 slot as inactive, but still owned by the machine
bool ws_PauseAnim8(Anim8 *myAnim8) {
	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_PauseAnim8()");

	if (!myAnim8)
		error_show(FL, 'WSAI');

	myAnim8->active = false;
	HideCCB(myAnim8->myCCB);
	return true;
}

// This procedure reactivates the anim8 slot owned by the machine
bool ws_ResumeAnim8(Anim8 *myAnim8) {
	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_PauseAnim8()");

	if (!myAnim8)
		error_show(FL, 'WSAI');

	myAnim8->active = true;
	if (myAnim8->myCCB) {
		ShowCCB(myAnim8->myCCB);
		myAnim8->myCCB->flags |= CCB_SKIP;
	}

	return true;
}

static bool ExtractArg(Anim8 *myAnim8, int32 myFormat, int32 myData, frac16 **argPtr, frac16 *argValue) {
	int32 myIndex;
	Anim8 *parentAnim8;
	uint32 *dataArray;
	Common::String prefix;

	// If the format indicates the argument is a local source (parent, register, or data)
	if (myFormat == FMT_LOCAL_SRC) {
		if (!myAnim8) {
			ws_LogErrorMsg(FL, "INTERNAL ERROR - ExtractArg() failed - An invalid Anim8* was passed.");
			return false;
		}

		// Find out if the index has been previously stored in a special index register
		if (myData & REG_SET_IDX_REG) {
			myIndex = _GWS(indexReg);
			prefix = "S";
		} else {
			// Else the index is part of the data segment for this arg
			myIndex = myData & REG_SET_IDX;
		}

		// Find the right register set
		switch (myData & LOCAL_FMT) {
		case LOCAL_FMT_PARENT:
			parentAnim8 = myAnim8->myParent;

			// Range check to make sure we don't index off into hyperspace
			if ((!parentAnim8) || (myIndex >= IDX_COUNT + parentAnim8->numLocalVars)) {
				if (!parentAnim8) {
					ws_LogErrorMsg(FL, "Trying to access a parent register - no parent exists");
				} else {
					ws_LogErrorMsg(FL, "Parent Reg Index out of range - max: %d, requested %d.",
						IDX_COUNT + parentAnim8->numLocalVars, myIndex);
				}
				return false;
			}
			*argPtr = &parentAnim8->myRegs[myIndex];
			prefix += "P";
			dbg_AddRegParamToCurrMachInstr(myIndex, prefix.c_str());
			break;

		case LOCAL_FMT_REG:
			// Range check to make sure we don't index off into hyperspace
			if ((myIndex >= IDX_COUNT + myAnim8->numLocalVars)) {
				ws_LogErrorMsg(FL, "Register Index out of range - max: %d, requested %d.",
					IDX_COUNT + myAnim8->numLocalVars, myIndex);
				return false;
			}
			*argPtr = &myAnim8->myRegs[myIndex];
			dbg_AddRegParamToCurrMachInstr(myIndex, prefix.c_str());
			break;

		case LOCAL_FMT_DATA:
			// Ensure we have a dataHandle
			if ((!myAnim8->dataHandle) || (!*(myAnim8->dataHandle))) {
				ws_LogErrorMsg(FL, "Trying to access a DATA field when no DATA has been set");
				return false;
			}

			// Dereferrence the dataHandle, add the offset to find the array of data for this anim8
			dataArray = (uint32 *)((intptr)*(myAnim8->dataHandle) + myAnim8->dataOffset);

			// Copy the data field into dataArg1, and set myArg1 to point to this location
			*argValue = (int32)FROM_LE_32(dataArray[myIndex]);
			*argPtr = argValue;
			prefix += Common::String::format("DATA %d", myIndex);
			dbg_AddParamToCurrMachInstr(prefix.c_str());
			break;
		}
	} else if (myFormat == FMT_GLOBAL_SRC) {
		// Else if the format indicates the argument is from the ws_globals register set
		// Find out if the index has been previously stored in a special index register
		if (myData & REG_SET_IDX_REG) {
			myIndex = _GWS(indexReg);
			prefix = "S";
		} else {
			// Else the index is part of the data segment for this arg
			myIndex = myData & REG_SET_IDX;
		}

		// Finally, set myArg1 to point to the location in the ws_globals array, whichever index
		*argPtr = &(_GWS(ws_globals)[myIndex]);
		dbg_AddGlobalParamToCurrMachInstr(myIndex, prefix.c_str());
	} else {
		// Else the argument is not a variable, but an actual value

		// The top bit of the data segment is a negative flag, the format determines how far the other
		// 15 bits of the data segment are shifted left, so the value requested is in frac16 format.
		// The value is stored in the frac16 (dataArg1), and...
		if (myData & OP_DATA_SIGN) {
			*argValue = -(myData & OP_DATA_VALUE) << (dataFormats[myFormat - 3]);
		} else {
			*argValue = (myData & OP_DATA_VALUE) << (dataFormats[myFormat - 3]);
		}

		// myArg1 will point to this location
		*argPtr = argValue;
		prefix += Common::String::format("%ld", *argValue);
		dbg_AddParamToCurrMachInstr(prefix.c_str());
	}

	return true;
}

int32 ws_PreProcessPcode(uint32 **PC, Anim8 *myAnim8) {
	int32 myInstruction, myFormat, myData; // myIndex;
	uint32 *myPC, opCode, word2;

	if (!PC) {
		ws_LogErrorMsg(FL, "INTERNAL ERROR - ws_PreProcessPcode() failed - An invalid PC was passed.");
		return -1;
	}

	myPC = *PC;

	// Get the opCode
	opCode = FROM_LE_32(*myPC++);

	// Get the instruction number
	myInstruction = (opCode & OP_INSTR) >> 25;
	dbg_AddOpcodeToMachineInstr(myInstruction);

	// Get the format for the first arg 
	myFormat = (opCode & OP_FORMAT1) >> 22;

	// Get the data for the first arg
	myData = opCode & OP_LOW_DATA;

	// Verify we have an argument
	if (myFormat) {
		if (!ExtractArg(myAnim8, myFormat, myData, &_GWS(myArg1), &_GWS(dataArg1))) {
			return -1;
		}
	} else {
		// Otherwise this argument is called with no args
		_GWS(myArg1) = nullptr;
		_GWS(myArg2) = nullptr;
		_GWS(myArg3) = nullptr;
		*PC = myPC;
		return myInstruction;
	}

	// Check for arg2
	myFormat = (opCode & OP_FORMAT2) >> 19;
	if (myFormat) {
		word2 = FROM_LE_32(*myPC++);
		myData = (word2 & OP_HIGH_DATA) >> 16;
		if (!ExtractArg(myAnim8, myFormat, myData, &_GWS(myArg2), &_GWS(dataArg2))) {
			return -1;
		}
	} else {
		_GWS(myArg2) = nullptr;
		_GWS(myArg3) = nullptr;
		*PC = myPC;
		return myInstruction;
	}

	// Finally check for arg3
	myFormat = (opCode & OP_FORMAT3) >> 16;
	if (myFormat) {
		myData = word2 & OP_LOW_DATA;
		if (!ExtractArg(myAnim8, myFormat, myData, &_GWS(myArg3), &_GWS(dataArg3))) {
			return -1;
		}
	} else {
		_GWS(myArg3) = nullptr;
		*PC = myPC;
		return myInstruction;
	}

	*PC = myPC;
	return myInstruction;
}


static void op_END(Anim8 *myAnim8) {
	_GWS(terminated) = true;
	_GWS(keepProcessing) = false;
}

static void op_CLEAR(Anim8 *myAnim8) {
	int32 i;

	// Now clear the registers, but set the scale = 100%
	for (i = 0; i <= IDX_COUNT + myAnim8->numLocalVars; i++) {
		myAnim8->myRegs[i] = 0;
	}
	myAnim8->myRegs[IDX_S] = 0x10000;
	myAnim8->myRegs[IDX_MACH_ID] = myAnim8->myMachine->machID;
}

static void op_SET(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = arg2  or  arg1 = rand(arg2, arg3)");
	}
	if (_GWS(myArg3)) {
		*_GWS(myArg1) = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		*_GWS(myArg1) = *_GWS(myArg2);
	}
}

static void op_COMPARE(Anim8 *myAnim8) {
	frac16 myArg;
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: cmp arg1, arg2  or  cmp arg1, rand(arg2, arg3) **sets CCR");
	}
	if (_GWS(myArg3)) {
		myArg = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		myArg = *_GWS(myArg2);
	}
	if (*_GWS(myArg1) < myArg) {
		_GWS(compareCCR) = -1;
	} else if (*_GWS(myArg1) > myArg) {
		_GWS(compareCCR) = 1;
	} else {
		_GWS(compareCCR) = 0;
	}
}

static void op_ADD(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 += arg2  or  arg1 += rand(arg2, arg3)");
	}
	if (_GWS(myArg3)) {
		*_GWS(myArg1) += imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		*_GWS(myArg1) += *_GWS(myArg2);
	}
}

static void op_SUB(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 -= arg2  or  arg1 -= rand(arg2, arg3)");
	}
	if (_GWS(myArg3)) {
		*_GWS(myArg1) -= imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		*_GWS(myArg1) -= *_GWS(myArg2);
	}
}

static void op_MULT(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 *= arg2  or  arg1 *= rand(arg2, arg3)");
	}
	if (_GWS(myArg3)) {
		*_GWS(myArg1) = MulSF16(*_GWS(myArg1), imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3)));
	} else {
		*_GWS(myArg1) = MulSF16(*_GWS(myArg1), *_GWS(myArg2));
	}
}

static void op_DIV(Anim8 *myAnim8) {
	frac16	divisor;
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 /= arg2  or  arg1 /= rand(arg2, arg3)");
	}
	if (_GWS(myArg3)) {
		divisor = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		divisor = *_GWS(myArg2);
	}
	if (divisor == 0) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0253, nullptr);
	} else {
		*_GWS(myArg1) = DivSF16(*_GWS(myArg1), divisor);
	}
}

static void op_SIN(Anim8 *myAnim8) {
	int32 tempAngle;

	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = sin(arg2)  or  arg1 = sin(rand(arg2, arg3))");
	}
	if (_GWS(myArg3)) {
		tempAngle = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3)) >> 16;
	} else {
		tempAngle = *_GWS(myArg2) >> 16;
	}
	if (tempAngle < 0) {
		tempAngle = 0x0100 - ((-tempAngle) & 0xff);
	} else {
		tempAngle &= 0xff;
	}

	*_GWS(myArg1) = -(int)cosTable[tempAngle];
}

static void op_COS(Anim8 *myAnim8) {
	int32 tempAngle;

	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = cos(arg2)  or  arg1 = cos(rand(arg2, arg3))");
	}
	if (_GWS(myArg3)) {
		tempAngle = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3)) >> 16;
	} else {
		tempAngle = *_GWS(myArg2) >> 16;
	}
	if (tempAngle < 0) {
		tempAngle = 0x0100 - ((-tempAngle) & 0xff);
	} else {
		tempAngle &= 0xff;
	}

	*_GWS(myArg1) = sinTable[tempAngle];
}

static void op_AND(Anim8 *myAnim8) {
	frac16 myArg;

	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 &= arg2  or  arg1 &= rand(arg2, arg3) **also sets CCR");
	}
	if (_GWS(myArg3)) {
		myArg = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		myArg = *_GWS(myArg2);
	}
	if ((*_GWS(myArg1)) & myArg) {
		_GWS(compareCCR) = 0;
	} else {
		_GWS(compareCCR) = 1;
	}

	*_GWS(myArg1) &= myArg;
}

static void op_OR(Anim8 *myAnim8) {
	frac16	myArg;

	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 |= arg2  or  arg1 |= rand(arg2, arg3) **also sets CCR");
	}
	if (_GWS(myArg3)) {
		myArg = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		myArg = *_GWS(myArg2);
	}
	if ((*_GWS(myArg1)) | myArg) {
		_GWS(compareCCR) = 0;
	} else {
		_GWS(compareCCR) = 1;
	}

	*_GWS(myArg1) |= myArg;
}

static void op_NOT(Anim8 *myAnim8) {
	if (!_GWS(myArg1)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0250, "functionality: arg1 = (arg1 ? 0 : 1) **also sets CCR");
	}
	if (*_GWS(myArg1) == 0) {
		*_GWS(myArg1) = 0x10000;
		_GWS(compareCCR) = 1;
	} else {
		*_GWS(myArg1) = 0;
		_GWS(compareCCR) = 0;
	}
}

static void op_ABS(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = abs(arg2)");
	}
	if (*_GWS(myArg2) < 0) {
		*_GWS(myArg1) = -(int)(*_GWS(myArg2));
	} else {
		*_GWS(myArg1) = *_GWS(myArg2);
	}
}

static void op_MIN(Anim8 *myAnim8) {
	if (!_GWS(myArg3)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0252, "functionality: arg1 = min(arg2, arg3)");
	}
	if (*_GWS(myArg2) < *_GWS(myArg3)) {
		*_GWS(myArg1) = *_GWS(myArg2);
	} else {
		*_GWS(myArg1) = *_GWS(myArg3);
	}
}

static void op_MAX(Anim8 *myAnim8) {
	if (!_GWS(myArg3)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0252, "functionality: arg1 = max(arg2, arg3)");
	}
	if (*_GWS(myArg2) < *_GWS(myArg3)) {
		*_GWS(myArg1) = *_GWS(myArg3);
	} else {
		*_GWS(myArg1) = *_GWS(myArg2);
	}
}

static void op_MOD(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 %= arg2  or  arg1 = arg2%arg3");
	}
	if (_GWS(myArg3)) {
		*_GWS(myArg1) = (*_GWS(myArg1)) % (imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3)));
	} else {
		*_GWS(myArg1) = (*_GWS(myArg1)) % (*_GWS(myArg2));
	}
}

static void op_FLOOR(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = floor(arg2)  or  arg1 = floor(rand(arg2,arg3))");
	}
	if (_GWS(myArg3)) {
		*_GWS(myArg1) = (imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3))) & 0xffff0000;
	} else {
		*_GWS(myArg1) = (*_GWS(myArg2)) & 0xffff0000;
	}
}

static void op_ROUND(Anim8 *myAnim8) {
	frac16 myArg;
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = round(arg2)  or  arg1 = round(rand(arg2,arg3))");
	}
	if (_GWS(myArg3)) {
		myArg = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		myArg = *_GWS(myArg2);
	}
	if ((myArg & 0xffff) >= 0x8000) {
		*_GWS(myArg1) = (myArg + 0x10000) & 0xffff0000;
	} else {
		*_GWS(myArg1) = myArg & 0xffff0000;
	}
}

static void op_CEIL(Anim8 *myAnim8) {
	frac16 myArg;
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = ceil(arg2)  or  arg1 = ceil(rand(arg2,arg3))");
	}
	if (_GWS(myArg3)) {
		myArg = imath_ranged_rand16(*_GWS(myArg2), *_GWS(myArg3));
	} else {
		myArg = *_GWS(myArg2);
	}
	if ((myArg & 0xffff) > 0) {
		*_GWS(myArg1) = (myArg + 0x10000) & 0xffff0000;
	} else {
		*_GWS(myArg1) = myArg & 0xffff0000;
	}
}

static void op_POINT(Anim8 *myAnim8) {
	if (!_GWS(myArg3)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0252, "functionality: arg1 = angle of line segment (x, y) , (arg2, arg3)");
	}
	*_GWS(myArg1) = Atan2F16(-(int)(*_GWS(myArg3)) + myAnim8->myRegs[IDX_Y], *_GWS(myArg2) - myAnim8->myRegs[IDX_X]);
}

static void op_DIST2D(Anim8 *myAnim8) {
	int32 temp1, temp2;
	if (!_GWS(myArg3)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0252, "functionality: arg1 = distance from (x, y) to (arg2, arg3)");
	}
	temp1 = (int32)(imath_abs(*_GWS(myArg2) - myAnim8->myRegs[IDX_X]));
	temp2 = (int32)(imath_abs(*_GWS(myArg3) - myAnim8->myRegs[IDX_Y]));
	if ((temp1 >= 0x800000) || (temp2 >= 0x800000)) {
		temp1 >>= 16;
		temp2 >>= 16;
		*_GWS(myArg1) = (frac16)(SqrtF16(temp1 * temp1 + temp2 * temp2) << 16);
	} else {
		*_GWS(myArg1) = SqrtF16(SquareSF16(temp1) + SquareSF16(temp2)) << 8;
	}
}

static void op_CRUNCH(Anim8 *myAnim8) {
	frac16	myArg;
	if (_GWS(myArg2)) {
		myArg = imath_ranged_rand16(*_GWS(myArg1), *_GWS(myArg2));
	} else if (_GWS(myArg1)) {
		myArg = *_GWS(myArg1);
	} else {
		myArg = 0;
	}
	myAnim8->startTime = _GWS(ws_globals)[GLB_TIME];
	if (myArg >= 0) {
		myAnim8->switchTime = _GWS(ws_globals)[GLB_TIME] + (myArg >> 16);
	} else {
		myAnim8->switchTime = -1;
	}
	if (myAnim8->transTime <= 0x10000) {
		myAnim8->flags &= ~((TAG_BEZ | TAG_TARGS) << 16);
	}
	_GWS(keepProcessing) = false;
}

static void op_BRANCH(Anim8 *myAnim8) {
	int32 myOffset;

	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "check the CCR, arg1 is the branch type, arg2 is the PC offset");
	}
	myOffset = *_GWS(myArg2) >> 14;
	switch (*_GWS(myArg1) >> 16) {
	case BRANCH_BR:
		myAnim8->pcOffset += myOffset;
		break;
	case BRANCH_BLT:
		if (_GWS(compareCCR) < 0) myAnim8->pcOffset += myOffset;
		break;
	case BRANCH_BLE:
		if (_GWS(compareCCR) <= 0) myAnim8->pcOffset += myOffset;
		break;
	case BRANCH_BE:
		if (_GWS(compareCCR) == 0) myAnim8->pcOffset += myOffset;
		break;
	case BRANCH_BNE:
		if (_GWS(compareCCR) != 0) myAnim8->pcOffset += myOffset;
		break;
	case BRANCH_BGE:
		if (_GWS(compareCCR) >= 0) myAnim8->pcOffset += myOffset;
		break;
	case BRANCH_BGT:
		if (_GWS(compareCCR) > 0) myAnim8->pcOffset += myOffset;
		break;
	}
}

static void op_SETCEL(Anim8 *myAnim8) {
	int32 myIndex;
	CCB *myCCB;
	frac16 *myRegs;

	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "setcel(arg1, arg2)  or  setcel(arg1, rand(arg2, arg3))");
	}
	if (_GWS(myArg3)) {
		myIndex = imath_ranged_rand(*_GWS(myArg2) >> 16, *_GWS(myArg3) >> 16);
	} else if (_GWS(myArg2)) {
		myIndex = *_GWS(myArg2) >> 16;
	} else {
		myIndex = (*_GWS(myArg1) & 0xff0000) >> 16;
	}

	if (!myAnim8->myCCB) {
		// Allocate and initialize a CCB structure
		if ((myAnim8->myCCB = (CCB *)mem_alloc(sizeof(CCB), "CCB")) == nullptr) {
			ws_LogErrorMsg(FL, "Out of memory - mem requested: %d bytes.", sizeof(CCB));
			ws_Error(myAnim8->myMachine, ERR_SEQU, 0x02fe, "setcel() failed.");
		}
		if (!InitCCB(myAnim8->myCCB)) {
			ws_Error(myAnim8->myMachine, ERR_SEQU, 0x025d, "setcel() failed.");
		}
	}

	myCCB = myAnim8->myCCB;
	if (myCCB->flags & CCB_DISC_STREAM) {
		ws_CloseSSstream(myCCB);
	}
	ShowCCB(myCCB);
	myCCB->flags |= CCB_SKIP;
	if ((myAnim8->myCCB = GetWSAssetCEL((uint32)(*_GWS(myArg1)) >> 24, (uint32)myIndex, myCCB)) == nullptr) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x025b, "setcel() failed.");
	}
	myRegs = myAnim8->myRegs;
	if (myRegs[IDX_W] < 0) {
		myRegs[IDX_W] = -myCCB->source->w << 16;
	} else {
		myRegs[IDX_W] = myCCB->source->w << 16;
	}

	myRegs[IDX_H] = myCCB->source->h << 16;
	_GWS(mapTheCel) = true;
}


static void op_SEQ_SEND_MSG(Anim8 *myAnim8) {
	frac16 msgValue;

	//_GWS(myArg1) is the recipient machine hash, _GWS(myArg2) is the msg hash, _GWS(myArg3) (if exists) is the msg value
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: send to machine arg1, the message arg2 or the message arg2, arg3");
	}

	if (_GWS(myArg3)) {
		msgValue = *_GWS(myArg3);
	} else {
		msgValue = 0;
	}

	sendWSMessage(*_GWS(myArg2), msgValue, nullptr, (*_GWS(myArg1)) >> 16, nullptr, 1);
	return;
}


static void op_PUSH(Anim8 *myAnim8) {
	uint32 *data;
	int32 direction, numOfArgs, i; //,startReg

	if (!_GWS(myArg1)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0250, "functionality: push arg1  or start with arg1, and push a total of arg2 values");
	}
	direction = 1;
	if (_GWS(myArg2)) {
		if (*_GWS(myArg2) > 0) numOfArgs = (*_GWS(myArg2)) >> 16;
		else {
			numOfArgs = -(int)(*_GWS(myArg2)) >> 16;
			direction = -1;
		}
	} else {
		numOfArgs = 1;
	}

	if (((byte *)_GWS(stackLimit) - (byte *)_GWS(stackTop)) < (numOfArgs << 2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0254, "overflow during push instruction");
		return;
	}
	if (_GWS(myArg2)) {
		data = (uint32 *)_GWS(myArg1);
		for (i = 0; i < numOfArgs; i++) {
			*_GWS(stackTop)++ = *data;
			data += direction;
		}
	} else {
		*_GWS(stackTop)++ = (uint32)(*_GWS(myArg1));
	}
}

static void op_POP(Anim8 *myAnim8) {
	uint32 *data;
	int32 direction, numOfArgs, i;	// startReg,

	if (!_GWS(myArg1)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0250, "functionality: pop into arg1  or start with arg1, and pop a total of arg2 values");
	}
	direction = 1;
	if (_GWS(myArg2)) {
		if (*_GWS(myArg2) > 0) numOfArgs = (*_GWS(myArg2)) >> 16;
		else {
			numOfArgs = -(int)(*_GWS(myArg2)) >> 16;
			direction = -1;
		}
	} else numOfArgs = 1;
	if (((byte *)_GWS(stackTop) - (byte *)_GWS(stackBase)) < (numOfArgs << 2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0255, "underflow during pop instruction");
	}
	if (_GWS(myArg2)) {
		data = (uint32 *)_GWS(myArg1);
		for (i = 0; i < numOfArgs; i++) {
			*data = *(--_GWS(stackTop));
			data += direction;
		}
	} else {
		*_GWS(myArg1) = (frac16)(*(--_GWS(stackTop)));
	}
}

static void op_JSR(Anim8 *myAnim8) {
	int32 dummy;

	if (myAnim8->returnStackIndex >= JSR_STACK_MAX) {
		ws_LogErrorMsg(FL, "Max number of nested jsr instructions is: %d", JSR_STACK_MAX);
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0256, "jsr() failed");
	}
	myAnim8->returnHashes[myAnim8->returnStackIndex] = myAnim8->sequHash;
	myAnim8->returnOffsets[myAnim8->returnStackIndex] = myAnim8->pcOffset;
	myAnim8->returnStackIndex++;

	// Find the sequence
	if ((myAnim8->sequHandle = ws_GetSEQU((uint32)*_GWS(myArg1) >> 16, &dummy, &myAnim8->pcOffset)) == nullptr) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x025f, "jsr() failed");
	}
	myAnim8->sequHash = (uint32)*_GWS(myArg1) >> 16;

	dbg_LaunchSequence(myAnim8);
}

static void op_RETURN(Anim8 *myAnim8) {
	int32 dummy, dummy2;
	uint32 returnSequHash, returnOffset;

	if (myAnim8->returnStackIndex <= 0) {
		op_END(myAnim8);
		return;
	}

	myAnim8->returnStackIndex--;
	returnSequHash = myAnim8->returnHashes[myAnim8->returnStackIndex];
	returnOffset = myAnim8->returnOffsets[myAnim8->returnStackIndex];

	// Find the sequence
	if ((myAnim8->sequHandle = ws_GetSEQU((uint32)returnSequHash, &dummy, &dummy2)) == nullptr) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x025f, "return() failed");
	}
	myAnim8->sequHash = returnSequHash;
	myAnim8->pcOffset = returnOffset;

	dbg_LaunchSequence(myAnim8);
}

static void op_GET_CELS_COUNT(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = series_count(arg2)");
	}
	*_GWS(myArg1) = GetWSAssetCELCount((uint32)(*_GWS(myArg2)) >> 24) << 16;
}

static void op_GET_CELS_FRAME_RATE(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = series_frame_rate(arg2)");
	}
	*_GWS(myArg1) = GetWSAssetCELFrameRate((uint32)(*_GWS(myArg2)) >> 24);
}

static void op_GET_CELS_PIX_SPEED(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: arg1 = series_pix_speed(arg2)");
	}
	*_GWS(myArg1) = GetWSAssetCELPixSpeed((uint32)(*_GWS(myArg2)) >> 24);
}

static void op_SET_INDEX(Anim8 *myAnim8) {
	if (!_GWS(myArg1)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0250, "functionality: index_reg = arg1");
	}
	_GWS(indexReg) = *_GWS(myArg1) >> 16;
}

static void op_SET_LAYER(Anim8 *myAnim8) {
	Anim8 *tempAnim8;
	int32 myLayer, newLayer;
	if (!_GWS(myArg1)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0250, "functionality: set_layer(arg1)");
	}

	newLayer = *_GWS(myArg1) >> 16;
	myLayer = myAnim8->myLayer;

	if (myLayer == newLayer) {
		return;
	}

	// If we are moving myAnim8 closer to the front (screen)	
	if ((newLayer < myLayer) && (myAnim8->infront)) {

		//search "upward" to find the new position for myAnim8
		tempAnim8 = myAnim8->infront;
		while (tempAnim8 && (tempAnim8->myLayer > newLayer)) {
			tempAnim8 = tempAnim8->infront;
		}

		// If myAnim8 is to be moved
		if (tempAnim8 != myAnim8->infront) {

			// Remove myAnim8 from the list
			myAnim8->infront->behind = myAnim8->behind;
			if (myAnim8->behind) {
				myAnim8->behind->infront = myAnim8->infront;
			} else {
				_GWS(myCruncher)->backLayerAnim8 = myAnim8->infront;
			}

			// If it belongs at the top layer of the list
			if (!tempAnim8) {
				myAnim8->infront = nullptr;
				myAnim8->behind = _GWS(myCruncher)->frontLayerAnim8;
				_GWS(myCruncher)->frontLayerAnim8->infront = myAnim8;
				_GWS(myCruncher)->frontLayerAnim8 = myAnim8;
			}

			// Else it belongs after tempAnim8
			else {
				myAnim8->infront = tempAnim8;
				myAnim8->behind = tempAnim8->behind;
				tempAnim8->behind->infront = myAnim8;
				tempAnim8->behind = myAnim8;
			}
		}
	}

	// Else we are moving myAnim8 close to the back (further from the screen)
	else if ((newLayer > myLayer) && (myAnim8->behind)) {

		//search "downward" to find the new position for myAnim8
		tempAnim8 = myAnim8->behind;
		while (tempAnim8 && (tempAnim8->myLayer < newLayer)) {
			tempAnim8 = tempAnim8->behind;
		}

		// If myAnim8 is to be moved
		if (tempAnim8 != myAnim8->behind) {

			// Remove it from the list
			if (myAnim8->infront) {
				myAnim8->infront->behind = myAnim8->behind;
			} else {
				_GWS(myCruncher)->frontLayerAnim8 = myAnim8->behind;
			}
			myAnim8->behind->infront = myAnim8->infront;

			// If it belongs at the bottom of the list
			if (!tempAnim8) {
				myAnim8->infront = _GWS(myCruncher)->backLayerAnim8;
				myAnim8->behind = nullptr;
				_GWS(myCruncher)->backLayerAnim8->behind = myAnim8;
				_GWS(myCruncher)->backLayerAnim8 = myAnim8;
			}

			// Else it belongs right before tempAnim8
			else {
				myAnim8->infront = tempAnim8->infront;
				myAnim8->behind = tempAnim8;
				tempAnim8->infront->behind = myAnim8;
				tempAnim8->infront = myAnim8;
			}
		}
	}

	// Now, make sure the layer is set in both myAnim8, and the register
	myAnim8->myLayer = newLayer;
	myAnim8->myRegs[IDX_LAYER] = newLayer << 16;
}

static void op_SET_DEPTH(Anim8 *myAnim8) {
	int32 myDepth;
	if (!_GWS(myArg1)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0250, "functionality: set_depth(arg1)");
	}
	if (!_GWS(myDepthTable)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x02ff, "op_SET_DEPTH() failed - no depth table.");
	}
	for (myDepth = 0; myDepth < 15; myDepth++) {
		if (_GWS(myDepthTable)[myDepth + 1] < (int)(*_GWS(myArg1) >> 16)) break;
	}
	_GWS(dataArg1) = (myAnim8->myRegs[IDX_LAYER] & 0xffffff) + (myDepth << 24);
	_GWS(myArg1) = &_GWS(dataArg1);
	op_SET_LAYER(myAnim8);
}

static void op_SET_DATA(Anim8 *myAnim8) {
	if (!_GWS(myArg2)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0251, "functionality: set_data(arg1, arg2)");
	}
	if ((myAnim8->dataHandle = ws_GetDATA(*_GWS(myArg1) >> 16, *_GWS(myArg2) >> 16, &myAnim8->dataOffset)) == nullptr) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x025f, "set_data() failed.");
	}
}

static void op_OPEN_STREAM_SS(Anim8 *myAnim8) {
	CCB *myCCB;

	if (!_GWS(myArg1)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0250, "functionality: stream_series(arg1)");
	}

	if (!myAnim8->myCCB) {
		// Allocate and initialize a CCB structure
		if ((myAnim8->myCCB = (CCB *)mem_alloc(sizeof(CCB), "CCB")) == nullptr) {
			ws_LogErrorMsg(FL, "Out of memory - mem requested: %d.", sizeof(CCB));
			ws_Error(myAnim8->myMachine, ERR_SEQU, 0x02fe, "open_ss_stream() failed.");
		}
		if (!InitCCB(myAnim8->myCCB)) {
			ws_Error(myAnim8->myMachine, ERR_SEQU, 0x025d, "open_ss_stream() failed.");
		}
	}

	myCCB = myAnim8->myCCB;
	ShowCCB(myCCB);
	myCCB->flags |= CCB_SKIP;

	if (!ws_OpenSSstream((SysFile *)(*_GWS(myArg1)), myAnim8)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0258, "open_ss_stream() failed.");
	}

	if (myAnim8->myRegs[IDX_W] < 0)
		myAnim8->myRegs[IDX_W] = -myCCB->source->w << 16;
	else
		myAnim8->myRegs[IDX_W] = myCCB->source->w << 16;

	myAnim8->myRegs[IDX_H] = myCCB->source->h << 16;
	_GWS(mapTheCel) = true;
}

static void op_NEXT_STREAM_SS(Anim8 *myAnim8) {
	CCB *myCCB;
	if (!myAnim8->myCCB) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0253, "next_ss_stream() failed.");
	}
	myCCB = myAnim8->myCCB;
	myCCB->flags |= CCB_SKIP;
	if (!ws_GetNextSSstreamCel(myAnim8)) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x0259, "next_ss_stream() failed.");
	}
	if (myAnim8->myRegs[IDX_W] < 0) {
		myAnim8->myRegs[IDX_W] = -myCCB->source->w << 16;
	} else {
		myAnim8->myRegs[IDX_W] = myCCB->source->w << 16;
	}
	myAnim8->myRegs[IDX_H] = myCCB->source->h << 16;
	_GWS(mapTheCel) = true;
}

static void op_CLOSE_STREAM_SS(Anim8 *myAnim8) {
	CCB *myCCB;
	if (!myAnim8->myCCB) {
		ws_Error(myAnim8->myMachine, ERR_SEQU, 0x02f3, "close_ss_stream() failed.");
	}
	myCCB = myAnim8->myCCB;
	ws_CloseSSstream(myCCB);
	HideCCB(myCCB);
}


void (*pCodeJmpTable[])(Anim8 *myAnim8) = {
	&op_END,					//0
	&op_CLEAR,					//1
	&op_SET,					//2
	&op_COMPARE,				//3
	&op_ADD,					//4
	&op_SUB,					//5
	&op_MULT,					//6
	&op_DIV,					//7
	&op_AND,					//8
	&op_OR,						//9
	&op_NOT,					//10
	&op_SIN,					//11
	&op_COS,					//12
	&op_ABS,					//13
	&op_MIN,					//14
	&op_MAX,					//15
	&op_MOD,					//16
	&op_FLOOR,					//17
	&op_ROUND,					//18
	&op_CEIL,					//19
	&op_POINT,					//20
	&op_DIST2D,					//21
	&op_CRUNCH,					//22
	&op_BRANCH,					//23
	&op_SETCEL,					//24
	&op_SEQ_SEND_MSG,			//25
	&op_PUSH,					//26
	&op_POP,					//27
	&op_JSR,					//28
	&op_RETURN,					//29
	&op_GET_CELS_COUNT,			//30
	&op_GET_CELS_FRAME_RATE,	//31
	&op_GET_CELS_PIX_SPEED,		//32
	&op_SET_INDEX,				//33
	&op_SET_LAYER,				//34
	&op_SET_DEPTH,				//35
	&op_SET_DATA,				//36
	&op_OPEN_STREAM_SS,			//37
	&op_NEXT_STREAM_SS,			//38
	&op_CLOSE_STREAM_SS			//39
};


// The guts of the engine.  This proc executes an anim8s program.
bool CrunchAnim8(Anim8 *myAnim8) {
	bool moveTheCel = false;
	frac16 timeElapsed, percentDist;
	int32 tempAngle;
	frac16 oldX, oldY, oldS;
	int32 oldW, oldH, oldR;
	int32 myInstruction;
	frac16 *myRegs;
	uint32 *myPC, *oldPC;

	// Get the register set for myAnim8
	myRegs = myAnim8->myRegs;

	// Initialize the globals (flags, mostly) used in processing op codes
	_GWS(keepProcessing) = false;
	_GWS(terminated) = false;
	_GWS(mapTheCel) = false;
	_GWS(compareCCR) = 0;

	//store the old values, so we can tell if we need to remap the Sprite
	oldX = myRegs[IDX_X];
	oldY = myRegs[IDX_Y];
	oldS = myRegs[IDX_S];
	oldW = myRegs[IDX_W] >> 16;
	oldH = myRegs[IDX_H] >> 16;
	oldR = myRegs[IDX_R] >> 16;

	// Check to see if we are still in an execution loop, or if it is time to 
	// Interpret further instructions
	if ((myAnim8->switchTime >= 0) && ((int)_GWS(ws_globals)[GLB_TIME] >= myAnim8->switchTime)) {
		_GWS(keepProcessing) = true;
	}

	// Decrement the timer register
	myRegs[IDX_TIMER] -= (_GWS(ws_globals)[GLB_WATCH_DOG] << 16);

	// Interpret pCode instructions until we hit something signalling to stop
	while (_GWS(keepProcessing)) {
		dbg_SetCurrSequInstr(myAnim8, _GWS(compareCCR));

		myPC = (uint32 *)((intptr)*(myAnim8->sequHandle) + myAnim8->pcOffset);
		oldPC = myPC;
		_GWS(pcOffsetOld) = myAnim8->pcOffset;

		dbg_SetCurrMachInstr(myAnim8->myMachine, myAnim8->pcOffset, true);

		if ((myInstruction = ws_PreProcessPcode(&myPC, myAnim8)) < 0) {
			ws_Error(myAnim8->myMachine, ERR_SEQU, 0x025c, nullptr);
		}

		dbg_EndCurrMachInstr();

		myAnim8->pcOffset += (intptr)myPC - (intptr)oldPC;
		pCodeJmpTable[myInstruction](myAnim8);
	}

	if (_GWS(bailOut)) {
		_GWS(bailOut) = false;
		return true;
	}

	if (_GWS(terminated)) {
		if (_GWS(mapTheCel) || (oldR != (int)(myRegs[IDX_R] >> 16)) ||
				(oldW != (int)(myRegs[IDX_W] >> 16)) || (oldH != (int)(myRegs[IDX_H] >> 16)) ||
				(oldS != (int)myRegs[IDX_S])) {
			myAnim8->flags |= TAG_MAP_CEL;
		} else if ((oldX != myRegs[IDX_X]) || (oldY != myRegs[IDX_Y])) {
			myAnim8->flags |= TAG_MOVE_CEL;
		}

		return false;
	}

	if (myAnim8->flags) {
		timeElapsed = (_GWS(ws_globals)[GLB_TIME] - myAnim8->startTime) << 16;

		// This must be checked before TAG_TARGS because a bez path can use a target scale and rotate
		// And we don't want to accidentally set up a target x or y.
		// NOTE: for both bez paths, and targets, the time for the anim8 to reach the target or traverse
		//      the path is stored in IDX_TRANS_TIME, however, in order to determine how far aint32 the
		//      path we should be, we normally compute the elapsed time divided by the trans time.
		//      therefore we eliminate a division if we store the reciprocated trans time.
		//      This also serves as an initialization flag - if the trans time is > 1.
		if ((myAnim8->flags >> 16) &TAG_BEZ) {
			if (myAnim8->transTime > 0x10000) {
				myAnim8->transTime = RecipUF16(myAnim8->transTime);
				GetBezCoeffs(&myRegs[IDX_BEZ_CTRL], &myRegs[IDX_BEZ_COEFF]);
				if ((myAnim8->flags >> 16) &TAG_TARGS) {
					myAnim8->start_s = myRegs[IDX_S];
					myAnim8->start_r = myRegs[IDX_R];
				}
			}
			percentDist = MulSF16(timeElapsed, myAnim8->transTime);
			if (percentDist < 0x10000) {
				GetBezPoint(&myRegs[IDX_X], &myRegs[IDX_Y], &myRegs[IDX_BEZ_COEFF], percentDist);
				if ((myAnim8->flags >> 16) &TAG_TARGS) {
					myRegs[IDX_S] = myAnim8->start_s + MulSF16(percentDist, myRegs[IDX_TARG_S] - myAnim8->start_s);
					myRegs[IDX_R] = myAnim8->start_r + MulSF16(percentDist, myRegs[IDX_TARG_R] - myAnim8->start_r);
					_GWS(mapTheCel) = true;
				} else moveTheCel = true;
			} else if ((myRegs[IDX_X] != myRegs[IDX_BEZ_CTRL + 6]) || (myRegs[IDX_Y] != myRegs[IDX_BEZ_CTRL + 7])) {
				myRegs[IDX_X] = myRegs[IDX_BEZ_CTRL + 6];
				myRegs[IDX_Y] = myRegs[IDX_BEZ_CTRL + 7];
				if ((myAnim8->flags >> 16) &TAG_TARGS) {
					myRegs[IDX_S] = myRegs[IDX_TARG_S];
					myRegs[IDX_R] = myRegs[IDX_TARG_R];
					_GWS(mapTheCel) = true;
				} else moveTheCel = true;
			}
		}
		// Vectors must be checked before deltas, since vectors are converted to deltas at
		// Initialization.
		else if (((myAnim8->flags >> 16) &TAG_VECTORS) && (timeElapsed == 0)) {
			myAnim8->start_x = myRegs[IDX_X];
			myAnim8->start_y = myRegs[IDX_Y];
			if ((myAnim8->flags >> 16) &TAG_DELTAS) {
				myAnim8->start_s = myRegs[IDX_S];
				myAnim8->start_r = myRegs[IDX_R];
			}
			tempAngle = (myRegs[IDX_THETA] >> 16) & 0xff;
			myRegs[IDX_DELTA_X] = MulSF16(myRegs[IDX_VELOCITY], sinTable[tempAngle]);
			myRegs[IDX_DELTA_Y] = MulSF16(myRegs[IDX_VELOCITY], -(int)cosTable[tempAngle]);
			myAnim8->flags |= (TAG_DELTAS << 16);
		} else if ((myAnim8->flags >> 16) &TAG_DELTAS) {
			if (timeElapsed == 0) {
				myAnim8->start_x = myRegs[IDX_X];
				myAnim8->start_y = myRegs[IDX_Y];
				myAnim8->start_s = myRegs[IDX_S];
				myAnim8->start_r = myRegs[IDX_R];
			} else {
				myRegs[IDX_X] = myAnim8->start_x + MulSF16(timeElapsed, myRegs[IDX_DELTA_X]);
				myRegs[IDX_Y] = myAnim8->start_y + MulSF16(timeElapsed, myRegs[IDX_DELTA_Y]);
				if (myRegs[IDX_DELTA_R] || myRegs[IDX_DELTA_S]) {
					myRegs[IDX_S] = myAnim8->start_s + MulSF16(timeElapsed, myRegs[IDX_DELTA_S]);
					myRegs[IDX_R] = myAnim8->start_r + MulSF16(timeElapsed, myRegs[IDX_DELTA_R]);
					_GWS(mapTheCel) = true;
				} else moveTheCel = true;
			}
		} else if ((myAnim8->flags >> 16) &TAG_TARGS) {
			if (myAnim8->transTime > 0x10000) {
				myAnim8->start_s = myRegs[IDX_S];
				myAnim8->start_r = myRegs[IDX_R];
				myAnim8->start_x = myRegs[IDX_X];
				myAnim8->start_y = myRegs[IDX_Y];
				myAnim8->transTime = RecipUF16(myAnim8->transTime);
			}
			percentDist = MulSF16(timeElapsed, myAnim8->transTime);
			if (percentDist < 0x10000) {
				myRegs[IDX_X] = myAnim8->start_x + MulSF16(percentDist, myRegs[IDX_TARG_X] - myAnim8->start_x);
				myRegs[IDX_Y] = myAnim8->start_y + MulSF16(percentDist, myRegs[IDX_TARG_Y] - myAnim8->start_y);
				if (myRegs[IDX_TARG_R] || (myRegs[IDX_TARG_S] != myRegs[IDX_S])) {
					myRegs[IDX_S] = myAnim8->start_s + MulSF16(percentDist, myRegs[IDX_TARG_S] - myAnim8->start_s);
					myRegs[IDX_R] = myAnim8->start_r + MulSF16(percentDist, myRegs[IDX_TARG_R] - myAnim8->start_r);
					_GWS(mapTheCel) = true;
				} else moveTheCel = true;
			} else {
				myRegs[IDX_X] = myRegs[IDX_TARG_X];
				myRegs[IDX_Y] = myRegs[IDX_TARG_Y];
				myRegs[IDX_S] = myRegs[IDX_TARG_S];
				myRegs[IDX_R] = myRegs[IDX_TARG_R];
				myAnim8->flags &= ~TAG_TARGS;
				_GWS(mapTheCel) = true;
			}
		}
	}
	if (_GWS(mapTheCel) || (oldR != (int)(myRegs[IDX_R] >> 16)) || (oldW != (int)(myRegs[IDX_W] >> 16)) ||
			(oldH != (int)(myRegs[IDX_H] >> 16)) || (oldS != (int)myRegs[IDX_S])) {
		_GWS(mapTheCel) = true;
	} else if ((oldX != myRegs[IDX_X]) || (oldY != myRegs[IDX_Y])) {
		moveTheCel = true;
	}
	if (moveTheCel || _GWS(mapTheCel) || (myAnim8->flags & (TAG_MAP_CEL | TAG_MOVE_CEL))) {
		Cel_msr(myAnim8);
	}
	return true;
}

void ws_CrunchAnim8s(int16 *depth_table) {
	Anim8 *currAnim8;
	EOSreq *tempEOSreq;

	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_CrunchAnim8s()");

	//set up some of the _GWS(ws_globals) vars used for processing
	_GWS(myDepthTable) = depth_table;

	_GWS(crunchNext) = nullptr;
	currAnim8 = _GWS(myCruncher)->firstAnim8ToCrunch;
	while (currAnim8) {
		_GWS(crunchNext) = currAnim8->next;
		if (currAnim8->active) {
			if (!CrunchAnim8(currAnim8)) {
				// If false was returned, this implies that an "END" op has been reached.
				// Remove from the active list.  Note: the machine still points to the anim8.
				currAnim8->active = false;
				if (currAnim8->eosReqOffset >= 0) {
					// If the above field has a value, this implies that an On end sequence
					//signal has been requested. If so, report back to the machine.
					if ((tempEOSreq = (EOSreq *)mem_get_from_stash(_GWS(memtypeEOS), "+EOS")) == nullptr) return;
					tempEOSreq->myAnim8 = currAnim8;
					tempEOSreq->prev = nullptr;
					tempEOSreq->next = _GWS(EOSreqList);
					if (_GWS(EOSreqList)) {
						_GWS(EOSreqList)->prev = tempEOSreq;
					}
					_GWS(EOSreqList) = tempEOSreq;
				}
			}
		}
		currAnim8 = _GWS(crunchNext);
	}
	_GWS(crunchNext) = nullptr;
}

void ws_CrunchEOSreqs(void) {
	int32 pcOffset, pcCount;
	machine *myXM;
	EOSreq *tempEOSreq;

	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_CrunchEOSreqs()");

	// Loop through, and handle all the eos requests
	tempEOSreq = _GWS(EOSreqList);
	while (tempEOSreq) {
		_GWS(EOSreqList) = _GWS(EOSreqList)->next;
		if (_GWS(EOSreqList)) {
			_GWS(EOSreqList)->prev = nullptr;
		}
		pcOffset = tempEOSreq->myAnim8->eosReqOffset;
		pcCount = tempEOSreq->myAnim8->eosReqCount;
		myXM = tempEOSreq->myAnim8->myMachine;
		tempEOSreq->myAnim8->eosReqOffset = -1;
		mem_free_to_stash((void *)tempEOSreq, _GWS(memtypeEOS));
		ws_StepWhile(myXM, pcOffset, pcCount);
		tempEOSreq = _GWS(EOSreqList);
	}
}

// This proc was designed to allow the state machine to issue an OnEndSeq request
// In which the cruncher will signal the state machine should an anim8 ever finish.
bool ws_OnEndSeqRequest(Anim8 *myAnim8, int32 pcOffset, int32 pcCount) {

	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_OnEndSeqRequest()");

	myAnim8->eosReqOffset = pcOffset;
	myAnim8->eosReqCount = pcCount;

	return true;
}

void ws_CancelOnEndSeq(Anim8 *myAnim8) {
	// Make sure the cruncher has been initialized
	VERIFY_INTIALIZED("ws_CancelOnEndSeq()");

	myAnim8->eosReqOffset = -1;
}

} // End of namespace M4
