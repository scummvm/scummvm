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

#include "common/scummsys.h"

#include "trecision/dialog.h"
#include "trecision/text.h"
#include "trecision/trecision.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"

namespace Trecision {

void ProcessTheMessage() {
	switch (g_vm->_curMessage->_class) {
	case MC_CHARACTER:
		g_vm->doCharacter();
		break;

	case MC_IDLE:
		g_vm->doIdle();
		break;

	case MC_MOUSE:
		g_vm->doMouse();
		break;

	case MC_SYSTEM:
		g_vm->doSystem();
		break;

	case MC_INVENTORY:
		g_vm->doInventory();
		break;

	case MC_ACTION:
		g_vm->doAction();
		break;

	case MC_STRING:
		g_vm->_textMgr->doString();
		break;

	case MC_DOING:
		g_vm->doDoing();
		break;

	case MC_DIALOG:
		g_vm->_dialogMgr->doDialog();
		break;

	case MC_SCRIPT:
		g_vm->doScript();
		break;

	default:
		break;
	}
}

inline uint8 MessageQueue::predEvent(uint8 i) {
	return i == 0 ? MAXMESSAGE - 1 : i - 1;
};

bool MessageQueue::getMessage() {
	if (!_len)
		return true;

	g_vm->_curMessage = _event[_head++];
	if (_head == MAXMESSAGE)
		_head = 0;
	_len--;

	return false;
}

void MessageQueue::initQueue() {
	_head = 0;
	_tail = 0;
	_len = 0;
}

void MessageQueue::orderEvents() {
	for (uint8 pos = predEvent(_tail); pos != _head; pos = predEvent(pos)) {
		if (_event[pos]->_priority > _event[predEvent(pos)]->_priority) {
			if (_event[pos]->_priority < MP_HIGH)
				_event[pos]->_priority++;
			SWAP(_event[pos], _event[predEvent(pos)]);
		}
	}
}

bool MessageQueue::testEmptyQueue(uint8 cls) {
	for (uint8 pos = _head; pos != _tail; pos = (pos + 1) % MAXMESSAGE) {
		if (_event[pos]->_class != cls)
			return false;
	}

	return true;
}

bool MessageQueue::testEmptyCharacterQueue4Script() {
	for (uint8 pos = _head; pos != _tail; pos = (pos + 1) % MAXMESSAGE) {
		if (_event[pos]->_class != MC_CHARACTER)
			continue;

		if (_event[pos]->_event == ME_CHARACTERACTION || _event[pos]->_event == ME_CHARACTERGOTO
			|| _event[pos]->_event == ME_CHARACTERGOTOACTION || _event[pos]->_event == ME_CHARACTERGOTOEXAMINE
			|| _event[pos]->_event == ME_CHARACTERCONTINUEACTION)
		return false;
	}

//	true when:
//	1) the queue is empty
//	2) or there's a particular action in progress

	return true;
}

} // End of namespace Trecision
