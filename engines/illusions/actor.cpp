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

#include "illusions/illusions.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/input.h"
#include "illusions/screen.h"

namespace Illusions {

// DefaultSequences

uint32 DefaultSequences::use(uint32 sequenceId) {
	ItemsIterator it = Common::find_if(_items.begin(), _items.end(), DefaultSequenceEqual(sequenceId));
	return it != _items.end() ? (*it)._newSequenceId : sequenceId;
}

void DefaultSequences::set(uint32 sequenceId, uint32 newSequenceId) {
	ItemsIterator it = Common::find_if(_items.begin(), _items.end(), DefaultSequenceEqual(sequenceId));
	if (it == _items.end())
		_items.push_back(DefaultSequence(sequenceId, newSequenceId));
	else if (sequenceId == newSequenceId)
		_items.remove_at(it - _items.begin());
	else
		(*it)._newSequenceId = newSequenceId;
}

// Actor

Actor::Actor(IllusionsEngine *vm)
	: _vm(vm), _pauseCtr(0) {
	
}

void Actor::pause() {
	++_pauseCtr;
}

void Actor::unpause() {
	--_pauseCtr;
}

void Actor::createSurface(SurfInfo &surfInfo) {
	_surface = _vm->_screen->allocSurface(surfInfo);
	if (_frameIndex) {
		if (_surfaceTextFlag) {
			/* TODO
			Font *font = _vm->findFont(_fontId);
			_surface->fillRect(Common::Rect(surfInfo._dimensions._width, surfInfo._dimensions._height), 0);
			gfx_sub_40CA70(_surface, font, _field18C, _surfInfo._dimensions, _field198);
			*/
			_flags |= 0x4000;
		}
		else {
			_flags |= 0x2000;
			_flags |= 0x4000;
		}
	}
}

void Actor::destroySurface() {
	if (_surface) {
		_surface->free();
		delete _surface;
		_surface = 0;
	}
}

// Control

Control::Control(IllusionsEngine *vm)
	: _vm(vm) {
}

Control::~Control() {
}

void Control::pause() {

	// TODO scrmgrSetObjectArtThread(control->objectId, 0);

	/* TODO
	if (_objectId == 0x40004)
		_cursor.control = 0;
	*/

	if (_actor && !(_actor->_flags & 0x0200))
		_actor->destroySurface();

}

void Control::unpause() {

	// TODO scrmgrSetObjectArtThread(control->objectId, control);

	/* TODO
	if (_objectId == 0x40004)
		_cursor.control = this;
	*/
  
	if (_actor && !(_actor->_flags & 0x0200)) {
		SurfInfo surfInfo;
		ActorType *actorType = _vm->findActorType(_actorTypeId);
		if (actorType)
			surfInfo = actorType->_surfInfo;
		else
			surfInfo = _actor->_surfInfo;
		_actor->createSurface(surfInfo);
	}

}

void Control::appearActor() {
	if (_objectId == 0x40004) {
		// TODO ++cursor._visibleCtr;
		// TODO if (cursor._visibleCtr > 0) 
		{
			_flags |= 1;
			_actor->_flags |= 1;
			if (_actor->_frameIndex) {
				_actor->_flags |= 0x2000;
				_actor->_flags |= 0x4000;
			}
			_vm->_input->discardButtons(0xFFFF);
		}
	} else {
		if (_actor->_frameIndex || _actorTypeId == 0x50004)
			_actor->_flags |= 1;
		else
			_actor->_flags |= 0x1000;
		for (uint i = 0; i < kSubObjectsCount; ++i)
			if (_actor->_subobjects[i]) {
				Control *subControl = _vm->findControl(_actor->_subobjects[i]);
				subControl->appearActor();
			}
	}
}

void Control::disappearActor() {
	if (_objectId == 0x40004) {
		// TODO --cursor.visibleCtr;
		// TODO if (cursor.visibleCtr <= 0) 
		{
			_flags &= 0xFFFEu;
			_actor->_flags &= 0xFFFE;
		}
	} else {
		_actor->_flags |= ~1;
		_actor->_flags |= ~0x1000;
		for (uint i = 0; i < kSubObjectsCount; ++i)
			if (_actor->_subobjects[i]) {
				Control *subControl = _vm->findControl(_actor->_subobjects[i]);
				subControl->disappearActor();
			}
	}
}

bool Control::isActorVisible() {
	return (_actor->_flags & 1) != 0;
}

void Control::activateObject() {
	_flags |= 1;
	for (uint i = 0; i < kSubObjectsCount; ++i)
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->findControl(_actor->_subobjects[i]);
			subControl->activateObject();
		}
}

void Control::deactivateObject() {
	_flags |= ~1;
	for (uint i = 0; i < kSubObjectsCount; ++i)
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->findControl(_actor->_subobjects[i]);
			subControl->deactivateObject();
		}
}

void Control::setActorPosition(Common::Point position) {
	_actor->_position = position;
}

Common::Point Control::getActorPosition() {
	if (_actor)
		return _actor->_position;
	return _position;
}

void Control::setActorScale(int scale) {
	_actor->_scale = scale;
	for (uint i = 0; i < kSubObjectsCount; ++i)
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->findControl(_actor->_subobjects[i]);
			subControl->activateObject();
		}
}

void Control::faceActor(uint facing) {
	_actor->_facing = facing;
	for (uint i = 0; i < kSubObjectsCount; ++i)
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->findControl(_actor->_subobjects[i]);
			subControl->faceActor(facing);
		}
}

void Control::linkToObject(uint32 parentObjectId, uint32 linkedObjectValue) {
	_actor->_parentObjectId = parentObjectId;
	_actor->_linkIndex = linkedObjectValue;
}

void Control::unlinkObject() {
	_actor->_parentObjectId = 0;
	_actor->_linkIndex = 0;
}

void Control::clearNotifyThreadId1() {
	_actor->_notifyThreadId1 = 0;
}

void Control::clearNotifyThreadId2() {
	for (uint i = 0; i < kSubObjectsCount; ++i)
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->findControl(_actor->_subobjects[i]);
			subControl->_actor->_flags &= ~0x80;
			subControl->_actor->_field30 = 0;
			subControl->_actor->_notifyThreadId2 = 0;
		}
	_actor->_flags &= ~0x80;
	_actor->_field30 = 0;
	_actor->_notifyThreadId2 = 0;
}

void Control::setPriority(int16 priority) {
	_priority = priority;
}

int Control::getPriority() {
	uint32 objectId;
	int16 positionY, priority, priority1;
	if (_actor) {
		if (_actor->_parentObjectId && (_actor->_flags & 0x40)) {
			uint32 objectId2 = getSubActorParent();
			Control *control2 = _vm->findControl(objectId2);
			objectId = control2->_objectId;
			priority = control2->_priority;
			positionY = control2->_actor->_position.y;
			priority1 = _priority;
		} else {
			objectId = _objectId;
			positionY = _actor->_position.y;
			priority = _priority;
			priority1 = 50;
		}
	} else {
		positionY = _position.y;
		objectId = _objectId;
		priority = _priority;
		priority1 = 1;
	}

	priority -= 1;
	int p = 50 * priority1 / 100;
	if (p)
		--p;

	positionY = CLIP<int16>(positionY, -5000, 5000);

	return p + 50 * ((objectId & 0x3F) + ((10000 * priority + positionY + 5000) << 6));
}

uint32 Control::getSubActorParent() {
	uint32 parentObjectId = _objectId;
	while (1) {
		Actor *actor = _vm->findControl(parentObjectId)->_actor;
		if (actor->_parentObjectId && (actor->_flags & 0x40))
			parentObjectId = actor->_parentObjectId;
		else
			break;
	}
	return parentObjectId;
}

void Control::getCollisionRectAccurate(Common::Rect &collisionRect) {

	if (_actor && _actor->_frameIndex) {
		collisionRect = Common::Rect(-_position.x, -_position.y,
			-_position.x + _actor->_surfInfo._dimensions._width - 1,
			-_position.y + _actor->_surfInfo._dimensions._height - 1);
	} else {
		collisionRect = Common::Rect(_unkPt.x, _unkPt.y, _pt.x, _pt.y);
	}

	if (_actor) {
		if (_actor->_scale != 100 ) {
			// scaledValue = value * scale div 100
			collisionRect.left = collisionRect.left * _actor->_scale / 100;
			collisionRect.top = collisionRect.top * _actor->_scale / 100;
			collisionRect.right = collisionRect.right * _actor->_scale / 100;
			collisionRect.bottom = collisionRect.bottom * _actor->_scale / 100;
		}
		collisionRect.translate(_actor->_position.x, _actor->_position.y);
	}

	if (_flags & 8) {
		Common::Point screenOffs = _vm->_camera->getScreenOffset();
		collisionRect.translate(screenOffs.x, screenOffs.y);
	}

}

void Control::setActorUsePan(int usePan) {
	if (usePan == 1)
		_flags &= ~8;
	else
		_flags |= 8;
}

void Control::setActorFrameIndex(int16 frameIndex) {
	if (frameIndex) {
		_actor->_frameIndex = frameIndex;
		const Frame &frame = _actor->_frames[frameIndex - 1];
		_actor->_surfInfo = frame._surfInfo;
		// TODO memcpy(&control->unkPt, (const void *)frame->config, 0x4Cu);
		_actor->_flags |= 0x2000;
		_actor->_flags |= 0x4000;
		_actor->_newFrameIndex = 0;
	}
}

// Controls


} // End of namespace Illusions
