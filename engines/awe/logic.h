/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef AWE_LOGIC_H
#define AWE_LOGIC_H

#include "awe/intern.h"

namespace Awe {

struct Resource;
struct Serializer;
struct SystemStub;
struct Video;

struct Logic {
	typedef void (Logic::*OpcodeStub)();

	enum ScriptVars {
		VAR_RANDOM_SEED          = 0x3C,
		
		VAR_LAST_KEYCHAR         = 0xDA,

		VAR_HERO_POS_UP_DOWN     = 0xE5,

		VAR_SCROLL_Y             = 0xF9,
		VAR_HERO_ACTION          = 0xFA,
		VAR_HERO_POS_JUMP_DOWN   = 0xFB,
		VAR_HERO_POS_LEFT_RIGHT  = 0xFC,
		VAR_HERO_POS_MASK        = 0xFD,
		VAR_HERO_ACTION_POS_MASK = 0xFE,
		VAR_PAUSE_SLICES         = 0xFF
	};
	
	static const OpcodeStub _opTable[];

	Resource *_res;
	Video *_vid;
	SystemStub *_stub;
	bool _copyProtection;

	int16 _scriptVar_0xBF = { 0 };
	int16 _scriptVars[0x100] = { 0 };
	uint16 _scriptStackCalls[0x40] = { 0 };
	uint16 _scriptPos[2][0x40] = { 0 };
	uint8 _scriptPaused[2][0x40] = { 0 };
	Ptr _scriptPtr;
	uint8 _stackPtr = 0;
	bool _scriptHalted = false;
	bool _fastMode = false;
	int _lastOpcodeOffset = 0;
	uint32 _lastTimestamp = 0;

	Logic(Resource *res, Video *vid, SystemStub *stub);
	void init();
	
	void op_movConst();
	void op_mov();
	void op_add();
	void op_addConst();
	void op_call();
	void op_ret();
	void op_break();
	void op_jmp();
	void op_setScriptPos();
	void op_jnz();
	void op_condJmp();
	void op_setPalette();
	void op_resetScript();
	void op_selectPage();
	void op_fillPage();
	void op_copyPage();
	void op_updateDisplay();
	void op_halt();	
	void op_drawString();
	void op_sub();
	void op_and();
	void op_or();
	void op_shl();
	void op_shr();
	void op_soundUnk1();
	void op_updateMemList();
	void op_soundUnk2();

	void restartAt(uint16 ptrId);
	void setupPtrs(uint16 ptrId);
	void setupScripts();
	void runScripts();
	void executeScript();

	void inp_updatePlayer();
	void inp_handleSpecialKeys();
	
	void saveOrLoad(Serializer &ser);
};

} // namespace Awe

#endif
