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

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/game.h"
#include "gob/parse.h"
#include "gob/draw.h"

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_v5, x)

const int Inter_v5::_goblinFuncLookUp[][2] = {
	{0, 0},
	{1, 0},
	{80, 1},
	{81, 2},
	{82, 3},
	{83, 4},
	{84, 5},
	{85, 6},
	{86, 7},
	{87, 0},
	{88, 0},
	{89, 0},
	{90, 0},
	{91, 0},
	{92, 8},
	{93, 0},
	{94, 0},
	{95, 9},
	{96, 10},
	{97, 11},
	{98, 12},
	{99, 0},
	{100, 13},
	{200, 14},
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
	{100, 39},
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

Inter_v5::Inter_v5(GobEngine *vm) : Inter_v4(vm) {
	setupOpcodes();
}

void Inter_v5::setupOpcodes() {
	static const OpcodeDrawEntryV5 opcodesDraw[256] = {
		/* 00 */
		OPCODE(o1_loadMult),
		OPCODE(o2_playMult),
		OPCODE(o2_freeMultKeys),
		{NULL, ""},
		/* 04 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		OPCODE(o1_initCursor),
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
		{NULL, ""},
		{NULL, ""},
		/* 20 */
		OPCODE(o2_playCDTrack),
		OPCODE(o2_waitCDTrackEnd),
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
		OPCODE(o2_loadFontToSprite),
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
		OPCODE(o2_copyVars),
		OPCODE(o2_pasteVars),
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
		OPCODE(o2_stopGoblin),
		OPCODE(o2_setGoblinState),
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
		OPCODE(o5_deleteFile),
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
		OPCODE(o4_initScreen),
		OPCODE(o2_scroll),
		OPCODE(o2_setScrollOffset),
		OPCODE(o4_playVmdOrMusic),
		/* 84 */
		OPCODE(o2_getImdInfo),
		OPCODE(o2_openItk),
		OPCODE(o2_closeItk),
		OPCODE(o2_setImdFrontSurf),
		/* 88 */
		OPCODE(o2_resetImdFrontSurf),
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

	static const OpcodeFuncEntryV5 opcodesFunc[80] = {
		/* 00 */
		OPCODE(o1_callSub),
		OPCODE(o1_callSub),
		OPCODE(o1_printTotText),
		OPCODE(o1_loadCursor),
		/* 04 */
		{NULL, ""},
		OPCODE(o1_switch),
		OPCODE(o1_repeatUntil),
		OPCODE(o1_whileDo),
		/* 08 */
		OPCODE(o1_if),
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
		OPCODE(o2_printText),
		OPCODE(o1_loadTot),
		OPCODE(o1_palLoad),
		/* 14 */
		OPCODE(o1_keyFunc),
		OPCODE(o1_capturePush),
		OPCODE(o1_capturePop),
		OPCODE(o2_animPalInit),
		/* 18 */
		OPCODE(o2_addCollision),
		OPCODE(o2_freeCollision),
		OPCODE(o3_getTotTextItemPart),
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
		OPCODE(o2_createSprite),
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
		OPCODE(o2_stopSound),
		OPCODE(o2_loadSound),
		OPCODE(o1_freeSoundSlot),
		/* 3C */
		OPCODE(o1_waitEndPlay),
		OPCODE(o1_playComposition),
		OPCODE(o2_getFreeMem),
		OPCODE(o2_checkData),
		/* 40 */
		{NULL, ""},
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

	static const OpcodeGoblinEntryV5 opcodesGoblin[71] = {
		/* 00 */
		OPCODE(o5_spaceShooter),
		OPCODE(o5_getSystemCDSpeed),
		OPCODE(o5_getSystemRAM),
		OPCODE(o5_getSystemCPUSpeed),
		/* 04 */
		OPCODE(o5_getSystemDrawSpeed),
		OPCODE(o5_totalSystemSpecs),
		OPCODE(o5_saveSystemSpecs),
		OPCODE(o5_loadSystemSpecs),
		/* 08 */
		OPCODE(o5_gob92),
		OPCODE(o5_gob95),
		OPCODE(o5_gob96),
		OPCODE(o5_gob97),
		/* 0C */
		OPCODE(o5_gob98),
		OPCODE(o5_gob100),
		OPCODE(o5_gob200),
		{NULL, ""},
		/* 10 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 14 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 18 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 1C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 20 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
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
		{NULL, ""},
		{NULL, ""},
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
	};

	_opcodesDrawV5 = opcodesDraw;
	_opcodesFuncV5 = opcodesFunc;
	_opcodesGoblinV5 = opcodesGoblin;
}

void Inter_v5::executeDrawOpcode(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)",
			i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV5 op = _opcodesDrawV5[i].proc;

	if (op == NULL)
		warning("unimplemented opcodeDraw: %d", i);
	else
		(this->*op) ();
}

bool Inter_v5::executeFuncOpcode(byte i, byte j, OpFuncParams &params) {
	debugC(1, kDebugFuncOp, "opcodeFunc %d.%d [0x%X.0x%X] (%s) - %s, %d, %d",
			i, j, i, j, getOpcodeFuncDesc(i, j), _vm->_game->_curTotFile,
			(uint) (_vm->_global->_inter_execPtr - _vm->_game->_totFileData),
			(uint) (_vm->_global->_inter_execPtr - _vm->_game->_totFileData - params.counter - 4));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcV5 op = _opcodesFuncV5[i*16 + j].proc;

	if (op == NULL)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (params);

	return false;
}

void Inter_v5::executeGoblinOpcode(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcV5 op = NULL;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinV5[_goblinFuncLookUp[j][1]].proc;
			break;
		}

	_vm->_global->_inter_execPtr -= 2;

	if (op == NULL) {
		warning("unimplemented opcodeGoblin: %d", i);

		int16 paramCount = load16();
		_vm->_global->_inter_execPtr += paramCount * 2;
	} else {
		params.extraData = i;

		(this->*op) (params);
	}
}

const char *Inter_v5::getOpcodeDrawDesc(byte i) {
	return _opcodesDrawV5[i].desc;
}

const char *Inter_v5::getOpcodeFuncDesc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncV5[i*16 + j].desc;
}

const char *Inter_v5::getOpcodeGoblinDesc(int i) {
	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i)
			return _opcodesGoblinV5[_goblinFuncLookUp[j][1]].desc;
	return "";
}

void Inter_v5::o5_deleteFile() {
	evalExpr(0);

	warning("Dynasty Stub: deleteFile \"%s\"", _vm->_global->_inter_resStr);
}

bool Inter_v5::o5_istrlen(OpFuncParams &params) {
	int16 strVar1, strVar2;
	int16 len;

	if (*_vm->_global->_inter_execPtr == 0x80) {
		_vm->_global->_inter_execPtr++;

		strVar1 = _vm->_parse->parseVarIndex();
		strVar2 = _vm->_parse->parseVarIndex();

		len = _vm->_draw->stringLength(GET_VARO_STR(strVar1), READ_VARO_UINT16(strVar2));

	} else {

		strVar1 = _vm->_parse->parseVarIndex();
		strVar2 = _vm->_parse->parseVarIndex();

		if (_vm->_global->_language == 10) {
			// Extra handling for Japanese strings

			for (len = 0; READ_VARO_UINT8(strVar1) != 0; strVar1++, len++)
				if (READ_VARO_UINT8(strVar1) >= 128)
					strVar1++;

		} else
			len = strlen(GET_VARO_STR(strVar1));
	}

	WRITE_VAR_OFFSET(strVar2, len);
	return false;
}

void Inter_v5::o5_spaceShooter(OpGobParams &params) {
	int16 paramCount = load16();

	warning("Dynasty Stub: Space shooter: %d, %d, %s",
			params.extraData, paramCount, _vm->_game->_curTotFile);

	if (paramCount < 4) {
		warning("Space shooter variable counter < 4");
		_vm->_global->_inter_execPtr += paramCount * 2;
		return;
	}

	uint32 var1 = load16() * 4;
	uint32 var2 = load16() * 4;
#if 1
	load16();
	load16();
#else
	uint32 var3 = load16() * 4;
	uint16 var4 = load16();
#endif

	if (params.extraData != 0) {
		WRITE_VARO_UINT32(var1, 0);
		WRITE_VARO_UINT32(var2, 0);
	} else {
		if (paramCount < 5) {
			warning("Space shooter variable counter < 5");
			return;
		}

		_vm->_global->_inter_execPtr += (paramCount - 4) * 2;
	}
}

void Inter_v5::o5_getSystemCDSpeed(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 89, 112, 144, 0, _vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		_vm->_util->freeFont(font);
	}
}

void Inter_v5::o5_getSystemRAM(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 168, 112, 144, 0, _vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		_vm->_util->freeFont(font);
	}
}

void Inter_v5::o5_getSystemCPUSpeed(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 248, 112, 144, 0, _vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		_vm->_util->freeFont(font);
	}
}

void Inter_v5::o5_getSystemDrawSpeed(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 326, 112, 144, 0, _vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		_vm->_util->freeFont(font);
	}
}

void Inter_v5::o5_totalSystemSpecs(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 405, 112, 144, 0, _vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		_vm->_util->freeFont(font);
	}
}

void Inter_v5::o5_saveSystemSpecs(OpGobParams &params) {
	warning("Dynasty Stub: Saving system specifications");

	_vm->_global->_inter_execPtr += 2;

/*
	FILE *f = fopen("SAVE\\SPEED.INF", w);
	fwrite(&_cdSpeed,   sizeof(_cdSpeed),   1, f);
	fwrite(&_ram,       sizeof(_ram),       1, f);
	fwrite(&_cpuSpeed,  sizeof(_cpuSpeed),  1, f);
	fwrite(&_drawSpeed, sizeof(_drawSpeed), 1, f);
	fwrite(&_total,     sizeof(_total),     1, f);
	fclose(f);
*/
}

void Inter_v5::o5_loadSystemSpecs(OpGobParams &params) {
	warning("Dynasty Stub: Loading system specifications");

	_vm->_global->_inter_execPtr += 2;

/*
	FILE *f = fopen("SAVE\\SPEED.INF", r);
	fread(&_cdSpeed,   sizeof(_cdSpeed),   1, f);
	fread(&_ram,       sizeof(_ram),       1, f);
	fread(&_cpuSpeed,  sizeof(_cpuSpeed),  1, f);
	fread(&_drawSpeed, sizeof(_drawSpeed), 1, f);
	fread(&_total,     sizeof(_total),     1, f);
	fclose(f);
*/

/*
	// Calculating whether speed throttling is necessary?

	var_E = MAX(_cdSpeed, 150);
	var_E += (_ram << 3);
	var_E += (_cpuSpeed << 3);
	var_E /= 17;

	byte_8A61E = (var_E > 81) ? 1 : 0;
	byte_8A5E0 = (_total >= 95) ? 1 : 0;

	if (byte_8A5E0 == 1) {
		word_8AEE2 = 100;
		byte_8AEE4 = 1;
		byte_8AEE5 = 1;
		word_8AEE6 = 0;
	} else {
		word_8AEE2 = 0;
		byte_8AEE4 = 0;
		byte_8AEE5 = 0;
		word_8AEE6 = 40;
	}
*/
}

void Inter_v5::o5_gob92(OpGobParams &params) {
	warning("Dynasty Stub: GobFunc 92");

	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int8) byte_86B9E)) */);
}

void Inter_v5::o5_gob95(OpGobParams &params) {
	warning("Dynasty Stub: GobFunc 95");

	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int16) word_8AEE6)) */);
	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int8)  byte_8AEE5)) */);
	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int8)  byte_8AEE4)) */);
	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int16) word_8AEE2)) */);
}

void Inter_v5::o5_gob96(OpGobParams &params) {
	int16 word_8AEE6, word_85B50, word_8AEE2;
	byte byte_8AEE5, byte_8AEE4;

	_vm->_global->_inter_execPtr += 2;

	word_8AEE6 = word_85B50 = READ_VAR_UINT16(load16());
	byte_8AEE5 = READ_VAR_UINT8(load16());
	byte_8AEE4 = READ_VAR_UINT8(load16());
	word_8AEE2 = READ_VAR_UINT16(load16());

	warning("Dynasty Stub: GobFunc 96: %d, %d, %d, %d",
			word_8AEE6, byte_8AEE5, byte_8AEE4, word_8AEE2);

	// .--- sub_194B0 ---

	int16 word_8A8F0, word_8A8F2, word_8A8F4, word_8A8F6, word_8A8F8, word_8A8FA;

	int16 word_8A62C = 1;
	int16 word_8A63C, word_8A640, word_8B464, word_8B466;

	byte byte_8A62E;

	int16 var_2, var_4;

	var_2 = word_85B50 + 31;
	word_8A8F0 = word_8A8F2 = var_2;
	word_8A8F4 = word_85B50;

	var_4 = 315 - word_85B50;
	word_8A8F6 = word_8A8F8 = var_4;

	word_8A8FA = 479 - word_85B50;

	if (word_8A62C == 0) {
		word_8A63C = word_8A8F0;
		word_8A640 = word_8A8F6;
		word_8B464 = word_8A8F0;
		word_8B466 = word_8A8F6;
	} else if (word_8A62C == 1) {
		word_8A63C = word_85B50;
		word_8A640 = word_8A8FA;
		word_8B464 = word_85B50;
		word_8B466 = word_8A8FA;
	} else if (word_8A62C == 2) {
		word_8A63C = word_8A8F4;
		word_8A640 = word_8A8FA;
		word_8B464 = word_8A8F4;
		word_8B466 = word_8A8FA;
	} else if (word_8A62C == 3) {
		word_8A63C = word_8A8F4;
		word_8A640 = word_8A8FA;
		word_8B464 = word_8A8F4;
		word_8B466 = word_8A8FA;
	} else if (word_8A62C == 4) {
		word_8A63C = word_8A8F4;
		word_8A640 = word_8A8FA;
		word_8B464 = word_8A8F4;
		word_8B466 = word_8A8FA;
	}

	byte_8A62E = 1;

// '--- ---

}

void Inter_v5::o5_gob97(OpGobParams &params) {
	_byte_8AA14 = 1;

	_vm->_global->_inter_execPtr += 2;
}

void Inter_v5::o5_gob98(OpGobParams &params) {
	_byte_8AA14 = 0;

	_vm->_global->_inter_execPtr += 2;
}

void Inter_v5::o5_gob100(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	uint16 var1 = READ_VAR_UINT16(load16());
	uint16 var2 = READ_VAR_UINT16(load16());
	uint16 var3 = READ_VAR_UINT16(load16());
	uint16 var4 = READ_VAR_UINT16(load16());

	warning("Dynasty Stub: GobFunc 100: %d, %d, %d, %d", var1, var2, var3, var4);

	var3 = (var3 + var1) - 1;
	var4 = (var4 + var2) - 1;
}

void Inter_v5::o5_gob200(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	uint16 var1 = load16(); // index into the spritesArray
	uint16 var2 = load16();
	uint16 var3 = load16();

	warning("Dynasty Stub: GobFunc 200: %d, %d, %d", var1, var2, var3);
}

} // End of namespace Gob
