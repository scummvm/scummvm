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

#define OPCODEVER Inter_Fascination
#define OPCODEDRAW(i, x)  _opcodesDraw[i]._OPCODEDRAW(OPCODEVER, x)
#define OPCODEFUNC(i, x)  _opcodesFunc[i]._OPCODEFUNC(OPCODEVER, x)
#define OPCODEGOB(i, x)   _opcodesGob[i]._OPCODEGOB(OPCODEVER, x)

Inter_Fascination::Inter_Fascination(GobEngine *vm) : Inter_v2(vm) {
}

void Inter_Fascination::setupOpcodesDraw() {
	Inter_v2::setupOpcodesDraw();

	OPCODEDRAW(0x03, oFascin_cdUnknown3);

	OPCODEDRAW(0x04, oFascin_cdUnknown4);
	OPCODEDRAW(0x05, oFascin_cdUnknown5);
	OPCODEDRAW(0x06, oFascin_cdUnknown6);

	OPCODEDRAW(0x0A, oFascin_setRenderFlags);
	OPCODEDRAW(0x0B, oFascin_cdUnknown11);

	CLEAROPCODEDRAW(0x50);
	CLEAROPCODEDRAW(0x51);
	CLEAROPCODEDRAW(0x52);
	CLEAROPCODEDRAW(0x53);

	CLEAROPCODEDRAW(0x54);
	CLEAROPCODEDRAW(0x55);
	CLEAROPCODEDRAW(0x56);

	CLEAROPCODEDRAW(0x80);
	CLEAROPCODEDRAW(0x81);
	CLEAROPCODEDRAW(0x82);
	CLEAROPCODEDRAW(0x83);

	CLEAROPCODEDRAW(0x84);
	CLEAROPCODEDRAW(0x85);
	CLEAROPCODEDRAW(0x86);
	CLEAROPCODEDRAW(0x87);

	CLEAROPCODEDRAW(0x88);
}

void Inter_Fascination::setupOpcodesFunc() {
	Inter_v2::setupOpcodesFunc();
}

void Inter_Fascination::setupOpcodesGob() {
	OPCODEGOB(   1, oFascin_geUnknown0);
	OPCODEGOB(   2, oFascin_geUnknown1);
	OPCODEGOB(   3, oFascin_geUnknown2);
	OPCODEGOB(   4, oFascin_geUnknown3);

	OPCODEGOB(   5, oFascin_geUnknown4);
	OPCODEGOB(   6, oFascin_geUnknown5);
	OPCODEGOB(   7, oFascin_geUnknown6);
	OPCODEGOB(   8, oFascin_geUnknown7);

	OPCODEGOB(   9, oFascin_geUnknown8);
	OPCODEGOB(  10, oFascin_geUnknown9);
	OPCODEGOB(  11, oFascin_geUnknown10);
	OPCODEGOB(  12, oFascin_geUnknown11);

	OPCODEGOB(1000, oFascin_geUnknown1000);
	OPCODEGOB(1001, oFascin_geUnknown1001); //protrackerPlay doesn't play correctly "mod.extasy"
	OPCODEGOB(1002, oFascin_geUnknown1002); //to be replaced by o2_stopProtracker when protrackerPlay is fixed
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
	_vm->_sound->adlibLoadTBR("extasy.tbr");
	_vm->_sound->adlibLoadMDY("extasy.mdy");
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
	_vm->_sound->adlibLoadTBR("music1.tbr");
	_vm->_sound->adlibLoadMDY("music1.mdy");
}

void Inter_Fascination::oFascin_geUnknown7(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("music2.tbr");
	_vm->_sound->adlibLoadMDY("music2.mdy");
}

void Inter_Fascination::oFascin_geUnknown8(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("music3.tbr");
	_vm->_sound->adlibLoadMDY("music3.mdy");
}

void Inter_Fascination::oFascin_geUnknown9(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("batt1.tbr");
	_vm->_sound->adlibLoadMDY("batt1.mdy");
}

void Inter_Fascination::oFascin_geUnknown10(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("batt2.tbr");
	_vm->_sound->adlibLoadMDY("batt2.mdy");
}

void Inter_Fascination::oFascin_geUnknown11(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("batt3.tbr");
	_vm->_sound->adlibLoadMDY("batt3.mdy");
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

void Inter_Fascination::oFascin_playProtracker(OpGobParams &params) {
	_vm->_sound->protrackerPlay("mod.extasy");
}

} // End of namespace Gob
