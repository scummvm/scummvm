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

#include "illusions/illusions.h"
#include "illusions/scriptthread.h"
#include "illusions/scriptman.h"
#include "illusions/scriptopcodes.h"

namespace Illusions {

// OpCall

void OpCall::skip(uint size) {
	_scriptCode += size;
}

byte OpCall::readByte() {
	return *_scriptCode++;
}

int16 OpCall::readSint16() {
	int16 value = READ_LE_UINT16(_scriptCode);
	_scriptCode += 2;
	return value;
}

uint32 OpCall::readUint32() {
	uint32 value = READ_LE_UINT32(_scriptCode);
	_scriptCode += 4;
	return value;
}

// ScriptThread

ScriptThread::ScriptThread(IllusionsEngine *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	byte *scriptCodeIp, uint32 value8, uint32 valueC, uint32 value10)
	: Thread(vm, threadId, callingThreadId, notifyFlags), _scriptCodeIp(scriptCodeIp), _value8(value8),
	_valueC(valueC), _value10(value10), _sequenceStalled(0) {
	_type = kTTScriptThread;
}

int ScriptThread::onUpdate() {
	OpCall opCall;
	opCall._result = kTSRun;
	while (!_terminated && opCall._result == 4) {
		opCall._op = _scriptCodeIp[0];
		opCall._opSize = _scriptCodeIp[1] >> 1;
		opCall._threadId = _scriptCodeIp[1] & 1 ? _threadId : 0;
		opCall._scriptCode = _scriptCodeIp + 2;
		opCall._deltaOfs = 0;
		execOpcode(opCall);
		_scriptCodeIp += opCall._opSize + opCall._deltaOfs;
	}
	if (_terminated)
		opCall._result = kTSTerminate;
	return opCall._result;
}

void ScriptThread::onSuspend() {
	// TODO
}

void ScriptThread::onNotify() {
	// TODO
}

void ScriptThread::onPause() {
	// TODO
}

void ScriptThread::onResume() {
	// TODO
}

void ScriptThread::onTerminated() {
	// TODO
}

void ScriptThread::execOpcode(OpCall &opCall) {
	// TODO Clean this up
	_vm->_scriptMan->_scriptOpcodes->execOpcode(this, opCall);
}

} // End of namespace Illusions
