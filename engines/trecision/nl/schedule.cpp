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

#include "trecision/nl/extern.h"
#include "trecision/nl/message.h"
#include "trecision/nl/proto.h"
#include "trecision/nl/struct.h"
#include "trecision/trecision.h"

namespace Trecision {

int maxmesg, maxmesh, maxmesa;

void doEvent(uint8 cls,  uint8 event,  uint8 priority,
			 uint16 u16Param1, uint16 u16Param2,
			 uint8 u8Param, uint32 u32Param) {
	MessageQueue *lq;

	if (cls <= CLASS_GAME)
		lq = &g_vm->_gameQueue;
	else if (cls <= CLASS_ANIM)
		lq = &g_vm->_animQueue;
	else
		lq = &g_vm->_characterQueue;

	if (lq->_len >= MAXMESSAGE)
		return;

	Message *lm = lq->_event[lq->_tail++];

	lm->_class  = cls;
	lm->_event  = event;
	lm->_priority = priority;
	lm->_u16Param1  = u16Param1;
	lm->_u16Param2  = u16Param2;
	lm->_u8Param  = u8Param;
	lm->_u32Param  = u32Param;
	lm->_timestamp = TheTime;

	if (lq->_tail == MAXMESSAGE)
		lq->_tail = 0;
	lq->_len++;

	if (lq == &g_vm->_gameQueue && lq->_len > maxmesg)
		maxmesg = lq->_len;
	else if (lq == &g_vm->_animQueue && lq->_len > maxmesa)
		maxmesa = lq->_len;
	else if (lq == &g_vm->_characterQueue && lq->_len > maxmesh)
		maxmesh = lq->_len;

	lq->orderEvents();
}

void Scheduler() {
	static uint8 token = CLASS_CHAR;
	static uint8 counter = 0;
	bool retry = true;

	while (retry) {
		retry = false;
		switch (token) {
		case CLASS_GAME:
			if (counter++ <= 30) {
				token = CLASS_ANIM;
				if (g_vm->_gameQueue.getMessage())
					g_vm->_curMessage = &g_vm->_idleMsg;
			} else {
				counter = 0;
				g_vm->_curMessage = &g_vm->_idleMsg;
			}
			break;

		case CLASS_ANIM:
			token = CLASS_CHAR;
			if (g_vm->_animQueue.getMessage())
				retry = true;
			break;

		case CLASS_CHAR:
			token = CLASS_GAME;
			if (g_vm->_flagPaintCharacter || g_vm->_characterQueue.getMessage())
				retry = true;
			break;

		default:
			break;
		}
	}
}

void ProcessTheMessage() {
	switch (g_vm->_curMessage->_class) {
	case MC_CHARACTER:
		doCharacter();
		break;

	case MC_IDLE:
		doIdle();
		break;

	case MC_MOUSE:
		doMouse();
		break;

	case MC_SYSTEM:
		doSystem();
		break;

	case MC_INVENTORY:
		g_vm->doInventory();
		break;

	case MC_ACTION:
		doAction();
		break;

	case MC_STRING:
		doString();
		break;

	case MC_DOING:
		doDoing();
		break;

	case MC_DIALOG:
		g_vm->_dialogMgr->doDialog();
		break;

	case MC_SCRIPT:
		doScript();
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
