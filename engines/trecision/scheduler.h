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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TRECISION_SCHEDULER_H
#define TRECISION_SCHEDULER_H

#include "common/scummsys.h"
#include "common/list.h"

#define MAXMESSAGE 128

namespace Trecision {

class TrecisionEngine;

struct Message {
	uint8 _class;    // message class
	uint8 _event;    // message name
	uint8 _priority; // message priority

	uint8 _u8Param;
	uint16 _u16Param1; // byte parameter 1
	uint16 _u16Param2; // byte parameter 2
	uint32 _u32Param;  // int parameter

	void set(Message *src) {
		_class = src->_class;
		_event = src->_event;
		_priority = src->_priority;
		_u8Param = src->_u8Param;
		_u16Param1 = src->_u16Param1;
		_u16Param2 = src->_u16Param2;
		_u32Param = src->_u32Param;
	}
};

class Scheduler {
	TrecisionEngine *_vm;
	uint8 _token;
	uint8 _counter;

	// Message system
	Message _idleMsg;
	Message _msg;
	Common::List<Message> _gameQueue;
	Common::List<Message> _characterQueue;

public:
	Scheduler(TrecisionEngine *vm);
	~Scheduler();

	void process();
	void doEvent(uint8 cls, uint8 event, uint8 priority, uint16 u16Param1, uint16 u16Param2, uint8 u8Param, uint32 u32Param);
	void leftClick(uint16 x, uint16 y);
	void rightClick(uint16 x, uint16 y);
	void mouseExamine(uint16 object);
	void mouseOperate(uint16 object);

	void init();
	void resetQueues();
	void initCharacterQueue();
	bool testEmptyQueues();
};

} // End of namespace Trecision
#endif

