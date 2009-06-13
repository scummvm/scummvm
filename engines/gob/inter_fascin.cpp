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

#include "gob/gob.h"
#include "gob/inter.h"
#include "gob/global.h"
#include "gob/util.h"
#include "gob/dataio.h"
#include "gob/draw.h"
#include "gob/parse.h"
#include "gob/game.h"
#include "gob/palanim.h"
#include "gob/video.h"
#include "gob/videoplayer.h"
#include "gob/sound/sound.h"

namespace Gob {

#define OPCODE(x) _OPCODE(Inter_Fascination, x)

const int Inter_Fascination::_goblinFuncLookUp[][2] = {
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
	{11, 10},
	{12, 11},
	{1000, 12},
	{1001, 13},
	{1002, 14}
};

Inter_Fascination::Inter_Fascination(GobEngine *vm) : Inter_v2(vm) {
	setupOpcodes();
}

void Inter_Fascination::setupOpcodes() {
	static const OpcodeDrawEntryFascination opcodesDraw[256] = {
		/* 00 */
		OPCODE(o1_loadMult),
		OPCODE(o2_playMult),
		OPCODE(o1_freeMultKeys),
		OPCODE(oFascin_cdUnknown3),
		/* 04 */
		OPCODE(oFascin_cdUnknown4),
		OPCODE(oFascin_cdUnknown5),
		OPCODE(oFascin_cdUnknown6),
		OPCODE(o1_initCursor),
		/* 08 */
		OPCODE(o1_initCursorAnim),
		OPCODE(o1_clearCursorAnim),
		OPCODE(oFascin_setRenderFlags),
		OPCODE(oFascin_cdUnknown11),
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
		{0, ""},//OPCODE(o2_loadMapObjects),
		{0, ""},//OPCODE(o2_freeGoblins),
		{0, ""},//OPCODE(o2_moveGoblin),
		{0, ""},//OPCODE(o2_writeGoblinPos),
		/* 54 */
		{0, ""},//OPCODE(o2_stopGoblin),
		{0, ""},//OPCODE(o2_setGoblinState),
		{0, ""},//OPCODE(o2_placeGoblin),
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
		{0, ""},//OPCODE(o2_initScreen),
		{0, ""},//OPCODE(o2_scroll),
		{0, ""},//OPCODE(o2_setScrollOffset),
		{0, ""},//OPCODE(o2_playImd),
		/* 84 */
		{0, ""},//OPCODE(o2_getImdInfo),
		{0, ""},//OPCODE(o2_openItk),
		{0, ""},//OPCODE(o2_closeItk),
		{0, ""},//OPCODE(o2_setImdFrontSurf),
		/* 88 */
		{0, ""},//OPCODE(o2_resetImdFrontSurf),
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

	static const OpcodeFuncEntryFascination opcodesFunc[80] = {
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
		OPCODE(o1_assign),
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
		OPCODE(o1_animPalInit),
		/* 18 */
		OPCODE(o2_addCollision),
		OPCODE(o2_freeCollision),
		{0, ""},
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

	static const OpcodeGoblinEntryFascination opcodesGoblin[15] = {
		/* 00 */
		OPCODE(oFascin_geUnknown0),
		OPCODE(oFascin_geUnknown1),
		OPCODE(oFascin_geUnknown2),
		OPCODE(oFascin_geUnknown3),
		/* 04 */
		OPCODE(oFascin_geUnknown4),
		OPCODE(oFascin_geUnknown5),
		OPCODE(oFascin_geUnknown6),
		OPCODE(oFascin_geUnknown7),
		/* 08 */
		OPCODE(oFascin_geUnknown8),
		OPCODE(oFascin_geUnknown9),
		OPCODE(oFascin_geUnknown10),
		OPCODE(oFascin_geUnknown11),
		/* 0C */
		OPCODE(oFascin_geUnknown1000),
		OPCODE(oFascin_geUnknown1001), //protrackerPlay doesn't play correctly "mod.extasy"
		OPCODE(oFascin_geUnknown1002), //to be replaced by o2_stopProtracker when protrackerPlay is fixed
	};

	_opcodesDrawFascination = opcodesDraw;
	_opcodesFuncFascination = opcodesFunc;
	_opcodesGoblinFascination = opcodesGoblin;
}

void Inter_Fascination::executeDrawOpcode(byte i) {
	debugC(1, kDebugDrawOp, "opcodeDraw %d [0x%X] (%s)",
			i, i, getOpcodeDrawDesc(i));

	OpcodeDrawProcFascination op = _opcodesDrawFascination[i].proc;

	if (op == 0)
		warning("Not yet implemented Fascination opcodeDraw: %d", i);
	else
		(this->*op) ();
}

void Inter_Fascination::oFascin_geUnknown0(OpGobParams &params) {
	warning("Fascination Unknown GE Function 0");
	warning("funcPlayImd with parameter : 'tirb.imd'");

	if (_vm->_vidPlayer->primaryOpen("tirb.imd")) {
		_vm->_vidPlayer->primaryPlay();
		_vm->_vidPlayer->primaryClose();
	}
	_vm->_draw->closeScreen();
}

void Inter_Fascination::oFascin_geUnknown1(OpGobParams &params) {
	warning("Fascination Unknown GE Function 1");
	warning("funcPlayImd with parameter : 'tira.imd'");
	if (_vm->_vidPlayer->primaryOpen("tira.imd")) {
		_vm->_vidPlayer->primaryPlay();
		_vm->_vidPlayer->primaryClose();
	}
	_vm->_draw->closeScreen();
}

void Inter_Fascination::oFascin_geUnknown2(OpGobParams &params) {
	_vm->_sound->adlibLoadTbr("extasy.tbr");
	_vm->_sound->adlibLoadMdy("extasy.mdy");
}

void Inter_Fascination::oFascin_geUnknown3(OpGobParams &params) {
	_vm->_sound->adlibPlay();
}

void Inter_Fascination::oFascin_geUnknown4(OpGobParams &params) {
	_vm->_sound->adlibStop();
}

void Inter_Fascination::oFascin_geUnknown5(OpGobParams &params) {
	_vm->_sound->adlibUnload();
}

void Inter_Fascination::oFascin_geUnknown6(OpGobParams &params) {
	_vm->_sound->adlibLoadTbr("music1.tbr");
	_vm->_sound->adlibLoadMdy("music1.mdy");
}

void Inter_Fascination::oFascin_geUnknown7(OpGobParams &params) {
	_vm->_sound->adlibLoadTbr("music2.tbr");
	_vm->_sound->adlibLoadMdy("music2.mdy");
}

void Inter_Fascination::oFascin_geUnknown8(OpGobParams &params) {
	_vm->_sound->adlibLoadTbr("music3.tbr");
	_vm->_sound->adlibLoadMdy("music3.mdy");
}

void Inter_Fascination::oFascin_geUnknown9(OpGobParams &params) {
	_vm->_sound->adlibLoadTbr("batt1.tbr");
	_vm->_sound->adlibLoadMdy("batt1.mdy");
}

void Inter_Fascination::oFascin_geUnknown10(OpGobParams &params) {
	_vm->_sound->adlibLoadTbr("batt2.tbr");
	_vm->_sound->adlibLoadMdy("batt2.mdy");
}

void Inter_Fascination::oFascin_geUnknown11(OpGobParams &params) {
	_vm->_sound->adlibLoadTbr("batt3.tbr");
	_vm->_sound->adlibLoadMdy("batt3.mdy");
}

void Inter_Fascination::oFascin_geUnknown1000(OpGobParams &params) {
	warning("Fascination Unknown GE Function 1000 - Load MOD music");
}

void Inter_Fascination::oFascin_geUnknown1001(OpGobParams &params) {
	warning("Fascination oFascin_playProtracker - MOD not compatible (sample > 32768), To Be Fixed");
}

void Inter_Fascination::oFascin_geUnknown1002(OpGobParams &params) {
	warning("Fascination o2_stopProtracker - Commented out");
}

bool Inter_Fascination::oFascin_feUnknown4(OpFuncParams &params) {
	warning("Fascination Unknown FE Function 4");
	return true;
}

bool Inter_Fascination::oFascin_feUnknown27(OpFuncParams &params) {
	warning("Fascination Unknown FE Function 27h");
	return true;
}

void Inter_Fascination::oFascin_cdUnknown3() {
	uint16 resVar, resVar2;
	int16 retVal1, retVal2, retVal3, retVal4, retVal5, retVal6, retVal7;

	warning("Fascination oFascin_cdUnknown3 - Variables initialisations");

	resVar = (uint16) load16();
	resVar2 = (uint16) load16();
	retVal1 = _vm->_parse->parseVarIndex();
	retVal2 = _vm->_parse->parseVarIndex();
	retVal3 = _vm->_parse->parseVarIndex();
	retVal4 = _vm->_parse->parseVarIndex();
	retVal5 = _vm->_parse->parseVarIndex();
	retVal6 = _vm->_parse->parseVarIndex();
	retVal7 = _vm->_parse->parseVarIndex();
	warning ("Width? :%d Height? :%d",resVar, resVar2);
	warning ("Fetched variables 1:%d 2:%d 3:%d 4:%d 5:%d 6:%d 7:%d", retVal1, retVal2, retVal3, retVal4, retVal5, retVal6, retVal7);
}

void Inter_Fascination::oFascin_cdUnknown4() {
	int16 expr;
	warning("Fascination oFascin_cdUnknown4");
	evalExpr(&expr);
	warning ("evalExpr: %d, the rest is not yet implemented",expr);
}

void Inter_Fascination::oFascin_cdUnknown5() {
	int16 retVal1,expr;
	warning("Fascination oFascin_cdUnknown5");
	evalExpr(&expr);
	retVal1 = _vm->_parse->parseVarIndex();
	warning ("evalExpr: %d Variable index %d, the rest is not yet implemented",expr, retVal1);
}

void Inter_Fascination::oFascin_cdUnknown6() {
	int16 retVal1,expr;
	warning("Fascination oFascin_cdUnknown6");
	evalExpr(&expr);
	retVal1 = _vm->_parse->parseVarIndex();
	warning ("evalExpr: %d Variable index %d, the rest is not yet implemented",expr, retVal1);
}

void Inter_Fascination::oFascin_setRenderFlags() {
	int16 expr;
//	warning("Fascination oFascin_cdUnknown10 (set render flags)");
	evalExpr(&expr);
	warning("_draw_renderFlags <- %d",expr);
	_vm->_draw->_renderFlags = expr;
}

void Inter_Fascination::oFascin_cdUnknown11() {
//	warning("Fascination oFascin_cdUnknown11 (set variable)");
	evalExpr(0);
}

bool Inter_Fascination::executeFuncOpcode(byte i, byte j, OpFuncParams &params) {
	debugC(1, kDebugFuncOp, "opcodeFunc %d.%d [0x%X.0x%X] (%s)",
			i, j, i, j, getOpcodeFuncDesc(i, j));

	if ((i > 4) || (j > 16)) {
		warning("Invalid opcodeFunc: %d.%d", i, j);
		return false;
	}

	OpcodeFuncProcFascination op = _opcodesFuncFascination[i*16 + j].proc;

	if (op == 0)
		warning("unimplemented opcodeFunc: %d.%d", i, j);
	else
		return (this->*op) (params);

	return false;
}

void Inter_Fascination::executeGoblinOpcode(int i, OpGobParams &params) {
	debugC(1, kDebugGobOp, "opcodeGoblin %d [0x%X] (%s)",
			i, i, getOpcodeGoblinDesc(i));

	OpcodeGoblinProcFascination op = 0;

	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i) {
			op = _opcodesGoblinFascination[_goblinFuncLookUp[j][1]].proc;
			break;
		}

	if (op == 0) {
		int16 val;

		_vm->_global->_inter_execPtr -= 2;
		val = load16();
		_vm->_global->_inter_execPtr += val << 1;
		warning("unimplemented opcodeGob: %d", i);
	} else
		(this->*op) (params);
}

const char *Inter_Fascination::getOpcodeDrawDesc(byte i) {
	return _opcodesDrawFascination[i].desc;
}

const char *Inter_Fascination::getOpcodeFuncDesc(byte i, byte j) {
	if ((i > 4) || (j > 15))
		return "";

	return _opcodesFuncFascination[i*16 + j].desc;
}

const char *Inter_Fascination::getOpcodeGoblinDesc(int i) {
	for (int j = 0; j < ARRAYSIZE(_goblinFuncLookUp); j++)
		if (_goblinFuncLookUp[j][0] == i)
			return _opcodesGoblinFascination[_goblinFuncLookUp[j][1]].desc;
	warning("Error in getOpcodeGoblinDesc %d",i);
	return "";
}

void Inter_Fascination::oFascin_playProtracker(OpGobParams &params) {
	_vm->_sound->protrackerPlay("mod.extasy");
}

} // End of namespace Gob
