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
#include "illusions/bbdou/bbdou_specialcode.h"
#include "illusions/bbdou/bbdou_bubble.h"
#include "illusions/bbdou/bbdou_cursor.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/scriptman.h"
#include "illusions/scriptopcodes.h"

namespace Illusions {

// BbdouSpecialCode

BbdouSpecialCode::BbdouSpecialCode(IllusionsEngine *vm)
	: SpecialCode(vm) {
	_bubble = new BbdouBubble(_vm, this);
	_cursor = new BbdouCursor(_vm, this);
}

BbdouSpecialCode::~BbdouSpecialCode() {
	delete _cursor;
	delete _bubble;
}

typedef Common::Functor1Mem<OpCall&, void, BbdouSpecialCode> SpecialCodeFunctionI;
#define SPECIAL(id, func) _map[id] = new SpecialCodeFunctionI(this, &BbdouSpecialCode::func);

void BbdouSpecialCode::init() {
	// TODO
	SPECIAL(0x00160006, spcInitCursor);
	SPECIAL(0x00160008, spcEnableCursor);
	SPECIAL(0x00160009, spcDisableCursor);
	SPECIAL(0x0016000A, spcAddCursorSequence);
	SPECIAL(0x00160013, spcInitBubble);
	SPECIAL(0x00160014, spcSetupBubble);
	SPECIAL(0x00160015, spcSetObjectInteractMode);
}

void BbdouSpecialCode::run(uint32 specialCodeId, OpCall &opCall) {
	MapIterator it = _map.find(specialCodeId);
	if (it != _map.end()) {
		(*(*it)._value)(opCall);
	} else {
		debug("BbdouSpecialCode::run() Unimplemented special code %08X", specialCodeId);
		_vm->notifyThreadId(opCall._callerThreadId);
	}
}

// Special codes

// Convenience macros
#define	ARG_SKIP(x) opCall.skip(x); 
#define ARG_INT16(name) int16 name = opCall.readSint16(); debug("ARG_INT16(" #name " = %d)", name);
#define ARG_UINT32(name) uint32 name = opCall.readUint32(); debug("ARG_UINT32(" #name " = %08X)", name);

void BbdouSpecialCode::spcInitCursor(OpCall &opCall) {
	ARG_UINT32(objectId);
	ARG_UINT32(progResKeywordId);
	_cursor->init(objectId, progResKeywordId);
	setCursorControlRoutine(objectId, 0);
	_vm->notifyThreadId(opCall._callerThreadId);
}

void BbdouSpecialCode::spcEnableCursor(OpCall &opCall) {
	ARG_UINT32(objectId);
	_cursor->enable(objectId);
	_vm->notifyThreadId(opCall._callerThreadId);
}

void BbdouSpecialCode::spcDisableCursor(OpCall &opCall) {
	ARG_UINT32(objectId);
	_cursor->disable(objectId);
	_vm->notifyThreadId(opCall._callerThreadId);
}

void BbdouSpecialCode::spcAddCursorSequence(OpCall &opCall) {
	ARG_SKIP(4);
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	_cursor->addCursorSequence(objectId, sequenceId);
	_vm->notifyThreadId(opCall._callerThreadId);
}

void BbdouSpecialCode::spcInitBubble(OpCall &opCall) {
	_bubble->init();
	_vm->notifyThreadId(opCall._callerThreadId);
}

void BbdouSpecialCode::spcSetupBubble(OpCall &opCall) {
	ARG_UINT32(sequenceId1);
	ARG_UINT32(sequenceId2);
	ARG_UINT32(progResKeywordId);
	ARG_UINT32(namedPointId);
	ARG_INT16(count);
	_bubble->addItem0(sequenceId1, sequenceId2, progResKeywordId, namedPointId,
		count, (uint32*)opCall._code);
	_vm->notifyThreadId(opCall._callerThreadId);
}

void BbdouSpecialCode::spcSetObjectInteractMode(OpCall &opCall) {
	ARG_SKIP(4);
	ARG_UINT32(objectId);
	ARG_INT16(value);
	// TODO Cursor_updateStruct8bs(objectId, v3);
	_vm->notifyThreadId(opCall._callerThreadId);
}

void BbdouSpecialCode::playSoundEffect(int soundIndex) {
	static const uint32 kSoundEffectIds[] = {
		      0, 1,
		0x900C1, 2,
		      0, 3,
		0x900C0, 4,
		0x900C2, 5, 
		      0, 6
	};
	uint32 soundEffectId = kSoundEffectIds[2 * soundIndex];
	if (soundEffectId) {
		// TODO _vm->startSound(soundEffectId, 255, 0);
	}
}

void BbdouSpecialCode::resetItem10(uint32 objectId, Item10 *item10) {
	if (item10->_playSound48 == 1) {
		_bubble->hide();
		item10->_verbId = 0x1B0000;
		item10->_playSound48 = 0;
		item10->_objectIds[0] = 0;
		item10->_objectIds[1] = 0;
	}
	_vm->_input->discardButtons(0xFFFF);
}

bool BbdouSpecialCode::testValueRange(int value) {
	return value >= 2 && value <= 7;
}

void BbdouSpecialCode::setCursorControlRoutine(uint32 objectId, int num) {
	Control *control = _vm->_dict->getObjectControl(objectId);
	if (num == 0)
		control->_actor->setControlRoutine(
			new Common::Functor2Mem<Control*, uint32, void, BbdouSpecialCode>(this, &BbdouSpecialCode::cursorInteractControlRoutine));
	else
		control->_actor->setControlRoutine(
			new Common::Functor2Mem<Control*, uint32, void, BbdouSpecialCode>(this, &BbdouSpecialCode::cursorControlRoutine2));
}

Common::Point BbdouSpecialCode::getBackgroundCursorPos(Common::Point cursorPos) {
	Common::Point pt = _vm->_camera->getScreenOffset();
	pt.x += cursorPos.x;
	pt.y += cursorPos.y;
	return pt;
}

bool BbdouSpecialCode::runCause(Control *control, CursorData &cursorData,
	uint32 verbId, uint32 objectId1, uint32 objectId2, int soundIndex) {
	// TODO
	return false;
}

void BbdouSpecialCode::showBubble(uint32 objectId, uint32 overlappedObjectId, uint32 holdingObjectId,
	Item10 *item10, uint32 progResKeywordId) {
	// TODO
}

bool BbdouSpecialCode::findVerbId(Item10 *item10, uint32 currOverlappedObjectId, int always0, uint32 &outVerbId) {
	// TODO
	return false;
}

void BbdouSpecialCode::cursorInteractControlRoutine(Control *cursorControl, uint32 deltaTime) {
	Actor *actor = cursorControl->_actor;
	CursorData &cursorData = _cursor->_data;
	
	if (cursorData._visibleCtr > 0) {

		Common::Point cursorPos = _vm->_input->getCursorPosition();

		if (cursorPos == actor->_position) {
			cursorData._idleCtr += deltaTime;
			if (cursorData._idleCtr > 3600)
				cursorData._idleCtr = 0;
		} else {
			actor->_position.x = cursorPos.x;
			actor->_position.y = cursorPos.y;
			cursorData._idleCtr = 0;
		}

		if (updateTrackingCursor(cursorControl))
			cursorData._flags |= 1;
		else
			cursorData._flags &= ~1;

		cursorPos = getBackgroundCursorPos(cursorPos);
		bool foundOverlapped = false;
		Control *overlappedControl = 0;
		
		if (cursorData._flags & 1) {
			foundOverlapped = 0;
		} else if (_vm->_scriptMan->_activeScenes.getCurrentScene() == 0x1000D) {
			/* TODO foundOverlapped = artcntrlGetOverlappedObjectAccurate(cursorControl, cursorPos,
			&overlappedControl, cursorData._item10._field58);*/
		} else {
			foundOverlapped = _vm->_controls->getOverlappedObject(cursorControl, cursorPos,
				&overlappedControl, cursorData._item10._field58);
			debug("overlappedControl: %p", (void*)overlappedControl);
		}
		
		if (foundOverlapped) {
			if (overlappedControl->_objectId != cursorData._currOverlappedObjectId) {
				if (cursorData._item10._playSound48)
					playSoundEffect(4);
				resetItem10(cursorControl->_objectId, &cursorData._item10);
				int value = _cursor->findStruct8bsValue(overlappedControl->_objectId);
				if (!testValueRange(value)) {
					if (cursorData._mode == 3)
						_cursor->restoreInfo();
					_cursor->show(cursorControl);
					cursorControl->setActorIndexTo2();
					if (cursorData._overlappedObjectId != overlappedControl->_objectId) {
						cursorData._overlappedObjectId = overlappedControl->_objectId;
						runCause(cursorControl, cursorData, 0x1B0009, 0, overlappedControl->_objectId, 0);
					}
					if (value == 10) {
						if (cursorData._holdingObjectId) {
							cursorData._item10._verbId = 0x1B0003;
							cursorData._currOverlappedObjectId = overlappedControl->_objectId;
						}
						else {
							cursorData._item10._verbId = 0x1B0002;
							cursorData._currOverlappedObjectId = overlappedControl->_objectId;
						}
					} else {
						playSoundEffect(3);
						showBubble(cursorControl->_objectId, overlappedControl->_objectId,
							cursorData._holdingObjectId, &cursorData._item10,
							cursorData._progResKeywordId);
						cursorData._currOverlappedObjectId = overlappedControl->_objectId;
					}
				} else {
					if (cursorData._mode != 3) {
						_cursor->saveInfo();
						cursorData._mode = 3;
						cursorData._item10._verbId = 0x1B0006;
						cursorData._holdingObjectId = 0;
					}
					cursorData._sequenceId = _cursor->getSequenceId1(value);
					_cursor->show(cursorControl);
					cursorData._currOverlappedObjectId = overlappedControl->_objectId;
				}
			}
		} else {
			if (cursorData._overlappedObjectId) {
				runCause(cursorControl, cursorData, 0x1B0009, 0, 0x40003, 0);
				cursorData._overlappedObjectId = 0;
			}
			if (cursorData._currOverlappedObjectId || cursorData._mode == 3) {
				if (cursorData._mode == 3)
					_cursor->restoreInfo();
				_cursor->show(cursorControl);
				cursorControl->setActorIndexTo1();
				if (cursorData._item10._playSound48)
					playSoundEffect(4);
				resetItem10(cursorControl->_objectId, &cursorData._item10);
			}
			cursorData._currOverlappedObjectId = 0;
		}
	}

	actor->_seqCodeValue1 = 100 * deltaTime;

	if (cursorData._visibleCtr <= 0) {
		if (cursorData._currOverlappedObjectId || cursorData._mode == 3 || cursorData._mode == 4) {
			if (cursorData._mode == 3) {
				_cursor->restoreInfo();
			} else if (cursorData._mode == 4) {
				_cursor->restoreAfterTrackingCursor();
			}
			cursorControl->setActorIndexTo1();
		}
		cursorData._currOverlappedObjectId = 0;
	} else if (cursorData._currOverlappedObjectId) {
		if (_vm->_input->pollButton(1)) {
			cursorData._idleCtr = 0;
			if (runCause(cursorControl, cursorData, cursorData._item10._verbId, cursorData._holdingObjectId, cursorData._currOverlappedObjectId, 1)) {
				resetItem10(cursorControl->_objectId, &cursorData._item10);
				cursorData._currOverlappedObjectId = 0;
				cursorControl->setActorIndexTo1();
			}
		} else if (_vm->_input->pollButton(2)) {
			uint32 verbId;
			cursorData._idleCtr = 0;
			if (cursorData._holdingObjectId) {
				runCause(cursorControl, cursorData, 0x1B000B, 0, 0x40003, 0);
				cursorData._currOverlappedObjectId = 0;
			} else if (findVerbId(&cursorData._item10, cursorData._currOverlappedObjectId, 0, verbId) &&
				runCause(cursorControl, cursorData, verbId, cursorData._holdingObjectId, cursorData._currOverlappedObjectId, 1)) {
				resetItem10(cursorControl->_objectId, &cursorData._item10);
				cursorData._currOverlappedObjectId = 0;
				cursorControl->setActorIndexTo1();
			}
		}
	} else {
		if (_vm->_input->pollButton(1)) {
			cursorData._idleCtr = 0;
			runCause(cursorControl, cursorData, 0x1B0002, 0, 0x40003, 0);
		} else if (_vm->_input->pollButton(4)) {
			cursorData._idleCtr = 0;
			if (cursorData._item10._field58 <= 1)
				runCause(cursorControl, cursorData, cursorData._holdingObjectId != 0 ? 0x1B000B : 0x1B0004, 0, 0x40003, 0);
		}
	}

}

void BbdouSpecialCode::cursorControlRoutine2(Control *cursorControl, uint32 deltaTime) {
	// TODO
}

bool BbdouSpecialCode::updateTrackingCursor(Control *cursorControl) {
	// TODO
	return false;
}

} // End of namespace Illusions
