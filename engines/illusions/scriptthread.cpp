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
	while (!_terminated && opCall._result == kTSRun) {
		opCall._op = _scriptCodeIp[0];
		opCall._opSize = _scriptCodeIp[1] >> 1;
		opCall._threadId = _scriptCodeIp[1] & 1 ? _threadId : 0;
		opCall._code = _scriptCodeIp + 2;
		opCall._deltaOfs = opCall._opSize;
		execOpcode(opCall);
		_scriptCodeIp += opCall._deltaOfs;
	}
	if (_terminated)
		opCall._result = kTSTerminate;
	return opCall._result;
}

void ScriptThread::onSuspend() {
	// TODO
	debug("ScriptThread::onSuspend()");
}

void ScriptThread::onNotify() {
	// TODO
	debug("ScriptThread::onNotify()");
}

void ScriptThread::onPause() {
	// TODO
	debug("ScriptThread::onPause()");
}

void ScriptThread::onResume() {
	// TODO
	debug("ScriptThread::onResume()");
}

void ScriptThread::onTerminated() {
	// TODO
	debug("ScriptThread::onTerminated()");
}

void ScriptThread::execOpcode(OpCall &opCall) {
	// TODO Clean this up
	_vm->_scriptMan->_scriptOpcodes->execOpcode(this, opCall);
}

} // End of namespace Illusions
