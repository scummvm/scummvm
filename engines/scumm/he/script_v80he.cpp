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

#define OPCODE(i, x)	_opcodes[i]._OPCODE(ScummEngine_v80he, x)

void ScummEngine_v80he::setupOpcodes() {
	/* 00 */
	OPCODE(0x00, o6_pushByte);
	OPCODE(0x01, o6_pushWord);
	OPCODE(0x02, o72_pushDWord);
	OPCODE(0x03, o6_pushWordVar);
	/* 04 */
	OPCODE(0x04, o72_getScriptString);
	OPCODE(0x07, o6_wordArrayRead);
	/* 08 */
	OPCODE(0x0b, o6_wordArrayIndexedRead);
	/* 0C */
	OPCODE(0x0c, o6_dup);
	OPCODE(0x0d, o6_not);
	OPCODE(0x0e, o6_eq);
	OPCODE(0x0f, o6_neq);
	/* 10 */
	OPCODE(0x10, o6_gt);
	OPCODE(0x11, o6_lt);
	OPCODE(0x12, o6_le);
	OPCODE(0x13, o6_ge);
	/* 14 */
	OPCODE(0x14, o6_add);
	OPCODE(0x15, o6_sub);
	OPCODE(0x16, o6_mul);
	OPCODE(0x17, o6_div);
	/* 18 */
	OPCODE(0x18, o6_land);
	OPCODE(0x19, o6_lor);
	OPCODE(0x1a, o6_pop);
	OPCODE(0x1b, o72_isAnyOf);
	/* 1C */
	/* 20 */
	/* 24 */
	/* 28 */
	/* 2C */
	/* 30 */
	/* 34 */
	/* 38 */
	/* 3C */
	/* 40 */
	OPCODE(0x43, o6_writeWordVar);
	/* 44 */
	OPCODE(0x45, o80_createSound);
	OPCODE(0x46, o80_getFileSize);
	OPCODE(0x47, o6_wordArrayWrite);
	/* 48 */
	OPCODE(0x48, o80_stringToInt);
	OPCODE(0x49, o80_getSoundVar);
	OPCODE(0x4a, o80_localizeArrayToRoom);
	OPCODE(0x4b, o6_wordArrayIndexedWrite);
	/* 4C */
	OPCODE(0x4c, o80_sourceDebug);
	OPCODE(0x4d, o80_readConfigFile);
	OPCODE(0x4e, o80_writeConfigFile);
	OPCODE(0x4f, o6_wordVarInc);
	/* 50 */
	OPCODE(0x50, o72_resetCutscene);
	OPCODE(0x52, o72_findObjectWithClassOf);
	OPCODE(0x53, o6_wordArrayInc);
	/* 54 */
	OPCODE(0x54, o72_getObjectImageX);
	OPCODE(0x55, o72_getObjectImageY);
	OPCODE(0x56, o72_captureWizImage);
	OPCODE(0x57, o6_wordVarDec);
	/* 58 */
	OPCODE(0x58, o72_getTimer);
	OPCODE(0x59, o72_setTimer);
	OPCODE(0x5a, o72_getSoundPosition);
	OPCODE(0x5b, o6_wordArrayDec);
	/* 5C */
	OPCODE(0x5c, o6_if);
	OPCODE(0x5d, o6_ifNot);
	OPCODE(0x5e, o72_startScript);
	OPCODE(0x5f, o6_startScriptQuick);
	/* 60 */
	OPCODE(0x60, o72_startObject);
	OPCODE(0x61, o72_drawObject);
	OPCODE(0x62, o72_printWizImage);
	OPCODE(0x63, o72_getArrayDimSize);
	/* 64 */
	OPCODE(0x64, o72_getNumFreeArrays);
	OPCODE(0x65, o6_stopObjectCode);
	OPCODE(0x66, o6_stopObjectCode);
	OPCODE(0x67, o6_endCutscene);
	/* 68 */
	OPCODE(0x68, o6_cutscene);
	OPCODE(0x6a, o6_freezeUnfreeze);
	OPCODE(0x6b, o80_cursorCommand);
	/* 6C */
	OPCODE(0x6c, o6_breakHere);
	OPCODE(0x6d, o6_ifClassOfIs);
	OPCODE(0x6e, o6_setClass);
	OPCODE(0x6f, o6_getState);
	/* 70 */
	OPCODE(0x70, o80_setState);
	OPCODE(0x71, o6_setOwner);
	OPCODE(0x72, o6_getOwner);
	OPCODE(0x73, o6_jump);
	/* 74 */
	OPCODE(0x74, o70_startSound);
	OPCODE(0x75, o6_stopSound);
	OPCODE(0x77, o6_stopObjectScript);
	/* 78 */
	OPCODE(0x78, o6_panCameraTo);
	OPCODE(0x79, o6_actorFollowCamera);
	OPCODE(0x7a, o6_setCameraAt);
	OPCODE(0x7b, o6_loadRoom);
	/* 7C */
	OPCODE(0x7c, o6_stopScript);
	OPCODE(0x7d, o6_walkActorToObj);
	OPCODE(0x7e, o6_walkActorTo);
	OPCODE(0x7f, o6_putActorAtXY);
	/* 80 */
	OPCODE(0x80, o6_putActorAtObject);
	OPCODE(0x81, o6_faceActor);
	OPCODE(0x82, o6_animateActor);
	OPCODE(0x83, o6_doSentence);
	/* 84 */
	OPCODE(0x84, o70_pickupObject);
	OPCODE(0x85, o6_loadRoomWithEgo);
	OPCODE(0x87, o6_getRandomNumber);
	/* 88 */
	OPCODE(0x88, o6_getRandomNumberRange);
	OPCODE(0x8a, o6_getActorMoving);
	OPCODE(0x8b, o6_isScriptRunning);
	/* 8C */
	OPCODE(0x8c, o70_getActorRoom);
	OPCODE(0x8d, o6_getObjectX);
	OPCODE(0x8e, o6_getObjectY);
	OPCODE(0x8f, o6_getObjectOldDir);
	/* 90 */
	OPCODE(0x90, o6_getActorWalkBox);
	OPCODE(0x91, o6_getActorCostume);
	OPCODE(0x92, o6_findInventory);
	OPCODE(0x93, o6_getInventoryCount);
	/* 94 */
	OPCODE(0x95, o6_beginOverride);
	OPCODE(0x96, o6_endOverride);
	/* 98 */
	OPCODE(0x98, o6_isSoundRunning);
	OPCODE(0x99, o6_setBoxFlags);
	OPCODE(0x9b, o70_resourceRoutines);
	/* 9C */
	OPCODE(0x9c, o72_roomOps);
	OPCODE(0x9d, o72_actorOps);
	OPCODE(0x9f, o6_getActorFromXY);
	/* A0 */
	OPCODE(0xa0, o72_findObject);
	OPCODE(0xa1, o6_pseudoRoom);
	OPCODE(0xa2, o6_getActorElevation);
	OPCODE(0xa3, o6_getVerbEntrypoint);
	/* A4 */
	OPCODE(0xa4, o72_arrayOps);
	OPCODE(0xa6, o6_drawBox);
	OPCODE(0xa7, o6_pop);
	/* A8 */
	OPCODE(0xa8, o6_getActorWidth);
	OPCODE(0xa9, o6_wait);
	OPCODE(0xaa, o6_getActorScaleX);
	OPCODE(0xab, o6_getActorAnimCounter);
	/* AC */
	OPCODE(0xac, o80_drawWizPolygon);
	OPCODE(0xad, o6_isAnyOf);
	OPCODE(0xae, o72_systemOps);
	OPCODE(0xaf, o6_isActorInBox);
	/* B0 */
	OPCODE(0xb0, o6_delay);
	OPCODE(0xb1, o6_delaySeconds);
	OPCODE(0xb2, o6_delayMinutes);
	OPCODE(0xb3, o6_stopSentence);
	/* B4 */
	OPCODE(0xb4, o6_printLine);
	OPCODE(0xb5, o6_printText);
	OPCODE(0xb6, o6_printDebug);
	OPCODE(0xb7, o6_printSystem);
	/* B8 */
	OPCODE(0xb8, o6_printActor);
	OPCODE(0xb9, o6_printEgo);
	OPCODE(0xba, o72_talkActor);
	OPCODE(0xbb, o72_talkEgo);
	/* BC */
	OPCODE(0xbc, o72_dimArray);
	OPCODE(0xbd, o6_stopObjectCode);
	OPCODE(0xbe, o6_startObjectQuick);
	OPCODE(0xbf, o6_startScriptQuick2);
	/* C0 */
	OPCODE(0xc0, o72_dim2dimArray);
	OPCODE(0xc1, o72_traceStatus);
	/* C4 */
	OPCODE(0xc4, o6_abs);
	OPCODE(0xc5, o6_distObjectObject);
	OPCODE(0xc6, o6_distObjectPt);
	OPCODE(0xc7, o6_distPtPt);
	/* C8 */
	OPCODE(0xc8, o72_kernelGetFunctions);
	OPCODE(0xc9, o71_kernelSetFunctions);
	OPCODE(0xca, o6_delayFrames);
	OPCODE(0xcb, o6_pickOneOf);
	/* CC */
	OPCODE(0xcc, o6_pickOneOfDefault);
	OPCODE(0xcd, o6_stampObject);
	OPCODE(0xce, o72_drawWizImage);
	OPCODE(0xcf, o72_debugInput);
	/* D0 */
	OPCODE(0xd0, o6_getDateTime);
	OPCODE(0xd1, o6_stopTalking);
	OPCODE(0xd2, o6_getAnimateVariable);
	/* D4 */
	OPCODE(0xd4, o6_shuffle);
	OPCODE(0xd5, o72_jumpToScript);
	OPCODE(0xd6, o6_band);
	OPCODE(0xd7, o6_bor);
	/* D8 */
	OPCODE(0xd8, o6_isRoomScriptRunning);
	OPCODE(0xd9, o60_closeFile);
	OPCODE(0xda, o72_openFile);
	OPCODE(0xdb, o72_readFile);
	/* DC */
	OPCODE(0xdc, o72_writeFile);
	OPCODE(0xdd, o72_findAllObjects);
	OPCODE(0xde, o72_deleteFile);
	OPCODE(0xdf, o72_rename);
	/* E0 */
	OPCODE(0xe0, o80_drawLine);
	OPCODE(0xe1, o72_getPixel);
	OPCODE(0xe2, o60_localizeArrayToScript);
	OPCODE(0xe3, o80_pickVarRandom);
	/* E4 */
	OPCODE(0xe4, o6_setBoxSet);
	/* E8 */
	OPCODE(0xe9, o60_seekFilePos);
	OPCODE(0xea, o72_redimArray);
	OPCODE(0xeb, o60_readFilePos);
	/* EC */
	OPCODE(0xec, o71_copyString);
	OPCODE(0xed, o71_getStringWidth);
	OPCODE(0xee, o70_getStringLen);
	OPCODE(0xef, o71_appendString);
	/* F0 */
	OPCODE(0xf0, o71_concatString);
	OPCODE(0xf1, o71_compareString);
	OPCODE(0xf2, o70_isResourceLoaded);
	OPCODE(0xf3, o72_readINI);
	/* F4 */
	OPCODE(0xf4, o72_writeINI);
	OPCODE(0xf5, o71_getStringLenForWidth);
	OPCODE(0xf6, o71_getCharIndexInString);
	OPCODE(0xf7, o71_findBox);
	/* F8 */
	OPCODE(0xf8, o72_getResourceSize);
	OPCODE(0xf9, o72_createDirectory);
	OPCODE(0xfa, o72_setSystemMessage);
	OPCODE(0xfb, o71_polygonOps);
	/* FC */
	OPCODE(0xfc, o71_polygonHit);
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
	byte buffer[256];

	copyScriptString(buffer, sizeof(buffer));
	const char *filename = (char *)buffer + convertFilePath(buffer);

	Common::SeekableReadStream *f = 0;
	if (!_saveFileMan->listSavefiles(filename).empty()) {
		f = _saveFileMan->openForLoading((const char *)filename);
	} else {
		Common::File *file = new Common::File();
		file->open((const char *)filename);
		if (!file->isOpen())
			delete file;
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

	if (_game.id == GID_TREASUREHUNT) {
		// WORKAROUND: Remove invalid characters
		if (!strcmp((char *)section, "Blue'sTreasureHunt-Disc1"))
			memcpy(section, "BluesTreasureHunt-Disc1\0", 24);
		else if (!strcmp((char *)section, "Blue'sTreasureHunt-Disc2"))
			memcpy(section, "BluesTreasureHunt-Disc2\0", 24);
	}

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

	if (_game.id == GID_TREASUREHUNT) {
		// WORKAROUND: Remove invalid characters
		if (!strcmp((char *)section, "Blue'sTreasureHunt-Disc1"))
			memcpy(section, "BluesTreasureHunt-Disc1\0", 24);
		else if (!strcmp((char *)section, "Blue'sTreasureHunt-Disc2"))
			memcpy(section, "BluesTreasureHunt-Disc2\0", 24);
	}

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
		ActorHE *a = (ActorHE *)derefActor(id, "drawLine");
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
			ActorHE *a = (ActorHE *)derefActor(id, "drawLine");
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
