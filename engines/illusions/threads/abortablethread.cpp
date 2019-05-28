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
#include "illusions/threads/abortablethread.h"
#include "illusions/input.h"
#include "illusions/time.h"

namespace Illusions {

// AbortableThread

AbortableThread::AbortableThread(IllusionsEngine *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	uint32 scriptThreadId, byte *scriptCodeIp)
	: Thread(vm, threadId, callingThreadId, notifyFlags), _scriptThreadId(scriptThreadId),
	_scriptCodeIp(scriptCodeIp), _status(1) {
	_type = kTTAbortableThread;
	_sceneId = _vm->getCurrentScene();
	_vm->_input->discardEvent(kEventAbort);
}

int AbortableThread::onUpdate() {
	if (_status != 1 || _pauseCtr < 0)
		return kTSTerminate;
	if (_vm->_input->pollEvent(kEventAbort)) {
		_vm->_threads->killThread(_scriptThreadId);
		++_pauseCtr;
		_vm->startTempScriptThread(_scriptCodeIp, _threadId, 0, 0, 0);
		_status = 2;
		return kTSSuspend;
	}
	return kTSYield;
}

} // End of namespace Illusions
