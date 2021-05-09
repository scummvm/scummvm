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

#include "trecision/nl/message.h"

namespace Trecision {

Scheduler::Scheduler(TrecisionEngine *vm) : _vm(vm) {
	maxmesg = 0;
	maxmesh = 0;
	maxmesa = 0;
}

Scheduler::~Scheduler() {

}

void Scheduler::process() {
	static uint8 token = CLASS_CHAR;
	static uint8 counter = 0;
	bool retry = true;

	while (retry) {
		retry = false;
		switch (token) {
		case CLASS_GAME:
			if (counter++ <= 30) {
				token = CLASS_ANIM;
				if (_vm->_gameQueue.getMessage())
					_vm->_curMessage = &_vm->_idleMsg;
			} else {
				counter = 0;
				_vm->_curMessage = &_vm->_idleMsg;
			}
			break;

		case CLASS_ANIM:
			token = CLASS_CHAR;
			if (_vm->_animQueue.getMessage())
				retry = true;
			break;

		case CLASS_CHAR:
			token = CLASS_GAME;
			if (_vm->_flagPaintCharacter || _vm->_characterQueue.getMessage())
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
		lq = &g_vm->_gameQueue;
	else if (cls <= CLASS_ANIM)
		lq = &g_vm->_animQueue;
	else
		lq = &g_vm->_characterQueue;

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
	lm->_timestamp = g_vm->_curTime;

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

} // End of namespace Trecision
