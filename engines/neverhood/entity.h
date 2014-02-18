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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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
#include "neverhood/sound.h"

namespace Neverhood {

class Entity;
class SoundResource;

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
	uint32 asInteger() const;
	NPoint asPoint() const;
	Entity *asEntity() const;
protected:
	union {
		uint32 _integer;
		NPoint _point;
		Entity *_entity;
	};
	MessageParamType _type;
};

// TODO: Disable heavy debug stuff in release mode

#define SetUpdateHandler(handler)												\
	do {																		\
		_updateHandlerCb = static_cast <void (Entity::*)(void)> (handler);		\
		debug(5, "SetUpdateHandler(" #handler ")");								\
		_updateHandlerCbName = #handler;										\
	} while (0)

#define SetMessageHandler(handler)												\
	do {																		\
		_messageHandlerCb = static_cast <uint32 (Entity::*)(int messageNum, const MessageParam &param, Entity *sender)> (handler);	\
		debug(5, "SetMessageHandler(" #handler ")");							\
		_messageHandlerCbName = #handler;										\
	} while (0)

const uint kMaxSoundResources = 16;

class Entity {
public:
	Common::String _updateHandlerCbName;
	Common::String _messageHandlerCbName;
	Entity(NeverhoodEngine *vm, int priority);
	virtual ~Entity();
	virtual void draw();
	void handleUpdate();
	uint32 receiveMessage(int messageNum, const MessageParam &param, Entity *sender);
	// NOTE: These were overloaded before for the various message parameter types
	// it caused some problems so each type gets its own sendMessage variant now
	uint32 sendMessage(Entity *receiver, int messageNum, const MessageParam &param);
	uint32 sendMessage(Entity *receiver, int messageNum, uint32 param);
	uint32 sendPointMessage(Entity *receiver, int messageNum, const NPoint &param);
	uint32 sendEntityMessage(Entity *receiver, int messageNum, Entity *param);
	// Shortcuts for game variable access
	uint32 getGlobalVar(uint32 nameHash);
	void setGlobalVar(uint32 nameHash, uint32 value);
	uint32 getSubVar(uint32 nameHash, uint32 subNameHash);
	void setSubVar(uint32 nameHash, uint32 subNameHash, uint32 value);
	void incGlobalVar(uint32 nameHash, int incrValue);
	void incSubVar(uint32 nameHash, uint32 subNameHash, int incrValue);
	int getPriority() const { return _priority; }
	bool hasMessageHandler() const { return _messageHandlerCb != nullptr; }
protected:
	void (Entity::*_updateHandlerCb)();
	uint32 (Entity::*_messageHandlerCb)(int messageNum, const MessageParam &param, Entity *sender);
	NeverhoodEngine *_vm;
	int _priority;
	SoundResource **_soundResources;
	SoundResource *getSoundResource(uint index);
	void loadSound(uint index, uint32 fileHash);
	void playSound(uint index, uint32 fileHash = 0);
	void stopSound(uint index);
	bool isSoundPlaying(uint index);
	void setSoundVolume(uint index, int volume);
	void setSoundPan(uint index, int pan);
	void deleteSoundResources();
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_ENTITY_H */
