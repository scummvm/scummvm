/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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

#include "common/config-file.h"
#include "common/config-manager.h"
#include "common/str.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/intern.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/resource_v7he.h"
#include "scumm/scumm.h"
#include "scumm/sound.h"
#include "scumm/util.h"

namespace Scumm {

#define OPCODE(x)	_OPCODE(ScummEngine_v80he, x)

void ScummEngine_v80he::setupOpcodes() {
	static const OpcodeEntryV80he opcodes[256] = {
		/* 00 */
		OPCODE(o6_pushByte),
		OPCODE(o6_pushWord),
		OPCODE(o72_pushDWord),
		OPCODE(o6_pushWordVar),
		/* 04 */
		OPCODE(o72_getScriptString),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayRead),
		/* 08 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_wordArrayIndexedRead),
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
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 20 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 24 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 28 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 2C */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 30 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 34 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* 38 */
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
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
		OPCODE(o80_loadSBNG),
		OPCODE(o80_getFileSize),
		OPCODE(o6_wordArrayWrite),
		/* 48 */
		OPCODE(o80_stringToInt),
		OPCODE(o80_getSoundVar),
		OPCODE(o80_localizeArrayToRoom),
		OPCODE(o6_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o6_invalid),
		OPCODE(o80_readConfigFile),
		OPCODE(o80_writeConfigFile),
		OPCODE(o6_wordVarInc),
		/* 50 */
		OPCODE(o72_resetCutscene),
		OPCODE(o6_invalid),
		OPCODE(o72_findObjectWithClassOf),
		OPCODE(o6_wordArrayInc),
		/* 54 */
		OPCODE(o72_getObjectImageX),
		OPCODE(o72_getObjectImageY),
		OPCODE(o72_captureWizImage),
		OPCODE(o6_wordVarDec),
		/* 58 */
		OPCODE(o72_getTimer),
		OPCODE(o72_setTimer),
		OPCODE(o72_getSoundElapsedTime),
		OPCODE(o6_wordArrayDec),
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
		OPCODE(o6_invalid),
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
		OPCODE(o70_startSound),
		OPCODE(o6_stopSound),
		OPCODE(o6_invalid),
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
		OPCODE(o70_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o70_getActorRoom),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		OPCODE(o6_getObjectOldDir),
		/* 90 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_findInventory),
		OPCODE(o6_getInventoryCount),
		/* 94 */
		OPCODE(o6_invalid),
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_setObjectName),
		/* 98 */
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_setBoxFlags),
		OPCODE(o6_invalid),
		OPCODE(o70_resourceRoutines),
		/* 9C */
		OPCODE(o72_roomOps),
		OPCODE(o72_actorOps),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorFromXY),
		/* A0 */
		OPCODE(o72_findObject),
		OPCODE(o6_pseudoRoom),
		OPCODE(o6_getActorElevation),
		OPCODE(o6_getVerbEntrypoint),
		/* A4 */
		OPCODE(o72_arrayOps),
		OPCODE(o6_invalid),
		OPCODE(o80_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o60_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o80_drawWizPolygon),
		OPCODE(o6_isAnyOf),
		OPCODE(o70_quitPauseRestart),
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
		OPCODE(o72_talkActor),
		OPCODE(o72_talkEgo),
		/* BC */
		OPCODE(o72_dimArray),
		OPCODE(o6_stopObjectCode),
		OPCODE(o6_startObjectQuick),
		OPCODE(o6_startScriptQuick2),
		/* C0 */
		OPCODE(o72_dim2dimArray),
		OPCODE(o72_traceStatus),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* C4 */
		OPCODE(o6_abs),
		OPCODE(o6_distObjectObject),
		OPCODE(o6_distObjectPt),
		OPCODE(o6_distPtPt),
		/* C8 */
		OPCODE(o72_kernelGetFunctions),
		OPCODE(o70_kernelSetFunctions),
		OPCODE(o6_delayFrames),
		OPCODE(o6_pickOneOf),
		/* CC */
		OPCODE(o6_pickOneOfDefault),
		OPCODE(o6_stampObject),
		OPCODE(o72_drawWizImage),
		OPCODE(o72_debugInput),
		/* D0 */
		OPCODE(o6_getDateTime),
		OPCODE(o6_stopTalking),
		OPCODE(o6_getAnimateVariable),
		OPCODE(o6_invalid),
		/* D4 */
		OPCODE(o6_shuffle),
		OPCODE(o72_jumpToScript),
		OPCODE(o6_band),
		OPCODE(o6_bor),
		/* D8 */
		OPCODE(o6_isRoomScriptRunning),
		OPCODE(o60_closeFile),
		OPCODE(o72_openFile),
		OPCODE(o72_readFile),
		/* DC */
		OPCODE(o72_writeFile),
		OPCODE(o72_findAllObjects),
		OPCODE(o72_deleteFile),
		OPCODE(o72_rename),
		/* E0 */
		OPCODE(o80_drawLine),
		OPCODE(o72_getPixel),
		OPCODE(o60_localizeArrayToScript),
		OPCODE(o80_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o70_seekFilePos),
		OPCODE(o72_redimArray),
		OPCODE(o60_readFilePos),
		/* EC */
		OPCODE(o70_copyString),
		OPCODE(o70_getStringWidth),
		OPCODE(o70_getStringLen),
		OPCODE(o70_appendString),
		/* F0 */
		OPCODE(o70_concatString),
		OPCODE(o70_compareString),
		OPCODE(o72_isResourceLoaded),
		OPCODE(o72_readINI),
		/* F4 */
		OPCODE(o72_writeINI),
		OPCODE(o70_getStringLenForWidth),
		OPCODE(o70_getCharIndexInString),
		OPCODE(o6_invalid),
		/* F8 */
		OPCODE(o80_getResourceSize),
		OPCODE(o72_setFilePath),
		OPCODE(o72_setWindowCaption),
		OPCODE(o70_polygonOps),
		/* FC */
		OPCODE(o70_polygonHit),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
	};

	_opcodesV80he = opcodes;
}

void ScummEngine_v80he::executeOpcode(byte i) {
	OpcodeProcV80he op = _opcodesV80he[i].proc;
	(this->*op) ();
}

const char *ScummEngine_v80he::getOpcodeDesc(byte i) {
	return _opcodesV80he[i].desc;
}

void ScummEngine_v80he::o80_loadSBNG() {
	// Loads SBNG sound resource
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 27:
		//loadSBNG(_heSBNGId, pop();
		pop();
		break;
	case 217:
		//loadSBNG(_heSBNGId, -1);
		break;
	case 232:
		_heSBNGId = pop();
		break;
	case 255:
		// dummy case
		break;
	default:
		error("o80_loadSBNG: default case %d", subOp);
	}
	debug(1,"o80_loadSBNG stub (%d)",subOp);
}

void ScummEngine_v80he::o80_getFileSize() {
	byte filename[256];

	copyScriptString(filename, sizeof(filename));

	File f;
	if (f.open((char *)filename) == false) {
		push(-1);
		return;
	}

	f.seek(0, SEEK_END);
	push(f.pos());
	f.close();
}

void ScummEngine_v80he::o80_stringToInt() {
	int id, len, val;
	byte *addr;
	char string[100];

	id = pop();

	addr = getStringAddress(id);
	if (!addr)
		error("o80_stringToInt: Reference to zeroed array pointer (%d)", id);

	len = resStrLen(getStringAddress(id)) + 1;
	memcpy(string, addr, len);
	val = atoi(string);
	push(val);
}

void ScummEngine_v80he::o80_getSoundVar() {
	// Checks sound variable
	int var = pop();
	int snd = pop();
	int rnd = _rnd.getRandomNumberRng(1, 3);

	checkRange(27, 0, var, "Illegal sound variable %d");
	push (rnd);
	debug(1,"o80_getSoundVar stub (snd %d, var %d)", snd, var);
}

void ScummEngine_v80he::o80_localizeArrayToRoom() {
	int slot = pop();
	localizeArray(slot, 0xFF);
}

void ScummEngine_v80he::o80_readConfigFile() {
	byte option[128], section[128], filename[256];
	ArrayHeader *ah;
	Common::String entry;
	int len;

	copyScriptString(option, sizeof(option));
	copyScriptString(section, sizeof(section));
	copyScriptString(filename, sizeof(filename));
	convertFilePath(filename, true);

	Common::ConfigFile ConfFile;
	ConfFile.loadFromFile((const char *)filename);

	byte type = fetchScriptByte();
	switch (type) {
	case 43: // HE 100
	case 6: // number
		ConfFile.getKey((const char *)option, (const char *)section, entry);

		push(atoi(entry.c_str()));
		break;
	case 77: // HE 100
	case 7: // string
		ConfFile.getKey((const char *)option, (const char *)section, entry);

		writeVar(0, 0);
		len = resStrLen((const byte *)entry.c_str());
		ah = defineArray(0, kStringArray, 0, 0, 0, len);
		memcpy(ah->data, entry.c_str(), len);
		push(readVar(0));
		break;
	default:
		error("o80_readConfigFile: default type %d", type);
	}

	debug(0, "o80_readConfigFile: Filename %s Section %s Option %s Value %s", filename, section, option, entry.c_str());
}

void ScummEngine_v80he::o80_writeConfigFile() {
	byte filename[256], section[256], option[256], string[1024];
	int value;

	byte type = fetchScriptByte();
	switch (type) {
	case 43: // HE 100
	case 6: // number
		value = pop();
		sprintf((char *)string, "%d", value);
		copyScriptString(option, sizeof(option));
		copyScriptString(section, sizeof(section));
		copyScriptString(filename, sizeof(filename));
		convertFilePath(filename, true);
		break;
	case 77: // HE 100
	case 7: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));
		copyScriptString(section, sizeof(section));
		copyScriptString(filename, sizeof(filename));
		convertFilePath(filename, true);
		break;
	default:
		error("o80_writeConfigFile: default type %d", type);
	}

	Common::ConfigFile ConfFile;
	ConfFile.loadFromFile((const char *)filename);
	ConfFile.setKey((char *)option, (char *)section, (char *)string); 
	ConfFile.saveToFile((const char *)filename);
	debug(0,"o80_writeConfigFile: Filename %s Section %s Option %s String %s", filename, section, option, string);
}

void ScummEngine_v80he::o80_cursorCommand() {
	int a, i;
	int args[16];
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0x13:
	case 0x14:
		a = pop();
		loadWizCursor(a);
		break;
	case 0x3C:
		pop();
		a = pop();
		loadWizCursor(a);
		break;
	case 0x90:		// SO_CURSOR_ON Turn cursor on
		_cursor.state = 1;
		break;
	case 0x91:		// SO_CURSOR_OFF Turn cursor off
		_cursor.state = 0;
		break;
	case 0x92:		// SO_USERPUT_ON
		_userPut = 1;
		break;
	case 0x93:		// SO_USERPUT_OFF
		_userPut = 0;
		break;
	case 0x94:		// SO_CURSOR_SOFT_ON Turn soft cursor on
		_cursor.state++;
		if (_cursor.state > 1)
			error("Cursor state greater than 1 in script");
		break;
	case 0x95:		// SO_CURSOR_SOFT_OFF Turn soft cursor off
		_cursor.state--;
		break;
	case 0x96:		// SO_USERPUT_SOFT_ON
		_userPut++;
		break;
	case 0x97:		// SO_USERPUT_SOFT_OFF
		_userPut--;
		break;
	case 0x9C:		// SO_CHARSET_SET
		initCharset(pop());
		break;
	case 0x9D:		// SO_CHARSET_COLOR
		getStackList(args, ARRAYSIZE(args));
		for (i = 0; i < 16; i++)
			_charsetColorMap[i] = _charsetData[_string[1]._default.charset][i] = (unsigned char)args[i];
		break;
	default:
		error("o80_cursorCommand: default case %x", subOp);
	}

	VAR(VAR_CURSORSTATE) = _cursor.state;
	VAR(VAR_USERPUT) = _userPut;
}

void ScummEngine_v80he::o80_setState() {
	int state = pop();
	int obj = pop();

	state &= 0x7FFF;
	putState(obj, state);
	removeObjectFromDrawQue(obj);
}

void ScummEngine_v80he::o80_drawBox() {
	int x, y, x2, y2, color;
	color = pop();
	y2 = pop();
	x2 = pop();
	y = pop();
	x = pop();

	if (color & 0x8000)
		color &= 0x7FFF;

	drawBox(x, y, x2, y2, color);
}

void ScummEngine_v80he::o80_drawWizPolygon() {
	WizImage wi;
	wi.x1 = wi.y1 = pop();
	wi.resNum = pop();
	wi.state = 0;
	wi.flags = kWIFIsPolygon;
	displayWizImage(&wi);	
}

void ScummEngine_v80he::drawLine(int x1, int y1, int x, int unk1, int unk2, int type, int id) {
	debug(0,"drawLine: x1 %d y1 %d x %d unk1 %d, unk2 %d type %d id %d", x1, y1, x, unk1, unk2, type, id);	

	int eax, ebx, ecx, edx, esi;
	int var_4, var_8, var_C, y;

	if (unk2 < 0) {
		unk2 = -unk2;
	}
	if (unk2 == 0) {
		unk2 = 1;
	}

	var_4 = unk1 - y1;
	var_8 = x - x1;

	x = x1;
	y = var_4;

	esi = var_8;
	if (var_4 > var_8) {
		esi = var_4;
	}

	if (type == 2) {
		Actor *a = derefActor(id, "drawLine");
		a->drawActorToBackBuf(x1, y1);
	} else if (type == 3) {
		WizImage wi;
		wi.flags = 0;
		wi.y1 = y1;
		wi.x1 = x1;
		wi.resNum = id;
		wi.state = 0;
		displayWizImage(&wi);
	} else {
		drawPixel(x1, y1, id);
	}

	x1 = 0;
	ebx = 0;
	edx = 0;
	var_C = 0;
	for (int i = 0; i <= esi; i++) {
		ecx = x1;
		eax = y;	
		ebx += var_8;
		ecx += eax;

		eax ^= eax;
		x1 = ecx;

		if (ebx > esi) {
			edx = var_8;
			edx -= esi;

			eax = 1;
			if (edx >= 0) {
				x++;
			} else {
				x--;
			}
		}

		if (ecx > esi) {
			eax = var_4;
			x1 -= esi;

			if (eax >= 0) {
				y++;
			} else {
				y--;
			}
		} else {
			if (eax == 0)
				continue;
		}

		ecx = var_C;
		eax = ecx;
		eax /= unk2;
		ecx++;
		var_C = ecx;

		if (edx != 0 && esi != i)
			continue;

		if (type == 2) {
			Actor *a = derefActor(id, "drawLine");
			a->drawActorToBackBuf(x, y);
		} else if (type == 3) {
			WizImage wi;
			wi.flags = 0;
			wi.y1 = y;
			wi.x1 = x;
			wi.resNum = id;
			wi.state = 0;
			displayWizImage(&wi);
		} else {
			drawPixel(x, y, id);
		}
	}		
}

void ScummEngine_v80he::drawPixel(int x, int y, int flags) {
	byte *src, *dst;
	VirtScreen *vs;

	if (x < 0 || x > 639)
		return;

	if (y < 0)
		return;

	if ((vs = findVirtScreen(y)) == NULL)
		return;

	markRectAsDirty(vs->number, x, y, x, y + 1);

	if (flags & 0x4000) {
		src = vs->getPixels(x, y);
		dst = vs->getBackPixels(x, y);
		*dst = *src;
	} else if (flags & 0x2000) {
		src = vs->getBackPixels(x, y);
		dst = vs->getPixels(x, y);
		*dst = *src;
	} else {
		dst = vs->getPixels(x, y);
		*dst = flags;
		if (flags & 0x8000) {
			dst = vs->getBackPixels(x, y);
			*dst = flags;
		}
	}
}

void ScummEngine_v80he::o80_drawLine() {
	int id, unk1, unk2, x, x1, y1;

	unk2 = pop();
	id = pop();
	unk1 = pop();
	x = pop();
	y1 = pop();
	x1 = pop();

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 55:
		drawLine(x1, y1, x, unk1, unk2, 2, id);
		break;
	case 63:
		drawLine(x1, y1, x, unk1, unk2, 3, id);
		break;
	case 66:
		drawLine(x1, y1, x, unk1, unk2, 1, id);
		break;
	default:
		error("o80_drawLine: default case %d", subOp);
	}

}

void ScummEngine_v80he::o80_pickVarRandom() {
	int num;
	int args[100];
	int32 dim1end;

	num = getStackList(args, ARRAYSIZE(args));
	int value = fetchScriptWord();

	if (readVar(value) == 0) {
		defineArray(value, kDwordArray, 0, 0, 0, num);
		if (value & 0x8000)
			localizeArray(readVar(value), 0xFF);
		else if (value & 0x4000)
			localizeArray(readVar(value), _currentScript);

		if (num > 0) {
			int16 counter = 0;
			do {
				writeArray(value, 0, counter + 1, args[counter]);
			} while (++counter < num);
		}

		shuffleArray(value, 1, num);
		writeArray(value, 0, 0, 2);
		push(readArray(value, 0, 1));
		return;
	}

	num = readArray(value, 0, 0);

	ArrayHeader *ah = (ArrayHeader *)getResourceAddress(rtString, readVar(value));
	dim1end = FROM_LE_32(ah->dim1end);

	if (dim1end < num) {
		int16 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, dim1end);
		num = 1;
		if (readArray(value, 0, 1) == var_2 && dim1end >= 3) {
			int16 tmp = readArray(value, 0, 2);
			writeArray(value, 0, num, tmp);
			writeArray(value, 0, 2, var_2);
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num));
}

void ScummEngine_v80he::o80_getResourceSize() {
	const byte *ptr;
	int size, type;

	int resid = pop();
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 13:
		push (getSoundResourceSize(resid));
		return;
	case 14:
		type = rtRoomImage;
		break;
	case 15:
		type = rtImage;
		break;
	case 16:
		type = rtCostume;
		break;
	case 17:
		type = rtScript;
		break;
	default:
		error("o80_getResourceSize: default type %d", subOp);
	}

	ptr = getResourceAddress(type, resid);
	assert(ptr);
	size = READ_BE_UINT32(ptr + 4) - 8;
	push(size);
}

} // End of namespace Scumm
