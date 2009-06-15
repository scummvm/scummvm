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
	{3, 0},
	{2, 0},
	{33, 0},
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
	{200, 14}
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
		OPCODE(o2_totSub),
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
		OPCODE(o4_playVmdOrMusic),
		/* 84 */
		OPCODE(o2_getImdInfo),
		OPCODE(o2_openItk),
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

	static const OpcodeFuncEntryV5 opcodesFunc[80] = {
		/* 00 */
		OPCODE(o1_callSub),
		OPCODE(o1_callSub),
		OPCODE(o1_printTotText),
		OPCODE(o1_loadCursor),
		/* 04 */
		{0, ""},
		OPCODE(o1_switch),
		OPCODE(o1_repeatUntil),
		OPCODE(o1_whileDo),
		/* 08 */
		OPCODE(o1_if),
		OPCODE(o2_assign),
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

	_opcodesDrawV5 = opcodesDraw;
	_opcodesFuncV5 = opcodesFunc;
	_opcodesGoblinV5 = opcodesGoblin;
}

void Inter_v5::executeDrawOpcode(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)",
			i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV5 op = _opcodesDrawV5[i].proc;

	if (op == 0)
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

	if (op == 0)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (params);

	return false;
}

void Inter_v5::executeGoblinOpcode(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcV5 op = 0;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinV5[_goblinFuncLookUp[j][1]].proc;
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

	warning("Dynasty Stub: deleteFile \"%s\"", _vm->_parse->_inter_resStr);
}

void Inter_v5::o5_initScreen() {
	int16 offY;
	int16 videoMode;
	int16 width, height;

	offY = load16();

	videoMode = offY & 0xFF;
	offY = (offY >> 8) & 0xFF;

	width = _vm->_parse->parseValExpr();
	height = _vm->_parse->parseValExpr();

	warning("initScreen: %d, %d, %d, %d", width, height, offY, videoMode);

	_vm->_video->clearScreen();

	if (videoMode == 0x13) {

		if (width == -1)
			width = 320;
		if (height == -1)
			height = 200;

		_vm->_width = 320;
		_vm->_height = 200;

		_vm->_video->setSize(false);

	} else if (_vm->_global->_videoMode == 0x13) {
		width = _vm->_width = 640;
		height = _vm->_height = 480;

		_vm->_video->setSize(true);
	}

	_vm->_global->_fakeVideoMode = videoMode;

	// Some versions require this
	if (videoMode == 0x18)
		_vm->_global->_fakeVideoMode = 0x37;

	if ((videoMode == _vm->_global->_videoMode) && (width == -1))
		return;

	if (width > 0)
		_vm->_video->_surfWidth = width;
	if (height > 0)
		_vm->_video->_surfHeight = height;

	_vm->_video->_screenDeltaX = 0;
	if (_vm->_video->_surfWidth < _vm->_width)
		_vm->_video->_screenDeltaX = (_vm->_width - _vm->_video->_surfWidth) / 2;

	_vm->_global->_mouseMinX = _vm->_video->_screenDeltaX;
	_vm->_global->_mouseMaxX = _vm->_video->_screenDeltaX + _vm->_video->_surfWidth - 1;

	_vm->_video->_splitStart = _vm->_video->_surfHeight - offY;

	_vm->_video->_splitHeight1 = MIN<int16>(_vm->_height, _vm->_video->_surfHeight);
	_vm->_video->_splitHeight2 = offY;

	if ((_vm->_video->_surfHeight + offY) < _vm->_height)
		_vm->_video->_screenDeltaY = (_vm->_height - (_vm->_video->_surfHeight + offY)) / 2;
	else
		_vm->_video->_screenDeltaY = 0;

	_vm->_global->_mouseMaxY = (_vm->_video->_surfHeight + _vm->_video->_screenDeltaY) - offY - 1;
	_vm->_global->_mouseMinY = _vm->_video->_screenDeltaY;

	_vm->_draw->closeScreen();
	_vm->_util->clearPalette();
	memset(_vm->_global->_redPalette, 0, 256);
	memset(_vm->_global->_greenPalette, 0, 256);
	memset(_vm->_global->_bluePalette, 0, 256);

	_vm->_video->_splitSurf.reset();
	_vm->_draw->_spritesArray[24].reset();
	_vm->_draw->_spritesArray[25].reset();

	_vm->_global->_videoMode = videoMode;
	_vm->_video->initPrimary(videoMode);
	WRITE_VAR(15, _vm->_global->_fakeVideoMode);

	_vm->_global->_setAllPalette = true;

	_vm->_util->setMousePos(_vm->_global->_inter_mouseX,
			_vm->_global->_inter_mouseY);
	_vm->_util->clearPalette();

	_vm->_draw->initScreen();

	_vm->_util->setScrollOffset();

	if (offY > 0) {
		_vm->_draw->_spritesArray[24] = SurfaceDescPtr(new SurfaceDesc(videoMode, _vm->_width, offY));
		_vm->_draw->_spritesArray[25] = SurfaceDescPtr(new SurfaceDesc(videoMode, _vm->_width, offY));
		_vm->_video->_splitSurf = _vm->_draw->_spritesArray[25];
	}
}

bool Inter_v5::o5_istrlen(OpFuncParams &params) {
	int16 strVar1, strVar2;
	int16 len;
	uint16 type;

	if (*_vm->_global->_inter_execPtr == 0x80) {
		_vm->_global->_inter_execPtr++;

		strVar1 = _vm->_parse->parseVarIndex();
		strVar2 = _vm->_parse->parseVarIndex(0, &type);

		len = _vm->_draw->stringLength(GET_VARO_STR(strVar1), READ_VARO_UINT16(strVar2));

	} else {

		strVar1 = _vm->_parse->parseVarIndex();
		strVar2 = _vm->_parse->parseVarIndex(0, &type);

		if (_vm->_global->_language == 10) {
			// Extra handling for Japanese strings

			for (len = 0; READ_VARO_UINT8(strVar1) != 0; strVar1++, len++)
				if (READ_VARO_UINT8(strVar1) >= 128)
					strVar1++;

		} else
			len = strlen(GET_VARO_STR(strVar1));
	}

	writeVar(strVar2, type, (int32) len);

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

	load16();
	load16();

	if (params.extraData != 0) {
		WRITE_VARO_UINT32(var1, 2);
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
		_vm->_draw->drawString("100 %", 402, 89, 112, 144, 0, *_vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_getSystemRAM(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 168, 112, 144, 0, *_vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_getSystemCPUSpeed(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 248, 112, 144, 0, *_vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_getSystemDrawSpeed(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 326, 112, 144, 0, *_vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		delete font;
	}
}

void Inter_v5::o5_totalSystemSpecs(OpGobParams &params) {
	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 100); // Fudging 100%

	Video::FontDesc *font;
	if ((font = _vm->_util->loadFont("SPEED.LET"))) {
		_vm->_draw->drawString("100 %", 402, 405, 112, 144, 0, *_vm->_draw->_backSurface, font);
		_vm->_draw->forceBlit();

		delete font;
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
	speedSum = MAX(_cdSpeed, 150);
	speedSum += (_ram << 3);
	speedSum += (_cpuSpeed << 3);
	speedSum /= 17;

	needThrottle2 = (speedSum > 81) ? 1 : 0;
	needThrottle1 = (_total >= 95) ? 1 : 0;

	if (needThrottle1 == 1) {
		speedThrottle1 = 100;
		speedThrottle2 = 1;
		speedThrottle3 = 1;
		speedThrottle4 = 0;
	} else {
		speedThrottle1 = 0;
		speedThrottle2 = 0;
		speedThrottle3 = 0;
		speedThrottle4 = 40;
	}
*/
}

void Inter_v5::o5_gob92(OpGobParams &params) {
	warning("Dynasty Stub: GobFunc 92");

	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int8) _gob92_1)) */);
}

void Inter_v5::o5_gob95(OpGobParams &params) {
	warning("Dynasty Stub: GobFunc 95");

	_vm->_global->_inter_execPtr += 2;

	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int16) speedThrottle4)) */);
	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int8)  speedThrottle3)) */);
	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int8)  speedThrottle2)) */);
	WRITE_VAR_UINT32(load16(), 0 /* (uint32) ((int32) ((int16) speedThrottle1)) */);
}

void Inter_v5::o5_gob96(OpGobParams &params) {
	int16 speedThrottle4, speedThrottle1;
	byte speedThrottle3, speedThrottle2;

	_vm->_global->_inter_execPtr += 2;

	speedThrottle4 = READ_VAR_UINT16(load16());
	speedThrottle3 = READ_VAR_UINT8(load16());
	speedThrottle2 = READ_VAR_UINT8(load16());
	speedThrottle1 = READ_VAR_UINT16(load16());

	warning("Dynasty Stub: GobFunc 96: %d, %d, %d, %d",
			speedThrottle4, speedThrottle3, speedThrottle2, speedThrottle1);

	// TODO
}

void Inter_v5::o5_gob97(OpGobParams &params) {
	_gob_97_98_val = 1;

	_vm->_global->_inter_execPtr += 2;
}

void Inter_v5::o5_gob98(OpGobParams &params) {
	_gob_97_98_val = 0;

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
