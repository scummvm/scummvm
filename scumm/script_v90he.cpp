/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2004 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */


#include "stdafx.h"

#include "common/config-manager.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/verbs.h"

#include "sound/mididrv.h"
#include "sound/mixer.h"

namespace Scumm {

#define OPCODE(x)	{ &ScummEngine_v90he::x, #x }

void ScummEngine_v90he::setupOpcodes() {
	static const OpcodeEntryV90he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o72_pushDWord),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o72_addMessageToStack),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o72_wordArrayRead),
		/* 08 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o72_wordArrayIndexedRead),
		/* 0C */
		OPCODE(o6_dup),
		OPCODE(o6_not),
		OPCODE(o6_eq),
		OPCODE(o6_neq),
		/* 10 */
		OPCODE(o6_gt),
		OPCODE(o6_lt),
		OPCODE(o6_le),
		OPCODE(o6_ge),
		/* 14 */
		OPCODE(o6_add),
		OPCODE(o6_sub),
		OPCODE(o6_mul),
		OPCODE(o6_div),
		/* 18 */
		OPCODE(o6_land),
		OPCODE(o6_lor),
		OPCODE(o6_pop),
		OPCODE(o72_isAnyOf),
		/* 1C */
		OPCODE(o90_unknown1C),
		OPCODE(o90_getLT),
		OPCODE(o90_getGT),
		OPCODE(o6_invalid),
		/* 20 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 24 */
		OPCODE(o6_invalid),
		OPCODE(o90_unknown25),
		OPCODE(o90_unknown26),
		OPCODE(o90_unknown27),
		/* 28 */
		OPCODE(o90_unknown28),
		OPCODE(o90_unknown29),
		OPCODE(o6_invalid),
		OPCODE(o90_startLocalScript),
		/* 2C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o90_unknown2F),
		/* 30 */
		OPCODE(o90_mod),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 34 */
		OPCODE(o90_unknown34),
		OPCODE(o6_invalid),
		OPCODE(o90_unknown36),
		OPCODE(o90_unknown37),
		/* 38 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o90_unknown3A),
		OPCODE(o6_invalid),
		/* 3C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 40 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_writeWordVar),
		/* 44 */
		OPCODE(o6_invalid),
		OPCODE(o80_unknown45),
		OPCODE(o6_invalid),
		OPCODE(o72_wordArrayWrite),
		/* 48 */
		OPCODE(o80_unknown48),
		OPCODE(o80_unknown49),
		OPCODE(o80_unknown4A),
		OPCODE(o72_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o80_readConfigFile),
		OPCODE(o80_writeConfigFile),
		OPCODE(o6_wordVarInc),
		/* 50 */
		OPCODE(o72_unknown50),
		OPCODE(o6_invalid),
		OPCODE(o72_findObjectWithClassOf),
		OPCODE(o72_wordArrayInc),
		/* 54 */
		OPCODE(o72_objectX),
		OPCODE(o72_objectY),
		OPCODE(o6_invalid),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o72_getTimer),
		OPCODE(o72_setTimer),
		OPCODE(o72_unknown5A),
		OPCODE(o72_wordArrayDec),
		/* 5C */
		OPCODE(o6_if),
		OPCODE(o6_ifNot),
		OPCODE(o72_startScript),
		OPCODE(o6_startScriptQuick),
		/* 60 */
		OPCODE(o72_startObject),
		OPCODE(o72_drawObject),
		OPCODE(o72_printWizImage),
		OPCODE(o72_getArrayDimSize),
		/* 64 */
		OPCODE(o72_getNumFreeArrays),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_endCutscene),
		/* 68 */
		OPCODE(o6_cutscene),
		OPCODE(o6_stopMusic),
		OPCODE(o6_freezeUnfreeze),
		OPCODE(o80_cursorCommand),
		/* 6C */
		OPCODE(o6_breakHere),
		OPCODE(o6_ifClassOfIs),
		OPCODE(o6_setClass),
		OPCODE(o6_getState),
		/* 70 */
		OPCODE(o80_setState),
		OPCODE(o6_setOwner),
		OPCODE(o6_getOwner),
		OPCODE(o6_jump),
		/* 74 */
		OPCODE(o7_startSound),
		OPCODE(o6_stopSound),
		OPCODE(o6_startMusic),
		OPCODE(o6_stopObjectScript),
		/* 78 */
		OPCODE(o6_panCameraTo),
		OPCODE(o6_actorFollowCamera),
		OPCODE(o6_setCameraAt),
		OPCODE(o6_loadRoom),
		/* 7C */
		OPCODE(o6_stopScript),
		OPCODE(o6_walkActorToObj),
		OPCODE(o6_walkActorTo),
		OPCODE(o6_putActorAtXY),
		/* 80 */
		OPCODE(o6_putActorAtObject),
		OPCODE(o6_faceActor),
		OPCODE(o6_animateActor),
		OPCODE(o6_doSentence),
		/* 84 */
		OPCODE(o7_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o7_getActorRoom),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		OPCODE(o6_getObjectOldDir),
		/* 90 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_findInventory),
		OPCODE(o6_getInventoryCount),
		/* 94 */
		OPCODE(o90_unknown94),
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_setObjectName),
		/* 98 */
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_setBoxFlags),
		OPCODE(o6_invalid),
		OPCODE(o7_resourceRoutines),
		/* 9C */
		OPCODE(o6_roomOps),
		OPCODE(o72_actorOps),
		OPCODE(o90_unknown9E),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o72_findObject),
		OPCODE(o6_pseudoRoom),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getVerbEntrypoint),
		/* A4 */
		OPCODE(o72_arrayOps),
		OPCODE(o6_saveRestoreVerbs),
		OPCODE(o6_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o6_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o80_drawWizPolygon),
		OPCODE(o6_isAnyOf),
		OPCODE(o7_quitPauseRestart),
		OPCODE(o6_isActorInBox),
		/* B0 */
		OPCODE(o6_delay),
		OPCODE(o6_delaySeconds),
		OPCODE(o6_delayMinutes),
		OPCODE(o6_stopSentence),
		/* B4 */
		OPCODE(o6_printLine),
		OPCODE(o6_printCursor),
		OPCODE(o6_printDebug),
		OPCODE(o6_printSystem),
		/* B8 */
		OPCODE(o6_printActor),
		OPCODE(o6_printEgo),
		OPCODE(o6_talkActor),
		OPCODE(o6_talkEgo),
		/* BC */
		OPCODE(o72_dimArray),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		/* C0 */
		OPCODE(o72_dim2dimArray),
		OPCODE(o72_unknownC1),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_abs),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* C8 */
		OPCODE(o6_kernelGetFunctions),
		OPCODE(o7_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o72_drawWizImage),
		OPCODE(o6_invalid),
		/* D0 */
		OPCODE(o6_getDateTime),
		OPCODE(o6_stopTalking),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o72_shuffle),
		OPCODE(o72_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o6_closeFile),
		OPCODE(o72_openFile),
		OPCODE(o72_readFile),
		/* DC */
		OPCODE(o72_writeFile),
		OPCODE(o72_findAllObjects),
		OPCODE(o72_deleteFile),
		OPCODE(o6_rename),
		/* E0 */
		OPCODE(o6_invalid),
		OPCODE(o72_getPixel),
		OPCODE(o6_localizeArray),
		OPCODE(o80_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o6_seekFilePos),
		OPCODE(o72_redimArray),
		OPCODE(o6_readFilePos),
		/* EC */
		OPCODE(o72_unknownEC),
		OPCODE(o72_unknownED),
		OPCODE(o7_stringLen),
		OPCODE(o72_unknownEF),
		/* F0 */
		OPCODE(o72_unknownF0),
		OPCODE(o72_unknownF1),
		OPCODE(o72_checkGlobQueue),
		OPCODE(o72_readINI),
		/* F4 */
		OPCODE(o72_writeINI),
		OPCODE(o72_unknownF5),
		OPCODE(o72_unknownF6),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o72_unknownF8),
		OPCODE(o72_setFilePath),
		OPCODE(o72_unknownFA),
		OPCODE(o7_polygonOps),
		/* FC */
		OPCODE(o7_polygonHit),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesV90he = opcodes;
}

void ScummEngine_v90he::executeOpcode(byte i) {
	OpcodeProcV90he op = _opcodesV90he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v90he::getOpcodeDesc(byte i) {
	return _opcodesV90he[i].desc;
}

void ScummEngine_v90he::o90_getLT() {
	int a = pop();
	int b = pop();

	if (b < a) {
		push(b);
	} else {
		push(a);
	}
}

void ScummEngine_v90he::o90_getGT() {
	int a = pop();
	int b = pop();

	if (b > a) {
		push(b);
	} else {
		push(a);
	}
}

void ScummEngine_v90he::o90_startLocalScript() {
	int args[16];
	int script, entryp;
	int flags;
	getStackList(args, ARRAYSIZE(args));
	entryp = pop();
	script = pop();
	flags = fetchScriptByte();
	runScript(script, (flags == 199 || flags == 200), (flags == 195 || flags == 200), args);
}

void ScummEngine_v90he::o90_unknown1C() {
	// For Pajame Sam 2 demo
	// Incomplete
	int subOp = fetchScriptByte();
	subOp -= 46;

	switch (subOp) {
		case -14: // HE99+
			pop();
			break;
		case -13: // HE99+
			pop();
			break;
		case 0:
			pop();
			break;
		case 1:
			pop();
			pop();
			pop();
			pop();
			break;
		case 2:
			//Sets a variable to 1
			break;
		case 3:
			//Gets a script string
			break;
		case 4:
			//Gets a script string
			pop();
			break;
		case 5:
			pop();
			pop();
			pop();
			pop();
			pop();
			break;
		case 6:
			pop();
			break;
		case 7:
			pop();
			break;
		case 8:
			pop();
			break;
		case 10:
		{
			int flags = pop();
			int state = pop();
			int y1 = pop();
			int x1 = pop();
			int resnum = pop();
			if (_fullRedraw) {
				assert(_wizImagesNum < ARRAYSIZE(_wizImages));
				WizImage *pwi = &_wizImages[_wizImagesNum];
				pwi->resnum = resnum;
				pwi->x1 = x1;
				pwi->y1 = y1;
				pwi->flags = flags;
				++_wizImagesNum;
			} else {
				drawWizImage(rtImage, resnum, state, x1, y1, flags);
			}
		}
			break;
		case 11:
			pop();
			break;
		case 19:
			pop();
			pop();
			break;
		case 87: // HE99+
			pop();
			pop();
			pop();
			pop();
			pop();
			break;
		case 91: // HE99+
			pop();
			break;
		case 171: // HE99+
			break;
		case 209:
			break;
		default:
			error("o90_unknown1C: unhandled case %d", subOp);
	}

	debug(1,"o90_unknown1C stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown25() {
	int args[16];
	int subOp = fetchScriptByte();
	subOp -= 30;

	debug(1,"o90_unknown25 stub (%d)", subOp);
	switch (subOp) {
		case 0:
			pop();
			break;
		case 1:
			pop();
			break;
		case 2:
			pop();
			break;
		case 3:
			pop();
			break;
		case 4:
			pop();
			break;
		case 5:
			pop();
			break;
		case 6:
			pop();
			break;
		case 7:
			pop();
			break;
		case 8:
			pop();
			break;
		case 9:
			pop();
			break;
		case 12:
			pop();
			pop();
			break;
		case 13:
			pop();
			break;
		case 15:
			if (_heversion == 99) {
				getStackList(args, ARRAYSIZE(args));
				pop();
				pop();
				pop();
				pop();
			} else if (_heversion == 98) {
				pop();
				pop();
				pop();
				pop();
			} else {
				pop();
				pop();
				pop();
			}
			break;
		case 22:
			pop();
			break;
		case 33:
			pop();
			break;
		case 38:
			pop();
			break;
		case 52:
			pop();
			break;
		case 62:
			pop();
			break;
		case 67:
			pop();
			break;
		case 68:
			pop();
			break;
		case 94:
			pop();
			break;
		case 95:
			getStackList(args, ARRAYSIZE(args));
			pop();
			break;
		case 168:
			pop();
			pop();
			break;
		default:
			error("o90_unknown25: Unknown case %d", subOp);
	}
	push(0);


}

void ScummEngine_v90he::o90_unknown26() {
	// Incomplete
	int args[16];
	int subOp = fetchScriptByte();
	subOp -= 34;

	switch (subOp) {
		case 0:
			pop();
			break;
		case 1:
			pop();
			break;
		case 3:
			pop();
			break;
		case 8:
			pop();
			pop();
			break;
		case 9:
			pop();
			break;
		case 10:
			pop();
			pop();
			break;
		case 18:
			pop();
			break;
		case 19:
			pop();
			break;
		case 23:
			pop();
			if (_gameId == GID_FREDDI4 || _heversion >= 99)
				pop();
			break;
		case 28: // HE99+
			pop();
			break;
		case 29:
			pop();
			break;
		case 31:
			pop();
			pop();
			break;
		case 34:
			pop();
			break;
		case 43:
			pop();
			pop();
			break;
		case 48:
			pop();
			break;
		case 52: // HE 98+
			pop();
			break;
		case 64:
			pop();
			break;
		case 90:
			pop();
			break;
		case 91:
			getStackList(args, ARRAYSIZE(args));
			break;
		case 105: // HE99+
			pop();
			pop();
			break;
		case 124:
			break;
		case 164:
			pop();
			pop();
			break;
		case 183:
			break;
		default:
			error("o90_unknown26: Unknown case %d", subOp);
	}
	debug(1,"o90_unknown26 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown27() {
	int subOp = fetchScriptByte();
	switch (subOp) {
		case 30:
			pop();
			break;
		case 31:
			pop();
			break;
		case 43:
			pop();
			break;
		default:
			error("o90_unknown27: Unknown case %d", subOp);
	}
	push(0);

	debug(1,"o90_unknown27 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown28() {
	// Incomplete
	int subOp = fetchScriptByte();
	subOp -= 37;

	switch (subOp) {
		case 6:
			pop();
			break;
		case 7:
			pop();
			pop();
			break;
		case 20:
			pop();
			break;
		case 28:
			pop();
			pop();
			break;
		case 30:
			pop();
			pop();
			pop();
			pop();
			break;
		case 180:
			break;
		default:
			error("o90_unknown28: Unknown case %d", subOp);
	}
	debug(1,"o90_unknown28 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown29() {
	int subOp = fetchScriptByte();
	subOp -= 30;

	switch (subOp) {
		case 0:
			pop();
			pop();
			break;
		case 1:
			pop();
			pop();
			break;
		case 2:
			pop();
			pop();
			break;
		case 3:
			pop();
			pop();
			break;
		case 6:
			pop();
			break;
		case 15:
			pop();
			pop();
			pop();
			pop();
			break;
		case 36:
			pop();
			pop();
			pop();
			pop();
			break;
		case 100:
			pop();
			pop();
			pop();
			pop();
			pop();
			pop();
			break;
		default:
			error("o90_unknown29: Unknown case %d", subOp);
	}
	push(0);

	debug(1,"o90_unknown29 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown2F() {
	int subOp = fetchScriptByte();
	subOp -= 54;

	switch (subOp) {
		case 0:
			pop();
			break;
		case 3:
			break;
		case 11:
			pop();
			pop();
			break;
		case 12:
			pop();
			break;
		case 13:
			pop();
			pop();
			pop();
			pop();
			break;
		case 201:
			break;
		default:
			error("o90_unknown28: Unknown case %d", subOp);
	}
	debug(1,"o90_unknown2F stub (%d)", subOp);
}

void ScummEngine_v90he::o90_mod() {
	int a = pop();
	if (a == 0)
		error("modulus by zero");
	push(pop() % a);
}

void ScummEngine_v90he::o90_unknown34() {
	// Incomplete
	int args[16];

	getStackList(args, ARRAYSIZE(args));
	int room = pop();
	int i = 1;

	if (room != _currentRoom)
		warning("o72_findAllObjects: current room is not %d", room);
	writeVar(0, 0);
	defineArray(0, kDwordArray, 0, 0, 0, _numLocalObjects + 1);
	writeArray(0, 0, 0, _numLocalObjects);
	
	while (i < _numLocalObjects) {
		writeArray(0, 0, i, _objs[i].obj_nr);
		i++;
	}
	
	push(readVar(0));
}

void ScummEngine_v90he::o90_unknown36() {
	int a = pop();
	int b = pop();
	int c = pop();

	if (!c)
		b = a;
	push(b);
}

void ScummEngine_v90he::o90_unknown37() {
	int data, dim1start, dim1end, dim2start, dim2end;
	int type = fetchScriptByte();

	switch (type) {
	case 2:		// SO_BIT_ARRAY
		data = kBitArray;
		break;
	case 3:		// SO_NIBBLE_ARRAY
		data = kNibbleArray;
		break;
	case 4:		// SO_BYTE_ARRAY
		data = kByteArray;
		break;
	case 5:		// SO_INT_ARRAY
		data = kIntArray;
		break;
	case 6:
		data = kDwordArray;
		break;
	case 7:		// SO_STRING_ARRAY
		data = kStringArray;
		break;
	default:
		error("o90_unknown37: default case %d", type);
	}

	if (pop() == 2) {
		dim1end = pop();
		dim1start = pop();
		dim2end = pop();
		dim2start = pop();
	} else {
		dim2end = pop();
		dim2start = pop();
		dim1end = pop();
		dim1start = pop();
	}

	defineArray(fetchScriptWord(), data, dim2start, dim2end, dim1start, dim1end);
}

void ScummEngine_v90he::o90_unknown3A() {
	// Sorts array via qsort
	int subOp = fetchScriptByte();

	switch (subOp) {
		case 129:
			fetchScriptWord();
			pop();
			pop();
			pop();
			pop();
			pop();
			break;
		default:
			error("o90_unknown3A: Unknown case %d", subOp);
	}
	debug(1,"o90_unknown3A stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown94() {
	int subOp = fetchScriptByte();
	subOp -= 45;

	switch (subOp) {
		case 0:
			pop();
			pop();
			pop();
			pop();
			pop();
			pop();
			break;
		case 7:
			pop();
			pop();
			pop();
			break;
		case 21:
			pop();
			pop();
			break;
		case 87:
			pop();
			pop();
			break;
		case 172:
			pop();
			pop();
			pop();
			break;
		default:
			error("o90_unknown94: Unknown case %d", subOp);
	}
	push(0);
	debug(1,"o90_unknown94 stub (%d)", subOp);
}

void ScummEngine_v90he::o90_unknown9E() {
	int subOp = fetchScriptByte();
	subOp -= 57;

	switch (subOp) {
		case 0:
			pop();
			break;
		case 6:
			pop();
			pop();
			break;
		case 9:
			pop();
			pop();
			pop();
			pop();
			pop();
			break;
		case 13:
			pop();
			pop();
			pop();
			break;
		case 19: //HE99+
			pop();
			break;
		case 29:
			pop();
			break;
		case 118:
			pop();
			pop();
			break;
		case 160:
			break;
		case 198:
			break;
		default:
			error("o90_unknown9E: Unknown case %d", subOp);
	}
	debug(1,"o90_unknown9E stub (%d)", subOp);
}

} // End of namespace Scumm
