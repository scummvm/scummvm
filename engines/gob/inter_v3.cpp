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
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/game.h"
#include "gob/parse.h"

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_v3, x)

const int Inter_v3::_goblinFuncLookUp[][2] = {
	{0, 0},
	{1, 1},
	{2, 2},
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

Inter_v3::Inter_v3(GobEngine *vm) : Inter_v2(vm) {
	setupOpcodes();
}

void Inter_v3::setupOpcodes() {
	static const OpcodeDrawEntryV3 opcodesDraw[256] = {
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
		{0, ""},
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
		OPCODE(o2_initScreen),
		OPCODE(o2_scroll),
		OPCODE(o2_setScrollOffset),
		OPCODE(o2_playImd),
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

	static const OpcodeFuncEntryV3 opcodesFunc[80] = {
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
		OPCODE(o3_copySprite),
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
		OPCODE(o2_readData),
		OPCODE(o2_writeData),
		OPCODE(o1_manageDataFile),
	};

	static const OpcodeGoblinEntryV3 opcodesGoblin[71] = {
		/* 00 */
		OPCODE(o2_loadInfogramesIns),
		OPCODE(o2_startInfogrames),
		OPCODE(o2_stopInfogrames),
		{0, ""},
		/* 04 */
		{0, ""},
		{0, ""},
		{0, ""},
		{0, ""},
		/* 08 */
		{0, ""},
		OPCODE(o2_playInfogrames),
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
		OPCODE(o3_wobble),
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

	_opcodesDrawV3 = opcodesDraw;
	_opcodesFuncV3 = opcodesFunc;
	_opcodesGoblinV3 = opcodesGoblin;
}

void Inter_v3::executeDrawOpcode(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)",
		i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcV3 op = _opcodesDrawV3[i].proc;

	if (op == 0)
		warning("unimplemented opcodeDraw: %d", i);
	else
		(this->*op) ();
}

bool Inter_v3::executeFuncOpcode(byte i, byte j, OpFuncParams &params) {
	debugC(1, kDebugFuncOp, "opcodeFunc %d.%d [0x%X.0x%X] (%s)",
		i, j, i, j, getOpcodeFuncDesc(i, j));

	if ((i > 4) || (j > 15)) {
		warning("unimplemented opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcV3 op = _opcodesFuncV3[i*16 + j].proc;

	if (op == 0)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (params);

	return false;
}

void Inter_v3::executeGoblinOpcode(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
		i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcV3 op = 0;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinV3[_goblinFuncLookUp[j][1]].proc;
			break;
		}

	if (op == 0) {
		int16 val;

		_vm->_global->_inter_execPtr -= 2;
		val = load16();
		_vm->_global->_inter_execPtr += val << 1;
	} else
		(this->*op) (params);
}

const char *Inter_v3::getOpcodeDrawDesc(byte i) {
	return _opcodesDrawV3[i].desc;
}

const char *Inter_v3::getOpcodeFuncDesc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncV3[i*16 + j].desc;
}

const char *Inter_v3::getOpcodeGoblinDesc(int i) {
	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i)
			return _opcodesGoblinV3[_goblinFuncLookUp[j][1]].desc;
	return "";
}

bool Inter_v3::o3_getTotTextItemPart(OpFuncParams &params) {
	byte *totData;
	int16 totTextItem;
	int16 part, curPart = 0;
	int16 offX = 0, offY = 0;
	int16 collId = 0, collCmd;
	uint32 stringStartVar, stringVar;
	bool end;

	totTextItem = load16();
	stringStartVar = _vm->_parse->parseVarIndex();
	part = _vm->_parse->parseValExpr();

	stringVar = stringStartVar;
	WRITE_VARO_UINT8(stringVar, 0);

	if (!_vm->_game->_totTextData)
		return false;

	totData = _vm->_game->_totTextData->dataPtr +
		_vm->_game->_totTextData->items[totTextItem].offset;

	// Skip background rectangles
	while (((int16) READ_LE_UINT16(totData)) != -1)
		totData += 9;
	totData += 2;

	while (*totData != 1) {
		switch (*totData) {
		case 2:
		case 5:
			totData++;
			offX = READ_LE_UINT16(totData);
			offY = READ_LE_UINT16(totData + 2);
			totData += 4;
			break;

		case 3:
		case 4:
			totData += 2;
			break;

		case 6:
			totData++;

			collCmd = *totData++;
			if (collCmd & 0x80) {
				collId = READ_LE_UINT16(totData);
				totData += 2;
			}

			// Skip collision coordinates
			if (collCmd & 0x40)
				totData += 8;

			if ((collCmd & 0x8F) && ((-collId - 1) == part)) {
				int n = 0;

				while (1) {
					if ((*totData < 1) || (*totData > 7)) {
						if (*totData >= 32) {
							WRITE_VARO_UINT8(stringVar++, *totData++);
							n++;
						} else
							totData++;
						continue;
					}

					if ((n != 0) || (*totData == 1) ||
							(*totData == 6) || (*totData == 7)) {
						WRITE_VARO_UINT8(stringVar, 0);
						return false;
					}

					switch (*totData) {
					case 2:
					case 5:
						totData += 5;
						break;

					case 3:
					case 4:
						totData += 2;
						break;
					}
				}

			}
			break;

		case 7:
		case 8:
		case 9:
			totData++;
			break;

		case 10:
			if (curPart == part) {
				WRITE_VARO_UINT8(stringVar++, 0xFF);
				WRITE_VARO_UINT16(stringVar, offX);
				WRITE_VARO_UINT16(stringVar + 2, offY);
				WRITE_VARO_UINT16(stringVar + 4,
						totData - _vm->_game->_totTextData->dataPtr);
				WRITE_VARO_UINT8(stringVar + 6, 0);
				return false;
			}

			end = false;
			while (!end) {
				switch (*totData) {
				case 2:
				case 5:
					if (ABS(offY - READ_LE_UINT16(totData + 3)) > 1)
						end = true;
					else
						totData += 5;
					break;

				case 3:
					totData += 2;
					break;

				case 10:
					totData += totData[1] * 2 + 2;
					break;

				default:
					if (*totData < 32)
						end = true;
					while (*totData >= 32)
						totData++;
					break;
				}
			}

			if (part >= 0)
				curPart++;
			break;

		default:
			while (1) {

				while (*totData >= 32)
					WRITE_VARO_UINT8(stringVar++, *totData++);
				WRITE_VARO_UINT8(stringVar, 0);

				if (((*totData != 2) && (*totData != 5)) ||
						(ABS(offY - READ_LE_UINT16(totData + 3)) > 1)) {

					if (curPart == part)
						return false;

					stringVar = stringStartVar;
					WRITE_VARO_UINT8(stringVar, 0);

					while (*totData >= 32)
						totData++;

					if (part >= 0)
						curPart++;
					break;

				} else
					totData += 5;

			}
			break;
		}
	}

	return false;
}

bool Inter_v3::o3_copySprite(OpFuncParams &params) {
	o1_copySprite(params);

	// For the close-up "fading" in the CD version
	if (_vm->_draw->_destSurface == 20)
		_vm->_video->sparseRetrace(20);
	return false;
}

void Inter_v3::o3_wobble(OpGobParams &params) {
	_vm->_draw->wobble(*_vm->_draw->_backSurface);
}

} // End of namespace Gob
