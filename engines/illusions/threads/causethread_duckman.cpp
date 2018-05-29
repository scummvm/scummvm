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

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/threads/causethread_duckman.h"
#include "illusions/actor.h"
#include "illusions/input.h"

namespace Illusions {

// CauseThread_Duckman

CauseThread_Duckman::CauseThread_Duckman(IllusionsEngine_Duckman *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	uint32 triggerThreadId)
	: Thread(vm, threadId, callingThreadId, notifyFlags), _vm(vm), _triggerThreadId(triggerThreadId), _flag(false) {
	_type = kTTCauseThread;
	_sceneId = _vm->getCurrentScene();
}

int CauseThread_Duckman::onUpdate() {
	if (_flag) {
		if (_vm->getCurrentScene() == _sceneId) {
			Control *cursorCursor = _vm->getObjectControl(Illusions::CURSOR_OBJECT_ID);
			cursorCursor->appearActor();
			_vm->_input->discardEvent(kEventLeftClick);
		}
		return kTSTerminate;
	} else {
		_sceneId = _vm->getCurrentScene();
		Control *cursorCursor = _vm->getObjectControl(Illusions::CURSOR_OBJECT_ID);
		cursorCursor->disappearActor();
		_vm->_input->discardEvent(kEventLeftClick);
		_vm->startScriptThread(_triggerThreadId, _threadId);
		_flag = true;
		return kTSSuspend;
	}
}

} // End of namespace Illusions
