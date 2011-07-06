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

#ifndef NEVERHOOD_ENTITY_H
#define NEVERHOOD_ENTITY_H

#include "neverhood/neverhood.h"
#include "neverhood/graphics.h"

namespace Neverhood {

struct MessageParam {
	union {
		uint32 _integer;
		NPoint _point;
		// TODO: Other types...
	};
	MessageParam(uint32 value) { _integer = value; }
	MessageParam(NPoint value) { _point = value; }
	// TODO: Constructors for the param types...
};

#define SetUpdateHandler(handler) _updateHandlerCb = static_cast <void (Entity::*)(void)> (handler)
#define SetMessageHandler(handler) _messageHandlerCb = static_cast <uint32 (Entity::*)(int messageNum, const MessageParam &param, Entity *sender)> (handler)

class Entity {
public:
	Common::String _name; // Entity name for debugging purposes
	Entity(NeverhoodEngine *vm, int priority)
		: _vm(vm), _updateHandlerCb(NULL), _messageHandlerCb(NULL), _priority(priority), _name("Entity") {
	}
	virtual ~Entity() {
	}
	virtual void draw() {
	}
	void handleUpdate() {
		//debug("Entity(%s).handleUpdate", _name.c_str());
		if (_updateHandlerCb)
			(this->*_updateHandlerCb)();
	}
	bool hasMessageHandler() const { return _messageHandlerCb != NULL; } 
	uint32 sendMessage(int messageNum, const MessageParam &param, Entity *sender) {
		return _messageHandlerCb ? (this->*_messageHandlerCb)(messageNum, param, sender) : 0;
	}
	// Overloaded for various message parameter types
	uint32 sendMessage(int messageNum, uint32 param, Entity *sender) {
		return sendMessage(messageNum, MessageParam(param), sender);
	}
	uint32 sendMessage(int messageNum, NPoint param, Entity *sender) {
		return sendMessage(messageNum, MessageParam(param), sender);
	}
	int getPriority() const { return _priority; }
protected:
	void (Entity::*_updateHandlerCb)();
	uint32 (Entity::*_messageHandlerCb)(int messageNum, const MessageParam &param, Entity *sender);
	NeverhoodEngine *_vm;
	int _priority;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_ENTITY_H */
