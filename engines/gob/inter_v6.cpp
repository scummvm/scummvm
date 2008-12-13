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
#include "gob/indeo3.h"

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

	static const OpcodeFuncEntryV6 opcodesFunc[80] = {
		/* 00 */
		OPCODE(o1_callSub),
		OPCODE(o1_callSub),
		OPCODE(o1_printTotText),
		OPCODE(o6_loadCursor),
		/* 04 */
		{NULL, ""},
		OPCODE(o1_switch),
		OPCODE(o1_repeatUntil),
		OPCODE(o1_whileDo),
		/* 08 */
		OPCODE(o1_if),
		OPCODE(o6_evaluateStore),
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

	static const OpcodeGoblinEntryV6 opcodesGoblin[71] = {
		/* 00 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 04 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 08 */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
		/* 0C */
		{NULL, ""},
		{NULL, ""},
		{NULL, ""},
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

	_opcodesDrawV6 = opcodesDraw;
	_opcodesFuncV6 = opcodesFunc;
	_opcodesGoblinV6 = opcodesGoblin;
}

void Inter_v6::executeDrawOpcode(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)",
			i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV6 op = _opcodesDrawV6[i].proc;

	if (op == NULL)
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

	if (op == NULL)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (params);

	return false;
}

void Inter_v6::executeGoblinOpcode(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcV6 op = NULL;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinV6[_goblinFuncLookUp[j][1]].proc;
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

bool Inter_v6::o6_loadCursor(OpFuncParams &params) {
	int16 width, height;
	byte *dataBuf;
	int16 id;
	int8 index;

	id = load16();

	if (id == -1) {
		byte str[10];

		for (int i = 0; i < 9; i++)
			str[i] = *_vm->_global->_inter_execPtr++;

		str[9] = '\0';

		uint16 var1 = load16();
		int8 var2 = *_vm->_global->_inter_execPtr++;

		warning("Urban Stub: loadCursor %d: \"%s\", %d, %d", id, str, var1, var2);

	} else if (id == -2) {

		uint16 var1 = load16();
		uint16 var2 = load16();
		int8 var3 = *_vm->_global->_inter_execPtr++;

		warning("Urban Stub: loadCursor %d: %d, %d, %d", id, var1, var2, var3);

	} else {
		index = (int8) *_vm->_global->_inter_execPtr++;

		if ((index * _vm->_draw->_cursorWidth) >= _vm->_draw->_cursorSprites->getWidth())
			return false;

		dataBuf = _vm->_game->loadTotResource(id, 0, &width, &height);

		_vm->_video->fillRect(_vm->_draw->_cursorSprites,
				index * _vm->_draw->_cursorWidth, 0,
				index * _vm->_draw->_cursorWidth + _vm->_draw->_cursorWidth - 1,
				_vm->_draw->_cursorHeight - 1, 0);

		_vm->_video->drawPackedSprite(dataBuf, width, height,
				index * _vm->_draw->_cursorWidth, 0, 0, _vm->_draw->_cursorSprites);
		_vm->_draw->_cursorAnimLow[index] = 0;
	}

	return false;
}

bool Inter_v6::o6_evaluateStore(OpFuncParams &params) {
	byte *savedPos;
	int16 varOff;
	int16 token;
	int16 result;
	byte loopCount;
	uint16 var_6, var_A;

	varOff = _vm->_parse->parseVarIndex(&var_6, &var_A);

	if (var_6 != 0) {
		int16 var_4;

		savedPos = _vm->_global->_inter_execPtr;

		var_4 = _vm->_parse->parseVarIndex(&var_6, 0);

		memcpy(_vm->_inter->_variables->getAddressOff8(varOff),
				_vm->_inter->_variables->getAddressOff8(var_4), var_6 * 4);

		_vm->_global->_inter_execPtr = savedPos;
		evalExpr(&var_4);

		return false;
	}

	if (*_vm->_global->_inter_execPtr == 98) {
		_vm->_global->_inter_execPtr++;
		loopCount = *_vm->_global->_inter_execPtr++;

		for (int i = 0; i < loopCount; i++) {
			uint8 c = *_vm->_global->_inter_execPtr++;
			uint16 n = load16();

			memset(_vm->_inter->_variables->getAddressOff8(varOff), c, n);

			varOff += n;
		}

		return false;

	} else if (*_vm->_global->_inter_execPtr == 99) {
		_vm->_global->_inter_execPtr++;
		loopCount = *_vm->_global->_inter_execPtr++;
	} else
		loopCount = 1;

	for (int i = 0; i < loopCount; i++) {
		token = evalExpr(&result);
		switch (var_A) {
		case 16:
		case 18:
			WRITE_VARO_UINT8(varOff + i, _vm->_global->_inter_resVal);
			break;

		case 17:
		case 27:
			WRITE_VARO_UINT16(varOff + i * 2, _vm->_global->_inter_resVal);
			break;

		case 23:
		case 26:
			WRITE_VAR_OFFSET(varOff + i * 4, _vm->_global->_inter_resVal);
			break;

		case 24:
			WRITE_VARO_UINT16(varOff + i * 4, _vm->_global->_inter_resVal);
			break;

		case 25:
		case 28:
			if (token == 20)
				WRITE_VARO_UINT8(varOff, result);
			else
				WRITE_VARO_STR(varOff, _vm->_global->_inter_resStr);
			break;
		}
	}

	return false;
}

bool Inter_v6::o6_palLoad(OpFuncParams &params) {
	o1_palLoad(params);

	if (_gotFirstPalette)
		_vm->_video->_palLUT->setPalette((const byte *) _vm->_global->_pPaletteDesc->vgaPal,
				PaletteLUT::kPaletteRGB, 6);

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

} // End of namespace Gob
