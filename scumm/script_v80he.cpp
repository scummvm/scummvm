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

#define OPCODE(x)	{ &ScummEngine_v80he::x, #x }

void ScummEngine_v80he::setupOpcodes() {
	static const OpcodeEntryV80he opcodes[256] = {
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
		OPCODE(o70he_startSound),
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
		OPCODE(o70he_pickupObject),
		OPCODE(o6_loadRoomWithEgo),
		OPCODE(o6_invalid),
		OPCODE(o6_getRandomNumber),
		/* 88 */
		OPCODE(o6_getRandomNumberRange),
		OPCODE(o6_invalid),
		OPCODE(o6_getActorMoving),
		OPCODE(o6_isScriptRunning),
		/* 8C */
		OPCODE(o70he_getActorRoom),
		OPCODE(o6_getObjectX),
		OPCODE(o6_getObjectY),
		OPCODE(o6_getObjectOldDir),
		/* 90 */
		OPCODE(o6_getActorWalkBox),
		OPCODE(o6_getActorCostume),
		OPCODE(o6_findInventory),
		OPCODE(o6_getInventoryCount),
		/* 94 */
		OPCODE(o6_getVerbFromXY),
		OPCODE(o6_beginOverride),
		OPCODE(o6_endOverride),
		OPCODE(o6_setObjectName),
		/* 98 */
		OPCODE(o6_isSoundRunning),
		OPCODE(o6_setBoxFlags),
		OPCODE(o6_invalid),
		OPCODE(o70he_resourceRoutines),
		/* 9C */
		OPCODE(o6he_roomOps),
		OPCODE(o72_actorOps),
		OPCODE(o72_verbOps),
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
		OPCODE(o6he_wait),
		OPCODE(o6_getActorScaleX),
		OPCODE(o6_getActorAnimCounter1),
		/* AC */
		OPCODE(o80_drawWizPolygon),
		OPCODE(o6_isAnyOf),
		OPCODE(o70he_quitPauseRestart),
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
		OPCODE(o6he_kernelGetFunctions),
		OPCODE(o70he_kernelSetFunctions),
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
		OPCODE(o6he_closeFile),
		OPCODE(o72_openFile),
		OPCODE(o72_readFile),
		/* DC */
		OPCODE(o72_writeFile),
		OPCODE(o72_findAllObjects),
		OPCODE(o72_deleteFile),
		OPCODE(o6he_rename),
		/* E0 */
		OPCODE(o6_invalid),
		OPCODE(o72_getPixel),
		OPCODE(o6he_localizeArray),
		OPCODE(o80_pickVarRandom),
		/* E4 */
		OPCODE(o6_setBoxSet),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		OPCODE(o6_invalid),
		/* E8 */
		OPCODE(o6_invalid),
		OPCODE(o6he_seekFilePos),
		OPCODE(o72_redimArray),
		OPCODE(o6he_readFilePos),
		/* EC */
		OPCODE(o72_unknownEC),
		OPCODE(o72_unknownED),
		OPCODE(o70he_stringLen),
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
		OPCODE(o70he_polygonOps),
		/* FC */
		OPCODE(o70he_polygonHit),
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

void ScummEngine_v80he::o80_unknown45() {
	// Sound related
	byte subOp = fetchScriptByte();

	switch (subOp) {
	case 27:
		pop();
		break;
	case 217:
		break;
	case 232:
		_heSndSoundId = pop();
		break;
	case 255:
		//Case doesn't match disasm.
		_sound->addSoundToQueue(_heSndSoundId);
		break;
	default:
		warning("o80_unknown45: default type %d", subOp);
	}
	debug(1,"o80_unknown45 stub (%d)",subOp);
}

void ScummEngine_v80he::o80_unknown48() {
	int id, len, val;
	byte *addr;
	char string[100];

	id = pop();

	addr = getStringAddress(id);
	if (!addr)
		error("o80_unknown48: Reference to zeroed array pointer (%d)", id);

	len = resStrLen(getStringAddress(id)) + 1;
	memcpy(string, addr, len);
	val = atoi(string);
	push(val);
}

void ScummEngine_v80he::o80_unknown49() {
	// Sound related
	int result = 0;
	int subOp = pop();
	int snd = pop();

	switch (subOp) {
	case 0:
		result = !_sound->isSoundRunning(snd);
		break;
	case 1:
		result = _sound->isSoundRunning(snd); 
		break;
	default:
		warning("o80_unknown49: default type %d", subOp);
	}

	push (result);
	debug(1,"o80_unknown49 stub (%d, %d)", subOp, snd);
}

void ScummEngine_v80he::o80_unknown4A() {
	int slot = pop();
	localizeArray(slot, 0xFFFFFFFF);
}

void ScummEngine_v80he::o80_readConfigFile() {
	byte name[128], section[128], filename[256];
	int type, retval;

	// we pretend that we don't have .ini file
	copyScriptString(section);
	copyScriptString(name);
	copyScriptString(filename);
	type = fetchScriptByte();

	switch (type) {
	case 6: // number
		push(0);
		break;
	case 7: // string
		writeVar(0, 0);
		defineArray(0, kStringArray, 0, 0, 0, 0);
		retval = readVar(0);
		writeArray(0, 0, 0, 0);
		push(retval); // var ID string
		break;
	default:
		error("o80_readConfigFile: default type %d", type);
	}
	debug(1, "o80_readConfigFile: Filename %s Section %s Name %s", filename, section, name);
}

void ScummEngine_v80he::o80_writeConfigFile() {
	byte filename[256], section[256], name[256], string[1024];
	int type, value;

	// we pretend that we don't have .ini file
	type = fetchScriptByte();

	switch (type) {
	case 6: // number
		value = pop();
		copyScriptString(section);
		copyScriptString(name);
		copyScriptString(filename);
		debug(1,"o80_writeConfigFile: Filename %s Section %s Name %s Value %d", filename, section, name, value);
		break;
	case 7: // string
		copyScriptString(string);
		copyScriptString(section);
		copyScriptString(name);
		copyScriptString(filename);
		debug(1,"o80_writeConfigFile: Filename %s Section %s Name %s String %s", filename, section, name, string);
		break;
	default:
		error("o80_writeConfigFile: default type %d", type);
	}
}

void ScummEngine_v80he::o80_cursorCommand() {
	int a, i;
	int args[16];
	int subOp = fetchScriptByte();

	switch (subOp) {
	case 0x13: // Loads cursors from another resource
		a = pop();
		loadWizCursor(a, rtInventory, 0);
		break;
	case 0x14:
		a = pop();
		loadWizCursor(a, rtInventory, 1);
		break;
	case 0x90:		// SO_CURSOR_ON Turn cursor on
		_cursor.state = 1;
		verbMouseOver(0);
		break;
	case 0x91:		// SO_CURSOR_OFF Turn cursor off
		_cursor.state = 0;
		verbMouseOver(0);
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
	case 0x99: 		// SO_CURSOR_IMAGE Set cursor image
		warning("o80_cursorCommand: Can't set cursors to ID. Use images.");
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

void ScummEngine_v80he::loadImgSpot(int resId, int state, Common::Point &spot) {
	const uint8 *dataPtr = getResourceAddress(rtImage, resId);
	if (!dataPtr)
		error("loadImgSpot: unknown Image %d", resId);

	const uint8 *spotPtr = findWrappedBlock(MKID('SPOT'), dataPtr, state, 0);

	if (!spotPtr) {
		spot.x = spot.y = 0;
	} else {
		spot.x = (int16)READ_LE_UINT32(spotPtr + 0);
		spot.y = (int16)READ_LE_UINT32(spotPtr + 4);
	}
}

void ScummEngine_v80he::loadWizCursor(int resId, int resType, bool state) {
	Common::Rect rc;
	Common::Point spot;

	loadImgSpot(resId, 0, spot);

	rc.top = spot.x;
	rc.right = spot.y;

	rc.top = MAX((int)rc.top, 0);
	rc.right = MAX((int)rc.right, 0);
	rc.top = MIN((int)rc.top, 32);
	rc.right = MIN((int)rc.right, 32);

	// FIXME: dirty hack. Cursor is set in drawWizImage, though should be set from here
	// it is unclear how height and width are passed from drawWizImage
	drawWizImage(rtImage, resId, 0, 0, 0, 0x20);
	setCursorHotspot(rc.top, rc.right);
}

void ScummEngine_v80he::o80_setState() {
	int state = pop();
	int obj = pop();

	state &= 0x7FFF;
	putState(obj, state);
	removeObjectFromDrawQue(obj);
}

void ScummEngine_v80he::o80_drawWizPolygon() {
	error("o80_drawWizPolygon");

	int xy1 = pop();
	int resnum = pop();

	if (_fullRedraw) {
		assert(_wizImagesNum < ARRAYSIZE(_wizImages));
		WizImage *pwi = &_wizImages[_wizImagesNum];
		pwi->resnum = resnum;
		pwi->x1 = xy1;
		pwi->y1 = xy1;
		pwi->flags = 64;
		++_wizImagesNum;
	} else {
		drawWizImage(rtImage, resnum, 0, xy1, xy1, 64);
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
			localizeArray(readVar(value), 0xFFFFFFFF);
		else if (value & 0x4000)
			localizeArray(readVar(value), vm.slot[_currentScript].number);

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

} // End of namespace Scumm
