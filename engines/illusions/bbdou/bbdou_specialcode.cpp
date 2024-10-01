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

#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/bbdou_specialcode.h"
#include "illusions/bbdou/bbdou_bubble.h"
#include "illusions/bbdou/bbdou_inventory.h"
#include "illusions/bbdou/bbdou_credits.h"
#include "illusions/bbdou/bbdou_cursor.h"
#include "illusions/bbdou/bbdou_foodctl.h"
#include "illusions/resources/scriptresource.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/scriptstack.h"
#include "illusions/scriptopcodes.h"
#include "illusions/sound.h"

namespace Illusions {

static const Struct10 kVerbIconSequenceIds[] = {
	{0x1B0000,       0,       0,       0},
	{0x1B0001, 0x6001A, 0x6001B, 0x6001C}, // TALK, LOOK
	{0x1B0002, 0x6001D, 0x6001E, 0x6001F}, // USE, LOOK
	{0x1B0003, 0x60020, 0x60021, 0x60022}, // USE
	{0x1B0004, 0x60023, 0x60024, 0x60025}, // USE, LOOK
	{0x1B0005, 0x60026, 0x60027, 0x60028}, // TALK, LOOK
	{0x1B0006,       0,       0,       0},
	{0x1B0007,       0,       0,       0},
	{0x1B0008,       0,       0,       0},
	{0x1B0009,       0,       0,       0},
	{0x1B000A,       0,       0,       0},
	{0x1B000B,       0,       0,       0},
	{0x1B000C,       0,       0,       0},
};

CauseThread_BBDOU::CauseThread_BBDOU(IllusionsEngine_BBDOU *vm, uint32 threadId, uint32 callingThreadId,
	BbdouSpecialCode *bbdou, uint32 cursorObjectId, uint32 sceneId, uint32 verbId,
	uint32 objectId2, uint32 objectId)
	: Thread(vm, threadId, callingThreadId, 0), _bbdou(bbdou), _cursorObjectId(cursorObjectId),
	_sceneId(sceneId), _verbId(verbId), _objectId2(objectId2), _objectId(objectId) {
	_type = kTTSpecialThread;
}

void CauseThread_BBDOU::onNotify() {
	_bbdou->_cursor->_data._causeThreadId1 = 0;
	terminate();
}

void CauseThread_BBDOU::onTerminated() {
	_bbdou->_cursor->_data._causeThreadId1 = 0;
	_bbdou->_cursor->enable(_cursorObjectId);
}

// RadarMicrophoneThread

RadarMicrophoneThread::RadarMicrophoneThread(IllusionsEngine_BBDOU *vm, uint32 threadId,
	uint32 callingThreadId, uint32 cursorObjectId)
	: Thread(vm, threadId, callingThreadId, 0), _cursorObjectId(cursorObjectId), _zonesCount(0) {
	_sceneId = _vm->getCurrentScene();
}

int RadarMicrophoneThread::onUpdate() {
	Control *control = _vm->getObjectControl(_cursorObjectId);
	int16 cursorX = control->getActorPosition().x;
	if (_currZoneIndex == 0 ||
		cursorX >= _zones[_currZoneIndex - 1]._x ||
		(_currZoneIndex >= 2 && cursorX < _zones[_currZoneIndex - 2]._x)) {//CHECKME
		for (uint i = 0; i < _zonesCount; ++i) {
			if (cursorX < _zones[i]._x) {
				_currZoneIndex = i + 1;
				_vm->startScriptThreadSimple(_zones[i]._threadId, 0);
				break;
			}
		}
	}
	return kTSYield;
}

void RadarMicrophoneThread::addZone(uint32 threadId) {
	_zones[_zonesCount++]._threadId = threadId;
}

void RadarMicrophoneThread::initZones() {
	for (uint i = 0; i < _zonesCount; ++i) {
		_zones[i]._x = (i + 1) * 640 / _zonesCount;
	}
	_zones[_zonesCount]._x = 640;
	_currZoneIndex = 0;
}

// ObjectInteractModeMap

ObjectInteractModeMap::ObjectInteractModeMap() {
}

void ObjectInteractModeMap::setObjectInteractMode(uint32 objectId, int value) {
	ObjectInteractMode *objectInteractMode = nullptr;
	for (uint i = 0; i < ARRAYSIZE(_objectVerbs); ++i) {
		if (_objectVerbs[i]._objectId == objectId) {
			objectInteractMode = &_objectVerbs[i];
			break;
		}
	}
	if (!objectInteractMode) {
		for (uint i = 0; i < ARRAYSIZE(_objectVerbs); ++i) {
			if (_objectVerbs[i]._objectId == 0) {
				objectInteractMode = &_objectVerbs[i];
				break;
			}
		}
	}
	if (value != 11) {
		objectInteractMode->_objectId = objectId;
		objectInteractMode->_interactMode = value;
	} else if (objectInteractMode->_objectId == objectId) {
		objectInteractMode->_objectId = 0;
		objectInteractMode->_interactMode = 0;
	}
}

int ObjectInteractModeMap::getObjectInteractMode(uint32 objectId) {
	for (uint i = 0; i < ARRAYSIZE(_objectVerbs); ++i) {
		if (_objectVerbs[i]._objectId == objectId)
			return _objectVerbs[i]._interactMode;
	}
	return 11;
}

// BbdouSpecialCode

BbdouSpecialCode::BbdouSpecialCode(IllusionsEngine_BBDOU *vm)
	: _vm(vm), _credits(nullptr) {
	_bubble = new BbdouBubble(_vm, this);
	_cursor = new BbdouCursor(_vm, this);
	_inventory = new BbdouInventory(_vm, this);
	_foodCtl = new BbdouFoodCtl(_vm);
}

BbdouSpecialCode::~BbdouSpecialCode() {
	delete _foodCtl;
	delete _inventory;
	delete _cursor;
	delete _bubble;

	for (MapIterator it = _map.begin(); it != _map.end(); ++it) {
		delete (*it)._value;
	}
}

typedef Common::Functor1Mem<OpCall&, void, BbdouSpecialCode> SpecialCodeFunctionI;
#define SPECIAL(id, func) _map[id] = new SpecialCodeFunctionI(this, &BbdouSpecialCode::func);

void BbdouSpecialCode::init() {
	// 0x00160001 only used for original debugging purposes
	SPECIAL(0x00160006, spcInitCursor);
	SPECIAL(0x00160008, spcEnableCursor);
	SPECIAL(0x00160009, spcDisableCursor);
	SPECIAL(0x0016000A, spcAddCursorSequence);
	SPECIAL(0x0016000B, spcCursorStartHoldingObjectId);
	SPECIAL(0x0016000C, spcCursorStopHoldingObjectId);
	SPECIAL(0x0016000F, spcSetCursorState);
	SPECIAL(0x00160013, spcInitBubble);
	SPECIAL(0x00160014, spcSetupBubble);
	SPECIAL(0x00160015, spcSetObjectInteractMode);
	SPECIAL(0x00160017, spcInitInventory);
	SPECIAL(0x00160018, spcClearInventory);
	SPECIAL(0x00160019, spcRegisterInventoryBag);
	SPECIAL(0x0016001A, spcRegisterInventorySlot);
	SPECIAL(0x0016001B, spcRegisterInventoryItem);
	SPECIAL(0x0016001C, spcOpenInventory);
	SPECIAL(0x0016001D, spcAddInventoryItem);
	SPECIAL(0x0016001E, spcRemoveInventoryItem);
	SPECIAL(0x0016001F, spcHasInventoryItem);
	SPECIAL(0x00160025, spcCloseInventory);
	SPECIAL(0x00160027, spcInitConversation);
	SPECIAL(0x00160028, spcClearConversation);
	SPECIAL(0x0016002B, spcClearBlockCounter);
	SPECIAL(0x00160030, spcResetCursor);
	SPECIAL(0x00160032, spcSetCursorField90);
	SPECIAL(0x00160034, spcFoodCtl);
	SPECIAL(0x00160035, spcTestFoodCtl);
	SPECIAL(0x00160036, spcInitMenu);
	SPECIAL(0x00160037, spcIsCursorHoldingObjectId);
	SPECIAL(0x00160038, spcInitRadarMicrophone);
	SPECIAL(0x00160039, spcCreditsCtl);
	SPECIAL(0x0016003A, spcSaladCtl);
	SPECIAL(0x0016003B, spcRunCause);
}

void BbdouSpecialCode::run(uint32 specialCodeId, OpCall &opCall) {
	MapIterator it = _map.find(specialCodeId);
	if (it != _map.end()) {
		(*(*it)._value)(opCall);
	} else {
		debug("BbdouSpecialCode::run() Unimplemented special code %08X", specialCodeId);
		_vm->notifyThreadId(opCall._threadId);
	}
}

void BbdouSpecialCode::resetBeforeResumeSavegame() {
	if (_vm->getCurrentScene() == 0x00010032)
		_inventory->close();
	_vm->_threads->terminateThreads(0);
	_vm->reset();
	_vm->_input->activateButton(0xFFFF);
	_vm->_soundMan->stopMusic();
	_vm->_scriptResource->_blockCounters.clear();
	_vm->_scriptResource->_properties.clear();
	_cursor->reset(0x0004001A);
	setCursorControlRoutine(0x0004001A, 0);
	_cursor->enable(0x0004001A);
}

// Special codes

void BbdouSpecialCode::spcInitCursor(OpCall &opCall) {
	ARG_UINT32(objectId);
	ARG_UINT32(progResKeywordId);
	_cursor->init(objectId, progResKeywordId);
	setCursorControlRoutine(objectId, 0);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcEnableCursor(OpCall &opCall) {
	ARG_UINT32(objectId);
	_cursor->enable(objectId);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcDisableCursor(OpCall &opCall) {
	ARG_UINT32(objectId);
	_cursor->disable(objectId);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcAddCursorSequence(OpCall &opCall) {
	ARG_SKIP(4);
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	_cursor->addCursorSequenceId(objectId, sequenceId);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcCursorStartHoldingObjectId(OpCall &opCall) {
	ARG_UINT32(objectId);
	ARG_UINT32(holdingObjectId);
	ARG_INT16(doPlaySound);
	startHoldingObjectId(objectId, holdingObjectId, doPlaySound != 0);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcCursorStopHoldingObjectId(OpCall &opCall) {
	ARG_UINT32(objectId);
	ARG_INT16(doPlaySound);
	stopHoldingObjectId(objectId, doPlaySound != 0);
	_cursor->_data._mode = 1;
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcSetCursorState(OpCall &opCall) {
	ARG_UINT32(objectId);
	ARG_UINT32(newState);
	_cursor->_data._verbState._cursorState = newState;
	_cursor->resetActiveVerbs();
	if (newState == 5)
		setCursorControlRoutine(objectId, 1);
	else
		setCursorControlRoutine(objectId, 0);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcInitBubble(OpCall &opCall) {
	_bubble->init();
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcSetupBubble(OpCall &opCall) {
	ARG_UINT32(sequenceId1);
	ARG_UINT32(sequenceId2);
	ARG_UINT32(progResKeywordId);
	ARG_UINT32(namedPointId);
	ARG_INT16(count);
	_bubble->addBubbleStyle(sequenceId1, sequenceId2, progResKeywordId, namedPointId,
		count, (uint32*)opCall._code);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcSetObjectInteractMode(OpCall &opCall) {
	ARG_SKIP(4);
	ARG_UINT32(objectId);
	ARG_INT16(value);
	_objectInteractModeMap.setObjectInteractMode(objectId, value);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcInitInventory(OpCall &opCall) {
	// Called but not used in the reimplementation since the
	// inventory is initialized in this class' constructor
}

void BbdouSpecialCode::spcClearInventory(OpCall &opCall) {
	_inventory->clear();
}

void BbdouSpecialCode::spcRegisterInventoryBag(OpCall &opCall) {
	ARG_UINT32(sceneId);
	_inventory->registerInventoryBag(sceneId);
}

void BbdouSpecialCode::spcRegisterInventorySlot(OpCall &opCall) {
	ARG_UINT32(namedPointId);
	_inventory->registerInventorySlot(namedPointId);
}

void BbdouSpecialCode::spcRegisterInventoryItem(OpCall &opCall) {
	ARG_UINT32(objectId);
	ARG_UINT32(sequenceId);
	_inventory->registerInventoryItem(objectId, sequenceId);
}

void BbdouSpecialCode::spcOpenInventory(OpCall &opCall) {
	_inventory->open();
}

void BbdouSpecialCode::spcAddInventoryItem(OpCall &opCall) {
	ARG_UINT32(objectId);
	_inventory->addInventoryItem(objectId);
}

void BbdouSpecialCode::spcRemoveInventoryItem(OpCall &opCall) {
	ARG_UINT32(objectId);
	_inventory->removeInventoryItem(objectId);
}

void BbdouSpecialCode::spcHasInventoryItem(OpCall &opCall) {
	ARG_UINT32(objectId);
	_vm->_stack->push(_inventory->hasInventoryItem(objectId) ? 1 : 0);
}

void BbdouSpecialCode::spcCloseInventory(OpCall &opCall) {
	_inventory->close();
}

void BbdouSpecialCode::spcInitConversation(OpCall &opCall) {
	// Conversations seem unused but this is still called
}

void BbdouSpecialCode::spcClearConversation(OpCall &opCall) {
	// Conversations seem unused but this is still called
}

void BbdouSpecialCode::spcClearBlockCounter(OpCall &opCall) {
	// Conversations seem unused but this is still called
	ARG_UINT32(index);
	_vm->_scriptResource->_blockCounters.set(index, 0);
}

void BbdouSpecialCode::spcResetCursor(OpCall &opCall) {
	ARG_UINT32(objectId);
	_cursor->reset(objectId);
	setCursorControlRoutine(objectId, 0);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcSetCursorField90(OpCall &opCall) {
	ARG_SKIP(4); // objectId unused
	_cursor->_data._field90 = 1;
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcFoodCtl(OpCall &opCall) {
	ARG_UINT32(cmd);
	switch (cmd) {
	case 1:
		{
			ARG_UINT32(minCount);
			ARG_UINT32(maxCount);
			_foodCtl->placeFood(minCount, maxCount);
		}
		break;
	case 2:
		{
			ARG_UINT32(propertyId);
			_foodCtl->addFood(propertyId);
		}
		break;
	case 3:
		_foodCtl->requestFirstFood();
		break;
	case 4:
		_foodCtl->requestNextFood();
		break;
	case 5:
		_foodCtl->serveFood();
		break;
	case 6:
		_foodCtl->resetFood();
		break;
	case 8:
		_foodCtl->nextRound();
		break;
	default:
		break;
	}
}

void BbdouSpecialCode::spcTestFoodCtl(OpCall &opCall) {
	ARG_UINT32(cmd);
	switch (cmd) {
	case 7:
		_vm->_stack->push(_foodCtl->hasReachedRequestedFoodCount() ? 1 : 0);
		break;
	case 9:
		_vm->_stack->push(_foodCtl->hasRoundFinished() ? 1 : 0);
		break;
	default:
		break;
	}
}

void BbdouSpecialCode::spcInitMenu(OpCall &opCall) {
	// Called but not used in the reimplementation
}

void BbdouSpecialCode::spcIsCursorHoldingObjectId(OpCall &opCall) {
	ARG_UINT32(cursorObjectId);
	ARG_UINT32(objectId);
	_vm->_stack->push(isHoldingObjectId(objectId) ? 1 : 0);
	_vm->notifyThreadId(opCall._threadId);
}

void BbdouSpecialCode::spcInitRadarMicrophone(OpCall &opCall) {
	ARG_UINT32(cursorObjectId);
	uint32 tempThreadId = _vm->newTempThreadId();
	RadarMicrophoneThread *radarMicrophoneThread = new RadarMicrophoneThread(_vm,
		tempThreadId, opCall._callerThreadId, cursorObjectId);
	for (uint i = 0; i < 7; ++i) {
		ARG_UINT32(zoneThreadId);
		if (zoneThreadId == 0)
			break;
		radarMicrophoneThread->addZone(zoneThreadId);
	}
	radarMicrophoneThread->initZones();
	_vm->_threads->startThread(radarMicrophoneThread);
}

void BbdouSpecialCode::spcCreditsCtl(OpCall &opCall) {
	ARG_UINT32(cmd);
	switch (cmd) {
	case 1:
		{
			ARG_UINT32(endSignalPropertyId);
			_credits = new BbdouCredits(_vm);
			_credits->start(endSignalPropertyId, 0.5);
		}
		break;
	case 2:
		_credits->drawNextLine();
		break;
	case 3:
		_credits->stop();
		delete _credits;
	default:
		break;
	}
}

void BbdouSpecialCode::spcSaladCtl(OpCall &opCall) {
	ARG_UINT32(cmd);
	ARG_UINT32(sequenceId);
	switch (cmd) {
	case 1:
		initSalad();
		break;
	case 2:
		addSalad(sequenceId);
		break;
	default:
		break;
	}
}

void BbdouSpecialCode::spcRunCause(OpCall &opCall) {
	ARG_UINT32(cursorObjectId);
	ARG_UINT32(verbId);
	ARG_UINT32(objectId1);
	ARG_UINT32(objectId2);
	Control *cursorControl = _vm->getObjectControl(cursorObjectId);
	debug(0, "runCause(%08X, %08X, %08X)", verbId, objectId1, objectId2);
	runCause(cursorControl, _cursor->_data, verbId, objectId1, objectId2, 0);
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
		_vm->_soundMan->playSound(soundEffectId, 255, 0);
	}
}

void BbdouSpecialCode::hideVerbBubble(uint32 objectId, VerbState *verbState) {
	if (verbState->_isBubbleVisible) {
		_bubble->hide();
		verbState->_verbId = 0x1B0000;
		verbState->_isBubbleVisible = false;
		verbState->_objectIds[0] = 0;
		verbState->_objectIds[1] = 0;
	}
	_vm->_input->discardAllEvents();
}

bool BbdouSpecialCode::testInteractModeRange(int value) {
	return value >= 2 && value <= 7;
}

void BbdouSpecialCode::setCursorControlRoutine(uint32 objectId, int num) {
	Control *control = _vm->getObjectControl(objectId);
	if (num == 0)
		control->_actor->setControlRoutine(
			new Common::Functor2Mem<Control*, uint32, void, BbdouSpecialCode>(this, &BbdouSpecialCode::cursorInteractControlRoutine));
	else
		control->_actor->setControlRoutine(
			new Common::Functor2Mem<Control*, uint32, void, BbdouSpecialCode>(this, &BbdouSpecialCode::cursorCrosshairControlRoutine));
}

Common::Point BbdouSpecialCode::getBackgroundCursorPos(Common::Point cursorPos) {
	return _vm->_camera->getScreenOffset() + cursorPos;
}

void BbdouSpecialCode::showBubble(uint32 objectId, uint32 overlappedObjectId, uint32 holdingObjectId,
	VerbState *verbState, uint32 progResKeywordId) {

	Common::Rect collisionRect;
	Control *overlappedControl, *control2;
	Common::Point bubbleSourcePt(320, 240), bubbleDestPt, currPan;

	overlappedControl = _vm->_dict->getObjectControl(overlappedObjectId);
	overlappedControl->getCollisionRect(collisionRect);

	currPan = _vm->_camera->getCurrentPan();
	bubbleDestPt.x = CLIP((collisionRect.right + collisionRect.left) / 2, currPan.x - 274, currPan.x + 274);
	bubbleDestPt.y = CLIP(collisionRect.top - (collisionRect.bottom - collisionRect.top) / 8, currPan.y - 204, currPan.y + 204);

	control2 = _vm->_dict->getObjectControl(0x4000F);
	if (!control2 || (control2->_actor && control2->_actor->_frameIndex == 0))
		control2 = _vm->_dict->getObjectControl(0x4000E);

	if (control2 && control2->_actor && control2->_actor->_frameIndex) {
		bubbleSourcePt.x = control2->_actor->_position.x - control2->_position.x + control2->_actor->_surfInfo._dimensions._width / 2;
		bubbleSourcePt.y = control2->_actor->_position.y - control2->_position.y;
		bubbleSourcePt.y = bubbleSourcePt.y >= 500 ? 500 : bubbleSourcePt.y + 32;
		if (ABS(bubbleSourcePt.x - bubbleDestPt.x) < ABS(bubbleSourcePt.y - bubbleDestPt.y) / 2)
			bubbleSourcePt.y += 80;
	}

	_bubble->selectBubbleStyle(1, bubbleSourcePt, bubbleDestPt, progResKeywordId);

	verbState->_objectIds[0] = _bubble->addBubbleIcon(0, 0x6005A);
	verbState->_objectIds[1] = _bubble->addBubbleIcon(0, 0x6005A);
	verbState->_index = 0;

	int value = _objectInteractModeMap.getObjectInteractMode(overlappedControl->_objectId);
	if (holdingObjectId) {
		verbState->_verbId = 0x1B0003;
	} else if (value == 9) {
		verbState->_verbId = 0x1B0005;
	} else if (value == 8) {
		verbState->_verbId = 0x1B0005;
	} else {
		verbState->_verbId = 0x1B0002;
	}

	_bubble->show();

	Control *verbIconControl = _vm->_dict->getObjectControl(verbState->_objectIds[0]);
	uint32 sequenceId = kVerbIconSequenceIds[verbState->_verbId & 0xFFFF]._sequenceId2;
	verbIconControl->startSequenceActor(sequenceId, 2, 0);
	verbIconControl->appearActor();
	verbIconControl->deactivateObject();

	verbState->_isBubbleVisible = true;
	_vm->_input->discardAllEvents();

}

bool BbdouSpecialCode::findVerbId(VerbState *verbState, uint32 currOverlappedObjectId, int always0, uint32 &outVerbId) {
	if (verbState->_isBubbleVisible) {
		int verbNum = verbState->_verbId & 0xFFFF;
		int verbNumI = verbNum + 1;
		while (1) {
			if (verbNumI >= 32)
				verbNumI = 0;
			if (verbNumI++ == verbNum)
				break;
			if (verbState->_verbActive[verbNumI] && testVerbId(verbNumI | 0x1B0000, always0, currOverlappedObjectId)) {
				outVerbId = verbNumI | 0x1B0000;
				return true;
			}
		}
	}
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

		if (_cursor->updateTrackingCursor(cursorControl))
			cursorData._flags |= 1;
		else
			cursorData._flags &= ~1;

		cursorPos = getBackgroundCursorPos(cursorPos);
		bool foundOverlapped = false;
		Control *overlappedControl = nullptr;

		if (cursorData._flags & 1) {
			foundOverlapped = false;
		} else if (_vm->getCurrentScene() == 0x1000D) {
			foundOverlapped = _vm->_controls->getOverlappedObjectAccurate(cursorControl, cursorPos,
				&overlappedControl, cursorData._verbState._minPriority);
		} else {
			foundOverlapped = _vm->_controls->getOverlappedObject(cursorControl, cursorPos,
				&overlappedControl, cursorData._verbState._minPriority);
		}

		if (foundOverlapped) {
			if (overlappedControl->_objectId != cursorData._currOverlappedObjectId) {
				if (cursorData._verbState._isBubbleVisible)
					playSoundEffect(4);
				hideVerbBubble(cursorControl->_objectId, &cursorData._verbState);
				int interactMode = _objectInteractModeMap.getObjectInteractMode(overlappedControl->_objectId);
				if (!testInteractModeRange(interactMode)) {
					if (cursorData._mode == 3)
						_cursor->restoreInfo();
					_cursor->show(cursorControl);
					cursorControl->setActorIndexTo2();
					if (cursorData._overlappedObjectId != overlappedControl->_objectId) {
						cursorData._overlappedObjectId = overlappedControl->_objectId;
						runCause(cursorControl, cursorData, 0x1B0009, 0, overlappedControl->_objectId, 0);
					}
					if (interactMode == 10) {
						if (cursorData._holdingObjectId) {
							cursorData._verbState._verbId = 0x1B0003;
							cursorData._currOverlappedObjectId = overlappedControl->_objectId;
						}
						else {
							cursorData._verbState._verbId = 0x1B0002;
							cursorData._currOverlappedObjectId = overlappedControl->_objectId;
						}
					} else {
						playSoundEffect(3);
						showBubble(cursorControl->_objectId, overlappedControl->_objectId,
							cursorData._holdingObjectId, &cursorData._verbState,
							cursorData._progResKeywordId);
						cursorData._currOverlappedObjectId = overlappedControl->_objectId;
					}
				} else {
					if (cursorData._mode != 3) {
						_cursor->saveInfo();
						cursorData._mode = 3;
						cursorData._verbState._verbId = 0x1B0006;
						cursorData._holdingObjectId = 0;
					}
					cursorData._sequenceId = _cursor->getSequenceId1(interactMode);
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
				if (cursorData._verbState._isBubbleVisible)
					playSoundEffect(4);
				hideVerbBubble(cursorControl->_objectId, &cursorData._verbState);
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
		if (_vm->_input->pollEvent(kEventLeftClick)) {
			cursorData._idleCtr = 0;
			if (runCause(cursorControl, cursorData, cursorData._verbState._verbId, cursorData._holdingObjectId, cursorData._currOverlappedObjectId, 1)) {
				hideVerbBubble(cursorControl->_objectId, &cursorData._verbState);
				cursorData._currOverlappedObjectId = 0;
				cursorControl->setActorIndexTo1();
			}
		} else if (_vm->_input->pollEvent(kEventRightClick)) {
			uint32 verbId;
			cursorData._idleCtr = 0;
			if (cursorData._holdingObjectId) {
				runCause(cursorControl, cursorData, 0x1B000B, 0, 0x40003, 0);
				cursorData._currOverlappedObjectId = 0;
			} else if (findVerbId(&cursorData._verbState, cursorData._currOverlappedObjectId, 0, verbId) &&
				runCause(cursorControl, cursorData, verbId, cursorData._holdingObjectId, cursorData._currOverlappedObjectId, 1)) {
				hideVerbBubble(cursorControl->_objectId, &cursorData._verbState);
				cursorData._currOverlappedObjectId = 0;
				cursorControl->setActorIndexTo1();
			}
		}
	} else {
		if (_vm->_input->pollEvent(kEventLeftClick)) {
			cursorData._idleCtr = 0;
			runCause(cursorControl, cursorData, 0x1B0002, 0, 0x40003, 0);
		} else if (_vm->_input->pollEvent(kEventInventory)) {
			cursorData._idleCtr = 0;
			if (cursorData._verbState._minPriority <= 1)
				runCause(cursorControl, cursorData, cursorData._holdingObjectId != 0 ? 0x1B000B : 0x1B0004, 0, 0x40003, 0);
		}
	}

}

void BbdouSpecialCode::cursorCrosshairControlRoutine(Control *cursorControl, uint32 deltaTime) {

	static const struct ShooterAnim {
		uint32 objectId;
		uint32 sequenceIds1[8];
		uint32 sequenceIds2[8];
	} kShooterAnims[] = {
		{0x401C4,
		{0x60637, 0x60638, 0x60639, 0x6063A, 0x6063B, 0x6063C, 0x6063D, 0x6063E},
		{0x6063F, 0x60640, 0x60641, 0x60642, 0x60643, 0x60644, 0x60645, 0x60646}},
		{0x401C3,
		{0x6064A, 0x6064B, 0x6064C, 0x6064D, 0x6064E, 0x6064F, 0x60650, 0x60651},
		{0x60652, 0x60653, 0x60654, 0x60655, 0x60656, 0x60657, 0x60658, 0x60659}}
	};

	static const uint32 kShooterObjectIds[] = {
		0x401DF, 0x401E0, 0x401E1, 0x401E2,
		0x401E3, 0x401E4, 0x401E5, 0x401E6,
	};

	Actor *actor = cursorControl->_actor;
	CursorData &cursorData = _cursor->_data;

	if (cursorData._visibleCtr <= 0) {
		if (cursorData._currOverlappedObjectId || cursorData._mode == 3) {
			if (cursorData._mode == 3)
				_cursor->restoreInfo();
			cursorControl->setActorIndexTo1();
		}
		cursorData._currOverlappedObjectId = 0;
		return;
	}

	Common::Point screenCursorPos = _vm->_input->getCursorPosition();

	if (screenCursorPos != actor->_position) {
		actor->_position = screenCursorPos;
		int16 gridX = 8 * screenCursorPos.x / 640;
		if (gridX >= 8)
			gridX = 4;

		for (uint i = 0; i < 2; ++i) {
			const ShooterAnim &anim = kShooterAnims[i];
			Control *control2 = _vm->getObjectControl(anim.objectId);
			if (control2 && control2->_actor) {
				if (_shooterStatus[i].gridX != gridX && (!_shooterStatus[i].flag || !control2->_actor->_seqCodeIp)) {
					_shooterStatus[i].gridX = gridX;
					control2->_actor->_seqCodeIp = nullptr;
					control2->startSequenceActor(anim.sequenceIds1[gridX], 2, 0);
				}
			}
		}

	}

	Common::Point cursorPos = getBackgroundCursorPos(screenCursorPos);
	bool foundOverlapped = false;
	Control *overlappedControl = nullptr;

	if (cursorData._flags & 1)
		foundOverlapped = false;
	else {
		foundOverlapped = _vm->_controls->getOverlappedObjectAccurate(cursorControl, cursorPos,
			&overlappedControl, cursorData._verbState._minPriority);
	}

	if (foundOverlapped) {
		if (overlappedControl->_objectId != cursorData._currOverlappedObjectId) {
			hideVerbBubble(cursorControl->_objectId, &cursorData._verbState);
			int interactMode = _objectInteractModeMap.getObjectInteractMode(overlappedControl->_objectId);
			if (!testInteractModeRange(interactMode)) {
				if (cursorData._mode == 3)
					_cursor->restoreInfo();
				_cursor->show(cursorControl);
				cursorControl->setActorIndexTo2();
				if (overlappedControl->_objectId) {
					cursorData._verbState._verbId = 0x1B0003;
					cursorData._currOverlappedObjectId = overlappedControl->_objectId;
				} else {
					cursorData._verbState._verbId = 0x1B0002;
					cursorData._currOverlappedObjectId = overlappedControl->_objectId;
				}
			} else {
				if (cursorData._mode != 3) {
					_cursor->saveInfo();
					cursorData._mode = 3;
					cursorData._verbState._verbId = 0x1B0006;
					cursorData._holdingObjectId = 0;
				}
				switch (interactMode) {
				case 2:
					cursorData._sequenceId = 0x60010;
					break;
				case 3:
					cursorData._sequenceId = 0x60011;
					break;
				case 4:
					cursorData._sequenceId = 0x60012;
					break;
				case 5:
					cursorData._sequenceId = 0x60013;
					break;
				case 6:
					cursorData._sequenceId = 0x60015;
					break;
				case 7:
					cursorData._sequenceId = 0x60014;
					break;
				default:
					break;
				}
				_cursor->show(cursorControl);
				cursorData._currOverlappedObjectId = overlappedControl->_objectId;
			}
		}
	} else {
		if (cursorData._currOverlappedObjectId || cursorData._mode == 3) {
			if (cursorData._mode == 3)
				_cursor->restoreInfo();
			_cursor->show(cursorControl);
			cursorControl->setActorIndexTo1();
			hideVerbBubble(cursorControl->_objectId, &cursorData._verbState);
		}
		cursorData._currOverlappedObjectId = 0;
	}

	actor->_seqCodeValue1 = 100 * deltaTime;

	if (cursorData._currOverlappedObjectId) {

		if (_vm->_input->pollEvent(kEventLeftClick)) {

			uint32 outSceneId, outVerbId, outObjectId2, outObjectId;
			bool success = getShooterCause(_vm->getCurrentScene(),
				cursorData._verbState._verbId, cursorData._holdingObjectId, cursorData._currOverlappedObjectId,
				outSceneId, outVerbId, outObjectId2, outObjectId);

			uint index = (uint)_vm->getRandom(2);
			const ShooterAnim &anim = kShooterAnims[index];
			uint32 objectId = anim.objectId;
			int gridX = _shooterStatus[index].gridX;
			Control *gunControl = _vm->getObjectControl(objectId);
			if (gunControl) {
				_shooterStatus[index].flag = true;
				gunControl->startSequenceActor(anim.sequenceIds2[gridX], 2, 0);
			}
			Control *hitControl = _vm->getObjectControl(kShooterObjectIds[_shooterObjectIdIndex]);
			if (hitControl) {
				hitControl->setActorPosition(actor->_position);
				hitControl->startSequenceActor(0x6068D, 2, 0);
			}
			++_shooterObjectIdIndex;
			if (_shooterObjectIdIndex >= ARRAYSIZE(kShooterObjectIds))
				_shooterObjectIdIndex = 0;

			if (success) {
				_cursor->hide(cursorControl->_objectId);
				uint32 threadId = startCauseThread(cursorControl->_objectId, _vm->getCurrentScene(), outVerbId, outObjectId2, outObjectId);
				if (cursorData._field90) {
					_vm->_threads->killThread(cursorData._causeThreadId2);
					cursorData._field90 = 0;
				}
				cursorData._causeThreadId1 = _vm->causeTrigger(outSceneId, outVerbId, outObjectId2, outObjectId, threadId);
				cursorData._causeThreadId2 = cursorData._causeThreadId1;
				hideVerbBubble(cursorControl->_objectId, &cursorData._verbState);
				cursorData._currOverlappedObjectId = 0;
				cursorControl->setActorIndexTo1();
			}

		} else if (_vm->_input->pollEvent(kEventRightClick) && cursorData._verbState._isBubbleVisible && !cursorData._verbState._flag56) {
			// I don't think this is used; _isBubbleVisible seems to be always 0 here
			warning("Cursor function not implemented");
		}

	} else if (_vm->_input->pollEvent(kEventLeftClick)) {
		uint index = (uint)_vm->getRandom(2);
		const ShooterAnim &anim = kShooterAnims[index];
		uint32 objectId = anim.objectId;
		int gridX = _shooterStatus[index].gridX;
		Control *gunControl = _vm->getObjectControl(objectId);
		if (gunControl) {
			_shooterStatus[index].flag = true;
			gunControl->startSequenceActor(anim.sequenceIds2[gridX], 2, 0);
		}
		Control *hitControl = _vm->getObjectControl(kShooterObjectIds[_shooterObjectIdIndex]);
		if (hitControl) {
			hitControl->setActorPosition(actor->_position);
			hitControl->startSequenceActor(0x6068D, 2, 0);
		}
		++_shooterObjectIdIndex;
		if (_shooterObjectIdIndex >= ARRAYSIZE(kShooterObjectIds))
			_shooterObjectIdIndex = 0;

	}

}

bool BbdouSpecialCode::testVerbId(uint32 verbId, uint32 holdingObjectId, uint32 overlappedObjectId) {
	static const uint32 kVerbIdsEE[] = {0x001B0002, 0x001B0001, 0};
	static const uint32 kVerbIdsE9[] = {0x001B0005, 0};
	static const uint32 kVerbIdsE8[] = {0x001B0005, 0x001B0001, 0};
	static const uint32 kVerbIdsHE[] = {0x001B0003, 0x001B0001, 0};
	static const uint32 kVerbIdsH9[] = {0x001B0003, 0};
	static const uint32 kVerbIdsH8[] = {0x001B0003, 0x001B0001, 0};

	const uint32 *verbIds;
	int interactMode = _objectInteractModeMap.getObjectInteractMode(overlappedObjectId);

	if (holdingObjectId) {
		if (interactMode == 9)
			verbIds = kVerbIdsH9;
		else if (interactMode == 8)
			verbIds = kVerbIdsH8;
		else
			verbIds = kVerbIdsHE;
	} else {
		if (interactMode == 9)
			verbIds = kVerbIdsE9;
		else if (interactMode == 8)
			verbIds = kVerbIdsE8;
		else
			verbIds = kVerbIdsEE;
	}

	for (; *verbIds; ++verbIds) {
		if (*verbIds == verbId)
			return true;
	}
	return false;
}

bool BbdouSpecialCode::getCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId,
	uint32 &outVerbId, uint32 &outObjectId2, uint32 &outObjectId) {
	bool success = false;
	objectId2 = verbId != 0x1B0003 ? 0 : objectId2;
	if (_vm->causeIsDeclared(sceneId, verbId, objectId2, objectId)) {
		outVerbId = verbId;
		outObjectId2 = objectId2;
		outObjectId = objectId;
		success = true;
	} else if (objectId2 != 0 && _vm->causeIsDeclared(sceneId, 0x1B0008, 0, objectId)) {
		outVerbId = 0x1B0008;
		outObjectId2 = 0;
		outObjectId = objectId;
		success = true;
	} else if (_vm->causeIsDeclared(sceneId, verbId, objectId2, 0x40001)) {
		outVerbId = verbId;
		outObjectId2 = objectId2;
		outObjectId = 0x40001;
		success = true;
	} else if (objectId2 != 0 && _vm->causeIsDeclared(sceneId, 0x1B0008, 0, 0x40001)) {
		outVerbId = 0x1B0008;
		outObjectId2 = 0;
		outObjectId = 0x40001;
		success = true;
	}

	if (success) {
		//debug("getCause() -> %08X %08X %08X", outVerbId, outObjectId2, outObjectId);
	}

	return success;
}

bool BbdouSpecialCode::runCause(Control *cursorControl, CursorData &cursorData,
	uint32 verbId, uint32 objectId2, uint32 objectId, int soundIndex) {
	//debug("runCause(%08X, %08X, %08X)", verbId, objectId2, objectId);
	uint32 sceneId = _vm->getCurrentScene();
	uint32 outVerbId, outObjectId2, outObjectId;
	bool success = false;

	if (getCause(_vm->getCurrentScene(), verbId, objectId2, objectId, outVerbId, outObjectId2, outObjectId)) {
		sceneId = _vm->getCurrentScene();
		success = true;
	} else if (getCause(0x10003, verbId, objectId2, objectId, outVerbId, outObjectId2, outObjectId)) {
		sceneId = 0x10003;
		success = true;
	}

	if (!success)
		return false;

	_cursor->hide(cursorControl->_objectId);

	uint32 threadId = startCauseThread(cursorControl->_objectId, _vm->getCurrentScene(), outVerbId, outObjectId2, outObjectId);

	if (cursorData._field90) {
		_vm->_threads->killThread(cursorData._causeThreadId2);
		cursorData._field90 = 0;
	}

	if (soundIndex)
		playSoundEffect(soundIndex);

	cursorData._causeThreadId1 = _vm->causeTrigger(sceneId, outVerbId, outObjectId2, outObjectId, threadId);
	cursorData._causeThreadId2 = cursorData._causeThreadId1;

	return true;
}

uint32 BbdouSpecialCode::startCauseThread(uint32 cursorObjectId, uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId) {
	uint32 tempThreadId = _vm->newTempThreadId();
	debug(3, "Starting cause thread %08X...", tempThreadId);
	CauseThread_BBDOU *causeThread = new CauseThread_BBDOU(_vm, tempThreadId, 0, this,
		cursorObjectId, sceneId, verbId, objectId2, objectId);
	_vm->_threads->startThread(causeThread);
	causeThread->suspend();
	return tempThreadId;
}

void BbdouSpecialCode::startHoldingObjectId(uint32 objectId1, uint32 holdingObjectId, bool doPlaySound) {
	Control *control = _vm->_dict->getObjectControl(objectId1);
	if (_cursor->_data._holdingObjectId)
		_inventory->putBackInventoryItem(_cursor->_data._holdingObjectId, control->_actor->_position);
	_cursor->_data._holdingObjectId = holdingObjectId;
	_cursor->_data._sequenceId = _cursor->findCursorSequenceId(holdingObjectId);
	if (_cursor->_data._visibleCtr > 0)
		_cursor->show(control);
	_cursor->_data._mode = 2;
	_cursor->_data._verbState._verbId = 0x1B0003;
	if (!doPlaySound)
		playSoundEffect(5);
	_inventory->removeInventoryItem(holdingObjectId);
}

void BbdouSpecialCode::stopHoldingObjectId(uint32 objectId1, bool doPlaySound) {
	Control *control = _vm->_dict->getObjectControl(objectId1);
	uint32 holdingObjectId = _cursor->_data._holdingObjectId;
	_cursor->_data._holdingObjectId = 0;
	_cursor->_data._sequenceId = 0x6000F;
	if (!doPlaySound && holdingObjectId)
		playSoundEffect(6);
	if (_cursor->_data._visibleCtr > 0)
		_cursor->show(control);
	_cursor->_data._verbState._verbId = 0x1B0001;
	if (_cursor->_data._mode == 3)
		holdingObjectId = _cursor->_data._holdingObjectId2;
	if (holdingObjectId)
		_inventory->putBackInventoryItem(holdingObjectId, control->_actor->_position);
}

bool BbdouSpecialCode::isHoldingObjectId(uint32 objectId) {
	return _cursor->_data._holdingObjectId == objectId;
}

void BbdouSpecialCode::initSalad() {
	for (uint i = 0; i < 12; ++i) {
		_saladObjectIds[i] = _vm->_controls->newTempObjectId();
		_vm->_controls->placeActor(0x00050192, Common::Point(0, 0), 0x00060C26, _saladObjectIds[i], 0);
	}
	_saladCount = 0;
}

void BbdouSpecialCode::addSalad(uint32 sequenceId) {
	if (_saladCount >= 12) {
		Control *control = _vm->_dict->getObjectControl(_saladObjectIds[_saladCount - 1]);
		control->unlinkObject();
	} else {
		++_saladCount;
	}
	Control *control = _vm->_dict->getObjectControl(_saladObjectIds[_saladCount - 1]);
	control->linkToObject(0x00040309, _saladCount);
	control->startSequenceActor(sequenceId, 2, 0);
	control->setPriority(_saladCount + 9);
	control->deactivateObject();
}

bool BbdouSpecialCode::getShooterCause(uint32 sceneId, uint32 verbId, uint32 objectId2, uint32 objectId,
	uint32 &outSceneId, uint32 &outVerbId, uint32 &outObjectId2, uint32 &outObjectId) {
	bool success = false;
	objectId2 = verbId != 0x1B0003 ? 0 : objectId2;
	if (_vm->causeIsDeclared(sceneId, verbId, objectId2, objectId)) {
		outSceneId = sceneId;
		outVerbId = verbId;
		outObjectId2 = objectId2;
		outObjectId = objectId;
		success = true;
	} else if (verbId == 0x1B0003 && _vm->causeIsDeclared(sceneId, 0x1B0008, 0, objectId)) {
		outSceneId = sceneId;
		outVerbId = 0x1B0003;
		outObjectId2 = 0;
		outObjectId = objectId;
		success = true;
	} else if (_vm->causeIsDeclared(sceneId, verbId, objectId2, 0x40001)) {
		outSceneId = sceneId;
		outVerbId = verbId;
		outObjectId2 = objectId2;
		outObjectId = 0x40001;
		success = true;
	} else if (verbId == 0x1B0003 && _vm->causeIsDeclared(sceneId, 0x1B0008, 0, 0x40001)) {
		outSceneId = sceneId;
		outVerbId = 0x1B0008;
		outObjectId2 = 0;
		outObjectId = 0x40001;
		success = true;
	} else if (_vm->causeIsDeclared(0x10003, verbId, objectId2, objectId)) {
		outSceneId = 0x10003;
		outVerbId = verbId;
		outObjectId2 = objectId2;
		outObjectId = objectId;
		success = true;
	} else if (verbId == 0x1B0003 && _vm->causeIsDeclared(0x10003, 0x1B0008, 0, objectId)) {
		outSceneId = 0x10003;
		outVerbId = verbId;
		outObjectId2 = 0;
		outObjectId = objectId;
		success = true;
	} else if (_vm->causeIsDeclared(0x10003, verbId, objectId2, 0x40001)) {
		outSceneId = 0x10003;
		outVerbId = verbId;
		outObjectId2 = objectId2;
		outObjectId = 0x40001;
		success = true;
	} else if (verbId == 0x1B0003 && _vm->causeIsDeclared(0x10003, 0x1B0008, 0, 0x40001)) {
		outSceneId = 0x10003;
		outVerbId = verbId;
		outObjectId2 = 0;
		outObjectId = 0x40001;
		success = true;
	}
	return success;
}

} // End of namespace Illusions
