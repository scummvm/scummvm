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
#include "illusions/cursor.h"
#include "illusions/dictionary.h"
#include "illusions/fixedpoint.h"
#include "illusions/input.h"
#include "illusions/screen.h"
#include "illusions/scriptopcodes.h"
#include "illusions/sequenceopcodes.h"
#include "illusions/thread.h"
#include "illusions/threads/talkthread.h"

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
	_pauseCtr = 0;
	_spriteFlags = 0;
	_drawFlags = 0;
	_flags = 0;
	_scale = 100;
	_frameIndex = 0;
	_newFrameIndex = 0;
	_surfInfo._pixelSize = 0;
	_surfInfo._dimensions._width = 0;
	_surfInfo._dimensions._height = 0;
	_surface = 0;
	_frames = 0;
	_scaleLayer = 0;
	_priorityLayer = 0;
	_regionLayer = 0;
	_pathWalkPoints = 0;
	_pathWalkRects = 0;
	_position.x = 0;
	_position.y = 0;
	_position2.x = 0;
	_position2.y = 0;
	_facing = 64;
	_regionIndex = 0;
	_fontId = 0;
	_actorIndex = 0;
	_parentObjectId = 0;
	_linkIndex = 0;
	_linkIndex2 = 0;
	for (uint i = 0; i < kSubObjectsCount; ++i) {
		_subobjects[i] = 0;
	}
	_notifyThreadId1 = 0;
	_notifyThreadId2 = 0;
	_entryTblPtr = 0;
	_seqCodeIp = 0;
	_sequenceId = 0;
	_seqCodeValue1 = 0;
	_seqCodeValue2 = 600;
	_seqCodeValue3 = 0;

	_notifyId3C = 0;

	_controlRoutine = 0;
	setControlRoutine(new Common::Functor2Mem<Control*, uint32, void, Controls>(_vm->_controls, &Controls::actorControlRoutine));

	_walkCallerThreadId1 = 0;
	_pathAngle = 0;
	_pathFlag50 = false;
	_pathCtrX = 0;
	_pathCtrY = 0;
	_pathInitialPosFlag = true;
	_pathInitialPos.x = 0;
	_pathInitialPos.y = 0;
	_pathPoints = 0;
	_pathPointIndex = 0;
	_pathPointsCount = 0;
	_pathNode = 0;

}

Actor::~Actor() {
	delete _controlRoutine;
}

void Actor::pause() {
	++_pauseCtr;
}

void Actor::unpause() {
	--_pauseCtr;
}

void Actor::createSurface(SurfInfo &surfInfo) {
	_surface = _vm->_screen->allocSurface(surfInfo);
	if (_vm->getGameId() == kGameIdDuckman) {
		if (_flags & Illusions::ACTOR_FLAG_IS_VISIBLE) {
			if (_frameIndex) {
				_flags |= Illusions::ACTOR_FLAG_2000;
			}
			_flags |= Illusions::ACTOR_FLAG_4000;
		}
	} else {
		if (_frameIndex) {
			_flags |= Illusions::ACTOR_FLAG_2000;
			_flags |= Illusions::ACTOR_FLAG_4000;
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

void Actor::initSequenceStack() {
	_seqStackCount = 0;
}

void Actor::pushSequenceStack(int16 value) {
	_seqStack[_seqStackCount++] = value;
}

int16 Actor::popSequenceStack() {
	return _seqStack[--_seqStackCount];
}

void Actor::setControlRoutine(ActorControlRoutine *controlRoutine) {
	delete _controlRoutine;
	_controlRoutine = controlRoutine;
}

void Actor::runControlRoutine(Control *control, uint32 deltaTime) {
	if (_controlRoutine)
		(*_controlRoutine)(control, deltaTime);
}

bool Actor::findNamedPoint(uint32 namedPointId, Common::Point &pt) {
	if (_namedPoints->findNamedPoint(namedPointId, pt)) {
		pt.x += _position.x;
		pt.y += _position.y;
		return true;
	}
	return false;
}

// Control

Control::Control(IllusionsEngine *vm)
	: _vm(vm) {
	_flags = 0;
	_pauseCtr = 0;
	_priority = 0;
	_objectId = 0;
	_bounds._topLeft.x = 0;
	_bounds._topLeft.y = 0;
	_bounds._bottomRight.x = 0;
	_bounds._bottomRight.y = 0;
	_feetPt.x = 0;
	_feetPt.y = 0;
	_position.x = 0;
	_position.y = 0;
	_actorTypeId = 0;
	_actor = 0;
	_sceneId = _vm->getCurrentScene();
}

Control::~Control() {
}

void Control::pause() {

	if (_vm->getGameId() == kGameIdBBDOU || !(_flags & 4)) {
		_vm->_dict->setObjectControl(_objectId, 0);
		if (_objectId == Illusions::CURSOR_OBJECT_ID)
			_vm->setCursorControl(0);
	}

	if (_actor && !(_actor->_flags & Illusions::ACTOR_FLAG_200))
		_actor->destroySurface();

}

void Control::unpause() {

	if (_vm->getGameId() == kGameIdBBDOU || !(_flags & 4)) {
		_vm->_dict->setObjectControl(_objectId, this);
		if (_objectId == Illusions::CURSOR_OBJECT_ID)
			_vm->setCursorControl(this);
	}
  
	if (_actor && !(_actor->_flags & Illusions::ACTOR_FLAG_200)) {
		SurfInfo surfInfo;
		ActorType *actorType = _vm->_dict->findActorType(_actorTypeId);
		if (actorType)
			surfInfo = actorType->_surfInfo;
		else
			surfInfo = _actor->_surfInfo;
		_actor->createSurface(surfInfo);
	}
}

void Control::appearActor() {
	if (_vm->getGameId() == kGameIdDuckman) {
		_flags |= 1;
		_actor->_flags |= Illusions::ACTOR_FLAG_IS_VISIBLE;
		if (_objectId == Illusions::CURSOR_OBJECT_ID) {
			if (_actor->_frameIndex) {
				_actor->_flags |= Illusions::ACTOR_FLAG_2000;
				_actor->_flags |= Illusions::ACTOR_FLAG_4000;
			}
			_vm->_input->discardAllEvents();
		}
	} else {
		if (_objectId == Illusions::CURSOR_OBJECT_ID) {
			_vm->showCursor();
		} else {
			if (_actor->_frameIndex || _actorTypeId == 0x50004) {
				_actor->_flags |= Illusions::ACTOR_FLAG_IS_VISIBLE;
			} else {
				_actor->_flags |= Illusions::ACTOR_FLAG_1000;
			}
			for (uint i = 0; i < kSubObjectsCount; ++i) {
				if (_actor->_subobjects[i]) {
					Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[i]);
					subControl->appearActor();
				}
			}
		}
	}
}

void Control::disappearActor() {
	if (_vm->getGameId() == kGameIdDuckman) {
		_flags &= ~1;
		_actor->_flags &= ~Illusions::ACTOR_FLAG_IS_VISIBLE;
	} else {
		if (_objectId == Illusions::CURSOR_OBJECT_ID) {
			_vm->hideCursor();
		} else {
			_actor->_flags &= ~Illusions::ACTOR_FLAG_IS_VISIBLE;
			_actor->_flags &= ~Illusions::ACTOR_FLAG_1000;
			for (uint i = 0; i < kSubObjectsCount; ++i) {
				if (_actor->_subobjects[i]) {
					Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[i]);
					subControl->disappearActor();
				}
			}
		}
	}
}

bool Control::isActorVisible() {
	return (_actor->_flags & Illusions::ACTOR_FLAG_IS_VISIBLE) != 0;
}

void Control::activateObject() {
	_flags |= 1;
	if (_actor) {
		for (uint i = 0; i < kSubObjectsCount; ++i) {
			if (_actor->_subobjects[i]) {
				Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[i]);
				subControl->activateObject();
			}
		}
	}
}

void Control::deactivateObject() {
	_flags &= ~1;
	if (_actor) {
		for (uint i = 0; i < kSubObjectsCount; ++i) {
			if (_actor->_subobjects[i]) {
				Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[i]);
				subControl->deactivateObject();
			}
		}
	}
}

void Control::readPointsConfig(byte *pointsConfig) {
	_bounds._topLeft.x = READ_LE_UINT16(pointsConfig + 0);
	_bounds._topLeft.y = READ_LE_UINT16(pointsConfig + 2);
	pointsConfig += 4;
	_bounds._bottomRight.x = READ_LE_UINT16(pointsConfig + 0);
	_bounds._bottomRight.y = READ_LE_UINT16(pointsConfig + 2);
	pointsConfig += 4;
	_feetPt.x = READ_LE_UINT16(pointsConfig + 0);
	_feetPt.y = READ_LE_UINT16(pointsConfig + 2);
	pointsConfig += 4;
	_position.x = READ_LE_UINT16(pointsConfig + 0);
	_position.y = READ_LE_UINT16(pointsConfig + 2);
	pointsConfig += 4;
	for (uint i = 0; i < kSubObjectsCount; ++i) {
		_subobjectsPos[i].x = READ_LE_UINT16(pointsConfig + 0);
		_subobjectsPos[i].y = READ_LE_UINT16(pointsConfig + 2);
		pointsConfig += 4;
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
	for (uint i = 0; i < kSubObjectsCount; ++i) {
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[i]);
			subControl->setActorScale(scale);
		}
	}
}

void Control::faceActor(uint facing) {
	_actor->_facing = facing;
	for (uint i = 0; i < kSubObjectsCount; ++i) {
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[i]);
			subControl->faceActor(facing);
		}
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
	for (uint i = 0; i < kSubObjectsCount; ++i) {
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[i]);
			subControl->_actor->_flags &= ~Illusions::ACTOR_FLAG_80;
			subControl->_actor->_entryTblPtr = 0;
			subControl->_actor->_notifyThreadId2 = 0;
		}
	}
	_actor->_flags &= ~Illusions::ACTOR_FLAG_80;
	_actor->_entryTblPtr = 0;
	_actor->_notifyThreadId2 = 0;
}

void Control::setPriority(int16 priority) {
	_priority = priority;
}

uint32 Control::getPriority() {
	uint32 objectId;
	int16 positionY, priority, priority1;
	if (_actor) {
		if (_actor->_parentObjectId && (_actor->_flags & Illusions::ACTOR_FLAG_40)) {
			uint32 parentObjectId = getSubActorParent();
			Control *parentControl = _vm->_dict->getObjectControl(parentObjectId);
			objectId = parentControl->_objectId;
			priority = parentControl->_priority;
			positionY = parentControl->_actor->_position.y;
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
	uint32 p = 50 * priority1 / 100;
	if (p)
		--p;

	positionY = CLIP<int16>(positionY, -5000, 5000);

	return p + 50 * ((objectId & 0x3F) + ((10000 * priority + positionY + 5000) << 6));
}

uint32 Control::getOverlapPriority() {
	if (_vm->getGameId() == kGameIdBBDOU)
		return getPriority();
	return _priority;
}

uint32 Control::getDrawPriority() {
	if (_vm->getGameId() == kGameIdBBDOU)
		return getPriority();
	return (_actor->_position.y + 32768) | (_priority << 16);
}

Common::Point Control::calcPosition(Common::Point posDelta) {
	Common::Point pos;
	if (_actor->_parentObjectId) {
		int16 accuX = 0, accuY = 0;
		Actor *actor = _actor;
		while (actor->_parentObjectId) {
			Control *parentControl = _vm->_dict->getObjectControl(actor->_parentObjectId);
			accuX += parentControl->_subobjectsPos[actor->_linkIndex - 1].x;
			accuY += parentControl->_subobjectsPos[actor->_linkIndex - 1].y;
			actor = parentControl->_actor;
		}
		pos = actor->_position;
		pos.x += accuX * actor->_scale / 100;
		pos.y += accuY * actor->_scale / 100;
		_actor->_position = pos;
		if (!(_flags & 8)) {
			pos.x -= posDelta.x;
			pos.y -= posDelta.y;
		}
	} else {
		pos = _actor->_position;
		if (!(_flags & 8)) {
			pos.x -= posDelta.x;
			pos.y -= posDelta.y;
		}
	}
	return pos;
}

uint32 Control::getSubActorParent() {
	uint32 parentObjectId = _objectId;
	while (1) {
		Actor *actor = _vm->_dict->getObjectControl(parentObjectId)->_actor;
		if (actor->_parentObjectId && (actor->_flags & Illusions::ACTOR_FLAG_40))
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
		collisionRect = Common::Rect(_bounds._topLeft.x, _bounds._topLeft.y, _bounds._bottomRight.x, _bounds._bottomRight.y);
	}

	if (_actor) {
		if (_actor->_scale != 100) {
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

void Control::getCollisionRect(Common::Rect &collisionRect) {
	collisionRect = Common::Rect(_bounds._topLeft.x, _bounds._topLeft.y, _bounds._bottomRight.x, _bounds._bottomRight.y);
	if (_actor) {
		if (_actor->_scale != 100) {
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
		const Frame &frame = (*_actor->_frames)[frameIndex - 1];
		_actor->_surfInfo = frame._surfInfo;
		readPointsConfig(frame._pointsConfig);
		_actor->_flags |= Illusions::ACTOR_FLAG_2000;
		_actor->_flags |= Illusions::ACTOR_FLAG_4000;
		_actor->_newFrameIndex = 0;
	}
}

void Control::stopActor() {
	_actor->_seqCodeIp = 0;
	if (_actor->_pathNode) {
		if (_actor->_flags & Illusions::ACTOR_FLAG_400) {
			delete _actor->_pathNode;
			_actor->_flags &= ~Illusions::ACTOR_FLAG_400;
		}
		_actor->_pathNode = 0;
		_actor->_pathPoints = 0;
		_actor->_pathPointsCount = 0;
		_actor->_pathPointIndex = 0;
		_actor->_walkCallerThreadId1 = 0;
	}
	if (_vm->getGameId() == kGameIdBBDOU) {
		_vm->notifyThreadId(_actor->_notifyId3C);
		_vm->notifyThreadId(_actor->_notifyThreadId1);
	}
}

void Control::startSequenceActor(uint32 sequenceId, int value, uint32 notifyThreadId) {
	startSequenceActorIntern(sequenceId, value, 0, notifyThreadId);
}

void Control::stopSequenceActor() {
	if (_actor->_flags & Illusions::ACTOR_FLAG_40) {
		stopActor();
		_actor->_frameIndex = 0;
		if ((_actor->_flags & Illusions::ACTOR_FLAG_IS_VISIBLE) || (_actor->_flags & Illusions::ACTOR_FLAG_1000)) {
			_actor->_flags &= ~Illusions::ACTOR_FLAG_IS_VISIBLE;
			_actor->_flags |= Illusions::ACTOR_FLAG_1000;
		}
	}
	for (uint i = 0; i < kSubObjectsCount; ++i) {
		if (_actor->_subobjects[i]) {
			Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[i]);
			subControl->stopSequenceActor();
		}
	}
}

void Control::startTalkActor(uint32 sequenceId, byte *entryTblPtr, uint32 threadId) {
	bool doSeq = true;
	if (_actor->_linkIndex2) {
		Control *subControl = _vm->_dict->getObjectControl(_actor->_subobjects[_actor->_linkIndex2 - 1]);
		if (subControl->_actor->_flags & Illusions::ACTOR_FLAG_IS_VISIBLE) {
			if (_actor->_pathNode) {
				doSeq = false;
				subControl->_actor->_notifyThreadId2 = threadId;
				subControl->_actor->_entryTblPtr = entryTblPtr;
				subControl->_actor->_flags |= Illusions::ACTOR_FLAG_80;
				Thread *thread = _vm->_threads->findThread(threadId);
				thread->sendMessage(kMsgClearSequenceId2, 0);
			}
		}
	}
	if (doSeq)
		startSequenceActorIntern(sequenceId, 2, entryTblPtr, threadId);
}

void Control::sequenceActor() {
	if (_actor->_pauseCtr > 0)
		return;

	OpCall opCall;
  	bool sequenceFinished = false;

	opCall._result = 0;
	_actor->_seqCodeValue3 -= _actor->_seqCodeValue1;

	while (_actor->_seqCodeValue3 <= 0 && !sequenceFinished) {
		bool breakInner = false;
		while (!breakInner) {
			//debug(1, "[%08X] SEQ[%08X] op: %08X", _objectId, _actor->_sequenceId, _actor->_seqCodeIp[0]);
			opCall._op = _actor->_seqCodeIp[0] & 0x7F;
			opCall._opSize = _actor->_seqCodeIp[1];
			opCall._code = _actor->_seqCodeIp + 2;
			opCall._deltaOfs = opCall._opSize;
			if (_actor->_seqCodeIp[0] & 0x80)
				breakInner = true;
			execSequenceOpcode(opCall);
			if (opCall._result == 1) {
				sequenceFinished = true;
				breakInner = true;
			} else if (opCall._result == 2) {
				breakInner = true;
			}
			_actor->_seqCodeIp += opCall._deltaOfs;
		}
		_actor->_seqCodeValue3 += _actor->_seqCodeValue2;
	}

	if (_actor->_newFrameIndex != 0) {
		//debug(1, "New frame %d", _actor->_newFrameIndex);
		setActorFrameIndex(_actor->_newFrameIndex);
		if (_vm->getGameId() == kGameIdBBDOU &&
			!(_actor->_flags & Illusions::ACTOR_FLAG_IS_VISIBLE) && (_actor->_flags & Illusions::ACTOR_FLAG_1000) && (_objectId != Illusions::CURSOR_OBJECT_ID)) {
			appearActor();
			_actor->_flags &= ~Illusions::ACTOR_FLAG_1000;
		}
		//debug(1, "New frame OK");
	}

	if (sequenceFinished) {
		//debug(1, "Sequence has finished");
		_actor->_seqCodeIp = 0;
	}
}

void Control::setActorIndex(int actorIndex) {
	_actor->_actorIndex = actorIndex;
}

void Control::setActorIndexTo1() {
	_actor->_actorIndex = 1;
}

void Control::setActorIndexTo2() {
	_actor->_actorIndex = 2;
}

void Control::startSubSequence(int linkIndex, uint32 sequenceId) {
	Control *linkedControl = _vm->_dict->getObjectControl(_actor->_subobjects[linkIndex - 1]);
	Actor *linkedActor = linkedControl->_actor;
	if (!linkedActor->_entryTblPtr)
		linkedActor->_flags &= ~Illusions::ACTOR_FLAG_80;
	linkedActor->_flags &= ~Illusions::ACTOR_FLAG_400;
	linkedActor->_flags |= Illusions::ACTOR_FLAG_100;
	linkedActor->_sequenceId = sequenceId;
	linkedActor->_notifyThreadId1 = 0;
	linkedActor->_notifyId3C = 0;
	linkedActor->_walkCallerThreadId1 = 0;

	Sequence *sequence = _vm->_dict->findSequence(sequenceId);
	linkedActor->_seqCodeIp = sequence->_sequenceCode;
	linkedActor->_frames = _vm->findActorSequenceFrames(sequence);
	linkedActor->_seqCodeValue3 = 0;
	linkedActor->_seqCodeValue1 = 0;
	linkedActor->_seqCodeValue2 = 600;
	linkedActor->initSequenceStack();
	linkedControl->sequenceActor();
	linkedControl->appearActor();
}

void Control::stopSubSequence(int linkIndex) {
	Control *linkedControl = _vm->_dict->getObjectControl(_actor->_subobjects[linkIndex - 1]);
	Actor *linkedActor = linkedControl->_actor;
	uint32 notifyThreadId2 = _actor->_notifyThreadId2;
	_actor->_linkIndex2 = linkIndex;
	if (_actor->_entryTblPtr) {
		linkedActor->_flags |= Illusions::ACTOR_FLAG_80;
		linkedActor->_entryTblPtr = _actor->_entryTblPtr;
		linkedActor->_notifyThreadId2 = _actor->_notifyThreadId2;
		linkedActor->_seqCodeValue1 = _actor->_seqCodeValue1;
		linkedActor->_seqCodeValue3 = _actor->_seqCodeValue3;
		_actor->_flags &= ~Illusions::ACTOR_FLAG_80;
		_actor->_entryTblPtr = 0;
		_actor->_notifyThreadId1 = 0;
		_actor->_notifyThreadId2 = 0;
	}
	if (notifyThreadId2) {
		Thread *talkThread = _vm->_threads->findThread(notifyThreadId2);
		talkThread->sendMessage(kMsgClearSequenceId2, 0);
	}
}

void Control::startMoveActor(uint32 sequenceId, Common::Point destPt, uint32 callerThreadId1, uint32 callerThreadId2) {

	PointArray *pathNode;
	ActorType *actorType = _vm->_dict->findActorType(_actorTypeId);

	_actor->_pathAngle = 0;
	_actor->_pathFlag50 = false;
	_actor->_seqCodeValue3 = 0;
	_actor->_seqCodeValue1 = 0;
	_actor->_pathInitialPosFlag = true;

	uint newFacing;
	if (_vm->calcPointDirection(_actor->_position, destPt, newFacing))
		faceActor(newFacing);

	if (actorType->_value1E)
		_actor->_pathCtrY = actorType->_value1E;
	else
		_actor->_pathCtrY = 140;

	pathNode = createPath(destPt);

	if (pathNode->size() == 1 &&
		_actor->_position.x == (*pathNode)[0].x &&
		_actor->_position.y == (*pathNode)[0].y) {
		delete pathNode;
		_vm->notifyThreadId(callerThreadId2);
	} else {
		_actor->_posXShl = _actor->_position.x << 16;
		_actor->_posYShl = _actor->_position.y << 16;
		startSequenceActor(sequenceId, 1, 0);
		_actor->_pathNode = pathNode;
		_actor->_pathPointsCount = pathNode->size();
		_actor->_pathPoints = pathNode->size();
		_actor->_flags |= Illusions::ACTOR_FLAG_400;
		_actor->_walkCallerThreadId1 = callerThreadId1;
		_vm->notifyThreadId(_actor->_notifyId3C);
		_actor->_notifyId3C = callerThreadId2;
		_actor->_pathPointIndex = 0;
		_vm->_input->discardEvent(kEventSkip);
	}

}

PointArray *Control::createPath(Common::Point destPt) {
	PointArray *walkPoints = (_actor->_flags & Illusions::ACTOR_FLAG_HAS_WALK_POINTS) ? _actor->_pathWalkPoints->_points : 0;
	PathLines *walkRects = (_actor->_flags & Illusions::ACTOR_FLAG_HAS_WALK_RECTS) ? _actor->_pathWalkRects->_rects : 0;
	PathFinder pathFinder;
	WidthHeight bgDimensions = _vm->_backgroundInstances->getMasterBgDimensions();
	PointArray *path = pathFinder.findPath(_vm->_camera, _actor->_position, destPt, walkPoints, walkRects, bgDimensions);
	return path;
}

void Control::updateActorMovement(uint32 deltaTime) {
	// TODO This needs some cleanup

	static const int16 kAngleTbl[] = {60, 0, 120, 0, 60, 0, 120, 0};
	bool fastWalked = false;

	do {

		if (!fastWalked && _vm->testMainActorFastWalk(this)) {
			fastWalked = true;
			disappearActor();
			_actor->_flags |= Illusions::ACTOR_FLAG_8000;
			_actor->_seqCodeIp = 0;
			deltaTime = 2;
		}

		if (_vm->testMainActorCollision(this))
			break;

		Common::Point prevPt;
		if (_actor->_pathPointIndex == 0) {
			if (_actor->_pathInitialPosFlag) {
				_actor->_pathCtrX = 0;
				_actor->_pathInitialPos = _actor->_position;
				_actor->_pathInitialPosFlag = false;
			}
			prevPt = _actor->_pathInitialPos;
		} else {
			prevPt = (*_actor->_pathNode)[_actor->_pathPointIndex - 1];
		}

		Common::Point currPt = (*_actor->_pathNode)[_actor->_pathPointIndex];

		int16 deltaX = currPt.x - prevPt.x;
		int16 deltaY = currPt.y - prevPt.y;

		if (!_actor->_pathFlag50) {

			FixedPoint16 angle;
			if (currPt.x == prevPt.x) {
				if (prevPt.y >= currPt.y)
					angle = fixedMul(-0x5A0000, 0x478);
				else
					angle = fixedMul(0x5A0000, 0x478);
			} else {
				angle = fixedAtan(fixedDiv(deltaY << 16, deltaX << 16));
			}
			_actor->_pathAngle = angle;

			int16 v13 = (fixedTrunc(fixedMul(angle, 0x394BB8)) + 360) % 360; // 0x394BB8 is 180 / pi
			if (deltaX >= 0)
				v13 += 180;
			v13 = (v13 + 90) % 360;
			int16 v15 = kAngleTbl[0] / -2;
			uint newFacing = 1;
			for (uint i = 0; i < 8; ++i) {
				v15 += kAngleTbl[i];
				if (v13 < v15) {
					newFacing = 1 << i;
					break;
				}
			}
			if (newFacing != _actor->_facing) {
				refreshSequenceCode();
				faceActor(newFacing);
			}

			_actor->_pathFlag50 = true;

		}

		FixedPoint16 deltaX24, deltaY24;

		if (_actor->_flags & Illusions::ACTOR_FLAG_400) {

			FixedPoint16 v20 = fixedMul((deltaTime + _actor->_pathCtrX) << 16, _actor->_pathCtrY << 16);
			FixedPoint16 v21 = fixedDiv(v20, 100 << 16);
			FixedPoint16 v22 = fixedMul(v21, _actor->_scale << 16);
			FixedPoint16 v23 = fixedDiv(v22, 100 << 16);
			_actor->_seqCodeValue1 = 100 * _actor->_pathCtrY * deltaTime / 100;
			if (v23) {
				FixedPoint16 prevDistance = fixedDistance(prevPt.x << 16, prevPt.y << 16, _actor->_posXShl, _actor->_posYShl);
				FixedPoint16 distance = prevDistance + v23;
				if (prevPt.x > currPt.x)
					distance = -distance;
				deltaX24 = fixedMul(fixedCos(_actor->_pathAngle), distance);
				deltaY24 = fixedMul(fixedSin(_actor->_pathAngle), distance);
			} else {
				deltaX24 = _actor->_posXShl - (prevPt.x << 16);
				deltaY24 = _actor->_posYShl - (prevPt.y << 16);
			}
		} else {
			if (100 * (int)deltaTime <= _actor->_seqCodeValue2)
				break;
			deltaX24 = deltaX << 16;
			deltaY24 = deltaY << 16;
		}

		if (ABS(deltaX24) < ABS(deltaX << 16) ||
			ABS(deltaY24) < ABS(deltaY << 16)) {
			FixedPoint16 newX = (prevPt.x << 16) + deltaX24;
			FixedPoint16 newY = (prevPt.y << 16) + deltaY24;
			if (newX == _actor->_posXShl &&	newY == _actor->_posYShl) {
				_actor->_pathCtrX += deltaTime;
			} else {
				_actor->_pathCtrX = 0;
				_actor->_posXShl = newX;
				_actor->_posYShl = newY;
				_actor->_position.x = fixedTrunc(_actor->_posXShl);
				_actor->_position.y = fixedTrunc(_actor->_posYShl);
			}
		} else {
			_actor->_position = currPt;
			_actor->_posXShl = _actor->_position.x << 16;
			_actor->_posYShl = _actor->_position.y << 16;
			--_actor->_pathPointsCount;
			++_actor->_pathPointIndex;
			++_actor->_pathPoints;
			_actor->_pathInitialPosFlag = true;
			if (_actor->_pathPointsCount == 0) {
				if (_actor->_flags & Illusions::ACTOR_FLAG_400) {
					delete _actor->_pathNode;
					_actor->_flags &= ~Illusions::ACTOR_FLAG_400;
				}
				_actor->_pathNode = 0;
				_actor->_pathPoints = 0;
				_actor->_pathPointsCount = 0;
				_actor->_pathPointIndex = 0;
				if (_actor->_notifyId3C) {
					_vm->notifyThreadId(_actor->_notifyId3C);
					_actor->_walkCallerThreadId1 = 0;
				}
				fastWalked = false;
			}
			_actor->_pathFlag50 = false;
		}

	} while (fastWalked);

}

void Control::refreshSequenceCode() {
	Sequence *sequence = _vm->_dict->findSequence(_actor->_sequenceId);
	_actor->_seqCodeIp = sequence->_sequenceCode;
}

void Control::getActorFrameDimensions(WidthHeight &dimensions) {
	dimensions._width = _actor->_surface->w;
	dimensions._height = _actor->_surface->h;
}

void Control::drawActorRect(const Common::Rect r, byte color) {
	_vm->_screen->fillSurfaceRect(_actor->_surface, r, color);
	_actor->_flags |= Illusions::ACTOR_FLAG_4000;
}

void Control::fillActor(byte color) {
	_vm->_screen->fillSurface(_actor->_surface, color);
	_actor->_flags |= Illusions::ACTOR_FLAG_4000;
}

bool Control::isPixelCollision(Common::Point &pt) {
	Frame *frame = &(*_actor->_frames)[_actor->_frameIndex - 1];
	return _vm->_screen->isSpritePixelSolid(pt, _position, _actor->_position,
		_actor->_surfInfo, _actor->_scale, frame->_flags, frame->_compressedPixels);
}

void Control::startSequenceActorIntern(uint32 sequenceId, int value, byte *entryTblPtr, uint32 notifyThreadId) {
	stopActor();

	_actor->_flags &= ~Illusions::ACTOR_FLAG_80;
	_actor->_flags &= ~Illusions::ACTOR_FLAG_400;
	_actor->_flags |= Illusions::ACTOR_FLAG_100;

	sequenceId = _actor->_defaultSequences.use(sequenceId);

	_actor->_sequenceId = sequenceId;
	_actor->_notifyThreadId1 = notifyThreadId;
	_actor->_notifyId3C = 0;
	_actor->_walkCallerThreadId1 = 0;
	_actor->_entryTblPtr = 0;

	Sequence *sequence = _vm->_dict->findSequence(sequenceId);

	if (!sequence && _vm->getGameId() == kGameIdDuckman) {
		//debug(1, "Load external sequence...");
		_vm->_resSys->loadResource(0x00060000 | (sequenceId & 0xFFFF), _vm->getCurrentScene(), 0);
		sequence = _vm->_dict->findSequence(sequenceId);
		_actor->_flags |= Illusions::ACTOR_FLAG_800;
	}

	_actor->_seqCodeIp = sequence->_sequenceCode;
	_actor->_frames = _vm->findActorSequenceFrames(sequence);

	_actor->_seqCodeValue3 = 0;
	_actor->_seqCodeValue1 = 0;

	if (_vm->getGameId() == kGameIdBBDOU) {
		_actor->_seqCodeValue2 = value == 1 ? 350 : 600;
	} else if (_vm->getGameId() == kGameIdDuckman) {
		_actor->_seqCodeValue2 = value == 1 ? 350 : 750;
	}

	_actor->initSequenceStack();

	if (_vm->getGameId() == kGameIdBBDOU)
		stopSequenceActor();

	_actor->_linkIndex2 = 0;

	if (entryTblPtr) {
		_actor->_flags |= Illusions::ACTOR_FLAG_80;
		_actor->_entryTblPtr = entryTblPtr;
		if (_vm->getGameId() == kGameIdBBDOU) {
			_actor->_notifyThreadId1 = 0;
			_actor->_notifyThreadId2 = notifyThreadId;
		}
	}

	if (_vm->getGameId() == kGameIdBBDOU)
		sequenceActor();

}

void Control::execSequenceOpcode(OpCall &opCall) {
	_vm->_controls->_sequenceOpcodes->execOpcode(this, opCall);
}

// Controls

Controls::Controls(IllusionsEngine *vm)
	: _vm(vm) {
	_sequenceOpcodes = new SequenceOpcodes(_vm);
	_nextTempObjectId = 0;
}

Controls::~Controls() {
	delete _sequenceOpcodes;
	destroyControls();
}

void Controls::placeBackgroundObject(BackgroundObject *backgroundObject) {
	Control *control = newControl();
	control->_objectId = backgroundObject->_objectId;
	control->_flags = backgroundObject->_flags;
	control->_priority = backgroundObject->_priority;
	control->readPointsConfig(backgroundObject->_pointsConfig);
	control->activateObject();
	_controls.push_front(control);
	_vm->_dict->setObjectControl(control->_objectId, control);
	debug(0, "Added background control. objectId: %08X", control->_objectId);
}

void Controls::placeActor(uint32 actorTypeId, Common::Point placePt, uint32 sequenceId, uint32 objectId, uint32 notifyThreadId) {
	Control *control = newControl();
	Actor *actor = newActor();
	ActorType *actorType = _vm->_dict->findActorType(actorTypeId);

	control->_objectId = objectId;
	control->_flags = actorType->_flags;
	control->_priority = actorType->_priority;
	control->readPointsConfig(actorType->_pointsConfig);
	control->_actorTypeId = actorTypeId;
	control->_actor = actor;

	if (_vm->isCursorObject(actorTypeId, objectId))
		_vm->setCursorControlRoutine(control);

	if (actorType->_surfInfo._dimensions._width > 0 || actorType->_surfInfo._dimensions._height > 0) {
		actor->createSurface(actorType->_surfInfo);
	} else {
		actor->_flags |= Illusions::ACTOR_FLAG_200;
	}

	actor->_position = placePt;
	actor->_position2 = placePt;
	Common::Point currPan = _vm->_camera->getCurrentPan();
	if (!_vm->calcPointDirection(placePt, currPan, actor->_facing))
		actor->_facing = 64;
	actor->_scale = actorType->_scale;
	actor->_namedPoints = &actorType->_namedPoints;

	BackgroundResource *bgRes = _vm->_backgroundInstances->getActiveBgResource();
	if (actorType->_pathWalkPointsIndex) {
		actor->_pathWalkPoints = bgRes->getPathWalkPoints(actorType->_pathWalkPointsIndex - 1);
		actor->_flags |= Illusions::ACTOR_FLAG_HAS_WALK_POINTS;
	}

	if (actorType->_scaleLayerIndex) {
		actor->_scaleLayer = bgRes->getScaleLayer(actorType->_scaleLayerIndex - 1);
		actor->_flags |= Illusions::ACTOR_FLAG_SCALED;
	}

	if (actorType->_pathWalkRectIndex) {
		actor->_pathWalkRects = bgRes->getPathWalkRects(actorType->_pathWalkRectIndex - 1);
		actor->_flags |= Illusions::ACTOR_FLAG_HAS_WALK_RECTS;
	}

	if (actorType->_priorityLayerIndex) {
		actor->_priorityLayer = bgRes->getPriorityLayer(actorType->_priorityLayerIndex - 1);
		actor->_flags |= Illusions::ACTOR_FLAG_PRIORITY;
	}

	if (actorType->_regionLayerIndex) {
		actor->_regionLayer = bgRes->getRegionLayer(actorType->_regionLayerIndex - 1);
		actor->_flags |= Illusions::ACTOR_FLAG_REGION;
	}

	actor->_pathCtrY = 140;

	_controls.push_front(control);
	_vm->_dict->setObjectControl(objectId, control);

	if (_vm->getGameId() == kGameIdDuckman) {
		control->appearActor();
	} else if (_vm->getGameId() == kGameIdBBDOU) {
		control->_flags |= 0x01;
		actor->_flags |= Illusions::ACTOR_FLAG_1000;
	}

	if (_vm->isCursorObject(actorTypeId, objectId))
		_vm->placeCursorControl(control, sequenceId);

	// TODO HACK at least we should restrict this to the sequenceId
	control->setActorIndex(1);

	control->startSequenceActor(sequenceId, 2, notifyThreadId);
}

void Controls::placeSequenceLessActor(uint32 objectId, Common::Point placePt, WidthHeight dimensions, int16 priority) {
	Control *control = newControl();
	Actor *actor = newActor();
	control->_flags = 0;
	control->_priority = priority;
	control->_objectId = objectId;
	control->_bounds._topLeft.x = 0;
	control->_bounds._topLeft.y = 0;
	control->_bounds._bottomRight.x = dimensions._width - 1;
	control->_bounds._bottomRight.y = dimensions._height - 1;
	control->_feetPt.x = dimensions._width / 2;
	control->_feetPt.y = dimensions._height / 2;
	control->_position.x = 0;
	control->_position.y = 0;
	control->_actorTypeId = 0x50004;
	control->_actor = actor;
	actor->setControlRoutine(0);
	actor->_surfInfo._pixelSize = dimensions._width * dimensions._height;
	actor->_surfInfo._dimensions = dimensions;
	actor->createSurface(actor->_surfInfo);
	actor->_position = placePt;
	actor->_position2 = placePt;
	actor->_facing = 64;
	actor->_scale = 100;
	actor->_namedPoints = 0;
	actor->_pathCtrY = 140;

	_controls.push_front(control);
	_vm->_dict->setObjectControl(objectId, control);
	control->appearActor();
}

void Controls::placeActorLessObject(uint32 objectId, Common::Point feetPt, Common::Point pt, int16 priority, uint flags) {
	Control *control = newControl();
	control->_flags = flags;
	control->_feetPt = feetPt;
	control->_priority = priority;
	control->_objectId = objectId;
	control->_bounds._topLeft = feetPt;
	control->_bounds._bottomRight = pt;
	control->_position.x = 0;
	control->_position.y = 0;
	control->_actorTypeId = 0;
	control->_actor = 0;
	_controls.push_front(control);
	_vm->_dict->setObjectControl(objectId, control);
}

void Controls::placeSubActor(uint32 objectId, int linkIndex, uint32 actorTypeId, uint32 sequenceId) {
	Control *parentControl = _vm->_dict->getObjectControl(objectId);
	uint32 tempObjectId = newTempObjectId();
	placeActor(actorTypeId, Common::Point(0, 0), sequenceId, tempObjectId, 0);
	parentControl->_actor->_subobjects[linkIndex - 1] = tempObjectId;
	Actor *subActor = _vm->_dict->getObjectControl(tempObjectId)->_actor;
	subActor->_flags |= Illusions::ACTOR_FLAG_40;
	subActor->_parentObjectId = parentControl->_objectId;
	subActor->_linkIndex = linkIndex;
}

void Controls::placeDialogItem(uint16 objectNum, uint32 actorTypeId, uint32 sequenceId, Common::Point placePt, int16 choiceJumpOffs) {
	Control *control = newControl();
	Actor *actor = newActor();
	ActorType *actorType = _vm->_dict->findActorType(actorTypeId);
	control->_flags = 0xC;
	control->_priority = actorType->_priority;
	control->_objectId = objectNum | 0x40000;
	control->readPointsConfig(actorType->_pointsConfig);
	control->_actorTypeId = actorTypeId;
	control->_actor = actor;
	actor->setControlRoutine(new Common::Functor2Mem<Control*, uint32, void, Controls>(this, &Controls::dialogItemControlRoutine));
	actor->_choiceJumpOffs = choiceJumpOffs;
	actor->createSurface(actorType->_surfInfo);
	actor->_position = placePt;
	actor->_position2 = placePt;
	actor->_scale = actorType->_scale;
	actor->_color = actorType->_color;
	_controls.push_front(control);
	control->appearActor();
	control->startSequenceActor(sequenceId, 2, 0);
	control->setActorIndex(1);
}

void Controls::destroyControls() {
	ItemsIterator it = _controls.begin();
	while (it != _controls.end()) {
		destroyControlInternal(*it);
		it = _controls.erase(it);
	}
}

void Controls::destroyActiveControls() {
	ItemsIterator it = _controls.begin();
	while (it != _controls.end()) {
		if ((*it)->_pauseCtr <= 0) {
			destroyControlInternal(*it);
			it = _controls.erase(it);
		} else
			++it;
	}
}

void Controls::destroyControlsBySceneId(uint32 sceneId) {
	ItemsIterator it = _controls.begin();
	while (it != _controls.end()) {
		if ((*it)->_sceneId == sceneId) {
			destroyControlInternal(*it);
			it = _controls.erase(it);
		} else
			++it;
	}
}

void Controls::destroyDialogItems() {
	ItemsIterator it = _controls.begin();
	while (it != _controls.end()) {
		if (((*it)->_pauseCtr == 0) && ((*it)->_flags & 4)) {
			destroyControlInternal(*it);
			it = _controls.erase(it);
		} else
			++it;
	}
}

void Controls::threadIsDead(uint32 threadId) {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		if (control->_actor &&
			(control->_actor->_notifyThreadId1 == threadId || control->_actor->_notifyId3C == threadId)) {
			control->_actor->_notifyThreadId1 = 0;
			control->_actor->_notifyId3C = 0;
		}
	}
}

void Controls::pauseControls() {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		++control->_pauseCtr;
		if (control->_pauseCtr == 1)
			control->pause();
	}
}

void Controls::unpauseControls() {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		--control->_pauseCtr;
		if (control->_pauseCtr == 0)
			control->unpause();
	}
}

void Controls::pauseControlsBySceneId(uint32 sceneId) {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		if (control->_sceneId == sceneId) {
			++control->_pauseCtr;
			if (control->_pauseCtr == 1)
				control->pause();
		}
	}
}

void Controls::unpauseControlsBySceneId(uint32 sceneId) {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		if (control->_sceneId == sceneId) {
			--control->_pauseCtr;
			if (control->_pauseCtr == 0)
				control->unpause();
		}
	}
}

bool Controls::getOverlappedObject(Control *control, Common::Point pt, Control **outOverlappedControl, int minPriority) {
	Control *foundControl = 0;
	uint32 foundPriority = 0;
	uint32 minPriorityExt = _vm->getPriorityFromBase(minPriority);

	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *testControl = *it;
		if (testControl != control && testControl->_pauseCtr == 0 &&
			(testControl->_flags & 1) && !(testControl->_flags & 0x10) &&
			(!testControl->_actor || (testControl->_actor->_flags & Illusions::ACTOR_FLAG_IS_VISIBLE))) {
			Common::Rect collisionRect;
			testControl->getCollisionRect(collisionRect);
			if (!collisionRect.isEmpty() && collisionRect.contains(pt)) {
				uint32 testPriority = testControl->getOverlapPriority();
				if ((!foundControl || foundPriority < testPriority) &&
					testPriority >= minPriorityExt) {
					foundControl = testControl;
					foundPriority = testPriority;
				}
			}
		}
	}

	if (foundControl) {
		if (foundControl->_actor && foundControl->_actor->_parentObjectId && (foundControl->_actor->_flags & Illusions::ACTOR_FLAG_40)) {
			uint32 parentObjectId = foundControl->getSubActorParent();
			foundControl = _vm->_dict->getObjectControl(parentObjectId);
		}
		*outOverlappedControl = foundControl;
	}

	return foundControl != 0;
}

bool Controls::getOverlappedObjectAccurate(Control *control, Common::Point pt, Control **outOverlappedControl, int minPriority) {
	Control *foundControl = 0;
	uint32 foundPriority = 0;
	uint32 minPriorityExt = _vm->getPriorityFromBase(minPriority);

	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *testControl = *it;
		if (testControl != control && testControl->_pauseCtr == 0 &&
			(testControl->_flags & 1) && !(testControl->_flags & 0x10) &&
			(!testControl->_actor || (testControl->_actor->_flags & Illusions::ACTOR_FLAG_IS_VISIBLE))) {
			Common::Rect collisionRect;
			testControl->getCollisionRectAccurate(collisionRect);
			if (!collisionRect.isEmpty() && collisionRect.contains(pt) &&
				(!testControl->_actor || testControl->isPixelCollision(pt))) {
				uint32 testPriority = testControl->getOverlapPriority();
				if ((!foundControl || foundPriority < testPriority) &&
					testPriority >= minPriorityExt) {
					foundControl = testControl;
					foundPriority = testPriority;
				}
			}
		}
	}

	if (foundControl) {
		if (foundControl->_actor && foundControl->_actor->_parentObjectId && (foundControl->_actor->_flags & Illusions::ACTOR_FLAG_40)) {
			uint32 parentObjectId = foundControl->getSubActorParent();
			foundControl = _vm->_dict->getObjectControl(parentObjectId);
		}
		*outOverlappedControl = foundControl;
	}

	return foundControl != 0;
}

bool Controls::getDialogItemAtPos(Control *control, Common::Point pt, Control **outOverlappedControl) {
	Control *foundControl = 0;
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *testControl = *it;
		if (testControl != control && testControl->_pauseCtr == 0 &&
			(testControl->_flags & 1) && (testControl->_flags & 4)) {
			Common::Rect collisionRect;
			testControl->getCollisionRect(collisionRect);
			if (!collisionRect.isEmpty() && collisionRect.contains(pt) &&
				(!foundControl || foundControl->_priority < testControl->_priority))
				foundControl = testControl;
		}
	}
	*outOverlappedControl = foundControl;
	return foundControl != 0;
}

bool Controls::getOverlappedWalkObject(Control *control, Common::Point pt, Control **outOverlappedControl) {
	Control *foundControl = 0;
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *testControl = *it;
		if (testControl != control && testControl->_pauseCtr == 0 &&
			(testControl->_flags & 1)) {
			Common::Rect collisionRect;
			testControl->getCollisionRect(collisionRect);
			if (!collisionRect.isEmpty() && collisionRect.contains(pt) &&
				(!foundControl || foundControl->_priority < testControl->_priority))
				foundControl = testControl;
		}
	}
	if (foundControl)
		*outOverlappedControl = foundControl;
	return foundControl != 0;
}

void Controls::destroyControl(Control *control) {
	_controls.remove(control);
	destroyControlInternal(control);
}

bool Controls::findNamedPoint(uint32 namedPointId, Common::Point &pt) {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		if (control->_pauseCtr == 0 && control->_actor && control->_actor->findNamedPoint(namedPointId, pt))
			return true;
	}
	return false;
}

void Controls::actorControlRoutine(Control *control, uint32 deltaTime) {

	Actor *actor = control->_actor;

	if (actor->_pauseCtr > 0)
		return;

	if (control->_actor->_pathNode) {
		control->updateActorMovement(deltaTime);
	} else {
		actor->_seqCodeValue1 = 100 * deltaTime;
	}

	if (actor->_flags & Illusions::ACTOR_FLAG_SCALED) {
		int scale = actor->_scaleLayer->getScale(actor->_position);
		control->setActorScale(scale);
	}

	if (actor->_flags & Illusions::ACTOR_FLAG_PRIORITY) {
		int16 priority = actor->_priorityLayer->getPriority(actor->_position);
		if (priority)
			control->setPriority(priority + 1);
	}

	if (actor->_flags & Illusions::ACTOR_FLAG_REGION) {
		// Update transition sequence
		int regionIndex = actor->_regionLayer->getRegionIndex(actor->_position);
		if (actor->_regionIndex != regionIndex) {
			if (regionIndex) {
				uint32 savedSequenceId = actor->_sequenceId;
				byte *savedSeqCodeIp = actor->_seqCodeIp;
				int savedSeqCodeValue1 = actor->_seqCodeValue1;
				int savedSeqCodeValue3 = actor->_seqCodeValue3;
				uint32 regionSequenceId = actor->_regionLayer->getRegionSequenceId(regionIndex);
				//debug(1, "Running transition sequence %08X", regionSequenceId);
				Sequence *sequence = _vm->_dict->findSequence(regionSequenceId);
				actor->_sequenceId = regionSequenceId;
				actor->_seqCodeIp = sequence->_sequenceCode;
				actor->_seqCodeValue3 = 0;
				control->sequenceActor();
				actor->_sequenceId = savedSequenceId;
				actor->_seqCodeIp = savedSeqCodeIp;
				actor->_seqCodeValue3 = savedSeqCodeValue3;
				actor->_seqCodeValue1 = savedSeqCodeValue1;
			}
			actor->_regionIndex = regionIndex;
		}
	}

}

void Controls::dialogItemControlRoutine(Control *control, uint32 deltaTime) {
	Actor *actor = control->_actor;
	if (actor->_pauseCtr <= 0)
		actor->_seqCodeValue1 = 100 * deltaTime;
}

Actor *Controls::newActor() {
	return new Actor(_vm);
}

Control *Controls::newControl() {
	return new Control(_vm);
}

uint32 Controls::newTempObjectId() {
	uint32 nextTempObjectId1 = _nextTempObjectId;
	uint32 nextTempObjectId2 = _nextTempObjectId + 0x1000;
	if (nextTempObjectId2 > 0xFFFF) {
		nextTempObjectId1 = 0;
		nextTempObjectId2 = 0x1000;
	}
	_nextTempObjectId = nextTempObjectId1 + 1;
	return nextTempObjectId2 | 0x40000;
}

void Controls::destroyControlInternal(Control *control) {

	if ((_vm->getGameId() == kGameIdBBDOU || !(control->_flags & 4)) && control->_pauseCtr <= 0)
		_vm->_dict->setObjectControl(control->_objectId, 0);

	if ((_vm->getGameId() == kGameIdBBDOU || !(control->_flags & 4)) && control->_objectId == Illusions::CURSOR_OBJECT_ID && control->_pauseCtr <= 0)
		_vm->setCursorControl(0);

	if (control->_actor) {
		if (control->_actor->_pathNode && (control->_actor->_flags & Illusions::ACTOR_FLAG_400))
			delete control->_actor->_pathNode;
		if (!(control->_actor->_flags & Illusions::ACTOR_FLAG_200))
			control->_actor->destroySurface();
		delete control->_actor;
		control->_actor = 0;
	}

	delete control;
}

void Controls::disappearActors() {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		if (control->_flags & 4 && control->_pauseCtr == 0) {
			control->disappearActor();
		}
	}
	Control *control = _vm->_dict->getObjectControl(0x40148);
	if (control) {
		control->disappearActor();
	}
}

void Controls::appearActors() {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		if (control->_flags & 4 && control->_pauseCtr == 0) {
			control->appearActor();
		}
	}
	Control *control = _vm->_dict->getObjectControl(0x40148);
	if (control) {
		control->appearActor();
	}
}

void Controls::pauseActors(uint32 objectId) {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		if (control->_actor && control->_objectId != objectId) {
			control->_actor->pause();
		}
	}
}

void Controls::unpauseActors(uint32 objectId) {
	for (ItemsIterator it = _controls.begin(); it != _controls.end(); ++it) {
		Control *control = *it;
		if (control->_actor && control->_objectId != objectId) {
			control->_actor->unpause();
		}
	}
	_vm->_unpauseControlActorFlag = true;
}


} // End of namespace Illusions
