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

#ifndef PICTURE_SCRIPT_H
#define PICTURE_SCRIPT_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/array.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/voc.h"
#include "sound/audiocd.h"

#include "engines/engine.h"

namespace Picture {

const int kMaxScriptSlots = 50;

class ScriptInterpreter {
public:
	ScriptInterpreter(PictureEngine *vm);
	~ScriptInterpreter();

	void loadScript(uint resIndex, uint slotIndex);
	void runScript(uint slotIndex);

	byte *getSlotData(int slotIndex) const { return _slots[slotIndex].data; }

protected:

	enum VarType {
		vtByte,
		vtWord
	};

	struct ScriptRegs {
		int16 reg0;
		int16 reg1;
		int16 reg2;
		int16 reg3;
		int16 reg4;
		int16 reg5;
		int16 reg6;
		int16 sp;
		int16 reg8;
	};

	struct ScriptSlot {
		byte *data;
		int32 size;
		uint resIndex;
	};

	PictureEngine *_vm;

	byte *_stack;

	byte *_code, *_subCode;
	byte *_localData;
	bool _switchStack1, _switchStack2, _switchStack3;
	bool _scriptFlag01;

	ScriptSlot _slots[kMaxScriptSlots];
	
	ScriptRegs _regs;
	int16 _savedSp;
	
	byte readByte();
	int16 readInt16();
	
	void execOpcode(byte opcode);
	void execKernelOpcode(uint16 kernelOpcode);

	VarType getGameVarType(uint variable);
	int16 getGameVar(uint variable);
	void setGameVar(uint variable, int16 value);

	byte arg8(int16 offset);
	int16 arg16(int16 offset);
	int32 arg32(int16 offset);

	void push8(byte value);
	byte pop8();
	void push16(int16 value);
	int16 pop16();
	void push32(int32 value);
	int32 pop32();

	void localWrite8(int16 offset, byte value);
	byte localRead8(int16 offset);
	void localWrite16(int16 offset, int16 value);
	int16 localRead16(int16 offset);
	void localWrite32(int16 offset, int32 value);
	int32 localRead32(int16 offset);
	byte *localPtr(int16 offset);

};


} // End of namespace Picture

#endif /* PICTURE_H */
