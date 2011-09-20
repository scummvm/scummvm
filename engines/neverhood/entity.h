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

#include "common/str.h"
#include "neverhood/neverhood.h"
#include "neverhood/gamevars.h"
#include "neverhood/graphics.h"

namespace Neverhood {

class Entity;

enum MessageParamType {
	mptInteger,
	mptPoint,
	mptEntity
};

struct MessageParam {
public:
	MessageParam(uint32 value) : _type(mptInteger), _integer(value) {}
	MessageParam(NPoint value) : _type(mptPoint), _point(value) {}
	MessageParam(Entity *entity) : _type(mptEntity), _entity(entity) {}
	uint32 asInteger() const { 
		assert(_type == mptInteger); 
		return _integer; 
	}
	NPoint asPoint() const { 
		assert(_type == mptInteger || _type == mptPoint);
		if (_type == mptInteger) {
			NPoint pt;
			pt.x = _integer & 0xFFFF;
			pt.y = (_integer >> 16) & 0xFFFF; 
			return pt;
		} 
		return _point; 
	}
	Entity *asEntity() const {
		assert(_type == mptEntity); 
		return _entity; 
	}
protected:
	union {
		uint32 _integer;
		NPoint _point;
		Entity *_entity;
		// TODO: Other types...
	};
	MessageParamType _type;
	// TODO: Constructors for the param types...
};

// TODO: Disable heavy debug stuff in release mode

#define SetUpdateHandler(handler) _updateHandlerCb = static_cast <void (Entity::*)(void)> (handler); debug(2, "SetUpdateHandler(" #handler ")"); _updateHandlerCbName = #handler
#define SetMessageHandler(handler) _messageHandlerCb = static_cast <uint32 (Entity::*)(int messageNum, const MessageParam &param, Entity *sender)> (handler); debug(2, "SetMessageHandler(" #handler ")"); _messageHandlerCbName = #handler

class Entity {
public:
	Common::String _name; // Entity name for debugging purposes
	Common::String _updateHandlerCbName;
	Common::String _messageHandlerCbName;
	Entity(NeverhoodEngine *vm, int priority)
		: _vm(vm), _updateHandlerCb(NULL), _messageHandlerCb(NULL), _priority(priority), _name("Entity") {
	}
	virtual ~Entity() {
	}
	virtual void draw() {
	}
	void handleUpdate() {
		//debug("Entity(%s).handleUpdate", _name.c_str());
		debug(2, "handleUpdate() -> [%s]", _updateHandlerCbName.c_str());
		if (_updateHandlerCb)
			(this->*_updateHandlerCb)();
	}
	bool hasMessageHandler() const { return _messageHandlerCb != NULL; } 
	uint32 receiveMessage(int messageNum, const MessageParam &param, Entity *sender) {
		debug(2, "receiveMessage(%04X) -> [%s]", messageNum, _messageHandlerCbName.c_str());
		return _messageHandlerCb ? (this->*_messageHandlerCb)(messageNum, param, sender) : 0;
	}
	// NOTE: These were overloaded before for the various message parameter types
	// it caused some problems so each type gets its own sendMessage variant now
	uint32 sendMessage(Entity *receiver, int messageNum, const MessageParam &param) {
		return receiver ? receiver->receiveMessage(messageNum, param, this) : 0;
	}
	uint32 sendMessage(Entity *receiver, int messageNum, uint32 param) {
		return sendMessage(receiver, messageNum, MessageParam(param));
	}
	uint32 sendPointMessage(Entity *receiver, int messageNum, const NPoint &param) {
		return sendMessage(receiver, messageNum, MessageParam(param));
	}
	uint32 sendEntityMessage(Entity *receiver, int messageNum, Entity *param) {
		return sendMessage(receiver, messageNum, MessageParam((Entity*)param));
	}
	int getPriority() const { return _priority; }
	// Shortcuts for game variable access
	uint32 getGlobalVar(uint32 nameHash) {
		return _vm->_gameVars->getGlobalVar(nameHash);
	}
	void setGlobalVar(uint32 nameHash, uint32 value) {
		_vm->_gameVars->setGlobalVar(nameHash, value);
	}
	uint32 getSubVar(uint32 nameHash, uint32 subNameHash) {
		return _vm->_gameVars->getSubVar(nameHash, subNameHash);
	}
	void setSubVar(uint32 nameHash, uint32 subNameHash, uint32 value) {
		_vm->_gameVars->setSubVar(nameHash, subNameHash, value);
	}
	void incGlobalVar(uint32 nameHash, int incrValue) {
		setGlobalVar(nameHash, getGlobalVar(nameHash) + incrValue);
	}
	void incSubVar(uint32 nameHash, uint32 subNameHash, int incrValue) {
		setSubVar(nameHash, subNameHash, getSubVar(nameHash, subNameHash) + incrValue);
	}
protected:
	void (Entity::*_updateHandlerCb)();
	uint32 (Entity::*_messageHandlerCb)(int messageNum, const MessageParam &param, Entity *sender);
	NeverhoodEngine *_vm;
	int _priority;
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_ENTITY_H */
