/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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

#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/sound.h"
#include "buried/scene_view.h"
#include "buried/environ/scene_common.h"

namespace Buried {

BasicDoor::BasicDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int timeZone, int environment, int node, int facing,
		int orientation, int depth, int transitionType, int transitionData,
		int transitionStartFrame, int transitionLength, int openingSoundID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clicked = false;

	_clickable = Common::Rect(left, top, right, bottom);

	_destData.destinationScene.timeZone = timeZone;
	_destData.destinationScene.environment = environment;
	_destData.destinationScene.node = node;
	_destData.destinationScene.facing = facing;
	_destData.destinationScene.orientation = orientation;
	_destData.destinationScene.depth = depth;

	_destData.transitionType = transitionType;
	_destData.transitionData = transitionData;
	_destData.transitionStartFrame = transitionStartFrame;
	_destData.transitionLength = transitionLength;

	_openingSoundID = openingSoundID;
}

int BasicDoor::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickable.contains(pointLocation))
		_clicked = true;

	return SC_TRUE;
}

int BasicDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clicked) {
		_clicked = false;

		if (_openingSoundID >= 0)
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _openingSoundID));

		if (_clickable.contains(pointLocation))
			((SceneViewWindow *)viewWindow)->moveToDestination(_destData);		
	}

	return SC_TRUE;
}

int BasicDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickable.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

TurnDepthPreChange::TurnDepthPreChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int flagOffset, int upDepth, int leftDepth, int rightDepth, int downDepth, int forwardDepth) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(flagOffset)) {
		if (upDepth >= 0)
			_staticData.destUp.destinationScene.depth = upDepth;

		if (leftDepth >= 0)
			_staticData.destUp.destinationScene.depth = leftDepth;

		if (rightDepth >= 0)
			_staticData.destUp.destinationScene.depth = rightDepth;

		if (downDepth >= 0)
			_staticData.destUp.destinationScene.depth = downDepth;

		if (forwardDepth >= 0)
			_staticData.destUp.destinationScene.depth = forwardDepth;
	}
}

GenericItemAcquire::GenericItemAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int itemID, int clearStillFrame, int itemFlagOffset) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_itemPresent = true;
	_itemID = itemID;
	_acquireRegion = Common::Rect(left, top, right, bottom);
	_fullFrameIndex = sceneStaticData.navFrameIndex;
	_clearFrameIndex = clearStillFrame;
	_itemFlagOffset = itemFlagOffset;

	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_itemFlagOffset) != 0) {
		_itemPresent = false;
		_staticData.navFrameIndex = _clearFrameIndex;
	}
}

int GenericItemAcquire::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_acquireRegion.contains(pointLocation) && _itemPresent) {
		_itemPresent = false;
		_staticData.navFrameIndex = _clearFrameIndex;

		if (_itemFlagOffset >= 0)
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_itemFlagOffset, 1);

		// Call inventory drag start function
		Common::Point ptInventoryWindow = viewWindow->convertPointToGlobal(pointLocation);
		ptInventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->convertPointToLocal(ptInventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(_itemID, ptInventoryWindow);

		// Update the biochips
		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();

		return SC_TRUE;
	}

	return SC_FALSE;
}

int GenericItemAcquire::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;

	if (itemID == _itemID && !_itemPresent) {
		// Redraw the background
		_itemPresent = true;
		_staticData.navFrameIndex = _fullFrameIndex;

		if (_itemFlagOffset >= 0)
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_itemFlagOffset, 0);

		viewWindow->invalidateWindow();

		// Update the biochips
		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int GenericItemAcquire::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_acquireRegion.contains(pointLocation) && _itemPresent)
		return kCursorOpenHand;

	return kCursorArrow;
}

PlaySoundExitingFromScene::PlaySoundExitingFromScene(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int soundFileNameID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_soundFileNameID = soundFileNameID;
}

int PlaySoundExitingFromScene::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_soundFileNameID >= 0 && _staticData.location.depth != newLocation.depth && _staticData.location.timeZone == newLocation.timeZone)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundFileNameID), 128, false, true);

	return SC_TRUE;
}

PlaySoundExitingFromSceneDeux::PlaySoundExitingFromSceneDeux(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int soundFileNameID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_soundFileNameID = soundFileNameID;
}

int PlaySoundExitingFromSceneDeux::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_soundFileNameID >= 0 && _staticData.location.node == newLocation.node && _staticData.location.timeZone == newLocation.timeZone)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundFileNameID), 128, false, true);

	return SC_TRUE;
}

PlayStingers::PlayStingers(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int stingerVolume, int lastStingerFlagOffset, int effectIDFlagOffset, int firstStingerFileID, int lastStingerFileID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_stingerVolume = stingerVolume;
	_lastStingerFlagOffset = lastStingerFlagOffset;
	_effectIDFlagOffset = effectIDFlagOffset;
	_firstStingerFileID = firstStingerFileID;
	_lastStingerFileID = lastStingerFileID;
}

int PlayStingers::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_effectIDFlagOffset >= 0) {
		// More evil.
		byte effectID = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_effectIDFlagOffset);

		if (!_vm->_sound->isSoundEffectPlaying(effectID - 1)) {
			byte lastStinger = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_lastStingerFlagOffset);
			lastStinger++;

			uint32 fileNameIndex = _vm->computeFileNameResourceID(_staticData.location.timeZone, _staticData.location.environment, _firstStingerFileID + lastStinger - 1);
			byte newStingerID = _vm->_sound->playSoundEffect(_vm->getFilePath(fileNameIndex), _stingerVolume, false, true) + 1;

			if (lastStinger > _lastStingerFileID - _firstStingerFileID)
				lastStinger = 0;

			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_effectIDFlagOffset, newStingerID);
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_lastStingerFlagOffset, lastStinger);
		}
	}

	return SC_TRUE;
}

ClickPlaySound::ClickPlaySound(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int flagOffset, int soundID, int cursorID, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_cursorID = cursorID;
	_soundID = soundID;
	_clickRegion = Common::Rect(left, top, right, bottom);
	_flagOffset = flagOffset;
}

int ClickPlaySound::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundID), 127, false, true);

		if (_flagOffset >= 0)
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 1);

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickPlaySound::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

DisplayMessageWithEvidenceWhenEnteringNode::DisplayMessageWithEvidenceWhenEnteringNode(BuriedEngine *vm, Window *viewWindow,
			const LocationStaticData &sceneStaticData, const Location &priorLocation, int evidenceID, int messageBoxTextID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_evidenceID = evidenceID;
	_messageBoxTextID = messageBoxTextID;
}

int DisplayMessageWithEvidenceWhenEnteringNode::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if ((_staticData.location.timeZone != priorLocation.timeZone ||
			_staticData.location.environment != priorLocation.environment ||
			_staticData.location.node != priorLocation.node ||
			_staticData.location.facing != priorLocation.facing ||
			_staticData.location.orientation != priorLocation.orientation ||
			_staticData.location.depth != priorLocation.depth) &&
			!((SceneViewWindow *)viewWindow)->isNumberInGlobalFlagTable(offsetof(GlobalFlags, evcapBaseID), offsetof(GlobalFlags, evcapNumCaptured), _evidenceID)) {
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_messageBoxTextID));
	}

	return SC_TRUE;
}

OneShotEntryVideoWarning::OneShotEntryVideoWarning(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animID, int flagOffset, int warningMessageID) : SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_animID = animID;
	_flagOffset = flagOffset;
	_warningMessageID = warningMessageID;
}

int OneShotEntryVideoWarning::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_flagOffset) == 0) {
		if (_warningMessageID >= 0)
			((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_warningMessageID));

		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animID);
		((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 1);
	}

	return SC_TRUE;
}

CycleEntryVideoWarning::CycleEntryVideoWarning(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animIDA, int animIDB, int flagOffset, int warningMessageID) : SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_animIDA = animIDA;
	_animIDB = animIDB;
	_flagOffset = flagOffset;
	_warningMessageID = warningMessageID;
}

int CycleEntryVideoWarning::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_warningMessageID >= 0)
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_warningMessageID));

	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_flagOffset) == 0) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animIDA);
		((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 1);
	} else {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animIDB);
		((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 0);
	}

	return SC_TRUE;
}

ClickPlayVideo::ClickPlayVideo(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int animID, int cursorID, int left, int top, int right, int bottom)
		: SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	
	_cursorID = cursorID;
	_animID = animID;
	_clickRegion = Common::Rect(left, top, right, bottom);
}

int ClickPlayVideo::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animID);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickPlayVideo::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

VideoDeath::VideoDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation, int deathID, int messageTextID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation), _deathID(deathID), _messageTextID(messageTextID) {
}

int VideoDeath::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_messageTextID >= -1)
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(_messageTextID));

	return SC_TRUE;
}

int VideoDeath::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (newLocation.timeZone == _staticData.location.timeZone &&
			newLocation.environment == _staticData.location.environment &&
			newLocation.node == _staticData.location.node &&
			newLocation.facing == _staticData.location.facing &&
			newLocation.orientation == _staticData.location.orientation &&
			newLocation.depth == _staticData.location.depth) {
		// Notify the player of his gruesome death
		((SceneViewWindow *)viewWindow)->showDeathScene(_deathID);
		return SC_DEATH;
	}

	return SC_TRUE;
}

ClickChangeDepth::ClickChangeDepth(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int newDepth, int cursorID, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_newDepth = newDepth;
	_cursorID = cursorID;
	_clickableRegion = Common::Rect(left, top, right, bottom);
}

int ClickChangeDepth::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableRegion.contains(pointLocation)) {
		DestinationScene clickDestination;
		clickDestination.destinationScene = _staticData.location;
		clickDestination.destinationScene.depth = _newDepth;
		clickDestination.transitionType = TRANSITION_FADE;
		clickDestination.transitionData = -1;
		clickDestination.transitionStartFrame = -1;
		clickDestination.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(clickDestination);
	}

	return SC_TRUE;
}

int ClickChangeDepth::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

ClickPlaySoundSynchronous::ClickPlaySoundSynchronous(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int flagOffset, int soundID, int cursorID, int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_cursorID = cursorID;
	_soundID = soundID;
	_clickRegion = Common::Rect(left, top, right, bottom);
	_flagOffset = flagOffset;
}

int ClickPlaySoundSynchronous::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _soundID), 127);

		if (_flagOffset >= 0)
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 1);

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ClickPlaySoundSynchronous::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

} // End of namespace Buried
