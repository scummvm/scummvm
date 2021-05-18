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

#include "trecision/trecision.h"
#include "trecision/scheduler.h"

namespace Trecision {

Scheduler::Scheduler(TrecisionEngine *vm) : _vm(vm) {
	_maxMessageGame = 0;
	_maxMessageCharacter = 0;
	_maxMessageAnim = 0;

	_token = CLASS_CHAR;
	_counter = 0;
}

Scheduler::~Scheduler() {
}

void Scheduler::process() {
	bool retry = true;

	while (retry) {
		retry = false;
		switch (_token) {
		case CLASS_GAME:
			if (_counter++ <= 30) {
				_token = CLASS_ANIM;
				if (_vm->_gameQueue.getMessage(&_vm->_curMessage))
					_vm->_curMessage = &_vm->_idleMsg;
			} else {
				_counter = 0;
				_vm->_curMessage = &_vm->_idleMsg;
			}
			break;

		case CLASS_ANIM:
			_token = CLASS_CHAR;
			if (_vm->_animQueue.getMessage(&_vm->_curMessage))
				retry = true;
			break;

		case CLASS_CHAR:
			_token = CLASS_GAME;
			if (_vm->_flagPaintCharacter || _vm->_characterQueue.getMessage(&_vm->_curMessage))
				retry = true;
			break;

		default:
			break;
		}
	}
}

void Scheduler::doEvent(uint8 cls, uint8 event, uint8 priority,
						uint16 u16Param1, uint16 u16Param2,
						uint8 u8Param, uint32 u32Param) {
	MessageQueue *lq;

	if (cls <= CLASS_GAME)
		lq = &_vm->_gameQueue;
	else if (cls <= CLASS_ANIM)
		lq = &_vm->_animQueue;
	else
		lq = &_vm->_characterQueue;

	if (lq->_len >= MAXMESSAGE)
		return;

	Message *lm = lq->_event[lq->_tail++];

	lm->_class = cls;
	lm->_event = event;
	lm->_priority = priority;
	lm->_u16Param1 = u16Param1;
	lm->_u16Param2 = u16Param2;
	lm->_u8Param = u8Param;
	lm->_u32Param = u32Param;
	lm->_timestamp = _vm->_curTime;

	if (lq->_tail == MAXMESSAGE)
		lq->_tail = 0;
	lq->_len++;

	if (lq == &_vm->_gameQueue && lq->_len > _maxMessageGame)
		_maxMessageGame = lq->_len;
	else if (lq == &_vm->_animQueue && lq->_len > _maxMessageAnim)
		_maxMessageAnim = lq->_len;
	else if (lq == &_vm->_characterQueue && lq->_len > _maxMessageCharacter)
		_maxMessageCharacter = lq->_len;

	lq->orderEvents();
}



uint8 MessageQueue::predEvent(uint8 i) {
	return i == 0 ? MAXMESSAGE - 1 : i - 1;
}

bool MessageQueue::getMessage(Message **msg) {
	if (!_len)
		return true;

	*msg = _event[_head++];
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

		if (_event[pos]->_event == ME_CHARACTERACTION || _event[pos]->_event == ME_CHARACTERGOTO || _event[pos]->_event == ME_CHARACTERGOTOACTION || _event[pos]->_event == ME_CHARACTERGOTOEXAMINE || _event[pos]->_event == ME_CHARACTERCONTINUEACTION)
			return false;
	}

	//	true when:
	//	1) the queue is empty
	//	2) or there's a particular action in progress

	return true;
}

} // End of namespace Trecision
