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

#include "common/endian.h"
#include "common/file.h"
#include "graphics/dither.h"

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/helper.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/parse.h"
#include "gob/draw.h"
#include "gob/sound/sound.h"
#include "gob/videoplayer.h"

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_v6, x)

const int Inter_v6::_goblinFuncLookUp[][2] = {
	{0, 0},
};

Inter_v6::Inter_v6(GobEngine *vm) : Inter_v5(vm) {
	_gotFirstPalette = false;

	setupOpcodes();
}

void Inter_v6::setupOpcodes() {
	static const OpcodeDrawEntryV6 opcodesDraw[256] = {
		/* 00 */
		OPCODE(o1_loadMult),
		OPCODE(o2_playMult),
		OPCODE(o2_freeMultKeys),
		{0, ""},
		/* 04 */
		{0, ""},
		{0, ""},
		{0, ""},
		OPCODE(o1_initCursor),
		/* 08 */
		OPCODE(o1_initCursorAnim),
		OPCODE(o1_clearCursorAnim),
		OPCODE(o2_setRenderFlags),
		{0, ""},
		/* 0C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 10 */
		OPCODE(o1_loadAnim),
		OPCODE(o1_freeAnim),
		OPCODE(o1_updateAnim),
		OPCODE(o2_multSub),
		/* 14 */
		OPCODE(o2_initMult),
		OPCODE(o1_freeMult),
		OPCODE(o1_animate),
		OPCODE(o2_loadMultObject),
		/* 18 */
		OPCODE(o1_getAnimLayerInfo),
		OPCODE(o1_getObjAnimSize),
		OPCODE(o1_loadStatic),
		OPCODE(o1_freeStatic),
		/* 1C */
		OPCODE(o2_renderStatic),
		OPCODE(o2_loadCurLayer),
		{0, ""},
		{0, ""},
		/* 20 */
		OPCODE(o2_playCDTrack),
		OPCODE(o2_waitCDTrackEnd),
		OPCODE(o2_stopCD),
		OPCODE(o2_readLIC),
		/* 24 */
		OPCODE(o2_freeLIC),
		OPCODE(o2_getCDTrackPos),
		{0, ""},
		{0, ""},
		/* 28 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 2C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 30 */
		OPCODE(o2_loadFontToSprite),
		OPCODE(o1_freeFontToSprite),
		{0, ""},
		{0, ""},
		/* 34 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 38 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 3C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 40 */
		OPCODE(o6_totSub),
		OPCODE(o2_switchTotSub),
		OPCODE(o2_pushVars),
		OPCODE(o2_popVars),
		/* 44 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 48 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 4C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 50 */
		OPCODE(o2_loadMapObjects),
		OPCODE(o2_freeGoblins),
		OPCODE(o2_moveGoblin),
		OPCODE(o2_writeGoblinPos),
		/* 54 */
		OPCODE(o2_stopGoblin),
		OPCODE(o2_setGoblinState),
		OPCODE(o2_placeGoblin),
		{0, ""},
		/* 58 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 5C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 60 */
		{0, ""},
		OPCODE(o5_deleteFile),
		{0, ""},
		{0, ""},
		/* 64 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 68 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 6C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 70 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 74 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 78 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 7C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 80 */
		OPCODE(o5_initScreen),
		OPCODE(o2_scroll),
		OPCODE(o2_setScrollOffset),
		OPCODE(o6_playVmdOrMusic),
		/* 84 */
		OPCODE(o2_getImdInfo),
		OPCODE(o6_openItk),
		OPCODE(o2_closeItk),
		OPCODE(o2_setImdFrontSurf),
		/* 88 */
		OPCODE(o2_resetImdFrontSurf),
		{0, ""},
		{0, ""},
		{0, ""},
		/* 8C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 90 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 94 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 98 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 9C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* A0 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* A4 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* A8 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* AC */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* B0 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* B4 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* B8 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* BC */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* C0 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* C4 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* C8 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* CC */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* D0 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* D4 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* D8 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* DC */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* E0 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* E4 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* E8 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* EC */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* F0 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* F4 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* F8 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* FC */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""}
	};

	static const OpcodeFuncEntryV6 opcodesFunc[80] = {
		/* 00 */
		OPCODE(o1_callSub),
		OPCODE(o1_callSub),
		OPCODE(o1_printTotText),
		OPCODE(o6_loadCursor),
		/* 04 */
		{0, ""},
		OPCODE(o1_switch),
		OPCODE(o1_repeatUntil),
		OPCODE(o1_whileDo),
		/* 08 */
		OPCODE(o1_if),
		OPCODE(o6_assign),
		OPCODE(o1_loadSpriteToPos),
		{0, ""},
		/* 0C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 10 */
		{0, ""},
		OPCODE(o2_printText),
		OPCODE(o1_loadTot),
		OPCODE(o6_palLoad),
		/* 14 */
		OPCODE(o1_keyFunc),
		OPCODE(o1_capturePush),
		OPCODE(o1_capturePop),
		OPCODE(o2_animPalInit),
		/* 18 */
		OPCODE(o2_addCollision),
		OPCODE(o6_freeCollision),
		OPCODE(o3_getTotTextItemPart),
		{0, ""},
		/* 1C */
		{0, ""},
		{0, ""},
		OPCODE(o1_drawOperations),
		OPCODE(o1_setcmdCount),
		/* 20 */
		OPCODE(o1_return),
		OPCODE(o1_renewTimeInVars),
		OPCODE(o1_speakerOn),
		OPCODE(o1_speakerOff),
		/* 24 */
		OPCODE(o1_putPixel),
		OPCODE(o2_goblinFunc),
		OPCODE(o1_createSprite),
		OPCODE(o1_freeSprite),
		/* 28 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 2C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 30 */
		OPCODE(o1_returnTo),
		OPCODE(o1_loadSpriteContent),
		OPCODE(o1_copySprite),
		OPCODE(o6_fillRect),
		/* 34 */
		OPCODE(o1_drawLine),
		OPCODE(o1_strToLong),
		OPCODE(o1_invalidate),
		OPCODE(o1_setBackDelta),
		/* 38 */
		OPCODE(o1_playSound),
		OPCODE(o2_stopSound),
		OPCODE(o2_loadSound),
		OPCODE(o1_freeSoundSlot),
		/* 3C */
		OPCODE(o1_waitEndPlay),
		OPCODE(o1_playComposition),
		OPCODE(o2_getFreeMem),
		OPCODE(o2_checkData),
		/* 40 */
		{0, ""},
		OPCODE(o1_prepareStr),
		OPCODE(o1_insertStr),
		OPCODE(o1_cutStr),
		/* 44 */
		OPCODE(o1_strstr),
		OPCODE(o5_istrlen),
		OPCODE(o1_setMousePos),
		OPCODE(o1_setFrameRate),
		/* 48 */
		OPCODE(o1_animatePalette),
		OPCODE(o1_animateCursor),
		OPCODE(o1_blitCursor),
		OPCODE(o1_loadFont),
		/* 4C */
		OPCODE(o1_freeFont),
		OPCODE(o2_readData),
		OPCODE(o2_writeData),
		OPCODE(o1_manageDataFile),
	};

	static const OpcodeGoblinEntryV6 opcodesGoblin[71] = {
		/* 00 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 04 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 08 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 0C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 10 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 14 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 18 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 1C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 20 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 24 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 28 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 2C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 30 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 34 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 38 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 3C */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 40 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 44 */
		{0, ""},
		{0, ""},
		{0, ""},
	};

	_opcodesDrawV6 = opcodesDraw;
	_opcodesFuncV6 = opcodesFunc;
	_opcodesGoblinV6 = opcodesGoblin;
}

void Inter_v6::executeDrawOpcode(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)",
			i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV6 op = _opcodesDrawV6[i].proc;

	if (op == 0)
		warning("unimplemented opcodeDraw: %d", i);
	else
		(this->*op) ();
}

bool Inter_v6::executeFuncOpcode(byte i, byte j, OpFuncParams &params) {
	_vm->_video->_palLUT->buildNext();

	debugC(1, kDebugFuncOp, "opcodeFunc %d.%d [0x%X.0x%X] (%s) - %s, %d, %d",
			i, j, i, j, getOpcodeFuncDesc(i, j), _vm->_game->_curTotFile,
			(uint) (_vm->_global->_inter_execPtr - _vm->_game->_totFileData),
			(uint) (_vm->_global->_inter_execPtr - _vm->_game->_totFileData - params.counter - 4));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcV6 op = _opcodesFuncV6[i*16 + j].proc;

	if (op == 0)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (params);

	return false;
}

void Inter_v6::executeGoblinOpcode(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcV6 op = 0;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinV6[_goblinFuncLookUp[j][1]].proc;
			break;
		}

	_vm->_global->_inter_execPtr -= 2;

	if (op == 0) {
		warning("unimplemented opcodeGoblin: %d", i);

		int16 paramCount = load16();
		_vm->_global->_inter_execPtr += paramCount * 2;
	} else {
		params.extraData = i;

		(this->*op) (params);
	}
}

const char *Inter_v6::getOpcodeDrawDesc(byte i) {
	return _opcodesDrawV6[i].desc;
}

const char *Inter_v6::getOpcodeFuncDesc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncV6[i*16 + j].desc;
}

const char *Inter_v6::getOpcodeGoblinDesc(int i) {
	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i)
			return _opcodesGoblinV6[_goblinFuncLookUp[j][1]].desc;
	return "";
}

void Inter_v6::o6_totSub() {
	char totFile[14];
	byte length;
	int flags;
	int i;

	length = *_vm->_global->_inter_execPtr++;
	if ((length & 0x7F) > 13)
		error("Length in o2_totSub is greater than 13 (%d)", length);

	if (length & 0x80) {
		evalExpr(0);
		strcpy(totFile, _vm->_parse->_inter_resStr);
	} else {
		for (i = 0; i < length; i++)
			totFile[i] = (char) *_vm->_global->_inter_execPtr++;
		totFile[i] = 0;
	}

	flags = *_vm->_global->_inter_execPtr++;

	if (flags & 0x40)
		warning("Urban Stub: o6_totSub(), flags & 0x40");

	_vm->_game->totSub(flags, totFile);
}

void Inter_v6::o6_playVmdOrMusic() {
	char fileName[128];
	int16 x, y;
	int16 startFrame;
	int16 lastFrame;
	int16 breakKey;
	int16 flags;
	int16 palStart;
	int16 palEnd;
	uint16 palCmd;
	bool close;

	evalExpr(0);
	strncpy0(fileName, _vm->_parse->_inter_resStr, 127);

	x = _vm->_parse->parseValExpr();
	y = _vm->_parse->parseValExpr();
	startFrame = _vm->_parse->parseValExpr();
	lastFrame = _vm->_parse->parseValExpr();
	breakKey = _vm->_parse->parseValExpr();
	flags = _vm->_parse->parseValExpr();
	palStart = _vm->_parse->parseValExpr();
	palEnd = _vm->_parse->parseValExpr();
	palCmd = 1 << (flags & 0x3F);

	debugC(1, kDebugVideo, "Playing video \"%s\" @ %d+%d, frames %d - %d, "
			"paletteCmd %d (%d - %d), flags %X", fileName, x, y, startFrame, lastFrame,
			palCmd, palStart, palEnd, flags);

	close = false;
	if (lastFrame == -1) {
		close = true;
	} else if (lastFrame == -5) {
		_vm->_sound->bgStop();
		return;
	} else if (lastFrame == -9) {
		if (!strchr(fileName, '.'))
			strcat(fileName, ".WA8");

		probe16bitMusic(fileName);

		_vm->_sound->bgStop();
		_vm->_sound->bgPlay(fileName, SOUND_WAV);
		return;
	} else if (lastFrame == -10) {
		_vm->_vidPlayer->primaryClose();
		warning("Urban Stub: Video/Music command -10 (close video?)");
		return;
	} else if (lastFrame < 0) {
		warning("Unknown Video/Music command: %d, %s", lastFrame, fileName);
		return;
	}

	if (startFrame == -2) {
		startFrame = 0;
		lastFrame = -1;
		close = false;
	}

	if ((fileName[0] != 0) && !_vm->_vidPlayer->primaryOpen(fileName, x, y, flags)) {
		WRITE_VAR(11, (uint32) -1);
		return;
	}

	if (startFrame >= 0) {
		_vm->_game->_preventScroll = true;
		_vm->_vidPlayer->primaryPlay(startFrame, lastFrame, breakKey,
				palCmd, palStart, palEnd, 0, -1, false, -1, true);
		_vm->_game->_preventScroll = false;
	}

	if (close)
		_vm->_vidPlayer->primaryClose();
}

void Inter_v6::o6_openItk() {
	char fileName[32];

	evalExpr(0);
	strncpy0(fileName, _vm->_parse->_inter_resStr, 27);
	if (!strchr(fileName, '.'))
		strcat(fileName, ".ITK");

	_vm->_dataIO->openDataFile(fileName, true);

	// WORKAROUND: The CD number detection in Urban Runner is quite daft
	// (it checks CD1.ITK - CD4.ITK and the first that's found determines
	// the CD number), while its NO_CD modus wants everything in CD1.ITK.
	// So we just open the other ITKs, too.
	if (_vm->_game->_noCd && !scumm_stricmp(fileName, "CD1.ITK")) {
		_vm->_dataIO->openDataFile("CD2.ITK", true);
		_vm->_dataIO->openDataFile("CD3.ITK", true);
		_vm->_dataIO->openDataFile("CD4.ITK", true);
	}
}

bool Inter_v6::o6_loadCursor(OpFuncParams &params) {
	int16 id = load16();

	if ((id == -1) || (id == -2)) {
		char file[10];

		if (id == -1) {
			for (int i = 0; i < 9; i++)
				file[i] = *_vm->_global->_inter_execPtr++;
		} else
			strncpy(file, GET_VAR_STR(load16()), 10);

		file[9] = '\0';

		uint16 start = load16();
		int8 index = (int8) *_vm->_global->_inter_execPtr++;

		int vmdSlot = _vm->_vidPlayer->slotOpen(file);

		if (vmdSlot == -1) {
			warning("Can't open video \"%s\" as cursor", file);
			return false;
		}

		int16 framesCount = _vm->_vidPlayer->getFramesCount(vmdSlot);

		for (int i = 0; i < framesCount; i++) {
			_vm->_vidPlayer->slotPlay(vmdSlot);
			_vm->_vidPlayer->slotCopyFrame(vmdSlot, _vm->_draw->_cursorSprites->getVidMem(),
					0, 0, _vm->_draw->_cursorWidth, _vm->_draw->_cursorWidth,
					(start + i) * _vm->_draw->_cursorWidth, 0,
					_vm->_draw->_cursorSprites->getWidth());
		}

		_vm->_vidPlayer->slotClose(vmdSlot);

		_vm->_draw->_cursorAnimLow[index] = start;
		_vm->_draw->_cursorAnimHigh[index] = framesCount + start - 1;
		_vm->_draw->_cursorAnimDelays[index] = 10;

		return false;
	}

	int8 index = (int8) *_vm->_global->_inter_execPtr++;

	if ((index * _vm->_draw->_cursorWidth) >= _vm->_draw->_cursorSprites->getWidth())
		return false;

	int16 width, height;
	byte *dataBuf = _vm->_game->loadTotResource(id, 0, &width, &height);

	_vm->_video->fillRect(*_vm->_draw->_cursorSprites,
			index * _vm->_draw->_cursorWidth, 0,
			index * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
			_vm->_draw->_cursorHeight - 1, 0);

	_vm->_video->drawPackedSprite(dataBuf, width, height,
			index * _vm->_draw->_cursorWidth, 0, 0, *_vm->_draw->_cursorSprites);
	_vm->_draw->_cursorAnimLow[index] = 0;

	return false;
}

bool Inter_v6::o6_assign(OpFuncParams &params) {
	uint16 size, destType;
	int16 dest = _vm->_parse->parseVarIndex(&size, &destType);

	if (size != 0) {
		int16 src;

		byte *savedPos = _vm->_global->_inter_execPtr;

		src = _vm->_parse->parseVarIndex(&size, 0);

		memcpy(_vm->_inter->_variables->getAddressOff8(dest),
				_vm->_inter->_variables->getAddressOff8(src), size * 4);

		_vm->_global->_inter_execPtr = savedPos;

		evalExpr(&src);

		return false;
	}

	byte loopCount;
	if (*_vm->_global->_inter_execPtr == 98) {
		_vm->_global->_inter_execPtr++;
		loopCount = *_vm->_global->_inter_execPtr++;

		for (int i = 0; i < loopCount; i++) {
			uint8 c = *_vm->_global->_inter_execPtr++;
			uint16 n = load16();

			memset(_vm->_inter->_variables->getAddressOff8(dest), c, n);

			dest += n;
		}

		return false;

	} else if (*_vm->_global->_inter_execPtr == 99) {
		_vm->_global->_inter_execPtr++;
		loopCount = *_vm->_global->_inter_execPtr++;
	} else
		loopCount = 1;

	for (int i = 0; i < loopCount; i++) {
		int16 result;
		int16 srcType = evalExpr(&result);

		switch (destType) {
		case TYPE_VAR_INT8:
		case TYPE_ARRAY_INT8:
			WRITE_VARO_UINT8(dest + i, _vm->_parse->_resultInt);
			break;

		case TYPE_VAR_INT16:
		case TYPE_ARRAY_INT16:
			WRITE_VARO_UINT16(dest + i * 2, _vm->_parse->_resultInt);
			break;

		case TYPE_VAR_INT32:
		case TYPE_ARRAY_INT32:
			WRITE_VAR_OFFSET(dest + i * 4, _vm->_parse->_resultInt);
			break;

		case TYPE_VAR_INT32_AS_INT16:
			WRITE_VARO_UINT16(dest + i * 4, _vm->_parse->_resultInt);
			break;

		case TYPE_VAR_STR:
		case TYPE_ARRAY_STR:
			if (srcType == TYPE_IMM_INT16)
				WRITE_VARO_UINT8(dest, result);
			else
				WRITE_VARO_STR(dest, _vm->_parse->_inter_resStr);
			break;
		}
	}

	return false;
}

bool Inter_v6::o6_palLoad(OpFuncParams &params) {
	o1_palLoad(params);

	if (_gotFirstPalette)
		_vm->_video->_palLUT->setPalette((const byte *) _vm->_global->_pPaletteDesc->vgaPal,
				Graphics::PaletteLUT::kPaletteRGB, 6, 0);

	_gotFirstPalette = true;
	return false;
}

bool Inter_v6::o6_freeCollision(OpFuncParams &params) {
	int16 id;

	id = _vm->_parse->parseValExpr();

	switch (id + 5) {
	case 0:
		_vm->_game->pushCollisions(1);
		break;
	case 1:
		_vm->_game->popCollisions();
		break;
	case 2:
		_vm->_game->pushCollisions(2);
		break;
	case 3:
		for (int i = 0; i < 150; i++) {
			if (((_vm->_game->_collisionAreas[i].id & 0xF000) == 0xD000) ||
					((_vm->_game->_collisionAreas[i].id & 0xF000) == 0x4000))
				_vm->_game->_collisionAreas[i].left = 0xFFFF;
		}
		break;
	case 4:
		for (int i = 0; i < 150; i++) {
			if ((_vm->_game->_collisionAreas[i].id & 0xF000) == 0xE000)
				_vm->_game->_collisionAreas[i].left = 0xFFFF;
		}
		break;
	default:
		_vm->_game->freeCollision(0xE000 + id);
		break;
	}

	return false;
}

bool Inter_v6::o6_fillRect(OpFuncParams &params) {
	int16 destSurf;

	_vm->_draw->_destSurface = destSurf = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteBottom = _vm->_parse->parseValExpr();

	evalExpr(0);

	_vm->_draw->_backColor = _vm->_parse->_resultInt & 0xFFFF;
	uint16 extraVar = _vm->_parse->_resultInt >> 16;

	if (extraVar != 0)
		warning("Urban Stub: o6_fillRect(), extraVar = %d", extraVar);

	if (_vm->_draw->_spriteRight < 0) {
		_vm->_draw->_destSpriteX += _vm->_draw->_spriteRight - 1;
		_vm->_draw->_spriteRight = -_vm->_draw->_spriteRight + 2;
	}
	if (_vm->_draw->_spriteBottom < 0) {
		_vm->_draw->_destSpriteY += _vm->_draw->_spriteBottom - 1;
		_vm->_draw->_spriteBottom = -_vm->_draw->_spriteBottom + 2;
	}

	if (destSurf & 0x80) {
		warning("Urban Stub: o6_fillRect(), destSurf & 0x80");
		return false;
	}

	if (!_vm->_draw->_spritesArray[(destSurf > 100) ? (destSurf - 80) : destSurf])
		return false;

	_vm->_draw->spriteOperation(DRAW_FILLRECT);
	return false;
}

void Inter_v6::probe16bitMusic(char *fileName) {
	int len = strlen(fileName);

	if (len < 4)
		return;

	if (scumm_stricmp(fileName + len - 4, ".WA8"))
		return;

	fileName[len - 1] = 'V';

	int16 handle;
	if ((handle = _vm->_dataIO->openData(fileName)) >= 0) {
		_vm->_dataIO->closeData(handle);
		return;
	}

	fileName[len - 1] = '8';
}

} // End of namespace Gob
