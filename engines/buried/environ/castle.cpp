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
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

#include "common/system.h"

namespace Buried {

class TopOfTowerGuardEncounter : public SceneBase {
public:
	TopOfTowerGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int paint(Window *viewWindow, Graphics::Surface *preBuffer);

private:
	bool _showGuard;
};

TopOfTowerGuardEncounter::TopOfTowerGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_showGuard = _staticData.location.timeZone != priorLocation.timeZone || _staticData.location.environment != priorLocation.environment;

	if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBloodyArrow))
		_staticData.destForward.destinationScene.depth = 1;		
}

int TopOfTowerGuardEncounter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_showGuard) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(0);
		_showGuard = false;
		viewWindow->invalidateWindow(false);
	}

	return SC_TRUE;
}

int TopOfTowerGuardEncounter::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	if (!_showGuard)
		return SceneBase::paint(viewWindow, preBuffer);

	const Graphics::Surface *newFrame = ((SceneViewWindow *)viewWindow)->getStillFrame(_staticData.miscFrameIndex);

	if (newFrame) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		_vm->_gfx->crossBlit(preBuffer, 0, 0, 432, 189, newFrame, 0, 0);
	}

	return SC_REPAINT;
}

class TowerStairsGuardEncounter : public SceneBase {
public:
	TowerStairsGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int timerCallback(Window *viewWindow);

private:
	bool _busy;
};

TowerStairsGuardEncounter::TowerStairsGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_busy = false;
}

int TowerStairsGuardEncounter::timerCallback(Window *viewWindow) {
	if (_frameCycleCount < 0 || _busy)
		return SC_FALSE;

	if (_frameCycleCount < _staticData.cycleStartFrame + _staticData.cycleFrameCount - 1) {
		_frameCycleCount++;
		viewWindow->invalidateWindow(false);
	} else {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 0) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(0);
			_busy = true;
			((SceneViewWindow *)viewWindow)->showDeathScene(0);
			return SC_DEATH;
		} else {
			_frameCycleCount = _staticData.cycleStartFrame;
			_busy = false;
		}
	}

	return SC_TRUE;
}

class WallSlideDeath : public SceneBase {
public:
	WallSlideDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation);	
};

WallSlideDeath::WallSlideDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int WallSlideDeath::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (newLocation.timeZone == _staticData.location.timeZone &&
			newLocation.environment == _staticData.location.environment &&
			newLocation.node == _staticData.location.node &&
			newLocation.facing == _staticData.location.facing &&
			newLocation.orientation == _staticData.location.orientation &&
			newLocation.depth == _staticData.location.depth) {
		// Notify the player of his gruesome death
		((SceneViewWindow *)viewWindow)->showDeathScene(1);
		return SC_DEATH;
	}

	return SC_TRUE;
}

enum {
	CATAPULT_TIMEOUT_VALUE = 6000
};

class ExplodingWallSafeDistance : public SceneBase {
public:
	ExplodingWallSafeDistance(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int timerCallback(Window *viewWindow);

private:
	bool _timerStarted;
	uint32 _startTime;
	bool _walkthrough;
};

ExplodingWallSafeDistance::ExplodingWallSafeDistance(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_timerStarted = false;

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultData == 0) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultData = g_system->getMillis();
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13), 127, false, true);
	}

	_walkthrough = ((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1;
}

int ExplodingWallSafeDistance::timerCallback(Window *viewWindow) {
	uint32 timer = ((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultData;
	if (_walkthrough || (timer + CATAPULT_TIMEOUT_VALUE < g_system->getMillis())) {
		// Play the explosion movie
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(2);

		// Change the background bitmap
		_staticData.navFrameIndex = 139;
		viewWindow->invalidateWindow(false);

		// Reset the safety flag
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgWallExploded = 1;

		// Move to our depth 1 counterpart
		Location newLocation = _staticData.location;
		newLocation.depth = 1;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
	}

	return SC_TRUE;
}

class ExplodingWallDeath : public SceneBase {
public:
	ExplodingWallDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int timerCallback(Window *viewWindow);
};

ExplodingWallDeath::ExplodingWallDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int ExplodingWallDeath::timerCallback(Window *viewWindow) {
	// clone2727 asks why this is a timer callback

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().cgWallExploded == 0) {
		// Play the explosion movie
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(3);

		// Notify the player of his gruesome death
		((SceneViewWindow *)viewWindow)->showDeathScene(2);
		return SC_DEATH;
	}

	return SC_TRUE;
}

class KeepInitialWallClimb : public SceneBase {
public:
	KeepInitialWallClimb(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);

private:
	Common::Rect _windowRect;
};

KeepInitialWallClimb::KeepInitialWallClimb(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_windowRect = Common::Rect(176, 40, 256, 80);
}

int KeepInitialWallClimb::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;

	if (_windowRect.contains(pointLocation) && itemID == kItemGrapplingHook) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_vm->isDemo() ? 3 : 1);

		DestinationScene newDest;
		newDest.destinationScene = _staticData.location;
		newDest.destinationScene.depth = 1;
		newDest.transitionType = TRANSITION_VIDEO;
		newDest.transitionData = _vm->isDemo() ? 7 : 4;
		newDest.transitionStartFrame = -1;
		newDest.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(newDest);

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int KeepInitialWallClimb::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (_windowRect.contains(pointLocation) && itemID == kItemGrapplingHook)
		return 1;

	return 0;
}

class KeepFinalWallClimb : public SceneBase {
public:
	KeepFinalWallClimb(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int preExitRoom(Window *viewWindow, const Location &priorLocation);
	int timerCallback(Window *viewWindow);

private:
	bool _exitStarted;
	uint32 _startTime;
};

KeepFinalWallClimb::KeepFinalWallClimb(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_exitStarted = false;
	_startTime = 0;
}

int KeepFinalWallClimb::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_startTime = g_system->getMillis();

	// Change the message for the demo
	if (_vm->isDemo())
		((SceneViewWindow *)viewWindow)->displayLiveText("What happens next?\n\nCall 1-800-943-3664 to order Buried in Time.");

	return SC_TRUE;
}

int KeepFinalWallClimb::preExitRoom(Window *viewWindow, const Location &priorLocation) {
	_exitStarted = true;
	return SC_TRUE;
}

int KeepFinalWallClimb::timerCallback(Window *viewWindow) {
	if (_exitStarted)
		return SC_TRUE;

	if (g_system->getMillis() > _startTime + (_vm->isDemo() ? 10000 : 8000)) {
		if (_vm->isDemo()) {
			// Return to the main menu
			((FrameWindow *)viewWindow->getParent()->getParent())->returnToMainMenu();
		} else {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(2);
			((SceneViewWindow *)viewWindow)->showDeathScene(3);
			return SC_DEATH;
		}
	}

	return SC_TRUE;
}

class StorageRoomDoor : public SceneBase {
public:
	StorageRoomDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int timeZone = -1, int environment = -1, int node = -1,
			int facing = -1, int orientation = -1, int depth = -1, int flagOffset = 0, int data = -1, int startFrame = -1,
			int length = -1, int animDB = -1);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	bool _clicked;
	Common::Rect _clickable;
	int _flagOffset;
	DestinationScene _destData;
	int _agent3VideoID;
};

StorageRoomDoor::StorageRoomDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int timeZone, int environment, int node,
		int facing, int orientation, int depth, int flagOffset, int data, int startFrame,
		int length, int animDB) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_agent3VideoID = animDB;
	_clicked = false;
	_flagOffset = flagOffset;
	_clickable = Common::Rect(left, top, right, bottom);

	_destData.destinationScene.timeZone = timeZone;
	_destData.destinationScene.environment = environment;
	_destData.destinationScene.node = node;
	_destData.destinationScene.facing = facing;
	_destData.destinationScene.orientation = orientation;
	_destData.destinationScene.depth = depth;

	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(flagOffset) != 0) {
		_destData.transitionType = 2; // constant?
		_destData.transitionData = data;
		_destData.transitionStartFrame = startFrame;
		_destData.transitionLength = length;
	} else {
		_destData.transitionType = TRANSITION_VIDEO;
		_destData.transitionData = _agent3VideoID;
		_destData.transitionStartFrame = -1;
		_destData.transitionLength = -1;
	}
}

int StorageRoomDoor::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickable.contains(pointLocation))
		_clicked = true;

	return SC_TRUE;
}

int StorageRoomDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clicked) {
		if (_clickable.contains(pointLocation)) {
			((SceneViewWindow *)viewWindow)->moveToDestination(_destData);
		} else {
			_clicked = false;
		}

		if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_flagOffset) == 0) {
			if (((SceneViewWindow *)viewWindow)->addNumberToGlobalFlagTable(offsetof(GlobalFlags, evcapBaseID), offsetof(GlobalFlags, evcapNumCaptured), 12, CASTLE_EVIDENCE_AGENT3))
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ACQUIRED));
			else
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagOffset, 1);
		}
	}

	return SC_TRUE;
}

int StorageRoomDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickable.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class KingsChamberGuardEncounter : public SceneBase {
public:
	KingsChamberGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int timerCallback(Window *viewWindow);

private:
	uint32 _startingTime;
	bool _finished;
};

KingsChamberGuardEncounter::KingsChamberGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_startingTime = 0;
}

int KingsChamberGuardEncounter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Start the audio cue playing and store the starting time
	_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14), 127, false, true);
	_startingTime = g_system->getMillis();
	return SC_TRUE;
}

int KingsChamberGuardEncounter::timerCallback(Window *viewWindow) {
	SceneBase::timerCallback(viewWindow);

	if (_startingTime + 16000 < g_system->getMillis()) {
		// Check the cloak biochip status
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 1) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(1);

			// Jump to next scene depth
			DestinationScene newDest;
			newDest.destinationScene = _staticData.location;
			newDest.destinationScene.depth = 0;
			newDest.transitionType = TRANSITION_NONE;
			newDest.transitionData = -1;
			newDest.transitionStartFrame = -1;
			newDest.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->jumpToScene(newDest.destinationScene);
		} else {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(0);
			((SceneViewWindow *)viewWindow)->showDeathScene(4);
			return SC_DEATH;
		}
	}

	return SC_TRUE;
}

bool SceneViewWindow::initializeCastleTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	// If we passed -1, initialize time zone, otherwise the environment
	if (environment == -1) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgWallExploded = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgHookPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemGrapplingHook) ? 1 : 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgArrowPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBloodyArrow) ? 1 : 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgHammerPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemHammer) ? 1 : 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyGuard = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgBaileyOneWayGuard = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgBaileyTwoWayGuards = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgTapestryFlag = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgStorageRoomVisit = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgBurnedLetterPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBurnedLetter) ? 1 : 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgGoldCoinsPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemGoldCoins) ? 1 : 0;

		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1) {
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = 6;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgTapestryFlag = 1;
		}
	}

	// Environment-specific
	if (environment == 4) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEnteredKeep = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMBVisited = 1;
	} else if (environment == 6) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgKCVisited = 1;
	} else if (environment == 10) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEnteredTreasureRoom = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgTRVisited = 1;
	}

	return true;
}

bool SceneViewWindow::startCastleAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	if (_vm->isDemo())
		return false;

	_vm->_sound->setAmbientSound(_vm->getFilePath(1, environment, SF_AMBIENT), fade, 64);
	return true;
}

SceneBase *SceneViewWindow::constructCastleSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 1:
		return new TopOfTowerGuardEncounter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 3:
		return new TowerStairsGuardEncounter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 4:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 114, 0, 324, 189, 1, 2, 5, 3, 1, 1, 2, 11, 395, 9);
	case 5:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 103, 0, 355, 189, 1, 3, 5, 1, 1, 1, 2, 11, 641, 8);
	case 6:
		return new WallSlideDeath(_vm, viewWindow, sceneStaticData, priorLocation);
	case 7:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgWallExploded), 0, 0, 1, 0, 0);
	case 8:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgWallExploded), 0, 1, 0, 0, 0);
	case 9:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 81, 25, 360, 189, 1, 4, 2, 1, 1, 1, 2, 11, 413, 25);
	case 10:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 24, 5, 415, 189, 1, 5, 0, 2, 1, 1, 2, 11, 72, 22);
	case 12:
		return new ExplodingWallSafeDistance(_vm, viewWindow, sceneStaticData, priorLocation);
	case 13:
		return new ExplodingWallDeath(_vm, viewWindow, sceneStaticData, priorLocation);
	case 14:
		return new KeepInitialWallClimb(_vm, viewWindow, sceneStaticData, priorLocation);
	case 15:
		return new KeepFinalWallClimb(_vm, viewWindow, sceneStaticData, priorLocation);
	case 16:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 131, 18, 322, 189, 1, 8, 10, 1, 1, 1, 2, 11, 307, 7);
	case 17:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 42, 0, 357, 189, 1, 8, 11, 3, 1, 1, 2, 11, 314, 7);
	case 18:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 26, 0, 432, 189, 1, 8, 6, 3, 1, 1, 2, 11, 288, 8);
	case 20:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHookPresent), 0, 0, 1, 0, 0);
	case 21:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHookPresent), 0, 1, 0, 0, 0);
	case 22:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 163, 83, 236, 162, kItemGrapplingHook, 75, offsetof(GlobalFlags, cgHookPresent));
	case 23:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgArrowPresent), 0, 0, 1, 0, 0);
	case 24:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgArrowPresent), 0, 1, 0, 0, 0);
	case 25:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 241, 7, 284, 92, kItemBloodyArrow, 57, offsetof(GlobalFlags, cgArrowPresent));
	case 26:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHammerPresent), 0, 0, 1, 0, 0);
		break;
	case 27:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHammerPresent), 0, 1, 0, 0, 0);
		else
			return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 28:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 184, 111, 237, 189, kItemHammer, 3, offsetof(GlobalFlags, cgHammerPresent));
	case 29:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHammerPresent), 0, 0, 0, 1, 0);
		break;
	case 30:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgHookPresent), 0, 0, 0, 1, 0);
	case 31:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 1, kCursorFinger, 51, 25, 149, 149);
	case 32:
		return new OneShotEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, 0, offsetof(GlobalFlags, cgSmithyGuard), IDS_HUMAN_PRESENCE_10METERS);
	case 33:
		return new OneShotEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, _vm->isDemo() ? 2 : 6, offsetof(GlobalFlags, cgBaileyOneWayGuard), IDS_HUMAN_PRESENCE_10METERS);
	case 34:
		return new CycleEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, _vm->isDemo() ? 5 : 7, _vm->isDemo() ? 6 : 8, offsetof(GlobalFlags, cgBaileyTwoWayGuards), IDS_HUMAN_PRESENCE_10METERS);
	case 37:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 175, 64, 237, 126, kItemBurnedLetter, 84, offsetof(GlobalFlags, cgBurnedLetterPresent));
	case 39:
		return new StorageRoomDoor(_vm, viewWindow, sceneStaticData, priorLocation, 38, 0, 386, 189, 1, 9, 5, 2, 1, 1, offsetof(GlobalFlags, cgStorageRoomVisit), 11, 130, 12, 0);
	case 47:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 2, kCursorFinger, 0, 75, 258, 123);
	case 48:
		return new KingsChamberGuardEncounter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 50:
		return new ClickPlaySoundSynchronous(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgTSTriedDoorA), 14, kCursorFinger, 72, 0, 372, 189);
	case 55:
		// Valid, but not implemented.
		break;
	case 57:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 58:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 70:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 12);
	case 71:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 72:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, -1, 12, kCursorFinger, 28, 34, 336, 189);
	case 73:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, -1, 13, kCursorFinger, 0, 0, 270, 189);
	case 74:
		return new PlaySoundExitingFromSceneDeux(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 77:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, cgTSTriedDoorB), 14, kCursorFinger, 72, 0, 372, 189);
	default:
		warning("TODO: Castle scene object %d", sceneStaticData.classID);
		break;
	}

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
