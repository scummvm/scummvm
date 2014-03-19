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

#ifndef ILLUSIONS_SCRIPTOPCODES_H
#define ILLUSIONS_SCRIPTOPCODES_H

#include "common/func.h"

namespace Illusions {

class IllusionsEngine;
class ScriptThread;

struct OpCall {
	byte _op;
	byte _opSize;
	uint32 _threadId;
	int16 _deltaOfs;
	byte *_code;
	int _result;
	void skip(uint size);
	byte readByte();
	int16 readSint16();
	uint32 readUint32();
};

typedef Common::Functor2<ScriptThread*, OpCall&, void> ScriptOpcode;

class ScriptOpcodes {
public:
	ScriptOpcodes(IllusionsEngine *vm);
	~ScriptOpcodes();
	void execOpcode(ScriptThread *scriptThread, OpCall &opCall);
protected:
	IllusionsEngine *_vm;
	ScriptOpcode *_opcodes[256];
	void initOpcodes();
	void freeOpcodes();

	// Opcodes
	void opSuspend(ScriptThread *scriptThread, OpCall &opCall);
	void opYield(ScriptThread *scriptThread, OpCall &opCall);
	void opStartScriptThread(ScriptThread *scriptThread, OpCall &opCall);
	void opStartTimerThread(ScriptThread *scriptThread, OpCall &opCall);
	void opLoadResource(ScriptThread *scriptThread, OpCall &opCall);
	void opEnterScene(ScriptThread *scriptThread, OpCall &opCall);
	void opSetDisplay(ScriptThread *scriptThread, OpCall &opCall);	
	void opIncBlockCounter(ScriptThread *scriptThread, OpCall &opCall);
	void opPlaceActor(ScriptThread *scriptThread, OpCall &opCall);
	void opDeactivateButton(ScriptThread *scriptThread, OpCall &opCall);
	void opActivateButton(ScriptThread *scriptThread, OpCall &opCall);
	void opDebug126(ScriptThread *scriptThread, OpCall &opCall);
	void opPlayVideo(ScriptThread *scriptThread, OpCall &opCall);
	void opSetSceneIdThreadId(ScriptThread *scriptThread, OpCall &opCall);
	void opSetFontId(ScriptThread *scriptThread, OpCall &opCall);
	
};

} // End of namespace Illusions

#endif // ILLUSIONS_SCRIPTOPCODES_H
