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

#include "neverhood/entity.h"
#include "neverhood/sound.h"

namespace Neverhood {

uint32 MessageParam::asInteger() const {
	assert(_type == mptInteger);
	return _integer;
}

NPoint MessageParam::asPoint() const {
	assert(_type == mptInteger || _type == mptPoint);
	if (_type == mptInteger) {
		NPoint pt;
		pt.x = _integer & 0xFFFF;
		pt.y = (_integer >> 16) & 0xFFFF;
		return pt;
	}
	return _point;
}

Entity *MessageParam::asEntity() const {
	assert(_type == mptEntity);
	return _entity;
}

Entity::Entity(NeverhoodEngine *vm, int priority)
	: _vm(vm), _updateHandlerCb(NULL), _messageHandlerCb(nullptr), _priority(priority), _soundResources(NULL) {
}

Entity::~Entity() {
	deleteSoundResources();
}

void Entity::draw() {
	// Empty
}

void Entity::handleUpdate() {
	debug(5, "handleUpdate() -> [%s]", _updateHandlerCbName.c_str());
	if (_updateHandlerCb)
		(this->*_updateHandlerCb)();
}

uint32 Entity::receiveMessage(int messageNum, const MessageParam &param, Entity *sender) {
	debug(5, "receiveMessage(%04X) -> [%s]", messageNum, _messageHandlerCbName.c_str());
	return _messageHandlerCb ? (this->*_messageHandlerCb)(messageNum, param, sender) : 0;
}

uint32 Entity::sendMessage(Entity *receiver, int messageNum, const MessageParam &param) {
	return receiver ? receiver->receiveMessage(messageNum, param, this) : 0;
}

uint32 Entity::sendMessage(Entity *receiver, int messageNum, uint32 param) {
	return sendMessage(receiver, messageNum, MessageParam(param));
}

uint32 Entity::sendPointMessage(Entity *receiver, int messageNum, const NPoint &param) {
	return sendMessage(receiver, messageNum, MessageParam(param));
}

uint32 Entity::sendEntityMessage(Entity *receiver, int messageNum, Entity *param) {
	return sendMessage(receiver, messageNum, MessageParam((Entity*)param));
}

uint32 Entity::getGlobalVar(uint32 nameHash) {
	return _vm->_gameVars->getGlobalVar(nameHash);
}

void Entity::setGlobalVar(uint32 nameHash, uint32 value) {
	_vm->_gameVars->setGlobalVar(nameHash, value);
}

uint32 Entity::getSubVar(uint32 nameHash, uint32 subNameHash) {
	return _vm->_gameVars->getSubVar(nameHash, subNameHash);
}

void Entity::setSubVar(uint32 nameHash, uint32 subNameHash, uint32 value) {
	_vm->_gameVars->setSubVar(nameHash, subNameHash, value);
}

void Entity::incGlobalVar(uint32 nameHash, int incrValue) {
	setGlobalVar(nameHash, getGlobalVar(nameHash) + incrValue);
}

void Entity::incSubVar(uint32 nameHash, uint32 subNameHash, int incrValue) {
	setSubVar(nameHash, subNameHash, getSubVar(nameHash, subNameHash) + incrValue);
}

SoundResource *Entity::getSoundResource(uint index) {
	assert(index < kMaxSoundResources);
	if (!_soundResources) {
		_soundResources = new SoundResource*[kMaxSoundResources];
		for (uint i = 0; i < kMaxSoundResources; ++i)
			_soundResources[i] = NULL;
	}
	if (!_soundResources[index])
		_soundResources[index] = new SoundResource(_vm);
	return _soundResources[index];
}

void Entity::loadSound(uint index, uint32 fileHash) {
	getSoundResource(index)->load(fileHash);
}

void Entity::playSound(uint index, uint32 fileHash) {
	if (fileHash)
		getSoundResource(index)->play(fileHash);
	else
		getSoundResource(index)->play();
}

void Entity::stopSound(uint index) {
	getSoundResource(index)->stop();
}

bool Entity::isSoundPlaying(uint index) {
	return getSoundResource(index)->isPlaying();
}

void Entity::setSoundVolume(uint index, int volume) {
	getSoundResource(index)->setVolume(volume);
}

void Entity::setSoundPan(uint index, int pan) {
	getSoundResource(index)->setPan(pan);
}

void Entity::deleteSoundResources() {
	if (_soundResources) {
		for (uint i = 0; i < kMaxSoundResources; ++i)
			delete _soundResources[i];
		delete[] _soundResources;
	}
}

} // End of namespace Neverhood
