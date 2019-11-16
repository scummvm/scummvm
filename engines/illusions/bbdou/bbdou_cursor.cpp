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

#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/bbdou_cursor.h"
#include "illusions/bbdou/bbdou_specialcode.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"

namespace Illusions {

// NOTE It's assumed there's only one game cursor object
// The original stores the _data inside the actor, here it's inside the Cursor class.

// BbdouCursor

BbdouCursor::BbdouCursor(IllusionsEngine_BBDOU *vm, BbdouSpecialCode *bbdou)
	: _vm(vm), _bbdou(bbdou) {
}

BbdouCursor::~BbdouCursor() {
}

void BbdouCursor::init(uint32 objectId, uint32 progResKeywordId) {

	Common::Point pos = _vm->_camera->getCurrentPan();
	_vm->_controls->placeActor(0x50001, pos, 0x6000C, objectId, 0);

	Control *control = _vm->_dict->getObjectControl(objectId);
	control->_flags |= 8;

	_data._mode = 1;
	_data._mode2 = 0;
	_data._verbId1 = 0x1B0000;
	_data._progResKeywordId = progResKeywordId;
	_data._currOverlappedObjectId = 0;
	_data._overlappedObjectId = 0;
	_data._sequenceId = 0x6000F;
	_data._holdingObjectId = 0;
	_data._holdingObjectId2 = 0;
	_data._visibleCtr = 0;
	_data._causeThreadId1 = 0;
	_data._causeThreadId2 = 0;
	_data._field90 = 0;
	_data._flags = 0;
	_data._verbState._minPriority = 1;
	_data._currCursorTrackingSequenceId = 0;
	_data._idleCtr = 0;
	_data._verbState._verbId = 0x1B0000;
	_data._verbState._cursorState = 1;
	_data._verbState._isBubbleVisible = 0;
	_data._verbState._objectIds[0] = 0;
	_data._verbState._objectIds[1] = 0;
	_data._verbState._index = 0;
	_data._verbState._flag56 = false;

	resetActiveVerbs();

	control->setActorIndexTo1();

}

void BbdouCursor::enable(uint32 objectId) {
	++_data._visibleCtr;
	if (_data._visibleCtr == 1) {
		Control *control = _vm->_dict->getObjectControl(objectId);
		show(control);
		_vm->_camera->pushCameraMode();
		_vm->_camera->panEdgeFollow(objectId, 360);
		_data._idleCtr = 0;
	}
	_vm->_input->discardAllEvents();
}

void BbdouCursor::disable(uint32 objectId) {
	hide(objectId);
}

void BbdouCursor::reset(uint32 objectId) {
	Control *control = _vm->_dict->getObjectControl(objectId);
	_data._verbState._cursorState = 1;
	_data._mode = 1;
	_data._mode2 = 0;
	_data._verbId1 = 0x1B0000;
	_data._currOverlappedObjectId = 0;
	_data._overlappedObjectId = 0;
	_data._sequenceId = 0x6000F;
	_data._holdingObjectId = 0;
	_data._holdingObjectId2 = 0;
	_data._visibleCtr = 0;
	_data._causeThreadId1 = 0;
	_data._flags = 0;
	_data._verbState._minPriority = 1;
	_data._currCursorTrackingSequenceId = 0;
	_data._idleCtr = 0;
	_data._verbState._verbId = 0x1B0000;
	_data._verbState._isBubbleVisible = 0;
	_data._verbState._objectIds[0] = 0;
	_data._verbState._objectIds[1] = 0;
	_data._verbState._index = 0;
	_data._verbState._flag56 = false;
	resetActiveVerbs();
	control->setActorIndexTo1();
	control->startSequenceActor(0x60029, 2, 0);
	_bbdou->hideVerbBubble(control->_objectId, &_data._verbState);
}

void BbdouCursor::addCursorSequenceId(uint32 objectId, uint32 sequenceId) {
	for (uint i = 0; i < kMaxCursorSequences; ++i) {
		if (_cursorSequences[i]._objectId == 0) {
			_cursorSequences[i]._objectId = objectId;
			_cursorSequences[i]._sequenceId = sequenceId;
			break;
		}
	}
}

uint32 BbdouCursor::findCursorSequenceId(uint32 objectId) {
	for (uint i = 0; i < kMaxCursorSequences; ++i) {
		if (_cursorSequences[i]._objectId == objectId)
			return _cursorSequences[i]._sequenceId;
	}
	return 0;
}

bool BbdouCursor::updateTrackingCursor(Control *control) {
	uint32 sequenceId;
	if (getTrackingCursorSequenceId(control, sequenceId)) {
		if (_data._currCursorTrackingSequenceId != sequenceId) {
			saveBeforeTrackingCursor(control, sequenceId);
			show(control);
			_data._currCursorTrackingSequenceId = sequenceId;
		}
		return true;
	} else {
		if (_data._currCursorTrackingSequenceId) {
			_data._currCursorTrackingSequenceId = 0;
			restoreAfterTrackingCursor();
			show(control);
		}
		return false;
	}
}

void BbdouCursor::saveInfo() {
	_data._mode2 = _data._mode;
	_data._sequenceId2 = _data._sequenceId;
	_data._holdingObjectId2 = _data._holdingObjectId;
}

void BbdouCursor::restoreInfo() {
	_data._mode = _data._mode2;
	_data._holdingObjectId = _data._holdingObjectId2;
	_data._sequenceId = _data._sequenceId2;
	_data._mode2 = 0;
	_data._holdingObjectId2 = 0;
	_data._sequenceId2 = 0;
}

void BbdouCursor::saveBeforeTrackingCursor(Control *control, uint32 sequenceId) {
	if (_data._currOverlappedObjectId || _data._mode == 3) {
		if (_data._mode == 3)
			restoreInfo();
		control->setActorIndexTo1();
		if (_data._verbState._isBubbleVisible)
			_bbdou->playSoundEffect(4);
		_bbdou->hideVerbBubble(control->_objectId, &_data._verbState);
	}
	_data._currOverlappedObjectId = 0;
	if (_data._mode != 4) {
		saveInfo();
		_data._mode = 4;
		_data._holdingObjectId = 0;
	}
	_data._sequenceId = sequenceId;
}

void BbdouCursor::restoreAfterTrackingCursor() {
	_data._holdingObjectId = _data._holdingObjectId2;
	if (_data._holdingObjectId2) {
		_data._mode = 2;
		_data._sequenceId = findCursorSequenceId(_data._holdingObjectId2);
	} else {
		_data._mode = 1;
		_data._sequenceId = 0x6000F;
	}
	_data._mode2 = 0;
	_data._sequenceId2 = 0;
	_data._holdingObjectId2 = 0;
	_data._currCursorTrackingSequenceId = 0;
}

uint32 BbdouCursor::getSequenceId1(int sequenceIndex) {
	switch (sequenceIndex) {
	case 2:
		return 0x60010;
	case 3:
		return 0x60011;
	case 4:
		return 0x60012;
	case 5:
		return 0x60013;
	case 6:
		return 0x60015;
	case 7:
		return 0x60014;
	default:
		return 0;
	}
}

uint BbdouCursor::calcTrackingFlags(Common::Point actorPos, Common::Point trackingLimits) {
	uint trackingFlags = 0;
	int16 x = actorPos.x - 320;
	int16 y = actorPos.y - 240;
	if (x < -trackingLimits.x)
		trackingFlags = 1;
	else if (x > trackingLimits.x)
		trackingFlags = 3;
	else
		trackingFlags = 2;
	if (y < -trackingLimits.y)
		trackingFlags += 0;
	else if (y > trackingLimits.y)
		trackingFlags += 6;
	else
		trackingFlags += 3;
	return trackingFlags;
}

uint BbdouCursor::calcTrackingCursorIndex(uint trackingFlags) {
	uint cursorIndex = 0;
	switch (trackingFlags) {
	case 1:
		if (_vm->_camera->isAtPanLimit(1)) {
			if (!_vm->_camera->isAtPanLimit(3))
				cursorIndex = 4;
		} else {
			if (!_vm->_camera->isAtPanLimit(3))
				cursorIndex = 1;
			else
				cursorIndex = 2;
		}
		break;
	case 2:
		if (!_vm->_camera->isAtPanLimit(1))
			cursorIndex = 2;
		break;
	case 3:
		if (_vm->_camera->isAtPanLimit(1)) {
			if (!_vm->_camera->isAtPanLimit(4))
				cursorIndex = 6;
		} else {
			if (!_vm->_camera->isAtPanLimit(4))
				cursorIndex = 3;
			else
				cursorIndex = 2;
		}
		break;
	case 4:
		if (!_vm->_camera->isAtPanLimit(3))
			cursorIndex = 4;
		break;
	case 6:
		if (!_vm->_camera->isAtPanLimit(4))
			cursorIndex = 6;
		break;
	case 7:
		if (_vm->_camera->isAtPanLimit(2)) {
			if (!_vm->_camera->isAtPanLimit(3))
				cursorIndex = 4;
		} else {
			if (!_vm->_camera->isAtPanLimit(3))
				cursorIndex = 8;
			else
				cursorIndex = 7;
		}
		break;
	case 8:
		if (!_vm->_camera->isAtPanLimit(2))
			cursorIndex = 8;
		break;
	case 9:
		if (_vm->_camera->isAtPanLimit(2)) {
			if (!_vm->_camera->isAtPanLimit(4))
				cursorIndex = 6;
		} else {
			if (!_vm->_camera->isAtPanLimit(4))
				cursorIndex = 9;
			else
				cursorIndex = 8;
		}
		break;
	default:
		break;
	}
	return cursorIndex;
}

bool BbdouCursor::getTrackingCursorSequenceId(Control *control, uint32 &outSequenceId) {
	static const uint32 kTrackingCursorSequenceIds[] = {
		0, 0x000609BF, 0x00060018, 0x000609C0, 0x00060016,
		0, 0x00060017, 0x000609C1, 0x00060019, 0x000609C2
	};
	Common::Point trackingLimits = _vm->_camera->getTrackingLimits();
	uint trackingFlags = calcTrackingFlags(control->_actor->_position, trackingLimits);
	uint cursorIndex = calcTrackingCursorIndex(trackingFlags);
	outSequenceId = kTrackingCursorSequenceIds[cursorIndex];
	return outSequenceId != 0;
}

void BbdouCursor::resetActiveVerbs() {
	for (uint i = 0; i < 32; ++i) {
		_data._verbState._verbActive[i] = false;
	}
	if (_data._verbState._cursorState == 1) {
		_data._verbState._verbActive[1] = true;
		_data._verbState._verbActive[2] = true;
		_data._verbState._verbActive[3] = true;
		_data._verbState._verbActive[5] = true;
	} else if (_data._verbState._cursorState == 3) {
		_data._verbState._verbActive[1] = true;
		_data._verbState._verbActive[2] = true;
	}
}

void BbdouCursor::show(Control *control) {
	control->startSequenceActor(_data._sequenceId, 2, 0);
	control->appearActor();
}

void BbdouCursor::hide(uint32 objectId) {
	--_data._visibleCtr;
	if (_data._visibleCtr == 0) {
		Control *control = _vm->_dict->getObjectControl(objectId);
		control->startSequenceActor(0x60029, 2, 0);
		_bbdou->hideVerbBubble(objectId, &_data._verbState);
		_vm->_camera->popCameraMode();
	}
	_vm->_input->discardAllEvents();
}

} // End of namespace Illusions
