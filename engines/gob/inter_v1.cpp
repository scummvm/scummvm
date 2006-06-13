/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 Ivan Dubrov
 * Copyright (C) 2004-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"
#include "common/endian.h"

#include "gob/gob.h"
#include "gob/global.h"
#include "gob/inter.h"
#include "gob/util.h"
#include "gob/scenery.h"
#include "gob/parse.h"
#include "gob/game.h"
#include "gob/draw.h"
#include "gob/mult.h"
#include "gob/goblin.h"
#include "gob/cdrom.h"
#include "gob/music.h"
#include "gob/map.h"
#include "gob/palanim.h"
#include "gob/anim.h"

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_v1, x)

const int Inter_v1::_goblinFuncLookUp[][2] = {
	{1, 0},
	{2, 1},
	{3, 2},
	{4, 3},
	{5, 4},
	{6, 5},
	{7, 6},
	{8, 7},
	{9, 8},
	{10, 9},
	{12, 10},
	{13, 11},
	{14, 12},
	{15, 13},
	{16, 14},
	{21, 15},
	{22, 16},
	{23, 17},
	{24, 18},
	{25, 19},
	{26, 20},
	{27, 21},
	{28, 22},
	{29, 23},
	{30, 24},
	{32, 25},
	{33, 26},
	{34, 27},
	{35, 28},
	{36, 29},
	{37, 30},
	{40, 31},
	{41, 32},
	{42, 33},
	{43, 34},
	{44, 35},
	{50, 36},
	{52, 37},
	{53, 38},
	{150, 39},
	{152, 40},
	{200, 41},
	{201, 42},
	{202, 43},
	{203, 44},
	{204, 45},
	{250, 46},
	{251, 47},
	{252, 48},
	{500, 49},
	{502, 50},
	{503, 51},
	{600, 52},
	{601, 53},
	{602, 54},
	{603, 55},
	{604, 56},
	{605, 57},
	{1000, 58},
	{1001, 59},
	{1002, 60},
	{1003, 61},
	{1004, 62},
	{1005, 63},
	{1006, 64},
	{1008, 65},
	{1009, 66},
	{1010, 67},
	{1011, 68},
	{1015, 69},
	{2005, 70}
};

Inter_v1::Inter_v1(GobEngine *vm) : Inter(vm) {
	setupOpcodes();
}

void Inter_v1::setupOpcodes(void) {
	static const OpcodeDrawEntryV1 opcodesDraw[256] = {
		/* 00 */
		OPCODE(o1_loadMult),
		OPCODE(o1_playMult),
		OPCODE(o1_freeMult),
		{NULL, ""},
		/* 04 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		OPCODE(o1_initCursor),
		/* 08 */
		OPCODE(o1_initCursorAnim),
		OPCODE(o1_clearCursorAnim),
		OPCODE(o1_setRenderFlags),
		{NULL, ""},
		/* 0C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 10 */
		OPCODE(o1_loadAnim),
		OPCODE(o1_freeAnim),
		OPCODE(o1_updateAnim),
		{NULL, ""},
		/* 14 */
		OPCODE(o1_initMult),
		OPCODE(o1_multFreeMult),
		OPCODE(o1_animate),
		OPCODE(o1_multLoadMult),
		/* 18 */
		OPCODE(o1_storeParams),
		OPCODE(o1_getObjAnimSize),
		OPCODE(o1_loadStatic),
		OPCODE(o1_freeStatic),
		/* 1C */
		OPCODE(o1_renderStatic),
		OPCODE(o1_loadCurLayer),
		{NULL, ""},
		{NULL, ""},
		/* 20 */
		OPCODE(o1_playCDTrack),
		OPCODE(o1_getCDTrackPos),
		OPCODE(o1_stopCD),
		{NULL, ""},
		/* 24 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 28 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 2C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 30 */
		OPCODE(o1_loadFontToSprite),
		OPCODE(o1_freeFontToSprite),
		{NULL, ""},
		{NULL, ""},
		/* 34 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 38 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 3C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 40 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 44 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 48 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 4C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 50 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 54 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 58 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 5C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 60 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 64 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 68 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 6C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 70 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 74 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 78 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 7C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 80 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 84 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 88 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 8C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 90 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 94 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 98 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 9C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* A8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* AC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* B8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* BC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* C8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* CC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* D8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* DC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* E8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* EC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F0 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F4 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* F8 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* FC */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""}
	};

	static const OpcodeFuncEntryV1 opcodesFunc[80] = {
		/* 00 */
		OPCODE(o1_callSub),
		OPCODE(o1_callSub),
		OPCODE(o1_drawPrintText),
		OPCODE(o1_loadCursor),
		/* 04 */
		{NULL, ""},
		OPCODE(o1_call),
		OPCODE(o1_repeatUntil),
		OPCODE(o1_whileDo),
		/* 08 */
		OPCODE(o1_callBool),
		OPCODE(o1_evaluateStore),
		OPCODE(o1_loadSpriteToPos),
		{NULL, ""},
		/* 0C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 10 */
		{NULL, ""},
		OPCODE(o1_printText),
		OPCODE(o1_loadTot),
		OPCODE(o1_palLoad),
		/* 14 */
		OPCODE(o1_keyFunc),
		OPCODE(o1_capturePush),
		OPCODE(o1_capturePop),
		OPCODE(o1_animPalInit),
		/* 18 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 1C */
		{NULL, ""},
		{NULL, ""},
		OPCODE(o1_drawOperations),
		OPCODE(o1_setcmdCount),
		/* 20 */
		OPCODE(o1_return),
		OPCODE(o1_renewTimeInVars),
		OPCODE(o1_speakerOn),
		OPCODE(o1_speakerOff),
		/* 24 */
		OPCODE(o1_putPixel),
		OPCODE(o1_goblinFunc),
		OPCODE(o1_createSprite),
		OPCODE(o1_freeSprite),
		/* 28 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 2C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 30 */
		OPCODE(o1_returnTo),
		OPCODE(o1_loadSpriteContent),
		OPCODE(o1_copySprite),
		OPCODE(o1_fillRect),
		/* 34 */
		OPCODE(o1_drawLine),
		OPCODE(o1_strToLong),
		OPCODE(o1_invalidate),
		OPCODE(o1_setBackDelta),
		/* 38 */
		OPCODE(o1_playSound),
		OPCODE(o1_stopSound),
		OPCODE(o1_loadSound),
		OPCODE(o1_freeSoundSlot),
		/* 3C */
		OPCODE(o1_waitEndPlay),
		OPCODE(o1_playComposition),
		OPCODE(o1_getFreeMem),
		OPCODE(o1_checkData),
		/* 40 */
		{NULL, ""},
		OPCODE(o1_prepareStr),
		OPCODE(o1_insertStr),
		OPCODE(o1_cutStr),
		/* 44 */
		OPCODE(o1_strstr),
		OPCODE(o1_istrlen),
		OPCODE(o1_setMousePos),
		OPCODE(o1_setFrameRate),
		/* 48 */
		OPCODE(o1_animatePalette),
		OPCODE(o1_animateCursor),
		OPCODE(o1_blitCursor),
		OPCODE(o1_loadFont),
		/* 4C */
		OPCODE(o1_freeFont),
		OPCODE(o1_readData),
		OPCODE(o1_writeData),
		OPCODE(o1_manageDataFile),
	};

	static const OpcodeGoblinEntryV1 opcodesGoblin[71] = {
		/* 00 */
		OPCODE(o1_setState),
		OPCODE(o1_setCurFrame),
		OPCODE(o1_setNextState),
		OPCODE(o1_setMultState),
		/* 04 */
		OPCODE(o1_setOrder),
		OPCODE(o1_setActionStartState),
		OPCODE(o1_setCurLookDir),
		OPCODE(o1_setType),
		/* 08 */
		OPCODE(o1_setNoTick),
		OPCODE(o1_setPickable),
		OPCODE(o1_setXPos),
		OPCODE(o1_setYPos),
		/* 0C */
		OPCODE(o1_setDoAnim),
		OPCODE(o1_setRelaxTime),
		OPCODE(o1_setMaxTick),
		OPCODE(o1_getState),
		/* 10 */
		OPCODE(o1_getCurFrame),
		OPCODE(o1_getNextState),
		OPCODE(o1_getMultState),
		OPCODE(o1_getOrder),
		/* 14 */
		OPCODE(o1_getActionStartState),
		OPCODE(o1_getCurLookDir),
		OPCODE(o1_getType),
		OPCODE(o1_getNoTick),
		/* 18 */
		OPCODE(o1_getPickable),
		OPCODE(o1_getObjMaxFrame),
		OPCODE(o1_getXPos),
		OPCODE(o1_getYPos),
		/* 1C */
		OPCODE(o1_getDoAnim),
		OPCODE(o1_getRelaxTime),
		OPCODE(o1_getMaxTick),
		OPCODE(o1_manipulateMap),
		/* 20 */
		OPCODE(o1_getItem),
		OPCODE(o1_manipulateMapIndirect),
		OPCODE(o1_getItemIndirect),
		OPCODE(o1_setPassMap),
		/* 24 */
		OPCODE(o1_setGoblinPosH),
		OPCODE(o1_getGoblinPosXH),
		OPCODE(o1_getGoblinPosYH),
		OPCODE(o1_setGoblinMultState),
		/* 28 */
		OPCODE(o1_setGoblinUnk14),
		OPCODE(o1_setItemIdInPocket),
		OPCODE(o1_setItemIndInPocket),
		OPCODE(o1_getItemIdInPocket),
		/* 2C */
		OPCODE(o1_getItemIndInPocket),
		OPCODE(o1_setItemPos),
		OPCODE(o1_setGoblinPos),
		OPCODE(o1_setGoblinState),
		/* 30 */
		OPCODE(o1_setGoblinStateRedraw),
		OPCODE(o1_decRelaxTime),
		OPCODE(o1_getGoblinPosX),
		OPCODE(o1_getGoblinPosY),
		/* 34 */
		OPCODE(o1_clearPathExistence),
		OPCODE(o1_setGoblinVisible),
		OPCODE(o1_setGoblinInvisible),
		OPCODE(o1_getObjectIntersect),
		/* 38 */
		OPCODE(o1_getGoblinIntersect),
		OPCODE(o1_setItemPos),
		OPCODE(o1_loadObjects),
		OPCODE(o1_freeObjects),
		/* 3C */
		OPCODE(o1_animateObjects),
		OPCODE(o1_drawObjects),
		OPCODE(o1_loadMap),
		OPCODE(o1_moveGoblin),
		/* 40 */
		OPCODE(o1_switchGoblin),
		OPCODE(o1_loadGoblin),
		OPCODE(o1_writeTreatItem),
		OPCODE(o1_moveGoblin0),
		/* 44 */
		OPCODE(o1_setGoblinTarget),
		OPCODE(o1_setGoblinObjectsPos),
		OPCODE(o1_initGoblin)
	};

	_opcodesDrawV1 = opcodesDraw;
	_opcodesFuncV1 = opcodesFunc; // EGroupe
	_opcodesGoblinV1 = opcodesGoblin;
}

bool Inter_v1::o1_setMousePos(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_global->_inter_mouseX = _vm->_parse->parseValExpr();
	_vm->_global->_inter_mouseY = _vm->_parse->parseValExpr();
	if (_vm->_global->_useMouse != 0)
		_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
	return false;
}

bool Inter_v1::o1_evaluateStore(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *savedPos;
	int16 token;
	int16 result;
	int16 varOff;

	savedPos = _vm->_global->_inter_execPtr;
	varOff = _vm->_parse->parseVarIndex();
	token = evalExpr(&result);
	switch (savedPos[0]) {
	case 23:
	case 26:
		WRITE_VAR_OFFSET(varOff, _vm->_global->_inter_resVal);
		break;

	case 25:
	case 28:
		if (token == 20)
			*(_vm->_global->_inter_variables + varOff) = result;
		else
			strcpy(_vm->_global->_inter_variables + varOff, _vm->_global->_inter_resStr);
		break;

	}
	return false;
}

bool Inter_v1::o1_capturePush(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 left;
	int16 top;
	int16 width;
	int16 height;

	left = _vm->_parse->parseValExpr();
	top = _vm->_parse->parseValExpr();
	width = _vm->_parse->parseValExpr();
	height = _vm->_parse->parseValExpr();
	_vm->_game->capturePush(left, top, width, height);
	(*_vm->_scenery->_pCaptureCounter)++;
	return false;
}

bool Inter_v1::o1_capturePop(char &cmdCount, int16 &counter, int16 &retFlag) {
	if (*_vm->_scenery->_pCaptureCounter != 0) {
		(*_vm->_scenery->_pCaptureCounter)--;
		_vm->_game->capturePop(1);
	}
	return false;
}

bool Inter_v1::o1_printText(char &cmdCount, int16 &counter, int16 &retFlag) {
	char buf[60];
	int16 i;

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->_backColor = _vm->_parse->parseValExpr();
	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->_fontIndex = _vm->_parse->parseValExpr();
	_vm->_draw->_destSurface = 21;
	_vm->_draw->_textToPrint = buf;
	_vm->_draw->_transparency = 0;

	if (_vm->_draw->_backColor >= 16) {
		_vm->_draw->_backColor = 0;
		_vm->_draw->_transparency = 1;
	}

	do {
		for (i = 0; *_vm->_global->_inter_execPtr != '.' && (byte)*_vm->_global->_inter_execPtr != 200;
			 i++, _vm->_global->_inter_execPtr++) {
			buf[i] = *_vm->_global->_inter_execPtr;
		}

		if ((byte)*_vm->_global->_inter_execPtr != 200) {
			_vm->_global->_inter_execPtr++;
			switch (*_vm->_global->_inter_execPtr) {
			case 23:
			case 26:
				sprintf(buf + i, "%d", VAR_OFFSET(_vm->_parse->parseVarIndex()));
				break;

			case 25:
			case 28:
				sprintf(buf + i, "%s", _vm->_global->_inter_variables + _vm->_parse->parseVarIndex());
				break;
			}
			_vm->_global->_inter_execPtr++;
		} else {
			buf[i] = 0;
		}
		_vm->_draw->spriteOperation(DRAW_PRINTTEXT);
	} while ((byte)*_vm->_global->_inter_execPtr != 200);
	_vm->_global->_inter_execPtr++;

	return false;
}

bool Inter_v1::o1_animPalInit(char &cmdCount, int16 &counter, int16 &retFlag) {
	_animPalDir[0] = load16();
	_animPalLowIndex[0] = _vm->_parse->parseValExpr();
	_animPalHighIndex[0] = _vm->_parse->parseValExpr();
	return false;
}

void Inter_v1::o1_loadMult(void) {
	int16 resId;

	resId = load16();
	_vm->_mult->loadMult(resId);
}

void Inter_v1::o1_playMult(void) {
	int16 checkEscape;

	checkEscape = load16();
	_vm->_mult->playMult(VAR(57), -1, checkEscape, 0);
}

void Inter_v1::o1_freeMult(void) {
	load16();		// unused
	_vm->_mult->freeMultKeys();
}

void Inter_v1::o1_initCursor(void) {
	int16 width;
	int16 height;
	int16 count;
	int16 i;

	_vm->_draw->_cursorXDeltaVar = _vm->_parse->parseVarIndex();
	_vm->_draw->_cursorYDeltaVar = _vm->_parse->parseVarIndex();

	width = load16();
	if (width < 16)
		width = 16;

	height = load16();
	if (height < 16)
		height = 16;

	count = load16();
	if (count < 2)
		count = 2;

	if (width != _vm->_draw->_cursorWidth || height != _vm->_draw->_cursorHeight ||
	    _vm->_draw->_cursorSprites->width != width * count) {

		_vm->_video->freeSurfDesc(_vm->_draw->_cursorSprites);
		_vm->_video->freeSurfDesc(_vm->_draw->_cursorBack);

		_vm->_draw->_cursorWidth = width;
		_vm->_draw->_cursorHeight = height;

		if (count < 0x80)
			_vm->_draw->_transparentCursor = 1;
		else
			_vm->_draw->_transparentCursor = 0;

		if (count > 0x80)
			count -= 0x80;

		_vm->_draw->_cursorSprites =
		    _vm->_video->initSurfDesc(_vm->_global->_videoMode, _vm->_draw->_cursorWidth * count,
		    _vm->_draw->_cursorHeight, 2);
		_vm->_draw->_spritesArray[23] = _vm->_draw->_cursorSprites;

		_vm->_draw->_cursorBack =
		    _vm->_video->initSurfDesc(_vm->_global->_videoMode, _vm->_draw->_cursorWidth,
		    _vm->_draw->_cursorHeight, 0);
		for (i = 0; i < 40; i++) {
			_vm->_draw->_cursorAnimLow[i] = -1;
			_vm->_draw->_cursorAnimDelays[i] = 0;
			_vm->_draw->_cursorAnimHigh[i] = 0;
		}
		_vm->_draw->_cursorAnimLow[1] = 0;
	}
}

void Inter_v1::o1_initCursorAnim(void) {
	int16 ind;

	ind = _vm->_parse->parseValExpr();
	_vm->_draw->_cursorAnimLow[ind] = load16();
	_vm->_draw->_cursorAnimHigh[ind] = load16();
	_vm->_draw->_cursorAnimDelays[ind] = load16();
}

void Inter_v1::o1_clearCursorAnim(void) {
	int16 ind;

	ind = _vm->_parse->parseValExpr();
	_vm->_draw->_cursorAnimLow[ind] = -1;
	_vm->_draw->_cursorAnimHigh[ind] = 0;
	_vm->_draw->_cursorAnimDelays[ind] = 0;
}

bool Inter_v1::o1_drawOperations(char &cmdCount, int16 &counter, int16 &retFlag) {
	byte cmd;

	cmd = *_vm->_global->_inter_execPtr++;

	executeDrawOpcode(cmd);

	return false;
}

bool Inter_v1::o1_getFreeMem(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 freeVar;
	int16 maxFreeVar;

	freeVar = _vm->_parse->parseVarIndex();
	maxFreeVar = _vm->_parse->parseVarIndex();

	// HACK
	WRITE_VAR_OFFSET(freeVar, 1000000);
	WRITE_VAR_OFFSET(maxFreeVar, 1000000);
	return false;
}

bool Inter_v1::o1_manageDataFile(char &cmdCount, int16 &counter, int16 &retFlag) {
	evalExpr(0);

	if (_vm->_global->_inter_resStr[0] != 0)
		_vm->_dataio->openDataFile(_vm->_global->_inter_resStr);
	else
		_vm->_dataio->closeDataFile();
	return false;
}

bool Inter_v1::o1_writeData(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 offset;
	int16 handle;
	int16 size;
	int16 dataVar;
	int16 retSize;

	evalExpr(0);
	dataVar = _vm->_parse->parseVarIndex();
	size = _vm->_parse->parseValExpr();
	offset = _vm->_parse->parseValExpr();

	WRITE_VAR(1, 1);
	handle = _vm->_dataio->openData(_vm->_global->_inter_resStr, Common::File::kFileWriteMode);

	if (handle < 0)
		return false;

	if (offset < 0) {
		_vm->_dataio->seekData(handle, -offset - 1, 2);
	} else {
		_vm->_dataio->seekData(handle, offset, 0);
	}

	retSize = _vm->_dataio->file_getHandle(handle)->write(_vm->_global->_inter_variables + dataVar, size);

	if (retSize == size)
		WRITE_VAR(1, 0);

	_vm->_dataio->closeData(handle);
	return false;
}

bool Inter_v1::o1_checkData(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 handle;
	int16 varOff;

	evalExpr(0);
	varOff = _vm->_parse->parseVarIndex();
	handle = _vm->_dataio->openData(_vm->_global->_inter_resStr);

	WRITE_VAR_OFFSET(varOff, handle);
	if (handle >= 0)
		_vm->_dataio->closeData(handle);
	return false;
}

bool Inter_v1::o1_readData(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 retSize;
	int16 size;
	int16 dataVar;
	int16 offset;
	int16 handle;
	char buf[4];

	evalExpr(0);
	dataVar = _vm->_parse->parseVarIndex();
	size = _vm->_parse->parseValExpr();
	offset = _vm->_parse->parseValExpr();

	if (_vm->_game->_extHandle >= 0)
		_vm->_dataio->closeData(_vm->_game->_extHandle);

	WRITE_VAR(1, 1);
	handle = _vm->_dataio->openData(_vm->_global->_inter_resStr);
	if (handle >= 0) {
		_vm->_draw->animateCursor(4);
		if (offset < 0)
			_vm->_dataio->seekData(handle, -offset - 1, 2);
		else
			_vm->_dataio->seekData(handle, offset, 0);

		if (((dataVar >> 2) == 59) && (size == 4)) {
			retSize = _vm->_dataio->readData(handle, buf, 4);
			WRITE_VAR(59, READ_LE_UINT32(buf));
		} else
			retSize = _vm->_dataio->readData(handle, _vm->_global->_inter_variables + dataVar, size);
		_vm->_dataio->closeData(handle);

		if (retSize == size)
			WRITE_VAR(1, 0);
	}

	if (_vm->_game->_extHandle >= 0)
		_vm->_game->_extHandle = _vm->_dataio->openData(_vm->_game->_curExtFile);
	return false;
}

bool Inter_v1::o1_loadFont(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;

	evalExpr(0);
	index = load16();

	if (_vm->_draw->_fonts[index] != 0)
		_vm->_util->freeFont(_vm->_draw->_fonts[index]);

	_vm->_draw->animateCursor(4);
	if (_vm->_game->_extHandle >= 0)
		_vm->_dataio->closeData(_vm->_game->_extHandle);

	_vm->_draw->_fonts[index] = _vm->_util->loadFont(_vm->_global->_inter_resStr);

	if (_vm->_game->_extHandle >= 0)
		_vm->_game->_extHandle = _vm->_dataio->openData(_vm->_game->_curExtFile);
	return false;
}

bool Inter_v1::o1_freeFont(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;

	index = load16();
	if (_vm->_draw->_fonts[index] != 0)
		_vm->_util->freeFont(_vm->_draw->_fonts[index]);

	_vm->_draw->_fonts[index] = 0;
	return false;
}

bool Inter_v1::o1_prepareStr(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 var;

	var = _vm->_parse->parseVarIndex();
	_vm->_util->prepareStr(_vm->_global->_inter_variables + var);
	return false;
}

bool Inter_v1::o1_insertStr(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 pos;
	int16 strVar;

	strVar = _vm->_parse->parseVarIndex();
	evalExpr(0);
	pos = _vm->_parse->parseValExpr();
	_vm->_util->insertStr(_vm->_global->_inter_resStr, _vm->_global->_inter_variables + strVar, pos);
	return false;
}

bool Inter_v1::o1_cutStr(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 var;
	int16 pos;
	int16 size;

	var = _vm->_parse->parseVarIndex();
	pos = _vm->_parse->parseValExpr();
	size = _vm->_parse->parseValExpr();
	_vm->_util->cutFromStr(_vm->_global->_inter_variables + var, pos, size);
	return false;
}

bool Inter_v1::o1_strstr(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 strVar;
	int16 resVar;
	int16 pos;

	strVar = _vm->_parse->parseVarIndex();
	evalExpr(0);
	resVar = _vm->_parse->parseVarIndex();

	char *res = strstr(_vm->_global->_inter_variables + strVar, _vm->_global->_inter_resStr);
	pos = res ? (res - (_vm->_global->_inter_variables + strVar)) : -1;
	WRITE_VAR_OFFSET(resVar, pos);
	return false;
}

bool Inter_v1::o1_setFrameRate(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_util->setFrameRate(_vm->_parse->parseValExpr());
	return false;
}

bool Inter_v1::o1_istrlen(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 len;
	int16 var;

	var = _vm->_parse->parseVarIndex();
	len = strlen(_vm->_global->_inter_variables + var);
	var = _vm->_parse->parseVarIndex();

	WRITE_VAR_OFFSET(var, len);
	return false;
}

bool Inter_v1::o1_strToLong(char &cmdCount, int16 &counter, int16 &retFlag) {
	char str[20];
	int16 strVar;
	int16 destVar;
	int32 res;

	strVar = _vm->_parse->parseVarIndex();
	strcpy(str, _vm->_global->_inter_variables + strVar);
	res = atol(str);

	destVar = _vm->_parse->parseVarIndex();
	WRITE_VAR_OFFSET(destVar, res);
	return false;
}

bool Inter_v1::o1_invalidate(char &cmdCount, int16 &counter, int16 &retFlag) {
	warning("invalidate: 'bugged' function");
	_vm->_draw->_destSurface = load16();
	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_INVALIDATE);
	return false;
}

bool Inter_v1::o1_loadSpriteContent(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->_spriteLeft = load16();
	_vm->_draw->_destSurface = load16();
	_vm->_draw->_transparency = load16();
	_vm->_draw->_destSpriteX = 0;
	_vm->_draw->_destSpriteY = 0;
	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);
	return false;
}

bool Inter_v1::o1_copySprite(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->_sourceSurface = load16();
	_vm->_draw->_destSurface = load16();

	_vm->_draw->_spriteLeft = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteTop = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->_transparency = load16();
	_vm->_draw->spriteOperation(DRAW_BLITSURF);
	return false;
}

bool Inter_v1::o1_putPixel(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->_destSurface = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_PUTPIXEL);
	return false;
}

bool Inter_v1::o1_fillRect(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->_destSurface = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->_backColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_FILLRECT);
	return false;
}

bool Inter_v1::o1_drawLine(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->_destSurface = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteRight = _vm->_parse->parseValExpr();
	_vm->_draw->_spriteBottom = _vm->_parse->parseValExpr();

	_vm->_draw->_frontColor = _vm->_parse->parseValExpr();
	_vm->_draw->spriteOperation(DRAW_DRAWLINE);
	return false;
}

bool Inter_v1::o1_createSprite(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;
	int16 height;
	int16 width;
	int16 flag;

	index = load16();
	width = load16();
	height = load16();

	flag = load16();
	if (flag == 1)
		_vm->_draw->_spritesArray[index] = _vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, 2);
	else
		_vm->_draw->_spritesArray[index] = _vm->_video->initSurfDesc(_vm->_global->_videoMode, width, height, 0);

	_vm->_video->clearSurf(_vm->_draw->_spritesArray[index]);
	return false;
}

bool Inter_v1::o1_freeSprite(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;

	index = load16();
	if (_vm->_draw->_spritesArray[index] == 0)
		return false;

	_vm->_video->freeSurfDesc(_vm->_draw->_spritesArray[index]);
	_vm->_draw->_spritesArray[index] = 0;
	return false;
}

bool Inter_v1::o1_playComposition(char &cmdCount, int16 &counter, int16 &retFlag) {
	static int16 composition[50];
	int16 i;
	int16 dataVar;
	int16 freqVal;

	dataVar = _vm->_parse->parseVarIndex();
	freqVal = _vm->_parse->parseValExpr();
	for (i = 0; i < 50; i++)
		composition[i] = (int16)VAR_OFFSET(dataVar + i * 4);

	_vm->_snd->playComposition(_vm->_game->_soundSamples, composition, freqVal);
	return false;
}

bool Inter_v1::o1_stopSound(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_music->stopPlay();
	_vm->_snd->stopSound(_vm->_parse->parseValExpr());
	_soundEndTimeKey = 0;
	return false;
}

bool Inter_v1::o1_playSound(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 frequency;
	int16 freq2;
	int16 repCount;
	int16 index;

	index = _vm->_parse->parseValExpr();
	repCount = _vm->_parse->parseValExpr();
	frequency = _vm->_parse->parseValExpr();

	_vm->_snd->stopSound(0);
	_soundEndTimeKey = 0;
	if (_vm->_game->_soundSamples[index] == 0)
		return false;

	if (repCount < 0) {
		if (_vm->_global->_soundFlags < 2)
			return false;

		repCount = -repCount;
		_soundEndTimeKey = _vm->_util->getTimeKey();

		if (frequency == 0) {
			freq2 = _vm->_game->_soundSamples[index]->frequency;
		} else {
			freq2 = frequency;
		}
		_soundStopVal =
		    (10 * (_vm->_game->_soundSamples[index]->size / 2)) / freq2;
		_soundEndTimeKey +=
		    ((_vm->_game->_soundSamples[index]->size * repCount -
			_vm->_game->_soundSamples[index]->size / 2) * 1000) / freq2;
	}
	_vm->_snd->playSample(_vm->_game->_soundSamples[index], repCount, frequency);
	return false;
}

bool Inter_v1::o1_loadCursor(char &cmdCount, int16 &counter, int16 &retFlag) {
	Game::TotResItem *itemPtr;
	int16 width;
	int16 height;
	int32 offset;
	char *dataBuf;
	int16 id;
	int8 index;

	id = load16();
	index = *_vm->_global->_inter_execPtr++;
	itemPtr = &_vm->_game->_totResourceTable->items[id];
	offset = itemPtr->offset;

	if (offset >= 0) {
		dataBuf =
		    _vm->_game->_totResourceTable->dataPtr + szGame_TotResTable +
		    szGame_TotResItem * _vm->_game->_totResourceTable->itemsCount + offset;
	} else {
		dataBuf = _vm->_game->_imFileData + (int32)READ_LE_UINT32(&((int32 *)_vm->_game->_imFileData)[-offset - 1]);
	}

	width = itemPtr->width;
	height = itemPtr->height;

	_vm->_video->fillRect(_vm->_draw->_cursorSprites, index * _vm->_draw->_cursorWidth, 0,
	    index * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
	    _vm->_draw->_cursorHeight - 1, 0);

	_vm->_video->drawPackedSprite((byte*)dataBuf, width, height,
	    index * _vm->_draw->_cursorWidth, 0, 0, _vm->_draw->_cursorSprites);
	_vm->_draw->_cursorAnimLow[index] = 0;

	return false;
}

bool Inter_v1::o1_loadSpriteToPos(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->_spriteLeft = load16();

	_vm->_draw->_destSpriteX = _vm->_parse->parseValExpr();
	_vm->_draw->_destSpriteY = _vm->_parse->parseValExpr();

	_vm->_draw->_transparency = _vm->_global->_inter_execPtr[0];
	_vm->_draw->_destSurface = (_vm->_global->_inter_execPtr[0] / 2) - 1;

	if (_vm->_draw->_destSurface < 0)
		_vm->_draw->_destSurface = 101;
	_vm->_draw->_transparency &= 1;
	_vm->_global->_inter_execPtr += 2;
	_vm->_draw->spriteOperation(DRAW_LOADSPRITE);

	return false;
}

bool Inter_v1::o1_loadTot(char &cmdCount, int16 &counter, int16 &retFlag) {
	char buf[20];
	int8 size;
	int16 i;

	if ((*_vm->_global->_inter_execPtr & 0x80) != 0) {
		_vm->_global->_inter_execPtr++;
		evalExpr(0);
		strcpy(buf, _vm->_global->_inter_resStr);
	} else {
		size = *_vm->_global->_inter_execPtr++;
		for (i = 0; i < size; i++)
			buf[i] = *_vm->_global->_inter_execPtr++;

		buf[size] = 0;
	}

	strcat(buf, ".tot");
	_terminate = true;
	strcpy(_vm->_game->_totToLoad, buf);

	return false;
}

bool Inter_v1::o1_keyFunc(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 flag;
	int16 key;

	// Gob2 busy-waits here, so add a delay
	_vm->_util->longDelay(1);

	flag = load16();
	animPalette();
	_vm->_draw->blitInvalidated();

	if (flag != 0) {

		if (flag != 1) {
			if (flag != 2) {
				_vm->_util->longDelay(flag);
				return false;
			}

			key = 0;

			if (_vm->_global->_pressedKeys[0x48])
				key |= 1;

			if (_vm->_global->_pressedKeys[0x50])
				key |= 2;

			if (_vm->_global->_pressedKeys[0x4d])
				key |= 4;

			if (_vm->_global->_pressedKeys[0x4b])
				key |= 8;

			if (_vm->_global->_pressedKeys[0x1c])
				key |= 0x10;

			if (_vm->_global->_pressedKeys[0x39])
				key |= 0x20;

			if (_vm->_global->_pressedKeys[1])
				key |= 0x40;

			if (_vm->_global->_pressedKeys[0x1d])
				key |= 0x80;

			if (_vm->_global->_pressedKeys[0x2a])
				key |= 0x100;

			if (_vm->_global->_pressedKeys[0x36])
				key |= 0x200;

			if (_vm->_global->_pressedKeys[0x38])
				key |= 0x400;

			if (_vm->_global->_pressedKeys[0x3b])
				key |= 0x800;

			if (_vm->_global->_pressedKeys[0x3c])
				key |= 0x1000;

			if (_vm->_global->_pressedKeys[0x3d])
				key |= 0x2000;

			if (_vm->_global->_pressedKeys[0x3e])
				key |= 0x4000;

			WRITE_VAR(0, key);
			_vm->_util->waitKey();
			return false;
		}
		key = _vm->_game->checkKeys(&_vm->_global->_inter_mouseX, &_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 0);

		storeKey(key);
		return false;
	} else {
		key = _vm->_game->checkCollisions(0, 0, 0, 0);
		storeKey(key);

		if (flag == 1)
			return false;

		_vm->_util->waitKey();
	}
	return false;
}

bool Inter_v1::o1_repeatUntil(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *blockPtr;
	int16 size;
	char flag;

	_nestLevel[0]++;
	blockPtr = _vm->_global->_inter_execPtr;

	do {
		_vm->_global->_inter_execPtr = blockPtr;
		size = READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		funcBlock(1);
		_vm->_global->_inter_execPtr = blockPtr + size + 1;
		flag = evalBoolResult();
	} while (flag == 0 && !_breakFlag && !_terminate && !_vm->_quitRequested);

	_nestLevel[0]--;

	if (*_breakFromLevel > -1) {
		_breakFlag = false;
		*_breakFromLevel = -1;
	}
	return false;
}

bool Inter_v1::o1_whileDo(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *blockPtr;
	char *savedIP;
	char flag;
	int16 size;

	_nestLevel[0]++;
	do {
		savedIP = _vm->_global->_inter_execPtr;
		flag = evalBoolResult();

		if (_terminate)
			return false;

		blockPtr = _vm->_global->_inter_execPtr;

		size = READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		if (flag != 0) {
			funcBlock(1);
			_vm->_global->_inter_execPtr = savedIP;
		} else {
			_vm->_global->_inter_execPtr += size;
		}

		if (_breakFlag || _terminate || _vm->_quitRequested) {
			_vm->_global->_inter_execPtr = blockPtr;
			_vm->_global->_inter_execPtr += size;
			break;
		}
	} while (flag != 0);

	_nestLevel[0]--;
	if (*_breakFromLevel > -1) {
		_breakFlag = false;
		*_breakFromLevel = -1;
	}
	return false;
}

void Inter_v1::o1_setRenderFlags(void) {
	_vm->_draw->_renderFlags = _vm->_parse->parseValExpr();
}

void Inter_v1::o1_loadAnim(void) {
	_vm->_scenery->loadAnim(0);
}

void Inter_v1::o1_freeAnim(void) {
	_vm->_scenery->freeAnim(-1);
}

void Inter_v1::o1_updateAnim(void) {
	int16 deltaX;
	int16 deltaY;
	int16 flags;
	int16 frame;
	int16 layer;
	int16 animation;

	evalExpr(&deltaX);
	evalExpr(&deltaY);
	evalExpr(&animation);
	evalExpr(&layer);
	evalExpr(&frame);
	flags = load16();
	_vm->_scenery->updateAnim(layer, frame, animation, flags, deltaX, deltaY, 1);
}

void Inter_v1::o1_initMult(void) {
	int16 oldAnimHeight;
	int16 oldAnimWidth;
	int16 oldObjCount;
	int16 i;
	int16 posXVar;
	int16 posYVar;
	int16 animDataVar;

	oldAnimWidth = _vm->_anim->_areaWidth;
	oldAnimHeight = _vm->_anim->_areaHeight;
	oldObjCount = _vm->_mult->_objCount;

	_vm->_anim->_areaLeft = load16();
	_vm->_anim->_areaTop = load16();
	_vm->_anim->_areaWidth = load16();
	_vm->_anim->_areaHeight = load16();
	_vm->_mult->_objCount = load16();
	posXVar = _vm->_parse->parseVarIndex();
	posYVar = _vm->_parse->parseVarIndex();
	animDataVar = _vm->_parse->parseVarIndex();

	if (_vm->_mult->_objects == 0) {
		_vm->_mult->_renderData = new int16[_vm->_mult->_objCount * 9];
		memset(_vm->_mult->_renderData, 0, _vm->_mult->_objCount * 9 * sizeof(int16));
		_vm->_mult->_objects = new Mult::Mult_Object[_vm->_mult->_objCount];
		memset(_vm->_mult->_objects, 0, _vm->_mult->_objCount * sizeof(Mult::Mult_Object));

		for (i = 0; i < _vm->_mult->_objCount; i++) {
			_vm->_mult->_objects[i].pPosX = (int32 *)(_vm->_global->_inter_variables + i * 4 + (posXVar / 4) * 4);
			_vm->_mult->_objects[i].pPosY = (int32 *)(_vm->_global->_inter_variables + i * 4 + (posYVar / 4) * 4);
			_vm->_mult->_objects[i].pAnimData =
			    (Mult::Mult_AnimData *) (_vm->_global->_inter_variables + animDataVar +
			    i * 4 * _vm->_global->_inter_animDataSize);

			_vm->_mult->_objects[i].pAnimData->isStatic = 1;
			_vm->_mult->_objects[i].tick = 0;
			_vm->_mult->_objects[i].lastLeft = -1;
			_vm->_mult->_objects[i].lastRight = -1;
			_vm->_mult->_objects[i].lastTop = -1;
			_vm->_mult->_objects[i].lastBottom = -1;
		}
	} else if (oldObjCount != _vm->_mult->_objCount) {
		error("o1_initMult: Object count changed, but storage didn't (old count = %d, new count = %d)",
		    oldObjCount, _vm->_mult->_objCount);
	}

	if (_vm->_anim->_animSurf != 0 &&
	    (oldAnimWidth != _vm->_anim->_areaWidth
		|| oldAnimHeight != _vm->_anim->_areaHeight)) {
		_vm->_video->freeSurfDesc(_vm->_anim->_animSurf);
		_vm->_anim->_animSurf = 0;
	}

	if (_vm->_anim->_animSurf == 0) {
		_vm->_anim->_animSurf = _vm->_video->initSurfDesc(_vm->_global->_videoMode,
		    _vm->_anim->_areaWidth, _vm->_anim->_areaHeight, 0);

		_vm->_draw->_spritesArray[22] = _vm->_anim->_animSurf;
	}

	_vm->_video->drawSprite(_vm->_draw->_backSurface, _vm->_anim->_animSurf,
	    _vm->_anim->_areaLeft, _vm->_anim->_areaTop,
	    _vm->_anim->_areaLeft + _vm->_anim->_areaWidth - 1,
	    _vm->_anim->_areaTop + _vm->_anim->_areaHeight - 1, 0, 0, 0);

	debugC(4, DEBUG_GRAPHICS, "o1_initMult: x = %d, y = %d, w = %d, h = %d",
		  _vm->_anim->_areaLeft, _vm->_anim->_areaTop, _vm->_anim->_areaWidth, _vm->_anim->_areaHeight);
	debugC(4, DEBUG_GRAPHICS, "    _vm->_mult->_objCount = %d, animation data size = %d", _vm->_mult->_objCount, _vm->_global->_inter_animDataSize);
}

void Inter_v1::o1_multFreeMult(void) {
	_vm->_mult->freeMult();
}

void Inter_v1::o1_animate(void) {
	_vm->_mult->animate();
}

void Inter_v1::o1_multLoadMult(void) {
	loadMult();
}

void Inter_v1::o1_storeParams(void) {
	_vm->_scenery->interStoreParams();
}

void Inter_v1::o1_getObjAnimSize(void) {
	_vm->_mult->interGetObjAnimSize();
}

void Inter_v1::o1_loadStatic(void) {
	_vm->_scenery->loadStatic(0);
}

void Inter_v1::o1_freeStatic(void) {
	_vm->_scenery->freeStatic(-1);
}

void Inter_v1::o1_renderStatic(void) {
	int16 layer;
	int16 index;

	_vm->_inter->evalExpr(&index);
	_vm->_inter->evalExpr(&layer);
	_vm->_scenery->renderStatic(index, layer);
}

void Inter_v1::o1_loadCurLayer(void) {
	evalExpr(&_vm->_scenery->_curStatic);
	evalExpr(&_vm->_scenery->_curStaticLayer);
}

void Inter_v1::o1_playCDTrack(void) {
	evalExpr(0);
	if (_vm->_features & GF_MAC)
		_vm->_music->playTrack(_vm->_global->_inter_resStr);
	else
		// Used in gob1 CD
		_vm->_cdrom->startTrack(_vm->_global->_inter_resStr);
}

void Inter_v1::o1_getCDTrackPos(void) {
	// Used in gob1 CD

	// Some scripts busy-wait while calling this opcode.
	// This is a very nasty thing to do, so let's add a
	// short delay here. It's probably a safe thing to do.

	_vm->_util->longDelay(1);

	int pos = _vm->_cdrom->getTrackPos();
	if (pos == -1)
		pos = 32767;
	WRITE_VAR(5, pos);
}

void Inter_v1::o1_stopCD(void) {
	if (_vm->_features & GF_MAC)
		_vm->_music->stopPlay();
	else
		// Used in gob1 CD
		_vm->_cdrom->stopPlaying();
}

void Inter_v1::o1_loadFontToSprite(void) {
	int16 i = load16();
	_vm->_draw->_fontToSprite[i].sprite = load16();
	_vm->_draw->_fontToSprite[i].base = load16();
	_vm->_draw->_fontToSprite[i].width = load16();
	_vm->_draw->_fontToSprite[i].height = load16();
}

void Inter_v1::o1_freeFontToSprite(void) {
	int16 i = load16();
	_vm->_draw->_fontToSprite[i].sprite = -1;
	_vm->_draw->_fontToSprite[i].base = -1;
	_vm->_draw->_fontToSprite[i].width = -1;
	_vm->_draw->_fontToSprite[i].height = -1;
}

void Inter_v1::executeDrawOpcode(byte i) {
	debugC(1, DEBUG_DRAWOP, "opcodeDraw %d [0x%x] (%s)", i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV1 op = _opcodesDrawV1[i].proc;

	if (op == NULL)
		warning("unimplemented opcodeDraw: %d", i);
	else
		(this->*op) ();
}

bool Inter_v1::executeFuncOpcode(byte i, byte j, char &cmdCount, int16 &counter, int16 &retFlag) {
	debugC(1, DEBUG_FUNCOP, "opcodeFunc %d.%d [0x%x.0x%x] (%s)", i, j, i, j, getOpcodeFuncDesc(i, j));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcV1 op = _opcodesFuncV1[i*16 + j].proc;

	if (op == NULL)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (cmdCount, counter, retFlag);
	return false;
}

void Inter_v1::executeGoblinOpcode(int i, int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	debugC(1, DEBUG_GOBOP, "opcodeGoblin %d [0x%x] (%s)", i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcV1 op = NULL;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinV1[_goblinFuncLookUp[j][1]].proc;
			break;
		}

	if (op == NULL) {
		warning("unimplemented opcodeGoblin: %d", i);
		_vm->_global->_inter_execPtr -= 2;
		int16 cmd = load16();
		_vm->_global->_inter_execPtr += cmd * 2;
	}
	else
		(this->*op) (extraData, retVarPtr, objDesc);
}

const char *Inter_v1::getOpcodeDrawDesc(byte i) {
	return _opcodesDrawV1[i].desc;
}

const char *Inter_v1::getOpcodeFuncDesc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncV1[i*16 + j].desc;
}

const char *Inter_v1::getOpcodeGoblinDesc(int i) {
	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i)
			return _opcodesGoblinV1[_goblinFuncLookUp[j][1]].desc;
	return "";
}

bool Inter_v1::o1_callSub(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *storedIP = _vm->_global->_inter_execPtr;

//	_vm->_global->_inter_execPtr = (char *)_vm->_game->_totFileData + READ_LE_UINT16(_vm->_global->_inter_execPtr);

	uint16 offset = READ_LE_UINT16(_vm->_global->_inter_execPtr);
	debugC(5, DEBUG_GAMEFLOW, "tot = \"%s\", offset = %d", _vm->_game->_curTotFile, offset);

	// Skipping the copy protection screen in Gobliiins
	if (!_vm->_copyProtection && (_vm->_features & GF_GOB1) && (offset == 3905)
			&& !scumm_stricmp(_vm->_game->_curTotFile, "intro.tot")) {
		debugC(2, DEBUG_GAMEFLOW, "Skipping copy protection screen");
		_vm->_global->_inter_execPtr += 2;
		return false;
	}
	// Skipping the copy protection screen in Gobliins 2
	if (!_vm->_copyProtection && (_vm->_features & GF_GOB2) && (offset == 1746)
			&& !scumm_stricmp(_vm->_game->_curTotFile, "intro0.tot")) {
		warning("=> Skipping copy protection screen");
		debugC(2, DEBUG_GAMEFLOW, "Skipping copy protection screen");
		_vm->_global->_inter_execPtr += 2;
		return false;
	}

	_vm->_global->_inter_execPtr = (char *)_vm->_game->_totFileData + offset;

	if (counter == cmdCount && retFlag == 2)
		return true;

	callSub(2);
	_vm->_global->_inter_execPtr = storedIP + 2;

	return false;
}

bool Inter_v1::o1_drawPrintText(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->printText();
	return false;
}

bool Inter_v1::o1_call(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *callAddr;

	checkSwitchTable(&callAddr);
	char *storedIP = _vm->_global->_inter_execPtr;
	_vm->_global->_inter_execPtr = callAddr;

	if (counter == cmdCount && retFlag == 2)
		return true;

	funcBlock(0);
	_vm->_global->_inter_execPtr = storedIP;

	return false;
}

bool Inter_v1::o1_callBool(char &cmdCount, int16 &counter, int16 &retFlag) {
	byte cmd;
	bool boolRes = evalBoolResult() != 0;
	if (boolRes != 0) {
		if (counter == cmdCount
				&& retFlag == 2)
			return true;

		char *storedIP = _vm->_global->_inter_execPtr;
		funcBlock(0);
		_vm->_global->_inter_execPtr = storedIP;

		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		debugC(5, DEBUG_GAMEFLOW, "cmd = %d", (int16)*_vm->_global->_inter_execPtr);
		cmd = (byte)(*_vm->_global->_inter_execPtr) >> 4;
		_vm->_global->_inter_execPtr++;
		if (cmd != 12)
			return false;

		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
	} else {
		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;

		debugC(5, DEBUG_GAMEFLOW, "cmd = %d", (int16)*_vm->_global->_inter_execPtr);
		cmd = (byte)(*_vm->_global->_inter_execPtr) >> 4;
		_vm->_global->_inter_execPtr++;
		if (cmd != 12)
			return false;

		if (counter == cmdCount && retFlag == 2)
			return true;

		char *storedIP = _vm->_global->_inter_execPtr;
		funcBlock(0);
		_vm->_global->_inter_execPtr = storedIP;
		_vm->_global->_inter_execPtr += READ_LE_UINT16(_vm->_global->_inter_execPtr + 2) + 2;
	}
	return false;
}

bool Inter_v1::o1_palLoad(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 i;
	int16 ind1;
	int16 ind2;
	byte cmd;
	char *palPtr;

	cmd = *_vm->_global->_inter_execPtr++;
	_vm->_draw->_applyPal = 0;
	if (cmd & 0x80)
		cmd &= 0x7f;
	else
		_vm->_draw->_applyPal = 1;

	if (cmd == 49) {
		warning("o1_palLoad: cmd == 49 is not supported");
		//var_B = 1;
		for (i = 0; i < 18; i++, _vm->_global->_inter_execPtr++) {
			if (i < 2) {
				if (_vm->_draw->_applyPal == 0)
					continue;

				_vm->_draw->_unusedPalette1[i] = *_vm->_global->_inter_execPtr;
				continue;
			}
			//if (*inter_execPtr != 0)
			//      var_B = 0;

			ind1 = *_vm->_global->_inter_execPtr >> 4;
			ind2 = (*_vm->_global->_inter_execPtr & 0xf);

			_vm->_draw->_unusedPalette1[i] =
			    ((_vm->_draw->_palLoadData1[ind1] + _vm->_draw->_palLoadData2[ind2]) << 8) +
			    (_vm->_draw->_palLoadData2[ind1] + _vm->_draw->_palLoadData1[ind2]);
		}

		_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
	}

	switch (cmd) {
	case 52:
		for (i = 0; i < 16; i++, _vm->_global->_inter_execPtr += 3) {
			_vm->_draw->_vgaSmallPalette[i].red = _vm->_global->_inter_execPtr[0];
			_vm->_draw->_vgaSmallPalette[i].green = _vm->_global->_inter_execPtr[1];
			_vm->_draw->_vgaSmallPalette[i].blue = _vm->_global->_inter_execPtr[2];
		}
		break;

	case 50:
		for (i = 0; i < 16; i++, _vm->_global->_inter_execPtr++)
			_vm->_draw->_unusedPalette2[i] = *_vm->_global->_inter_execPtr;
		break;

	case 53:
		palPtr = _vm->_game->loadTotResource(_vm->_inter->load16());
		memcpy((char *)_vm->_draw->_vgaPalette, palPtr, 768);
		break;

	case 54:
		memset((char *)_vm->_draw->_vgaPalette, 0, 768);
		break;
	}
	if (!_vm->_draw->_applyPal) {
		_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
		_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;

		if (_vm->_global->_videoMode != 0x13)
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)_vm->_draw->_vgaSmallPalette;
		else
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)_vm->_draw->_vgaPalette;

		_vm->_palanim->fade((Video::PalDesc *) _vm->_global->_pPaletteDesc, 0, 0);
	}
	return false;
}

bool Inter_v1::o1_setcmdCount(char &cmdCount, int16 &counter, int16 &retFlag) {
	cmdCount = *_vm->_global->_inter_execPtr++;
	counter = 0;
	return false;
}

bool Inter_v1::o1_return(char &cmdCount, int16 &counter, int16 &retFlag) {
	if (retFlag != 2)
		_breakFlag = true;

	_vm->_global->_inter_execPtr = 0;
	return false;
}

bool Inter_v1::o1_renewTimeInVars(char &cmdCount, int16 &counter, int16 &retFlag) {
	renewTimeInVars();
	return false;
}

bool Inter_v1::o1_speakerOn(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_snd->speakerOn(_vm->_parse->parseValExpr(), -1);
	return false;
}

bool Inter_v1::o1_speakerOff(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_snd->speakerOff();
	return false;
}

bool Inter_v1::o1_goblinFunc(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 cmd;
	int16 extraData = 0;
	Goblin::Gob_Object *objDesc = NULL;
	int32 *retVarPtr;
	bool objDescSet = false;

	retVarPtr = (int32 *)VAR_ADDRESS(59);

	cmd = load16();
	_vm->_global->_inter_execPtr += 2;
	if (cmd > 0 && cmd < 17) {
		extraData = load16();
		objDesc = _vm->_goblin->_objects[extraData];
		objDescSet = true;
		extraData = load16();
	}

	if (cmd > 90 && cmd < 107) {
		extraData = load16();
		objDesc = _vm->_goblin->_goblins[extraData];
		objDescSet = true;
		extraData = load16();
		cmd -= 90;
	}

	if (cmd > 110 && cmd < 128) {
		extraData = load16();
		objDesc = _vm->_goblin->_goblins[extraData];
		objDescSet = true;
		cmd -= 90;
	} else if (cmd > 20 && cmd < 38) {
		extraData = load16();
		objDesc = _vm->_goblin->_objects[extraData];
		objDescSet = true;
	}

/*
	NB: The original gobliiins engine did not initialize the objDesc
	variable, so we manually check if objDesc is properly set before
	checking if it is zero. If it was not set, we do not return. This
	fixes a crash in the EGA version if the life bar is depleted, because
	interFunc is called multiple times with cmd == 39.
	Bug #1324814
*/

	if (cmd < 40 && objDescSet && objDesc == 0)
		return false;

	executeGoblinOpcode(cmd, extraData, retVarPtr, objDesc);

	return false;
}

bool Inter_v1::o1_returnTo(char &cmdCount, int16 &counter, int16 &retFlag) {
	if (retFlag == 1) {
		_breakFlag = true;
		_vm->_global->_inter_execPtr = 0;
		return true;
	}

	if (*_nestLevel == 0)
		return false;

	*_breakFromLevel = *_nestLevel;
	_breakFlag = true;
	_vm->_global->_inter_execPtr = 0;
	return true;
}

bool Inter_v1::o1_setBackDelta(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->_backDeltaX = _vm->_parse->parseValExpr();
	_vm->_draw->_backDeltaY = _vm->_parse->parseValExpr();
	return false;
}

bool Inter_v1::o1_loadSound(char &cmdCount, int16 &counter, int16 &retFlag) {
	loadSound(-1);
	return false;
}

bool Inter_v1::o1_freeSoundSlot(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_game->freeSoundSlot(-1);
	return false;
}

bool Inter_v1::o1_waitEndPlay(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_snd->waitEndPlay();
	return false;
}

bool Inter_v1::o1_animatePalette(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->blitInvalidated();
	_vm->_util->waitEndFrame();
	animPalette();
	storeKey(_vm->_game->checkKeys(&_vm->_global->_inter_mouseX,
		&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons, 0));
	return false;
}

bool Inter_v1::o1_animateCursor(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->animateCursor(1);
	return false;
}

bool Inter_v1::o1_blitCursor(char &cmdCount, int16 &counter, int16 &retFlag) {
	_vm->_draw->blitCursor();
	return false;
}

void Inter_v1::o1_setState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->state = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemStateVarPtr = extraData;
}

void Inter_v1::o1_setCurFrame(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->curFrame = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemFrameVarPtr = extraData;
}

void Inter_v1::o1_setNextState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->nextState = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemNextStateVarPtr = extraData;
}

void Inter_v1::o1_setMultState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->multState = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemMultStateVarPtr = extraData;
}

void Inter_v1::o1_setOrder(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->order = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemOrderVarPtr = extraData;
}

void Inter_v1::o1_setActionStartState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->actionStartState = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemActStartStVarPtr = extraData;
}

void Inter_v1::o1_setCurLookDir(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->curLookDir = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemLookDirVarPtr = extraData;
}

void Inter_v1::o1_setType(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->type = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemTypeVarPtr = extraData;

	if (extraData == 0)
		objDesc->toRedraw = 1;
}

void Inter_v1::o1_setNoTick(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->noTick = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemNoTickVarPtr = extraData;
}

void Inter_v1::o1_setPickable(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->pickable = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemPickableVarPtr = extraData;
}

void Inter_v1::o1_setXPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->xPos = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemScrXVarPtr = extraData;
}

void Inter_v1::o1_setYPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->yPos = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemScrYVarPtr = extraData;
}

void Inter_v1::o1_setDoAnim(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->doAnim = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemDoAnimVarPtr = extraData;
}

void Inter_v1::o1_setRelaxTime(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->relaxTime = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemRelaxVarPtr = extraData;
}

void Inter_v1::o1_setMaxTick(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	objDesc->maxTick = extraData;
	if (objDesc == _vm->_goblin->_actDestItemDesc)
		*_vm->_goblin->_destItemMaxTickVarPtr = extraData;
}

void Inter_v1::o1_getState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->state;
}

void Inter_v1::o1_getCurFrame(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->curFrame;
}

void Inter_v1::o1_getNextState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->nextState;
}

void Inter_v1::o1_getMultState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->multState;
}

void Inter_v1::o1_getOrder(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->order;
}

void Inter_v1::o1_getActionStartState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->actionStartState;
}

void Inter_v1::o1_getCurLookDir(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->curLookDir;
}

void Inter_v1::o1_getType(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->type;
}

void Inter_v1::o1_getNoTick(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->noTick;
}

void Inter_v1::o1_getPickable(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->pickable;
}

void Inter_v1::o1_getObjMaxFrame(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = _vm->_goblin->getObjMaxFrame(objDesc);
}

void Inter_v1::o1_getXPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->xPos;
}

void Inter_v1::o1_getYPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->yPos;
}

void Inter_v1::o1_getDoAnim(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->doAnim;
}

void Inter_v1::o1_getRelaxTime(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->relaxTime;
}

void Inter_v1::o1_getMaxTick(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = objDesc->maxTick;
}

void Inter_v1::o1_manipulateMap(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 xPos = load16();
	int16 yPos = load16();
	int16 item = load16();

	manipulateMap(xPos, yPos, item);
}

void Inter_v1::o1_getItem(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 xPos = load16();
	int16 yPos = load16();

	if ((_vm->_map->_itemsMap[yPos][xPos] & 0xff00) != 0)
		*retVarPtr = (_vm->_map->_itemsMap[yPos][xPos] & 0xff00) >> 8;
	else
		*retVarPtr = _vm->_map->_itemsMap[yPos][xPos];
}

void Inter_v1::o1_manipulateMapIndirect(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 xPos = load16();
	int16 yPos = load16();
	int16 item = load16();

	xPos = VAR(xPos);
	yPos = VAR(yPos);
	item = VAR(item);

	manipulateMap(xPos, yPos, item);
}

void Inter_v1::o1_getItemIndirect(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 xPos = load16();
	int16 yPos = load16();

	xPos = VAR(xPos);
	yPos = VAR(yPos);

	if ((_vm->_map->_itemsMap[yPos][xPos] & 0xff00) != 0)
		*retVarPtr = (_vm->_map->_itemsMap[yPos][xPos] & 0xff00) >> 8;
	else
		*retVarPtr = _vm->_map->_itemsMap[yPos][xPos];
}

void Inter_v1::o1_setPassMap(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 xPos = load16();
	int16 yPos = load16();
	int16 val = load16();
	_vm->_map->setPass(xPos, yPos, val);
}

void Inter_v1::o1_setGoblinPosH(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 layer;
	int16 item = load16();
	int16 xPos = load16();
	int16 yPos = load16();

	_vm->_goblin->_gobPositions[item].x = xPos * 2;
	_vm->_goblin->_gobPositions[item].y = yPos * 2;

	objDesc = _vm->_goblin->_goblins[item];
	objDesc->nextState = 21;

	_vm->_goblin->nextLayer(objDesc);

	layer = objDesc->stateMach[objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
			objDesc->xPos, objDesc->yPos, 0);

	objDesc->yPos =
			(_vm->_goblin->_gobPositions[item].y * 6 + 6) - (_vm->_scenery->_toRedrawBottom -
			_vm->_scenery->_animTop);
	objDesc->xPos =
			_vm->_goblin->_gobPositions[item].x * 12 - (_vm->_scenery->_toRedrawLeft -
			_vm->_scenery->_animLeft);

	objDesc->curFrame = 0;
	objDesc->state = 21;
	if (_vm->_goblin->_currentGoblin == item) {
		*_vm->_goblin->_curGobScrXVarPtr = objDesc->xPos;
		*_vm->_goblin->_curGobScrYVarPtr = objDesc->yPos;

		*_vm->_goblin->_curGobFrameVarPtr = 0;
		*_vm->_goblin->_curGobStateVarPtr = 18;
		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[item].x;
		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[item].y;
	}
}

void Inter_v1::o1_getGoblinPosXH(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 item = load16();
	*retVarPtr = _vm->_goblin->_gobPositions[item].x >> 1;
}

void Inter_v1::o1_getGoblinPosYH(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 item = load16();
	*retVarPtr = _vm->_goblin->_gobPositions[item].y >> 1;
}

void Inter_v1::o1_setGoblinMultState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 layer;
	int16 item = load16();
	int16 xPos = load16();
	int16 yPos = load16();

	objDesc = _vm->_goblin->_goblins[item];
	if (yPos == 0) {
		objDesc->multState = xPos;
		objDesc->nextState = xPos;
		_vm->_goblin->nextLayer(objDesc);

		layer = objDesc->stateMach[objDesc->state][0]->layer;

		objDesc->xPos =
				_vm->_scenery->_animations[objDesc->animation].layers[layer].posX;
		objDesc->yPos =
				_vm->_scenery->_animations[objDesc->animation].layers[layer].posY;

		*_vm->_goblin->_curGobScrXVarPtr = objDesc->xPos;
		*_vm->_goblin->_curGobScrYVarPtr = objDesc->yPos;
		*_vm->_goblin->_curGobFrameVarPtr = 0;
		*_vm->_goblin->_curGobStateVarPtr = objDesc->state;
		*_vm->_goblin->_curGobNextStateVarPtr = objDesc->nextState;
		*_vm->_goblin->_curGobMultStateVarPtr = objDesc->multState;
		*_vm->_goblin->_curGobMaxFrameVarPtr =
				_vm->_goblin->getObjMaxFrame(objDesc);
		_vm->_goblin->_noPick = 1;
		return;
	}

	objDesc->multState = 21;
	objDesc->nextState = 21;
	objDesc->state = 21;
	_vm->_goblin->nextLayer(objDesc);
	layer = objDesc->stateMach[objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
			objDesc->xPos, objDesc->yPos, 0);

	objDesc->yPos =
			(yPos * 6 + 6) - (_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
	objDesc->xPos =
			xPos * 12 - (_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

	_vm->_goblin->_gobPositions[item].x = xPos;
	_vm->_goblin->_pressedMapX = xPos;
	_vm->_map->_curGoblinX = xPos;

	_vm->_goblin->_gobPositions[item].y = yPos;
	_vm->_goblin->_pressedMapY = yPos;
	_vm->_map->_curGoblinY = yPos;

	*_vm->_goblin->_curGobScrXVarPtr = objDesc->xPos;
	*_vm->_goblin->_curGobScrYVarPtr = objDesc->yPos;
	*_vm->_goblin->_curGobFrameVarPtr = 0;
	*_vm->_goblin->_curGobStateVarPtr = 21;
	*_vm->_goblin->_curGobNextStateVarPtr = 21;
	*_vm->_goblin->_curGobMultStateVarPtr = -1;
	_vm->_goblin->_noPick = 0;
}

void Inter_v1::o1_setGoblinPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 layer;
	int16 item = load16();
	int16 xPos = load16();
	int16 yPos = load16();

	_vm->_goblin->_gobPositions[item].x = xPos;
	_vm->_goblin->_gobPositions[item].y = yPos;

	objDesc = _vm->_goblin->_goblins[item];
	objDesc->nextState = 21;
	_vm->_goblin->nextLayer(objDesc);

	layer = objDesc->stateMach[objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
			objDesc->xPos, objDesc->yPos, 0);

	objDesc->yPos =
			(yPos * 6 + 6) - (_vm->_scenery->_toRedrawBottom - _vm->_scenery->_animTop);
	objDesc->xPos =
			xPos * 12 - (_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);

	objDesc->curFrame = 0;
	objDesc->state = 21;

	if (_vm->_goblin->_currentGoblin == item) {
		*_vm->_goblin->_curGobScrXVarPtr = objDesc->xPos;
		*_vm->_goblin->_curGobScrYVarPtr = objDesc->yPos;
		*_vm->_goblin->_curGobFrameVarPtr = 0;
		*_vm->_goblin->_curGobStateVarPtr = 18;

		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[item].x;
		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[item].y;
	}
}

void Inter_v1::o1_setGoblinState(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 layer;
	int16 item = load16();
	int16 state = load16();

	objDesc = _vm->_goblin->_goblins[item];
	objDesc->nextState = state;

	_vm->_goblin->nextLayer(objDesc);
	layer = objDesc->stateMach[objDesc->state][0]->layer;

	objDesc->xPos =
			_vm->_scenery->_animations[objDesc->animation].layers[layer].posX;
	objDesc->yPos =
			_vm->_scenery->_animations[objDesc->animation].layers[layer].posY;

	if (item == _vm->_goblin->_currentGoblin) {
		*_vm->_goblin->_curGobScrXVarPtr = objDesc->xPos;
		*_vm->_goblin->_curGobScrYVarPtr = objDesc->yPos;
		*_vm->_goblin->_curGobFrameVarPtr = 0;
		*_vm->_goblin->_curGobStateVarPtr = objDesc->state;
		*_vm->_goblin->_curGobMultStateVarPtr = objDesc->multState;
	}
}

void Inter_v1::o1_setGoblinStateRedraw(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 layer;
	int16 item = load16();
	int16 state = load16();
	objDesc = _vm->_goblin->_objects[item];

	objDesc->nextState = state;

	_vm->_goblin->nextLayer(objDesc);
	layer = objDesc->stateMach[objDesc->state][0]->layer;
	objDesc->xPos =
			_vm->_scenery->_animations[objDesc->animation].layers[layer].posX;
	objDesc->yPos =
			_vm->_scenery->_animations[objDesc->animation].layers[layer].posY;

	objDesc->toRedraw = 1;
	objDesc->type = 0;
	if (objDesc == _vm->_goblin->_actDestItemDesc) {
		*_vm->_goblin->_destItemScrXVarPtr = objDesc->xPos;
		*_vm->_goblin->_destItemScrYVarPtr = objDesc->yPos;

		*_vm->_goblin->_destItemStateVarPtr = objDesc->state;
		*_vm->_goblin->_destItemNextStateVarPtr = -1;
		*_vm->_goblin->_destItemMultStateVarPtr = -1;
		*_vm->_goblin->_destItemFrameVarPtr = 0;
	}
}

void Inter_v1::o1_setGoblinUnk14(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 item = load16();
	int16 val = load16();
	objDesc = _vm->_goblin->_objects[item];
	objDesc->unk14 = val;
}

void Inter_v1::o1_setItemIdInPocket(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->_itemIdInPocket = load16();
}

void Inter_v1::o1_setItemIndInPocket(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->_itemIndInPocket = load16();
}

void Inter_v1::o1_getItemIdInPocket(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = _vm->_goblin->_itemIdInPocket;
}

void Inter_v1::o1_getItemIndInPocket(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	*retVarPtr = _vm->_goblin->_itemIndInPocket;
}

void Inter_v1::o1_setItemPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 item = load16();
	int16 xPos = load16();
	int16 yPos = load16();
	int16 val = load16();

	_vm->_map->_itemPoses[item].x = xPos;
	_vm->_map->_itemPoses[item].y = yPos;
	_vm->_map->_itemPoses[item].orient = val;
}

void Inter_v1::o1_decRelaxTime(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	objDesc = _vm->_goblin->_objects[extraData];

	objDesc->relaxTime--;
	if (objDesc->relaxTime < 0 &&
			_vm->_goblin->getObjMaxFrame(objDesc) == objDesc->curFrame) {
		objDesc->relaxTime = _vm->_util->getRandom(100) + 50;
		objDesc->curFrame = 0;
		objDesc->toRedraw = 1;
	}
}

void Inter_v1::o1_getGoblinPosX(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 item = load16();
	*retVarPtr = _vm->_goblin->_gobPositions[item].x;
}

void Inter_v1::o1_getGoblinPosY(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 item = load16();
	*retVarPtr = _vm->_goblin->_gobPositions[item].y;
}

void Inter_v1::o1_clearPathExistence(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->_pathExistence = 0;
}

void Inter_v1::o1_setGoblinVisible(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	_vm->_goblin->_goblins[extraData]->visible = 1;
}

void Inter_v1::o1_setGoblinInvisible(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	_vm->_goblin->_goblins[extraData]->visible = 0;
}

void Inter_v1::o1_getObjectIntersect(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	int16 item = load16();

	objDesc = _vm->_goblin->_objects[extraData];
	if (_vm->_goblin->objIntersected(objDesc, _vm->_goblin->_goblins[item]) != 0)
		*retVarPtr = 1;
	else
		*retVarPtr = 0;
}

void Inter_v1::o1_getGoblinIntersect(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	int16 item = load16();

	objDesc = _vm->_goblin->_goblins[extraData];
	if (_vm->_goblin->objIntersected(objDesc, _vm->_goblin->_goblins[item]) != 0)
		*retVarPtr = 1;
	else
		*retVarPtr = 0;
}

void Inter_v1::o1_loadObjects(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	if (_vm->_game->_extHandle >= 0)
		_vm->_dataio->closeData(_vm->_game->_extHandle);

	_vm->_goblin->loadObjects((char *)VAR_ADDRESS(extraData));
	_vm->_game->_extHandle = _vm->_dataio->openData(_vm->_game->_curExtFile);
}

void Inter_v1::o1_freeObjects(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->freeAllObjects();
}

void Inter_v1::o1_animateObjects(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->animateObjects();
}

void Inter_v1::o1_drawObjects(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->drawObjects();

	if (_vm->_features & GF_MAC)
		_vm->_music->playBgMusic();
	else if (_vm->_cdrom->getTrackPos() == -1)
		_vm->_cdrom->playBgMusic();
}

void Inter_v1::o1_loadMap(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_map->loadMapsInitGobs();
}

void Inter_v1::o1_moveGoblin(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	int16 item;
	extraData = load16();
	int16 xPos = load16();

	if ((uint16)VAR(xPos) == 0) {
		item =
				_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin], 1,
				(uint16)VAR(extraData));
	} else {
		item =
				_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin], 1, 3);
	}

	if (item != 0)
		_vm->_goblin->switchGoblin(item);
}

void Inter_v1::o1_switchGoblin(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->switchGoblin(0);
}

void Inter_v1::o1_loadGoblin(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->loadGobDataFromVars();
}

void Inter_v1::o1_writeTreatItem(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	int16 cmd = load16();
	int16 xPos = load16();

	if ((uint16)VAR(xPos) == 0) {
		WRITE_VAR(cmd, _vm->_goblin->treatItem((uint16)VAR(extraData)));
		return;
	}

	WRITE_VAR(cmd, _vm->_goblin->treatItem(3));
}

void Inter_v1::o1_moveGoblin0(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	_vm->_goblin->doMove(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin], 0, 0);
}

void Inter_v1::o1_setGoblinTarget(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	if (VAR(extraData) != 0)
		_vm->_goblin->_goesAtTarget = 1;
	else
		_vm->_goblin->_goesAtTarget = 0;
}

void Inter_v1::o1_setGoblinObjectsPos(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	extraData = load16();
	extraData = VAR(extraData);
	_vm->_goblin->_objects[10]->xPos = extraData;

	extraData = load16();
	extraData = VAR(extraData);
	_vm->_goblin->_objects[10]->yPos = extraData;
}

void Inter_v1::o1_initGoblin(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	Goblin::Gob_Object *gobDesc = _vm->_goblin->_goblins[0];
	int16 xPos;
	int16 yPos;
	int16 layer;

	if (_vm->_goblin->_currentGoblin != 0) {
		_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]->doAnim = 1;
		_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]->nextState = 21;

		_vm->_goblin->nextLayer(_vm->_goblin->_goblins[_vm->_goblin->_currentGoblin]);
		_vm->_goblin->_currentGoblin = 0;

		gobDesc->doAnim = 0;
		gobDesc->type = 0;
		gobDesc->toRedraw = 1;

		_vm->_goblin->_pressedMapX = _vm->_goblin->_gobPositions[0].x;
		_vm->_map->_destX = _vm->_goblin->_gobPositions[0].x;
		_vm->_goblin->_gobDestX = _vm->_goblin->_gobPositions[0].x;

		_vm->_goblin->_pressedMapY = _vm->_goblin->_gobPositions[0].y;
		_vm->_map->_destY = _vm->_goblin->_gobPositions[0].y;
		_vm->_goblin->_gobDestY = _vm->_goblin->_gobPositions[0].y;

		*_vm->_goblin->_curGobVarPtr = 0;
		_vm->_goblin->_pathExistence = 0;
		_vm->_goblin->_readyToAct = 0;
	}

	if (gobDesc->state != 10 && _vm->_goblin->_itemIndInPocket != -1 &&
			_vm->_goblin->getObjMaxFrame(gobDesc) == gobDesc->curFrame) {

		gobDesc->stateMach = gobDesc->realStateMach;
		xPos = _vm->_goblin->_gobPositions[0].x;
		yPos = _vm->_goblin->_gobPositions[0].y;

		gobDesc->nextState = 10;
		layer = _vm->_goblin->nextLayer(gobDesc);

		_vm->_scenery->updateAnim(layer, 0, gobDesc->animation, 0,
				gobDesc->xPos, gobDesc->yPos, 0);

		gobDesc->yPos =
				(yPos * 6 + 6) - (_vm->_scenery->_toRedrawBottom -
				_vm->_scenery->_animTop);
		gobDesc->xPos =
				xPos * 12 - (_vm->_scenery->_toRedrawLeft - _vm->_scenery->_animLeft);
	}

	if (gobDesc->state != 10)
		return;

	if (_vm->_goblin->_itemIndInPocket == -1)
		return;

	if (gobDesc->curFrame != 10)
		return;

	objDesc = _vm->_goblin->_objects[_vm->_goblin->_itemIndInPocket];
	objDesc->type = 0;
	objDesc->toRedraw = 1;
	objDesc->curFrame = 0;

	objDesc->order = gobDesc->order;
	objDesc->animation =
			objDesc->stateMach[objDesc->state][0]->animation;

	layer = objDesc->stateMach[objDesc->state][0]->layer;

	_vm->_scenery->updateAnim(layer, 0, objDesc->animation, 0,
			objDesc->xPos, objDesc->yPos, 0);

	objDesc->yPos +=
			(_vm->_goblin->_gobPositions[0].y * 6 + 5) - _vm->_scenery->_toRedrawBottom;

	if (gobDesc->curLookDir == 4) {
		objDesc->xPos += _vm->_goblin->_gobPositions[0].x * 12 + 14
				- (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	} else {
		objDesc->xPos += _vm->_goblin->_gobPositions[0].x * 12
				- (_vm->_scenery->_toRedrawLeft + _vm->_scenery->_toRedrawRight) / 2;
	}

	_vm->_goblin->_itemIndInPocket = -1;
	_vm->_goblin->_itemIdInPocket = -1;
	_vm->_util->beep(50);
}

int16 Inter_v1::loadSound(int16 slot) {
	char *dataPtr;
	int16 id;

	if (slot == -1)
		slot = _vm->_parse->parseValExpr();

	id = load16();
	if (id == -1) {
		_vm->_global->_inter_execPtr += 9;
		return 0;
	}

	if (id >= 30000) {
		dataPtr = _vm->_game->loadExtData(id, 0, 0);
		_vm->_game->_soundFromExt[slot] = 1;
	} else {
		dataPtr = _vm->_game->loadTotResource(id);
		_vm->_game->_soundFromExt[slot] = 0;
	}

	_vm->_game->loadSound(slot, dataPtr);
	return 0;
}

void Inter_v1::loadMult(void) {
	int16 val;
	int16 objIndex;
	int16 i;
	char *lmultData;

	debugC(4, DEBUG_GAMEFLOW, "Inter_v1::loadMult(): Loading...");

	evalExpr(&objIndex);
	evalExpr(&val);
	*_vm->_mult->_objects[objIndex].pPosX = val;
	evalExpr(&val);
	*_vm->_mult->_objects[objIndex].pPosY = val;

	lmultData = (char *)_vm->_mult->_objects[objIndex].pAnimData;
	for (i = 0; i < 11; i++) {
		if ((char)READ_LE_UINT16(_vm->_global->_inter_execPtr) == (char)99) {
			evalExpr(&val);
			lmultData[i] = val;
		} else {
			_vm->_global->_inter_execPtr++;
		}
	}
}

void Inter_v1::storeMouse(void) {
	WRITE_VAR(2, _vm->_global->_inter_mouseX);
	WRITE_VAR(3, _vm->_global->_inter_mouseY);
	WRITE_VAR(4, _vm->_game->_mouseButtons);
}

void Inter_v1::animPalette(void) {
	int16 i;
	Video::Color col;

	if (_animPalDir[0] == 0)
		return;

	_vm->_video->waitRetrace(_vm->_global->_videoMode);

	if (_animPalDir[0] == -1) {
		col = _vm->_draw->_vgaSmallPalette[_animPalLowIndex[0]];

		for (i = _animPalLowIndex[0]; i < _animPalHighIndex[0]; i++)
			_vm->_draw->_vgaSmallPalette[i] = _vm->_draw->_vgaSmallPalette[i + 1];

		_vm->_draw->_vgaSmallPalette[_animPalHighIndex[0]] = col;
	} else {
		col = _vm->_draw->_vgaSmallPalette[_animPalHighIndex[0]];
		for (i = _animPalHighIndex[0]; i > _animPalLowIndex[0]; i--)
			_vm->_draw->_vgaSmallPalette[i] = _vm->_draw->_vgaSmallPalette[i - 1];

		_vm->_draw->_vgaSmallPalette[_animPalLowIndex[0]] = col;
	}

	_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaSmallPalette;
	_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

} // End of namespace Gob
