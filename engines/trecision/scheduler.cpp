/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "trecision/trecision.h"
#include "trecision/scheduler.h"

namespace Trecision {

Scheduler::Scheduler(TrecisionEngine *vm) : _vm(vm) {
	_token = CLASS_CHAR;
	_counter = 0;

	Message msg = { MC_IDLE, 0, MP_DEFAULT, 0, 0, 0, 0 };
	_idleMsg = _msg = msg;
}

Scheduler::~Scheduler() {
}

void Scheduler::process() {
	bool retry = true;

	while (retry) {
		retry = false;
		switch (_token) {
		case CLASS_GAME:
			if (_counter <= 30) {
				++_counter;
				_token = CLASS_CHAR;
				if (!_gameQueue.empty()) {
					_msg = _gameQueue.front();
					_vm->_curMessage = &_msg;
					_gameQueue.pop_front();
				} else {
					_vm->_curMessage = &_idleMsg;
				}
			} else {
				_counter = 0;
				_vm->_curMessage = &_idleMsg;
			}
			break;

		case CLASS_CHAR:
			_token = CLASS_GAME;
			if (_vm->_flagPaintCharacter || _characterQueue.empty()) {
				retry = true;
			} else {
				_msg = _characterQueue.front();
				_vm->_curMessage = &_msg;
				_characterQueue.pop_front();
			}
			break;

		default:
			break;
		}
	}
}

struct MessageComparator {
	bool operator()(const Message &x, const Message &y) const {
		return x._priority < y._priority;
	}
};

void Scheduler::doEvent(uint8 cls, uint8 event, uint8 priority,
						uint16 u16Param1, uint16 u16Param2,
						uint8 u8Param, uint32 u32Param) {
	Message m;

	m._class = cls;
	m._event = event;
	m._priority = priority;
	m._u16Param1 = u16Param1;
	m._u16Param2 = u16Param2;
	m._u8Param = u8Param;
	m._u32Param = u32Param;

	if (cls <= CLASS_GAME) {
		_gameQueue.push_back(m);
		Common::sort(_gameQueue.begin(), _gameQueue.end(), MessageComparator());
	} else {
		_characterQueue.push_back(m);
		Common::sort(_characterQueue.begin(), _characterQueue.end(), MessageComparator());
	}
}

void Scheduler::leftClick(uint16 x, uint16 y) {
	doEvent(MC_MOUSE, ME_MLEFT, MP_DEFAULT, x, y, 0, 0);
}

void Scheduler::rightClick(uint16 x, uint16 y) {
	doEvent(MC_MOUSE, ME_MRIGHT, MP_DEFAULT, x, y, 0, 0);
}

void Scheduler::mouseExamine(uint16 object) {
	doEvent(MC_ACTION, ME_MOUSEEXAMINE, MP_DEFAULT, 0, 0, 0, object);
}

void Scheduler::mouseOperate(uint16 object) {
	doEvent(MC_ACTION, ME_MOUSEOPERATE, MP_DEFAULT, 0, 0, 0, object);
}

void Scheduler::init() {
	resetQueues();

	_vm->_curMessage = &_idleMsg;
}

void Scheduler::resetQueues() {
	_gameQueue.clear();
	_characterQueue.clear();
}

void Scheduler::initCharacterQueue() {
	_characterQueue.clear();
}

bool Scheduler::testEmptyQueues() {
	bool onlyDialogEventsInGameQueue = true;
	bool noActionInProgress = true;

	for (Common::List<Message>::iterator it = _gameQueue.begin(); it != _gameQueue.end(); ++it) {
		if (it->_class != MC_DIALOG) {
			onlyDialogEventsInGameQueue = false;
			break;
		}
	}

	for (Common::List<Message>::iterator it = _characterQueue.begin(); it != _characterQueue.end(); ++it) {
		if (it->_class == MC_CHARACTER) {
			if (it->_event == ME_CHARACTERACTION ||
				it->_event == ME_CHARACTERGOTO ||
				it->_event == ME_CHARACTERGOTOACTION ||
				it->_event == ME_CHARACTERGOTOEXAMINE ||
				it->_event == ME_CHARACTERCONTINUEACTION) {
				noActionInProgress = false;
				break;
			}
		}
	}

	return noActionInProgress && onlyDialogEventsInGameQueue;
}

} // End of namespace Trecision
