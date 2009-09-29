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
#include "gob/game.h"
#include "gob/script.h"
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

	OPCODEDRAW(0x03, oFascin_setWinSize);
	OPCODEDRAW(0x04, oFascin_closeWin);
	OPCODEDRAW(0x05, oFascin_activeWin);
	OPCODEDRAW(0x06, oFascin_openWin);

	OPCODEDRAW(0x0A, oFascin_setRenderFlags);
	OPCODEDRAW(0x0B, oFascin_setWinFlags);

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

	OPCODEFUNC(0x09, o1_assign);
}

void Inter_Fascination::setupOpcodesGob() {
	OPCODEGOB(   1, oFascin_playTirb);
	OPCODEGOB(   2, oFascin_playTira);
	OPCODEGOB(   3, oFascin_loadExtasy);
	OPCODEGOB(   4, oFascin_adlibPlay);

	OPCODEGOB(   5, oFascin_adlibStop);
	OPCODEGOB(   6, oFascin_adlibUnload);
	OPCODEGOB(   7, oFascin_loadMus1);
	OPCODEGOB(   8, oFascin_loadMus2);

	OPCODEGOB(   9, oFascin_loadMus3);
	OPCODEGOB(  10, oFascin_loadBatt1);
	OPCODEGOB(  11, oFascin_loadBatt2);
	OPCODEGOB(  12, oFascin_loadBatt3);

	OPCODEGOB(1000, oFascin_geUnknown1000);
	OPCODEGOB(1001, oFascin_geUnknown1001); //protrackerPlay doesn't play correctly "mod.extasy"
	OPCODEGOB(1002, oFascin_geUnknown1002); //to be replaced by o2_stopProtracker when protrackerPlay is fixed
}

void Inter_Fascination::oFascin_playTirb(OpGobParams &params) {
	warning("Fascination Unknown GE Function 0");
	warning("funcPlayImd with parameter : 'tirb.imd'");

	if (_vm->_vidPlayer->primaryOpen("tirb.imd")) {
		_vm->_vidPlayer->primaryPlay();
		_vm->_vidPlayer->primaryClose();
	}
	_vm->_draw->closeScreen();
}

void Inter_Fascination::oFascin_playTira(OpGobParams &params) {
	warning("Fascination Unknown GE Function 1");
	warning("funcPlayImd with parameter : 'tira.imd'");
	if (_vm->_vidPlayer->primaryOpen("tira.imd")) {
		_vm->_vidPlayer->primaryPlay();
		_vm->_vidPlayer->primaryClose();
	}
	_vm->_draw->closeScreen();
}

void Inter_Fascination::oFascin_loadExtasy(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("extasy.tbr");
	_vm->_sound->adlibLoadMDY("extasy.mdy");
}

void Inter_Fascination::oFascin_adlibPlay(OpGobParams &params) {
	_vm->_sound->adlibPlay();
}

void Inter_Fascination::oFascin_adlibStop(OpGobParams &params) {
	_vm->_sound->adlibStop();
}

void Inter_Fascination::oFascin_adlibUnload(OpGobParams &params) {
	_vm->_sound->adlibUnload();
}

void Inter_Fascination::oFascin_loadMus1(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("music1.tbr");
	_vm->_sound->adlibLoadMDY("music1.mdy");
}

void Inter_Fascination::oFascin_loadMus2(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("music2.tbr");
	_vm->_sound->adlibLoadMDY("music2.mdy");
}

void Inter_Fascination::oFascin_loadMus3(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("music3.tbr");
	_vm->_sound->adlibLoadMDY("music3.mdy");
}

void Inter_Fascination::oFascin_loadBatt1(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("batt1.tbr");
	_vm->_sound->adlibLoadMDY("batt1.mdy");
}

void Inter_Fascination::oFascin_loadBatt2(OpGobParams &params) {
	_vm->_sound->adlibLoadTBR("batt2.tbr");
	_vm->_sound->adlibLoadMDY("batt2.mdy");
}

void Inter_Fascination::oFascin_loadBatt3(OpGobParams &params) {
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

void Inter_Fascination::oFascin_setWinSize() {
	_vm->_draw->_winMaxWidth  = _vm->_game->_script->readUint16();
	_vm->_draw->_winMaxHeight = _vm->_game->_script->readUint16();
	_vm->_draw->_winVarArrayLeft   = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayTop    = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayWidth  = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayHeight = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayStatus = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayLimitsX = _vm->_game->_script->readVarIndex();
	_vm->_draw->_winVarArrayLimitsY = _vm->_game->_script->readVarIndex();
	warning("Inter_Fascination::oFascin_setWinSize lmax %d hmax %d x %d y %d l %d h %d stat %d limx %d limy %d",
			 _vm->_draw->_winMaxWidth, _vm->_draw->_winMaxHeight,
			 _vm->_draw->_winVarArrayLeft,  _vm->_draw->_winVarArrayTop,
			 _vm->_draw->_winVarArrayWidth, _vm->_draw->_winVarArrayHeight, _vm->_draw->_winVarArrayStatus, 
			 _vm->_draw->_winVarArrayLimitsX, _vm->_draw->_winVarArrayLimitsY);
}

void Inter_Fascination::oFascin_closeWin() {
	int16 id;
	_vm->_game->_script->evalExpr(&id);
	warning("Inter_Fascination::closeWin id %d", id);
	_vm->_draw->activeWin(id);
	_vm->_draw->closeWin(id);	
}

void Inter_Fascination::oFascin_activeWin() {
	int16 id;
	_vm->_game->_script->evalExpr(&id);
	warning("Inter_Fascination::activeWin id %d", id);
	_vm->_draw->activeWin(id);
}

void Inter_Fascination::oFascin_openWin() {
	int16 retVal,id;
	_vm->_game->_script->evalExpr(&id);
	retVal = _vm->_game->_script->readVarIndex();
	warning("Inter_Fascination::openWin index %d, id %d", retVal, id);
	WRITE_VAR((retVal / 4), (int32) _vm->_draw->openWin(id));
}

void Inter_Fascination::oFascin_setRenderFlags() {
	int16 expr;
	_vm->_game->_script->evalExpr(&expr);
	_vm->_draw->_renderFlags = expr;
	warning("Inter_Fascination::oFascin_setRenderFlags: _draw_renderFlags <- %d",expr);
}

void Inter_Fascination::oFascin_setWinFlags() {
	int16 expr;
	_vm->_game->_script->evalExpr(&expr);
	_vm->_global->_curWinId = expr;
	warning("Inter_Fascination::oFascin_setWinFlags: _global_DEVwnd <- %d",expr);
}

void Inter_Fascination::oFascin_playProtracker(OpGobParams &params) {
	_vm->_sound->protrackerPlay("mod.extasy");
}

} // End of namespace Gob
