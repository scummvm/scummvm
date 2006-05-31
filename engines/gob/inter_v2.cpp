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
#include "gob/palanim.h"
#include "gob/anim.h"
#include "gob/music.h"
#include "gob/map.h"

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_v2, x)

const int Inter_v2::_goblinFuncLookUp[][2] = {
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

Inter_v2::Inter_v2(GobEngine *vm) : Inter_v1(vm) {
	setupOpcodes();
}

void Inter_v2::setupOpcodes(void) {
	static const OpcodeDrawEntryV2 opcodesDraw[256] = {
		/* 00 */
		OPCODE(o1_loadMult),
		OPCODE(o2_playMult),
		OPCODE(o1_freeMult),
		{NULL, ""},
		/* 04 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		OPCODE(o2_initCursor),
		/* 08 */
		OPCODE(o1_initCursorAnim),
		OPCODE(o1_clearCursorAnim),
		OPCODE(o2_setRenderFlags),
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
		OPCODE(o2_multSub),
		/* 14 */
		OPCODE(o2_initMult),
		OPCODE(o1_multFreeMult),
		OPCODE(o1_animate),
		OPCODE(o1_multLoadMult),
		/* 18 */
		OPCODE(o1_storeParams),
		OPCODE(o1_getObjAnimSize),
		OPCODE(o1_loadStatic),
		OPCODE(o1_freeStatic),
		/* 1C */
		OPCODE(o2_renderStatic),
		OPCODE(o2_loadCurLayer),
		{NULL, ""},
		{NULL, ""},
		/* 20 */
		OPCODE(o2_playCDTrack),
		OPCODE(o2_drawStub),
		OPCODE(o2_stopCD),
		OPCODE(o2_readLIC),
		/* 24 */
		OPCODE(o2_freeLIC),
		OPCODE(o2_getCDTrackPos),
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
		OPCODE(o2_totSub),
		OPCODE(o2_switchTotSub),
		OPCODE(o2_drawStub),
		OPCODE(o2_drawStub),
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
		OPCODE(o2_loadMapObjects),
		OPCODE(o2_freeGoblins),
		OPCODE(o2_moveGoblin),
		OPCODE(o2_writeGoblinPos),
		/* 54 */
		OPCODE(o2_stub0x54),
		OPCODE(o2_drawStub),
		OPCODE(o2_placeGoblin),
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
		OPCODE(o2_stub0x80),
		OPCODE(o2_drawStub),
		OPCODE(o2_stub0x82),
		OPCODE(o2_playImd),
		/* 84 */
		OPCODE(o2_drawStub),
		OPCODE(o2_stub0x85),
		OPCODE(o2_drawStub),
		OPCODE(o2_drawStub),
		/* 88 */
		OPCODE(o2_drawStub),
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

	static const OpcodeFuncEntryV2 opcodesFunc[80] = {
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
		OPCODE(o2_evaluateStore),
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
		OPCODE(o2_loadTot),
		OPCODE(o2_palLoad),
		/* 14 */
		OPCODE(o1_keyFunc),
		OPCODE(o1_capturePush),
		OPCODE(o1_capturePop),
		OPCODE(o2_animPalInit),
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
		OPCODE(o2_goblinFunc),
		OPCODE(o1_createSprite),
		OPCODE(o2_freeSprite),
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
		OPCODE(o2_playSound),
		OPCODE(o1_stopSound),
		OPCODE(o2_loadSound),
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

	static const OpcodeGoblinEntryV2 opcodesGoblin[71] = {
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
		OPCODE(o2_setPickable),
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

	_opcodesDrawV2 = opcodesDraw;
	_opcodesFuncV2 = opcodesFunc;
	_opcodesGoblinV2 = opcodesGoblin;
}

void Inter_v2::executeDrawOpcode(byte i) {
	debugC(1, DEBUG_DRAWOP, "opcodeDraw %d [0x%x] (%s)", i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV2 op = _opcodesDrawV2[i].proc;

	if (op == NULL)
		warning("unimplemented opcodeDraw: %d", i);
	else
		(this->*op) ();
}

bool Inter_v2::executeFuncOpcode(byte i, byte j, char &cmdCount, int16 &counter, int16 &retFlag) {
	debugC(1, DEBUG_FUNCOP, "opcodeFunc %d.%d [0x%x.0x%x] (%s)", i, j, i, j, getOpcodeFuncDesc(i, j));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcV2 op = _opcodesFuncV2[i*16 + j].proc;

	if (op == NULL)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (cmdCount, counter, retFlag);
	return false;
}

void Inter_v2::executeGoblinOpcode(int i, int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	debugC(1, DEBUG_GOBOP, "opcodeGoblin %d [0x%x] (%s)", i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcV2 op = NULL;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinV2[_goblinFuncLookUp[j][1]].proc;
			break;
		}

	if (op == NULL) {
		warning("unimplemented opcodeGoblin: %d", i);
		_vm->_global->_inter_execPtr -= 2;
		_vm->_global->_inter_execPtr += load16() * 2;
	}
	else
		(this->*op) (extraData, retVarPtr, objDesc);
}

const char *Inter_v2::getOpcodeDrawDesc(byte i) {
	return _opcodesDrawV2[i].desc;
}

const char *Inter_v2::getOpcodeFuncDesc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncV2[i*16 + j].desc;
}

const char *Inter_v2::getOpcodeGoblinDesc(int i) {
	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i)
			return _opcodesGoblinV2[_goblinFuncLookUp[j][1]].desc;
	return "";
}

void Inter_v2::o2_stub0x54(void) {
	int16 index = _vm->_parse->parseValExpr();

	_vm->_mult->_objects[index].pAnimData->field_12 = 4;
}

void Inter_v2::o2_stub0x80(void) {
	int16 start;
	int16 videoMode;
	int16 width;
	int16 height;

	start = load16();

	videoMode = start & 0xFF;
	start = (start >> 8) & 0xFF;

	width = _vm->_parse->parseValExpr();
	height = _vm->_parse->parseValExpr();

	if ((videoMode == _vm->_global->_videoMode) && (width == -1))
		return;

	if (videoMode == 0x14) {
		videoMode = 0x13;
		_vm->_video->_extraMode = true;
	}
	else
		_vm->_video->_extraMode = false;
	
	_vm->_game->sub_BB28();
	_vm->_util->clearPalette();
	memset(_vm->_global->_redPalette, 0, 256);
	memset(_vm->_global->_greenPalette, 0, 256);
	memset(_vm->_global->_bluePalette, 0, 256);
//	warning("GOB2 Stub! _vid_setStubDriver");

	if (videoMode == 0x10) {
		_vm->_global->_videoMode = 0x12;
		_vm->_video->initPrimary(0xE);
		_vm->_global->_videoMode = 0x10;
		warning("GOB2 Stub! Set VGA CRT Maximum Scan Line to 0");
		_vm->_draw->_frontSurface->height = 400;
	} else {
		_vm->_global->_videoMode = videoMode;
		_vm->_video->initPrimary(videoMode);
	}
	WRITE_VAR(15, _vm->_global->_videoMode);

	_vm->_global->_setAllPalette = 1;

	if ((width != -1) && _vm->_video->_extraMode) {
		_vm->_game->_byte_2FC9B = 1;
/*		if (width > 960)
			width = 960;
		_vm->_draw->_frontSurface->width = width;
		_vm->_draw->_frontSurface->height = height;
		warning("GOB2 Stub! _vid_setVirtRes(_vm->_draw->_frontSurface);");
		_vm->_global->_mouseMaxCol = width;
		_vm->_global->_mouseMaxRow = height;*/
	}

	_vm->_util->setMousePos(_vm->_global->_inter_mouseX, _vm->_global->_inter_mouseY);
	_vm->_util->clearPalette();

	if (start == 0)
		_vm->_game->_word_2E51F = 0;
	else
		_vm->_game->_word_2E51F = _vm->_global->_primaryHeight - start;
	_vm->_game->sub_ADD2();

	if (_vm->_game->_off_2E51B != 0) {
		warning("GOB2 Stub! _vid_setSplit(_vm->_global->_primaryHeight - start);");
		warning("GOB2 Stub! _vid_setPixelShift(0, start);");
	}
}

void Inter_v2::o2_stub0x82(void) {
	int16 expr;

	expr = _vm->_parse->parseValExpr();

	if (expr == -1) {
		if (_vm->_game->_byte_2FC9B != 0)
			_vm->_game->_byte_2FC9B = 1;
		_vm->_parse->parseValExpr();
		WRITE_VAR(2, _vm->_game->_word_2FC9E);
		WRITE_VAR(3, _vm->_game->_word_2FC9C);
	} else {
		_vm->_game->_word_2FC9E = expr;
		_vm->_game->_word_2FC9C = _vm->_parse->parseValExpr();
	}
/*	if (_vm->_game->_off_2E51B != 0)
		warning("GOB2 Stub! _vid_setPixelShift(_vm->_game->_word_2FC9E, _vm->_game->_word_2FC9C + 200 - _vm->_game->_word_2E51F)");
	else
		warning("GOB2 Stub! _vid_setPixelShift(_vm->_game->_word_2FC9E, _vm->_game->_word_2FC9C);");*/
}

void Inter_v2::o2_stub0x85(void) {
	char dest[32];

	evalExpr(0);
	strcpy(dest, _vm->_global->_inter_resStr);
	strcat(dest, ".ITK");

	warning("STUB: Gob2 drawOperation 0x85 (\"%s\")", dest);
	_vm->_dataio->openDataFile(dest);
}

int16 Inter_v2::loadSound(int16 search) {
	int16 id; // si
	int16 slot; // di
	int32 i;
	bool isADL;
	char sndfile[14];
	char *extData;
	char *dataPtr;
	Snd::SoundDesc *soundDesc;

	memset(sndfile, 0, 14 * sizeof(char));

	isADL = false;
	if (search == 0) {
		slot = _vm->_parse->parseValExpr();
		if (slot < 0) {
			isADL = true;
			slot = -slot;
		}
		id = load16();
	} else {
		id = load16();

		for (slot = 0; slot < 60; slot++)
			if ((_vm->_game->_soundSamples[slot] != 0) && (_vm->_game->_soundIds[slot] == id))
				return slot | 0x8000;

		for (slot = 59; slot >= 0; slot--)
			if (_vm->_game->_soundSamples[slot] == 0) break;
	}
	
	if (_vm->_game->_soundSamples[slot] != 0)
		_vm->_game->freeSoundSlot(slot);

	_vm->_game->_soundIds[slot] = id;

	if (id == -1) { // loc_969D
		strcpy(sndfile, _vm->_global->_inter_execPtr);
		_vm->_global->_inter_execPtr += 9;
		if (!isADL) {
			strcat(sndfile, ".SND");
			_vm->_game->_soundSamples[slot] = _vm->_game->loadSND(sndfile, 3);
		} else {
			strcat(sndfile, ".ADL");
			// TODO: This is very ugly (and doesn't work with Snd::freeSoundDesc())
			_vm->_game->_soundSamples[slot] = (Snd::SoundDesc *) _vm->_dataio->getData(sndfile);
		}
		_vm->_game->_soundTypes[slot] = 2;
	} else { // loc_9735
		if (id >= 30000) { // loc_973E
			if (!isADL && (_vm->_game->_totFileData[0x29] >= 51)) { // loc_9763
				if (_vm->_inter->_terminate != 0)
					return slot;
				soundDesc = new Snd::SoundDesc;
				extData = _vm->_game->loadExtData(id, 0, 0);
				if (extData == 0) {
					delete soundDesc;
					return slot;
				}
				soundDesc->data = extData + 6;
				soundDesc->frequency = (extData[4] << 8) + extData[5];
				soundDesc->size = (extData[1] << 16) + (extData[2] << 8) + extData[3];
				soundDesc->flag = 0;
				if (soundDesc->frequency < 4700)
					soundDesc->frequency = 4700;
				soundDesc->frequency = -soundDesc->frequency;
				for (i = 0, dataPtr = soundDesc->data; i < soundDesc->size; i++, dataPtr++)
					*dataPtr ^= 0x80;
				_vm->_game->_soundTypes[slot] = 4;
				_vm->_game->_soundSamples[slot] = soundDesc;
			} else { // loc_99BC
				extData = _vm->_game->loadExtData(id, 0, 0);
				if (extData == 0)
					return slot;
				_vm->_game->_soundTypes[slot] = 1;
				if (!isADL)
					_vm->_game->loadSound(slot, extData);
				else
					// TODO: This is very ugly (and doesn't work with Snd::freeSoundDesc())
					_vm->_game->_soundSamples[slot] = (Snd::SoundDesc *) extData;
			}
		} else { // loc_9A13
			extData = _vm->_game->loadTotResource(id);
			if (!isADL)
				_vm->_game->loadSound(slot, extData);
			else
				// TODO: This is very ugly (and doesn't work with Snd::freeSoundDesc())
				_vm->_game->_soundSamples[slot] = (Snd::SoundDesc *) extData;
		}
	}
	// loc_9A4E

	if (isADL)
		_vm->_game->_soundTypes[slot] |= 8;

	return slot;
}

void Inter_v2::o2_loadMapObjects(void) {
	_vm->_map->loadMapObjects(0);
}

void Inter_v2::o2_freeGoblins(void) {
	_vm->_goblin->freeObjects();
}

void Inter_v2::o2_placeGoblin(void) {
	int16 index;
	int16 x;
	int16 y;
	int16 state;

	index = _vm->_parse->parseValExpr();
	x = _vm->_parse->parseValExpr();
	y = _vm->_parse->parseValExpr();
	state = _vm->_parse->parseValExpr();

	_vm->_goblin->placeObject(0, 0, index, x, y, state);
}

void Inter_v2::o2_moveGoblin(void) {
	Mult::Mult_Object *obj;
	Mult::Mult_AnimData *objAnim;
	int16 destX = _vm->_parse->parseValExpr();
	int16 destY = _vm->_parse->parseValExpr();
	int16 index = _vm->_parse->parseValExpr();

	obj = &_vm->_mult->_objects[index];
	objAnim = obj->pAnimData;

	obj->gobDestX = destX;
	obj->gobDestY = destY;
	objAnim->field_13 = destX;
	objAnim->field_14 = destY;
	if (objAnim->someFlag != 0) {
		if ((destX == -1) && (destY == -1)) {
			warning("STUB: Gob2 drawOperation moveGoblin (%d %d %d), someFlag: %d", destX, destY, index, objAnim->someFlag);
		}
	}
	_vm->_goblin->initiateMove(index);
}

void Inter_v2::o2_writeGoblinPos(void) {
	int16 var1;
	int16 var2;
	int16 index;

	var1 = _vm->_parse->parseVarIndex() >> 2;
	var2 = _vm->_parse->parseVarIndex() >> 2;
	index = _vm->_parse->parseValExpr();
	WRITE_VAR(var1, _vm->_mult->_objects[index].goblinX);
	WRITE_VAR(var2, _vm->_mult->_objects[index].goblinY);
}

void Inter_v2::o2_multSub(void) {
	_vm->_mult->multSub(_vm->_parse->parseValExpr());
}

void Inter_v2::o2_renderStatic(void) {
	int16 layer;
	int16 index;

	index = _vm->_parse->parseValExpr();
	layer = _vm->_parse->parseValExpr();
	_vm->_scenery->renderStatic(index, layer);
}

void Inter_v2::loadMult(void) {
	int16 val;
	int16 objIndex; // si
	int16 i;
	int16 animation;
	char *lmultData;
	Mult::Mult_Object *obj;
	Mult::Mult_AnimData *objAnim;

	debugC(4, DEBUG_GAMEFLOW, "Inter_v2::loadMult(): Loading...");

	objIndex = _vm->_parse->parseValExpr();
	val = _vm->_parse->parseValExpr();
	*_vm->_mult->_objects[objIndex].pPosX = val;
	val = _vm->_parse->parseValExpr();
	*_vm->_mult->_objects[objIndex].pPosY = val;

	lmultData = (char *)_vm->_mult->_objects[objIndex].pAnimData;
	for (i = 0; i < 11; i++) {
		if (*_vm->_global->_inter_execPtr != 99)
			lmultData[i] = _vm->_parse->parseValExpr();
		else
			_vm->_global->_inter_execPtr++;
	}

	if (_vm->_mult->_objects[objIndex].pAnimData->animType == 100) {
		if (_vm->_goblin->_gobsCount >= 0) {
			obj = &_vm->_mult->_objects[objIndex];
			objAnim = obj->pAnimData;

			val = *obj->pPosX % 256;
			obj->destX = val;
			obj->gobDestX = val;
			obj->goblinX = val;
			val = *obj->pPosY % 256;
			obj->destY = val;
			obj->gobDestY = val;
			obj->goblinY = val;
			*obj->pPosX *= _vm->_map->_tilesWidth;
			objAnim->field_15 = objAnim->unknown;
			objAnim->field_E = -1;
			objAnim->field_F = -1;
			objAnim->field_12 = 0;
			objAnim->state = objAnim->layer;
			objAnim->layer = obj->goblinStates[objAnim->state][0].layer;
			objAnim->animation = obj->goblinStates[objAnim->state][0].animation;
			animation = objAnim->animation;
			_vm->_scenery->updateAnim(objAnim->state, 0, 0, 0, *obj->pPosX, *obj->pPosY, 0);
			if (!_vm->_map->_bigTiles) {
				*obj->pPosY = (obj->goblinY + 1) * _vm->_map->_tilesHeight
					- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			} else {
				*obj->pPosY = ((obj->goblinY + 1) / 2) * _vm->_map->_tilesHeight
					- (_vm->_scenery->_animBottom - _vm->_scenery->_animTop);
			}
			*obj->pPosX = obj->goblinX * _vm->_map->_tilesWidth;
		}
	}
	if (_vm->_mult->_objects[objIndex].pAnimData->animType == 101) {
		if (_vm->_goblin->_gobsCount >= 0) {
			obj = &_vm->_mult->_objects[objIndex];
			objAnim = obj->pAnimData;

			objAnim->field_E = -1;
			objAnim->field_F = -1;
			objAnim->state = objAnim->layer;
			objAnim->layer = obj->goblinStates[objAnim->state][0].layer;
			objAnim->animation = obj->goblinStates[objAnim->state][0].animation;
			if ((*obj->pPosX == 1000) && (*obj->pPosY == 1000)) {
				*obj->pPosX = _vm->_scenery->_animations[objAnim->animation].layers[objAnim->state]->posX;
				*obj->pPosY = _vm->_scenery->_animations[objAnim->animation].layers[objAnim->state]->posY;
			}
			_vm->_scenery->updateAnim(objAnim->state, 0, objAnim->animation, 0, *obj->pPosX, *obj->pPosY, 0);
		}
	}
}

bool Inter_v2::o2_animPalInit(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;

	index = load16();
	if (index > 0) {
		index--;
		_animPalLowIndex[index] = _vm->_parse->parseValExpr();
		_animPalHighIndex[index] = _vm->_parse->parseValExpr();
		_animPalDir[index] = 1;
	} else if (index == 0) {
		memset(_animPalDir, 0, 8 * sizeof(int16));
		_vm->_parse->parseValExpr();
		_vm->_parse->parseValExpr();
	} else {
		index = -index - 1;
		_animPalLowIndex[index] = _vm->_parse->parseValExpr();
		_animPalHighIndex[index] = _vm->_parse->parseValExpr();
		_animPalDir[index] = -1;
	}
	return false;
}

bool Inter_v2::o2_playSound(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 frequency;
	int16 freq2;
	int16 repCount; // di
	int16 index; // si

	index = _vm->_parse->parseValExpr();
	repCount = _vm->_parse->parseValExpr();
	frequency = _vm->_parse->parseValExpr();

	_soundEndTimeKey = 0;
	if (_vm->_game->_soundSamples[index] == 0)
		return false;

	if (repCount < 0) {
		if (_vm->_global->_soundFlags < 2)
			return false;

		repCount = -repCount;
		_soundEndTimeKey = _vm->_util->getTimeKey();

		if (frequency == 0)
			freq2 = _vm->_game->_soundSamples[index]->frequency;
		else
			freq2 = frequency;
		_soundStopVal =
		    (10 * (_vm->_game->_soundSamples[index]->size / 2)) / freq2;
		_soundEndTimeKey +=
		    ((_vm->_game->_soundSamples[index]->size * repCount -
			_vm->_game->_soundSamples[index]->size / 2) * 1000) / freq2;
	}
	// loc_E2F3
	if (_vm->_game->_soundTypes[index] & 8) {
		_vm->_music->loadFromMemory((byte *) _vm->_game->_soundSamples[index]);
		_vm->_music->setRepeating(repCount - 1);
		_vm->_music->startPlay();
	} else {
		_vm->_snd->stopSound(0);
		_vm->_snd->playSample(_vm->_game->_soundSamples[index], repCount, frequency);
	}

	return false;
}

bool Inter_v2::o2_goblinFunc(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 cmd;

	cmd = load16();
	_vm->_global->_inter_execPtr += 2;

	if (cmd == 100) {
		_vm->_goblin->_word_2F9C0 = VAR(load16());
		_vm->_goblin->_word_2F9BE = VAR(load16());
		_vm->_goblin->_dword_2F9B6 = load16();
		_vm->_goblin->_dword_2F9B2 = load16();
		_vm->_goblin->_word_2F9BC = VAR(load16());
		_vm->_goblin->_word_2F9BA = VAR(load16());
		_vm->_goblin->sub_19BD3();
	} else if (cmd != 101) {
		_vm->_global->_inter_execPtr -= 2;
		cmd = load16();
		_vm->_global->_inter_execPtr += cmd << 1;
	}

	return false;
}

bool Inter_v2::o2_evaluateStore(char &cmdCount, int16 &counter, int16 &retFlag) {
	char *savedPos;
	int16 varOff;
	int16 token;
	int16 result;
	byte loopCount;

	savedPos = _vm->_global->_inter_execPtr;
	varOff = _vm->_parse->parseVarIndex();

	if (*_vm->_global->_inter_execPtr == 99) {
		_vm->_global->_inter_execPtr++;
		loopCount = *_vm->_global->_inter_execPtr++;
	}
	else
		loopCount = 1;

	for (int i = 0; i < loopCount; i++) {
		token = evalExpr(&result);
		switch (savedPos[0]) {
		case 16:
		case 18:
			*(_vm->_global->_inter_variables + varOff + i) = _vm->_global->_inter_resVal;
			break;

		case 17:
		case 27:
			*(uint16*)(_vm->_global->_inter_variables + varOff + i * 2) = _vm->_global->_inter_resVal;
			break;

		case 23:
		case 26:
			WRITE_VAR_OFFSET(varOff + i * 4, _vm->_global->_inter_resVal);
			break;

		case 24:
			*(uint16*)(_vm->_global->_inter_variables + varOff + i * 4) = _vm->_global->_inter_resVal;
			break;

		case 25:
		case 28:
			if (token == 20)
				*(_vm->_global->_inter_variables + varOff) = result;
			else
				strcpy(_vm->_global->_inter_variables + varOff, _vm->_global->_inter_resStr);
			break;
		}
	}

	return false;
}

bool Inter_v2::o2_palLoad(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 i;
	int16 ind1;
	int16 ind2;
	byte cmd;
	char *palPtr;

	cmd = *_vm->_global->_inter_execPtr++;

	switch(cmd & 0x7f) {
	case 48:
		if ((_vm->_global->_videoMode < 0x32) || (_vm->_global->_videoMode > 0x63)) {
			_vm->_global->_inter_execPtr += 48;
			return false;
		}
		break;

	case 49:
		if ((_vm->_global->_videoMode != 5) && (_vm->_global->_videoMode != 7)) {
			_vm->_global->_inter_execPtr += 18;
			return false;
		}
		break;

	case 50:
		if (_vm->_global->_videoMode != 0x0D) { // || (word_2D479 == 256) {
			_vm->_global->_inter_execPtr += 16;
			return false;
		}
		break;

	case 51:
		if (_vm->_global->_videoMode < 0x64) {
			_vm->_global->_inter_execPtr += 2;
			return false;
		}
		break;

	case 52:
		if (_vm->_global->_videoMode != 0x0D) { // || (word_2D479 == 256) {
			_vm->_global->_inter_execPtr += 48;
			return false;
		}
		break;

	case 53:
		if (_vm->_global->_videoMode < 0x13) {
			_vm->_global->_inter_execPtr += 2;
			return false;
		}
		break;

	case 54:
		if (_vm->_global->_videoMode < 0x13) {
			return false;
		}
		break;

	case 61:
		if (_vm->_global->_videoMode < 0x13) {
			*_vm->_global->_inter_execPtr += 4;
			return false;
		}
		break;
	}

	if ((cmd & 0x7f) == 0x30) {
		_vm->_global->_inter_execPtr += 48;
		return false;
	}

	_vm->_draw->_applyPal = 0;
	if (cmd & 0x80)
		cmd &= 0x7f;
	else
		_vm->_draw->_applyPal = 1;

	if (cmd == 49) {
		int dl = 0;
		for (i = 2; i < 18; i++) {
			dl = 1;
			if(_vm->_global->_inter_execPtr[i] != 0)
				dl = 0;
		}
		if (dl != 0) {
			warning("GOB2 Stub! sub_27413");
/*			sub_27413(_draw_frontSurface);
			byte_2E521 = 0;
			_vm->_global->_inter_execPtr += 18;
			break;*/
		}
//		byte_2E521 = 1;

		for (i = 0; i < 18; i++, _vm->_global->_inter_execPtr++) {
			if (i < 2) {
				if (_vm->_draw->_applyPal == 0)
					continue;

				_vm->_draw->_unusedPalette1[i] = *_vm->_global->_inter_execPtr;
				continue;
			}

			ind1 = *_vm->_global->_inter_execPtr >> 4;
			ind2 = (*_vm->_global->_inter_execPtr & 0xf);

			_vm->_draw->_unusedPalette1[i] =
			    ((_vm->_draw->_palLoadData1[ind1] + _vm->_draw->_palLoadData2[ind2]) << 8) +
			    (_vm->_draw->_palLoadData2[ind1] + _vm->_draw->_palLoadData1[ind2]);
		}

		_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
		return false;
	}

	switch (cmd) {
	case 50:
		for (i = 0; i < 16; i++, _vm->_global->_inter_execPtr++)
			_vm->_draw->_unusedPalette2[i] = *_vm->_global->_inter_execPtr;
		break;

	case 52:
		for (i = 0; i < 16; i++, _vm->_global->_inter_execPtr += 3) {
			_vm->_draw->_vgaSmallPalette[i].red = _vm->_global->_inter_execPtr[0];
			_vm->_draw->_vgaSmallPalette[i].green = _vm->_global->_inter_execPtr[1];
			_vm->_draw->_vgaSmallPalette[i].blue = _vm->_global->_inter_execPtr[2];
		}
		_vm->_global->_inter_execPtr += 48;
		if (_vm->_global->_videoMode >= 0x13)
			return false;
		break;

	case 53:
		palPtr = _vm->_game->loadTotResource(_vm->_inter->load16());
		memcpy((char *)_vm->_draw->_vgaPalette, palPtr, 768);
		break;

	case 54:
		memset((char *)_vm->_draw->_vgaPalette, 0, 768);
		break;

	case 61:
		ind1 = *_vm->_global->_inter_execPtr++;
		ind2 = (*_vm->_global->_inter_execPtr++ - ind1 + 1) * 3;
		palPtr = _vm->_game->loadTotResource(_vm->_inter->load16());
		memcpy((char *)_vm->_draw->_vgaPalette + ind1 * 3, palPtr + ind1 * 3, ind2);
		if (_vm->_draw->_applyPal) {
			_vm->_draw->_applyPal = 0;
			_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
			return false;
		}
		break;
	}
	
	if (!_vm->_draw->_applyPal) {
		_vm->_global->_pPaletteDesc->unused2 = _vm->_draw->_unusedPalette2;
		_vm->_global->_pPaletteDesc->unused1 = _vm->_draw->_unusedPalette1;
		if (_vm->_global->_videoMode < 0x13) {
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)_vm->_draw->_vgaSmallPalette;
			_vm->_palanim->fade((Video::PalDesc *) _vm->_global->_pPaletteDesc, 0, 0);
			return false;
		}
		if ((_vm->_global->_videoMode < 0x32) || (_vm->_global->_videoMode >= 0x64)) {
			_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)_vm->_draw->_vgaPalette;
			_vm->_palanim->fade((Video::PalDesc *) _vm->_global->_pPaletteDesc, 0, 0);
			return false;
		}
		_vm->_global->_pPaletteDesc->vgaPal = (Video::Color *)_vm->_draw->_vgaSmallPalette;
		_vm->_palanim->fade((Video::PalDesc *) _vm->_global->_pPaletteDesc, 0, 0);
	}

	return false;
}

bool Inter_v2::o2_loadTot(char &cmdCount, int16 &counter, int16 &retFlag) {
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

	if (strcmp(buf, "INSTALL") == 0) {
		warning("GOB2 Stub! word_2E515 = _inter_variables[0E8h]");
	}

	strcat(buf, ".tot");
	if (_terminate != 2)
		_terminate = true;
	strcpy(_vm->_game->_totToLoad, buf);

	return false;
}

bool Inter_v2::o2_freeSprite(char &cmdCount, int16 &counter, int16 &retFlag) {
	int16 index;

	index = load16();
	if (_vm->_draw->_spritesArray[index] == 0)
		return false;

	_vm->_draw->freeSprite(index);

	return false;
}

bool Inter_v2::o2_loadSound(char &cmdCount, int16 &counter, int16 &retFlag) {
	loadSound(0);
	return false;
}

void Inter_v2::o2_setRenderFlags(void) {
	int16 expr;

	expr = _vm->_parse->parseValExpr();
	
	if (expr & 0x8000) {
		_vm->_draw->_renderFlags |= expr & 0x3fff;
	}
	else {
		if (expr & 0x4000)
			_vm->_draw->_renderFlags &= expr & 0x3fff;
		else
			_vm->_draw->_renderFlags = _vm->_parse->parseValExpr();
	}
}

void Inter_v2::o2_initMult(void) {
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
		_vm->_mult->_renderData2 = new Mult::Mult_Object*[_vm->_mult->_objCount];
		memset(_vm->_mult->_renderData2, 0, _vm->_mult->_objCount * sizeof(Mult::Mult_Object*));
		_vm->_mult->_renderData = new int16[_vm->_mult->_objCount * 9];
		memset(_vm->_mult->_renderData, 0, _vm->_mult->_objCount * 9 * sizeof(int16));
		if (_vm->_inter->_terminate)
			return;
		_vm->_mult->_orderArray = new int8[_vm->_mult->_objCount];
		memset(_vm->_mult->_orderArray, 0, _vm->_mult->_objCount * sizeof(int8));
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
		error("o2_initMult: Object count changed, but storage didn't (old count = %d, new count = %d)",
		    oldObjCount, _vm->_mult->_objCount);
	}

	if (_vm->_anim->_animSurf != 0 &&
	    (oldAnimWidth != _vm->_anim->_areaWidth
		|| oldAnimHeight != _vm->_anim->_areaHeight)) {
		if (_vm->_anim->_animSurf->flag & 0x80)
			_vm->_draw->freeSprite(0x16);
		else
			delete _vm->_anim->_animSurf;
	}

	_vm->_draw->adjustCoords(0, &_vm->_anim->_areaWidth, &_vm->_anim->_areaHeight);

	if (_vm->_anim->_animSurf == 0) {
		if (_vm->_global->_videoMode == 18) {
			_vm->_anim->_animSurf = new Video::SurfaceDesc;
			memcpy(_vm->_anim->_animSurf, _vm->_draw->_frontSurface, sizeof(Video::SurfaceDesc));
			_vm->_anim->_animSurf->width = (_vm->_anim->_areaLeft + _vm->_anim->_areaWidth - 1) | 7;
			_vm->_anim->_animSurf->width -= (_vm->_anim->_areaLeft & 0x0FFF8) - 1;
			_vm->_anim->_animSurf->height = _vm->_anim->_areaHeight;
			_vm->_anim->_animSurf->vidPtr += 0x0C000;
		} else {
			if (_vm->_global->_videoMode == 20) {
				if (((_vm->_draw->_backSurface->width * _vm->_draw->_backSurface->height) / 2
						+ (_vm->_anim->_areaWidth * _vm->_anim->_areaHeight) / 4) < 65536) {
					_vm->_anim->_animSurf = new Video::SurfaceDesc;
					memcpy(_vm->_anim->_animSurf, _vm->_draw->_frontSurface, sizeof(Video::SurfaceDesc));
					_vm->_anim->_animSurf->width = (_vm->_anim->_areaLeft + _vm->_anim->_areaWidth - 1) | 7;
					_vm->_anim->_animSurf->width -= (_vm->_anim->_areaLeft & 0x0FF8) - 1;
					_vm->_anim->_animSurf->height = _vm->_anim->_areaHeight;
					_vm->_anim->_animSurf->vidPtr = _vm->_draw->_backSurface->vidPtr +
						_vm->_draw->_backSurface->width * _vm->_draw->_backSurface->height / 4;
				} else
					_vm->_draw->initBigSprite(0x16, _vm->_anim->_areaWidth, _vm->_anim->_areaHeight, 0);
			} else
				_vm->_draw->initBigSprite(0x16, _vm->_anim->_areaWidth, _vm->_anim->_areaHeight, 0);
		}
		if (_terminate)
			return;
	}

	_vm->_draw->adjustCoords(1, &_vm->_anim->_areaWidth, &_vm->_anim->_areaHeight);

	_vm->_draw->_sourceSurface = 21;
	_vm->_draw->_destSurface = 22;
	_vm->_draw->_spriteLeft = _vm->_anim->_areaLeft;
	_vm->_draw->_spriteTop = _vm->_anim->_areaTop;
	_vm->_draw->_spriteRight = _vm->_anim->_areaWidth;
	_vm->_draw->_spriteBottom = _vm->_anim->_areaHeight;
	_vm->_draw->_destSpriteX = 0;
	_vm->_draw->_destSpriteY = 0;
	_vm->_draw->spriteOperation(0);

	debugC(4, DEBUG_GRAPHICS, "o2_initMult: x = %d, y = %d, w = %d, h = %d",
		  _vm->_anim->_areaLeft, _vm->_anim->_areaTop, _vm->_anim->_areaWidth, _vm->_anim->_areaHeight);
	debugC(4, DEBUG_GRAPHICS, "    _vm->_mult->_objCount = %d, animation data size = %d", _vm->_mult->_objCount, _vm->_global->_inter_animDataSize);
}

void Inter_v2::o2_loadCurLayer(void) {
	_vm->_scenery->_curStatic = _vm->_parse->parseValExpr();
	_vm->_scenery->_curStaticLayer = _vm->_parse->parseValExpr();
}

void Inter_v2::o2_playCDTrack(void) {
	if ((_vm->_draw->_renderFlags & 0x200) == 0)
		_vm->_draw->blitInvalidated();
	evalExpr(NULL);
	_vm->_cdrom->startTrack(_vm->_global->_inter_resStr);
}

void Inter_v2::o2_stopCD(void) {
	_vm->_cdrom->stopPlaying();
}

void Inter_v2::o2_readLIC(void) {
	byte result;
	char path[40];
	
	result = evalExpr(NULL);
	strcpy(path, _vm->_global->_inter_resStr);
	strcat(path, ".LIC");

	_vm->_cdrom->readLIC(path);
}

void Inter_v2::o2_freeLIC(void) {
	_vm->_cdrom->freeLICbuffer();
}

void Inter_v2::o2_getCDTrackPos(void) {
	int16 trackpospos;
	int16 tracknamepos;
	int32 trackpos;

	_vm->_util->longDelay(1);

	trackpospos = _vm->_parse->parseVarIndex();
	// The currently playing trackname would be written there to
	// notice trackbound overruns. Since we stop on trackend and
	// CDROM::getTrackPos() returns -1 then anyway, we can ignore it.
	tracknamepos = _vm->_parse->parseVarIndex();
	trackpos = _vm->_cdrom->getTrackPos();
	if (trackpos == -1)
		trackpos = 32767;

	WRITE_VAR(trackpospos >> 2, trackpos);
}

void Inter_v2::o2_playMult(void) {
	int16 checkEscape;

	checkEscape = load16();

	_vm->_mult->setMultData(checkEscape >> 1);
	_vm->_mult->playMult(VAR(57), -1, checkEscape & 0x1, 0);
}

void Inter_v2::o2_initCursor(void) {
	int16 width;
	int16 height;
	int16 count;
	int16 i;

	_vm->_draw->_cursorXDeltaVar = _vm->_parse->parseVarIndex() / 4;
	_vm->_draw->_cursorYDeltaVar = _vm->_parse->parseVarIndex() / 4;

	width = load16();
	if (width < 16)
		width = 16;

	height = load16();
	if (height < 16)
		height = 16;

	_vm->_draw->adjustCoords(0, &width, &height);

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

void Inter_v2::o2_playImd(void) {
	char imd[128];
	int i;
	int16 x;
	int16 y;
	int16 startFrame; // di
	int16 lastFrame; // si
	int16 breakKey;
	int16 flags;
	int16 expr7;
	int16 expr8;

	evalExpr(0);
	_vm->_global->_inter_resStr[8] = 0;
	strcpy(imd, _vm->_global->_inter_resStr);
	x = _vm->_parse->parseValExpr();
	y = _vm->_parse->parseValExpr();
	startFrame = _vm->_parse->parseValExpr();
	lastFrame = _vm->_parse->parseValExpr();
	breakKey = _vm->_parse->parseValExpr();
	flags = _vm->_parse->parseValExpr();
	expr7 = _vm->_parse->parseValExpr();
	expr8 = _vm->_parse->parseValExpr();
	
	if (_vm->_game->openImd(imd, x, y, startFrame, flags) == 0)
		return;

	int16 var_C;

	var_C = lastFrame;
	if (lastFrame < 0)
		lastFrame = _vm->_game->_imdFile->framesCount - 1;
	for (i = startFrame; i <= lastFrame; i++) {
		_vm->_game->playImd(i, 1 << (flags & 0x3F), expr7, expr8, 0, lastFrame);
		WRITE_VAR(11, i);
		if (breakKey != 0) {
			_vm->_util->getMouseState(&_vm->_global->_inter_mouseX,
					&_vm->_global->_inter_mouseY, &_vm->_game->_mouseButtons);
			storeKey(_vm->_util->checkKey());
			if (VAR(0) == (unsigned) breakKey)
				return;
		}
	}
	if (var_C == -1)
		_vm->_game->closeImd();
}

void Inter_v2::o2_totSub(void) {
	char totFile[14];
	int flags;
	int length;
	int i;

	length = *_vm->_global->_inter_execPtr++;
	if (length > 13)
		error("Length in o2_totSub is greater than 13 (%d)", length);
	if (length & 0x80) {
		evalExpr(0);
		strcpy(totFile, _vm->_global->_inter_resStr);
	} else {
		for (i = 0; i < length; i++)
			totFile[i] = *_vm->_global->_inter_execPtr++;
		totFile[i] = 0;
	}

	_vm->_global->_inter_execPtr++;
	flags = *_vm->_global->_inter_execPtr;
	_vm->_game->totSub(flags, totFile);
}

void Inter_v2::o2_switchTotSub(void) {
	int16 index;
	int16 skipPlay;

	index = load16();
	skipPlay = load16();

	_vm->_game->switchTotSub(index, skipPlay);
}

void Inter_v2::storeMouse(void) {
	int16 x;
	int16 y;

	x = _vm->_global->_inter_mouseX;
	y = _vm->_global->_inter_mouseY;
	_vm->_draw->adjustCoords(1, &x, &y);

	WRITE_VAR(2, x);
	WRITE_VAR(3, y);
	WRITE_VAR(4, _vm->_game->_mouseButtons);
}

void Inter_v2::o2_setPickable(int16 &extraData, int32 *retVarPtr, Goblin::Gob_Object *objDesc) {
	warning("GOB2 Stub! o2_setPickable");
}

void Inter_v2::animPalette(void) {
	int16 i;
	int16 j;
	Video::Color col;
	bool first;

	first = true;
	for (j = 0; j < 8; j ++) {
		if (_animPalDir[j] == 0)
			continue;

		if (first) {
			_vm->_video->waitRetrace(_vm->_global->_videoMode);
			first = false;
		}

		if (_animPalDir[j] == -1) {
			col = _vm->_global->_pPaletteDesc->vgaPal[_animPalLowIndex[j]];

			for (i = _animPalLowIndex[j]; i < _animPalHighIndex[j]; i++)
				_vm->_draw->_vgaPalette[i] = _vm->_global->_pPaletteDesc->vgaPal[i];

			_vm->_global->_pPaletteDesc->vgaPal[_animPalHighIndex[j]] = col;
		} else {
			col = _vm->_global->_pPaletteDesc->vgaPal[_animPalHighIndex[j]];
			for (i = _animPalHighIndex[j]; i > _animPalLowIndex[j]; i--)
				_vm->_draw->_vgaPalette[i] = _vm->_global->_pPaletteDesc->vgaPal[i];

			_vm->_global->_pPaletteDesc->vgaPal[_animPalLowIndex[j]] = col;
		}
		_vm->_global->_pPaletteDesc->vgaPal = _vm->_draw->_vgaPalette;
	}
	if (!first)
		_vm->_video->setFullPalette(_vm->_global->_pPaletteDesc);
}

} // End of namespace Gob
