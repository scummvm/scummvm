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
 */

#ifndef DRAGONS_SPECIALOPCODES_H
#define DRAGONS_SPECIALOPCODES_H

#include "common/func.h"
#include "common/str.h"


namespace Dragons {

#define DRAGONS_NUM_SPECIAL_OPCODES 0x8c

class DragonsEngine;

typedef Common::Functor0<void> SpecialOpcode;

class SpecialOpcodes {
public:
	SpecialOpcodes(DragonsEngine *vm);
	~SpecialOpcodes();
	void run(int16 opcode);
protected:
	DragonsEngine *_vm;
	SpecialOpcode *_opcodes[DRAGONS_NUM_SPECIAL_OPCODES];
	Common::String _opcodeNames[DRAGONS_NUM_SPECIAL_OPCODES];

	void initOpcodes();
	void freeOpcodes();

	// Opcodes
	void spcClearEngineFlag10(); // 3
	void spcSetEngineFlag10();   // 4

	void spcUnk9();
	void spcUnkA();

	void spcUnkC();

	void spcClearEngineFlag8(); // 0x14
	void spcSetEngineFlag8(); // 0x15

	void spcLoadScene1(); // 0x49
	void spcUnk4e();
	void spcUnk4f();

	void spcSetEngineFlag0x4000000(); // 0x54
	void spcSetCursorSequenceIdToZero(); // 0x55
	void spcUnk5e();
	void spcUnk5f();

	void spcSetCameraXToZero(); //0x7b
private:
	void panCamera(int16 mode);
};

} // End of namespace Dragons

#endif // DRAGONS_SPECIALOPCODES_H
