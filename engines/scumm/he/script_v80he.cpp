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
 */



#include "common/config-file.h"
#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/str.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/he/intern_he.h"
#include "scumm/object.h"
#include "scumm/resource.h"
#include "scumm/he/resource_he.h"
#include "scumm/scumm.h"
#include "scumm/he/sound_he.h"

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
		OPCODE(o80_createSound),
		OPCODE(o80_getFileSize),
		OPCODE(o6_wordArrayWrite),
		/* 48 */
		OPCODE(o80_stringToInt),
		OPCODE(o80_getSoundVar),
		OPCODE(o80_localizeArrayToRoom),
		OPCODE(o6_wordArrayIndexedWrite),
		/* 4C */
		OPCODE(o80_sourceDebug),
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
		OPCODE(o72_getSoundPosition),
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
		OPCODE(o6_invalid),
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
		OPCODE(o6_drawBox),
		OPCODE(o6_pop),
		/* A8 */
		OPCODE(o6_getActorWidth),
		OPCODE(o6_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter),
		/* AC */
		OPCODE(o80_drawWizPolygon),
		OPCODE(o6_isAnyOf),
		OPCODE(o72_systemOps),
		OPCODE(o6_isActorInBox),
		/* B0 */
		OPCODE(o6_delay),
		OPCODE(o6_delaySeconds),
		OPCODE(o6_delayMinutes),
		OPCODE(o6_stopSentence),
		/* B4 */
		OPCODE(o6_printLine),
		OPCODE(o6_printText),
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
		OPCODE(o71_kernelSetFunctions),
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
		OPCODE(o60_seekFilePos),
		OPCODE(o72_redimArray),
		OPCODE(o60_readFilePos),
		/* EC */
		OPCODE(o71_copyString),
		OPCODE(o71_getStringWidth),
		OPCODE(o70_getStringLen),
		OPCODE(o71_appendString),
		/* F0 */
		OPCODE(o71_concatString),
		OPCODE(o71_compareString),
		OPCODE(o70_isResourceLoaded),
		OPCODE(o72_readINI),
		/* F4 */
		OPCODE(o72_writeINI),
		OPCODE(o71_getStringLenForWidth),
		OPCODE(o71_getCharIndexInString),
		OPCODE(o71_findBox),
		/* F8 */
		OPCODE(o72_getResourceSize),
		OPCODE(o72_createDirectory),
		OPCODE(o72_setSystemMessage),
		OPCODE(o71_polygonOps),
		/* FC */
		OPCODE(o71_polygonHit),
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

void ScummEngine_v80he::o80_createSound() {
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 27:
		createSound(_heSndResId, pop());
		break;
	case 217:
		createSound(_heSndResId, -1);
		break;
	case 232:
		_heSndResId = pop();
		break;
	case 255:
		// dummy case
		break;
	default:
		error("o80_createSound: default case %d", subOp);
	}
}

void ScummEngine_v80he::o80_getFileSize() {
	byte filename[256];

	copyScriptString(filename, sizeof(filename));
	convertFilePath(filename);

	Common::SeekableReadStream *f = _saveFileMan->openForLoading((const char *)filename);
	if (!f) {
		Common::File *file = new Common::File();
		file->open((const char *)filename);
		if (!file->isOpen())
			delete f;
		else
			f = file;
	}

	if (!f) {
		push(-1);
	} else {
		push(f->size());
		delete f;
	}
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
	int var = pop();
	int snd = pop();
	push(((SoundHE *)_sound)->getSoundVar(snd, var));
}

void ScummEngine_v80he::o80_localizeArrayToRoom() {
	int slot = pop();
	localizeArray(slot, 0xFF);
}

void ScummEngine_v80he::o80_sourceDebug() {
	fetchScriptDWord();
	fetchScriptDWord();
}

void ScummEngine_v80he::o80_readConfigFile() {
	byte option[128], section[128], filename[256];
	byte *data;
	Common::String entry;
	int len, r;

	copyScriptString(option, sizeof(option));
	copyScriptString(section, sizeof(section));
	copyScriptString(filename, sizeof(filename));
	r = convertFilePath(filename);

	Common::ConfigFile ConfFile;
	if (!strcmp((char *)filename + r, "map.ini"))
		ConfFile.loadFromFile((const char *)filename + r);
	else
		ConfFile.loadFromSaveFile((const char *)filename + r);

	byte subOp = fetchScriptByte();

	switch (subOp) {
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
		data = defineArray(0, kStringArray, 0, 0, 0, len);
		memcpy(data, entry.c_str(), len);
		push(readVar(0));
		break;
	default:
		error("o80_readConfigFile: default type %d", subOp);
	}

	debug(1, "o80_readConfigFile: Filename %s Section %s Option %s Value %s", filename, section, option, entry.c_str());
}

void ScummEngine_v80he::o80_writeConfigFile() {
	byte filename[256], section[256], option[256], string[1024];
	int r, value;

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 43: // HE 100
	case 6: // number
		value = pop();
		sprintf((char *)string, "%d", value);
		copyScriptString(option, sizeof(option));
		copyScriptString(section, sizeof(section));
		copyScriptString(filename, sizeof(filename));
		break;
	case 77: // HE 100
	case 7: // string
		copyScriptString(string, sizeof(string));
		copyScriptString(option, sizeof(option));
		copyScriptString(section, sizeof(section));
		copyScriptString(filename, sizeof(filename));
		break;
	default:
		error("o80_writeConfigFile: default type %d", subOp);
	}

	r = convertFilePath(filename);

	Common::ConfigFile ConfFile;
	ConfFile.loadFromSaveFile((const char *)filename + r);
	ConfFile.setKey((char *)option, (char *)section, (char *)string);
	ConfFile.saveToSaveFile((const char *)filename + r);

	debug(1,"o80_writeConfigFile: Filename %s Section %s Option %s String %s", filename, section, option, string);
}

void ScummEngine_v80he::o80_cursorCommand() {
	int a, i;
	int args[16];

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 0x13:
	case 0x14:
		a = pop();
		_wiz->loadWizCursor(a);
		break;
	case 0x3C:
		pop();
		a = pop();
		_wiz->loadWizCursor(a);
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

void ScummEngine_v80he::o80_drawWizPolygon() {
	WizImage wi;
	wi.x1 = wi.y1 = pop();
	wi.resNum = pop();
	wi.state = 0;
	wi.flags = kWIFIsPolygon;
	_wiz->displayWizImage(&wi);
}

/**
 * Draw a 'line' between two points.
 *
 * @param x1	the starting x coordinate
 * @param y1	the starting y coordinate
 * @param x		the ending x coordinate
 * @param y		the ending y coordinate
 * @param step	the step size used to render the line, only ever 'step'th point is drawn
 * @param type	the line type -- points are rendered by drawing actors (type == 2),
 *              wiz images (type == 3), or pixels (any other type)
 * @param id	the id of an actor, wizimage or color (low bit) & flag (high bit)
 */
void ScummEngine_v80he::drawLine(int x1, int y1, int x, int y, int step, int type, int id) {
	if (step < 0) {
		step = -step;
	}
	if (step == 0) {
		step = 1;
	}

	const int dx = x - x1;
	const int dy = y - y1;

	const int absDX = ABS(dx);
	const int absDY = ABS(dy);

	const int maxDist = MAX(absDX, absDY);

	y = y1;
	x = x1;


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
		_wiz->displayWizImage(&wi);
	} else {
		drawPixel(x, y, id);
	}

	int stepCount = 0;
	int tmpX = 0;
	int tmpY = 0;
	for (int i = 0; i <= maxDist; i++) {
		tmpX += absDX;
		tmpY += absDY;

		int drawFlag = 0;

		if (tmpX > maxDist) {
			drawFlag = 1;
			tmpX -= maxDist;

			if (dx >= 0) {
				x++;
			} else {
				x--;
			}
		}
		if (tmpY > maxDist) {
			drawFlag = dy;
			tmpY -= maxDist;

			if (dy >= 0) {
				y++;
			} else {
				y--;
			}
		}

		if (drawFlag == 0)
			continue;

		if ((stepCount++ % step) != 0 && maxDist != i)
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
			_wiz->displayWizImage(&wi);
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

	if ((flags & 0x4000) || (flags & 0x2000000)) {
		src = vs->getPixels(x, y);
		dst = vs->getBackPixels(x, y);
		*dst = *src;
	} else if ((flags & 0x2000) || (flags & 4000000)) {
		src = vs->getBackPixels(x, y);
		dst = vs->getPixels(x, y);
		*dst = *src;
	} else if (flags & 0x8000000) {
		error("drawPixel: unsupported flag 0x%x", flags);
	} else {
		dst = vs->getPixels(x, y);
		*dst = flags;
		if ((flags & 0x8000) || (flags & 0x1000000)) {
			dst = vs->getBackPixels(x, y);
			*dst = flags;
		}
	}
}

void ScummEngine_v80he::o80_drawLine() {
	int id, step, x, y, x1, y1;

	step = pop();
	id = pop();
	y = pop();
	x = pop();
	y1 = pop();
	x1 = pop();

	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 55:
		drawLine(x1, y1, x, y, step, 2, id);
		break;
	case 63:
		drawLine(x1, y1, x, y, step, 3, id);
		break;
	case 66:
		drawLine(x1, y1, x, y, step, 1, id);
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
		int32 var_2 = readArray(value, 0, num - 1);
		shuffleArray(value, 1, dim1end);
		num = 1;
		if (readArray(value, 0, 1) == var_2 && dim1end >= 3) {
			int32 tmp = readArray(value, 0, 2);
			writeArray(value, 0, num, tmp);
			writeArray(value, 0, 2, var_2);
		}
	}

	writeArray(value, 0, 0, num + 1);
	push(readArray(value, 0, num));
}

} // End of namespace Scumm
