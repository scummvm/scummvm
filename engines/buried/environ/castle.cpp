/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
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
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int paint(Window *viewWindow, Graphics::Surface *preBuffer) override;

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

	if (newFrame)
		_vm->_gfx->crossBlit(preBuffer, 0, 0, 432, 189, newFrame, 0, 0);

	return SC_REPAINT;
}

class TowerStairsGuardEncounter : public SceneBase {
public:
	TowerStairsGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;
	int timerCallback(Window *viewWindow) override;

private:
	bool _busy;
};

TowerStairsGuardEncounter::TowerStairsGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_busy = false;
}

int TowerStairsGuardEncounter::timerCallback(Window *viewWindow) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_frameCycleCount < 0 || _busy)
		return SC_FALSE;

	if (_frameCycleCount < _staticData.cycleStartFrame + _staticData.cycleFrameCount - 1) {
		_frameCycleCount++;
		viewWindow->invalidateWindow(false);
	} else {
		if (globalFlags.bcCloakingEnabled == 0) {
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

int TowerStairsGuardEncounter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Force enable frame cycling
	((SceneViewWindow *)viewWindow)->forceEnableCycling(true);

	// Disable frame caching
	((SceneViewWindow *)viewWindow)->enableCycleFrameCache(false);

	// Force open the video
	((SceneViewWindow *)viewWindow)->changeCycleFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_CYCLES));
	return SC_TRUE;
}

int TowerStairsGuardEncounter::preExitRoom(Window *viewWindow, const Location &newLocation) {
	((SceneViewWindow *)viewWindow)->forceEnableCycling(false);
	return SC_TRUE;
}

class WallSlideDeath : public SceneBase {
public:
	WallSlideDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
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
	int timerCallback(Window *viewWindow) override;

private:
	bool _timerStarted;
	//uint32 _startTime;
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
	int timerCallback(Window *viewWindow) override;
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
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;

private:
	Common::Rect _windowRect;
};

KeepInitialWallClimb::KeepInitialWallClimb(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_windowRect = Common::Rect(176, 40, 256, 80);
}

int KeepInitialWallClimb::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

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
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &priorLocation) override;
	int timerCallback(Window *viewWindow) override;

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

class KingsStudyGuard : public SceneBase {
public:
	KingsStudyGuard(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int preExitRoom(Window *viewWindow, const Location &priorLocation) override;
	int postExitRoom(Window *viewWindow, const Location &priorLocation) override;
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int timerCallback(Window *viewWindow) override;
};

KingsStudyGuard::KingsStudyGuard(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int KingsStudyGuard::preExitRoom(Window *viewWindow, const Location &newLocation) {
	((SceneViewWindow *)viewWindow)->forceEnableCycling(false);
	return SC_TRUE;
}

int KingsStudyGuard::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_staticData.location.timeZone == newLocation.timeZone)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14));
	return SC_TRUE;
}

int KingsStudyGuard::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Force enable frame cycling
	((SceneViewWindow *)viewWindow)->forceEnableCycling(true);

	// Disable frame caching
	((SceneViewWindow *)viewWindow)->enableCycleFrameCache(false);

	// Force open the video
	((SceneViewWindow *)viewWindow)->changeCycleFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_CYCLES));

	// Display warning
	((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_HUMAN_PRESENCE_3METERS));
	return SC_TRUE;
}

int KingsStudyGuard::timerCallback(Window *viewWindow) {
	if (_frameCycleCount < _staticData.cycleStartFrame + _staticData.cycleFrameCount - 1) {
		_frameCycleCount++;
		viewWindow->invalidateWindow(false);
	} else {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 0) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(0);
			((SceneViewWindow *)viewWindow)->showDeathScene(5);
			return SC_DEATH;
		} else {
			_frameCycleCount = _staticData.cycleStartFrame;
		}
	}

	return SC_TRUE;
}

class SmithyBench : public SceneBase {
public:
	SmithyBench(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	byte _status;
	Common::Rect _pan;
	Common::Rect _mold;
	Common::Rect _bellows;

	void resetBackgroundBitmap();
};

SmithyBench::SmithyBench(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_status = ((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus;
	resetBackgroundBitmap();

	_pan = Common::Rect(0, 73, 227, 123);
	_mold = Common::Rect(333, 57, 423, 105);
	_bellows = Common::Rect(0, 0, 302, 34);
}

int SmithyBench::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_pan.contains(pointLocation) && (_status == 2 || _status == 3)) {
		_status %= 2;
		resetBackgroundBitmap();
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;

		Common::Point ptInventoryWindow = viewWindow->convertPointToWindow(pointLocation, ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(kItemCopperMedallion, ptInventoryWindow);
		return SC_TRUE;
	} else if (_mold.contains(pointLocation) && _status == 6) {
		_status = 1;
		resetBackgroundBitmap();
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;

		Common::Point ptInventoryWindow = viewWindow->convertPointToWindow(pointLocation, ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(kItemCopperKey, ptInventoryWindow);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int SmithyBench::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_mold.contains(pointLocation) && _status < 6 && (!_vm->isDemo() || ((FrameWindow *)_vm->_mainWindow)->_reviewerMode)) {
		if ((_status % 2) == 1) {
			// Brick has been removed, so play the returning movie
			_status--;
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_status + 8);
			resetBackgroundBitmap();
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgBSFoundMold = 1;
		} else {
			// The brick is still covering the mold, so remove it
			_status++;
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_status + 6);
			resetBackgroundBitmap();
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;
		}

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	} else if (_pan.contains(pointLocation) && _status == 5) {
		_status = 6;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(13);
		resetBackgroundBitmap();

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	} else if (_bellows.contains(pointLocation) && _status < 4) {
		switch (_status) {
		case 0:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(3);
			break;
		case 1:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(4);
			break;
		case 2:
			_status = 4;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(5);
			resetBackgroundBitmap();
			break;
		case 3:
			_status = 5;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(6);
			resetBackgroundBitmap();
			break;
		}

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int SmithyBench::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemCopperKey)
		return 2; // Third dragging bitmap

	return 0;
}

int SmithyBench::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if (_pan.contains(pointLocation) && itemID == kItemCopperMedallion && _status < 2) {
		// Did we drop the medallion in the pan?
		_status += 2;
		resetBackgroundBitmap();
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;
		viewWindow->invalidateWindow();

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SIC_ACCEPT;
	} else if (_mold.contains(pointLocation) && itemID == kItemCopperKey && _status == 1) {
		// Did we drop the key?
		_status = 6;
		resetBackgroundBitmap();
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSmithyStatus = _status;
		viewWindow->invalidateWindow();

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int SmithyBench::specifyCursor(Window *viewWindow, const Common::Point &pointLocation){
	if (_bellows.contains(pointLocation) && _status < 4)
		return kCursorFinger;

	if (_mold.contains(pointLocation) && _status < 6 && (!_vm->isDemo() || ((FrameWindow *)_vm->_mainWindow)->_reviewerMode))
		return kCursorFinger;

	if (_pan.contains(pointLocation) && (_status == 2 || _status == 3))
		return kCursorOpenHand;

	if (_mold.contains(pointLocation) && _status == 6)
		return kCursorOpenHand;

	if (_pan.contains(pointLocation) && _status == 5)
		return kCursorFinger;

	return kCursorArrow;
}

void SmithyBench::resetBackgroundBitmap() {
	switch (_status) {
	case 0: // Nothing in with brick in place
		_staticData.navFrameIndex = 52;
		break;
	case 1: // Nothing in with brick removed
		_staticData.navFrameIndex = 53;
		break;
	case 2: // Unmelted medallion, brick -inplace
		_staticData.navFrameIndex = 55;
		break;
	case 3: // Unmelted medallion, brick moved
		_staticData.navFrameIndex = 57;
		break;
	case 4: // Melted medallion in with brick in place
		_staticData.navFrameIndex = 56;
		break;
	case 5: // Melted medallion in with brick removed
		_staticData.navFrameIndex = 58;
		break;
	case 6: // Poured key with brick removed
		_staticData.navFrameIndex = 59;
		break;
	}
}

class PickupKingsStudyBooksA : public SceneBase {
public:
	PickupKingsStudyBooksA(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _diaryA, _diaryB;
};

PickupKingsStudyBooksA::PickupKingsStudyBooksA(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_diaryA = Common::Rect(148, 46, 344, 168);
	_diaryB = Common::Rect(216, 0, 306, 48);
}

int PickupKingsStudyBooksA::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_diaryA.contains(pointLocation)) {
		// Move to the first diary
		DestinationScene pickUpDestination;
		pickUpDestination.destinationScene.timeZone = 1;
		pickUpDestination.destinationScene.environment = 8;
		pickUpDestination.destinationScene.node = 5;
		pickUpDestination.destinationScene.facing = 2;
		pickUpDestination.destinationScene.orientation = 0;
		pickUpDestination.destinationScene.depth = 2;
		pickUpDestination.transitionType = TRANSITION_VIDEO;
		pickUpDestination.transitionData = 1;
		pickUpDestination.transitionStartFrame = -1;
		pickUpDestination.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(pickUpDestination);
		return SC_TRUE;
	} else if (_diaryB.contains(pointLocation)) {
		// Move to the second diary
		DestinationScene pickUpDestination;
		pickUpDestination.destinationScene.timeZone = 1;
		pickUpDestination.destinationScene.environment = 8;
		pickUpDestination.destinationScene.node = 5;
		pickUpDestination.destinationScene.facing = 2;
		pickUpDestination.destinationScene.orientation = 0;
		pickUpDestination.destinationScene.depth = 1;
		pickUpDestination.transitionType = TRANSITION_VIDEO;
		pickUpDestination.transitionData = 3;
		pickUpDestination.transitionStartFrame = -1;
		pickUpDestination.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(pickUpDestination);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int PickupKingsStudyBooksA::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_diaryA.contains(pointLocation) || _diaryB.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

class MainWallCatapultService : public SceneBase {
public:
	MainWallCatapultService(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int timerCallback(Window *viewWindow) override;
};

MainWallCatapultService::MainWallCatapultService(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int MainWallCatapultService::timerCallback(Window *viewWindow) {
	SceneBase::timerCallback(viewWindow);

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultData > 0) {
		uint32 timer = ((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultData;

		if (timer + CATAPULT_TIMEOUT_VALUE < g_system->getMillis()) {
			// Play the catapult sound
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14), 127, false, true);

			// Reset the timer flags
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultData = 0;

			// Determine an interval to wait until the next launch
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultOffset = g_system->getMillis();
		}
	} else if ((((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultOffset + 20000) < g_system->getMillis()) {
		// No boulder going, but it's time for another launch
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgMWCatapultData = g_system->getMillis();
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13), 127, false, true);
	}

	return SC_TRUE;
}

class MiddleBaileyFootprintCapture : public SceneBase {
public:
	MiddleBaileyFootprintCapture(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int locateAttempted(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _footprint;
};

MiddleBaileyFootprintCapture::MiddleBaileyFootprintCapture(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_footprint = Common::Rect(307, 134, 361, 160);
}

int MiddleBaileyFootprintCapture::locateAttempted(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcLocateEnabled == 1) {
		if (_footprint.contains(pointLocation)) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(9);

			// Add it to the list
			if (((SceneViewWindow *)viewWindow)->addNumberToGlobalFlagTable(CASTLE_EVIDENCE_FOOTPRINT))
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ACQUIRED));
			else
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

			// Turn off capture
			((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->disableEvidenceCapture();
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int MiddleBaileyFootprintCapture::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcLocateEnabled == 1) {
		if (_footprint.contains(pointLocation))
			return -2;

		return -1;
	}

	return kCursorArrow;
}

class TreasureRoomSwordCapture : public SceneBase {
public:
	TreasureRoomSwordCapture(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int locateAttempted(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _footprint;
};

TreasureRoomSwordCapture::TreasureRoomSwordCapture(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_footprint = Common::Rect(118, 28, 190, 108);
}

int TreasureRoomSwordCapture::locateAttempted(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcLocateEnabled == 1) {
		if (_footprint.contains(pointLocation)) {
			// Play the animation
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(0);

			// Play Arthur's comment
			if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
				_vm->_sound->playSynchronousSoundEffect("BITDATA/CASTLE/CGTR_C01.BTA");

			// Set the located flag
			((SceneViewWindow *)viewWindow)->getGlobalFlags().cgTRFoundSword = 1;

			// Attempt to add it to the biochip
			if (((SceneViewWindow *)viewWindow)->addNumberToGlobalFlagTable(CASTLE_EVIDENCE_SWORD))
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_RIPPLE_DOCUMENTED));
			else
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

			// Turn off evidence capture
			((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->disableEvidenceCapture();

			// Reset the AI biochip display
			((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();

			// Set the scoring flag
			((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreFoundSwordDiamond = 1;
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int TreasureRoomSwordCapture::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcLocateEnabled == 1) {
		if (_footprint.contains(pointLocation))
			return -2; // Over the item, return the capture cursor

		return -1;
	}

	return kCursorArrow;
}

class StorageRoomDoor : public SceneBase {
public:
	StorageRoomDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int timeZone = -1, int environment = -1, int node = -1,
			int facing = -1, int orientation = -1, int depth = -1, int data = -1, int startFrame = -1,
			int length = -1, int animDB = -1);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	bool _clicked;
	Common::Rect _clickable;
	DestinationScene _destData;
	int _agent3VideoID;
};

StorageRoomDoor::StorageRoomDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int timeZone, int environment, int node,
		int facing, int orientation, int depth, int data, int startFrame,
		int length, int animDB) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	_agent3VideoID = animDB;
	_clicked = false;
	_clickable = Common::Rect(left, top, right, bottom);

	_destData.destinationScene.timeZone = timeZone;
	_destData.destinationScene.environment = environment;
	_destData.destinationScene.node = node;
	_destData.destinationScene.facing = facing;
	_destData.destinationScene.orientation = orientation;
	_destData.destinationScene.depth = depth;

	if (globalFlags.cgStorageRoomVisit != 0) {
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
		SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
		GlobalFlags &globalFlags = sceneView->getGlobalFlags();
		BuriedEngine *vm = _vm;
		if (_clickable.contains(pointLocation)) {
			((SceneViewWindow *)viewWindow)->moveToDestination(_destData);
		} else {
			_clicked = false;
		}

		if (globalFlags.cgStorageRoomVisit == 0) {
			if (((SceneViewWindow *)viewWindow)->addNumberToGlobalFlagTable(CASTLE_EVIDENCE_AGENT3))
				((SceneViewWindow *)viewWindow)->displayLiveText(vm->getString(IDS_MBT_EVIDENCE_ACQUIRED));
			else
				((SceneViewWindow *)viewWindow)->displayLiveText(vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

			globalFlags.cgStorageRoomVisit = 1;
		}
	}

	return SC_TRUE;
}

int StorageRoomDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickable.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class StorageRoomCheckUnlock : public SceneBase {
public:
	StorageRoomCheckUnlock(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int itemID = 0, int filledFrameIndex = 0, int animID = 0, int depthA = 0, int depthB = 0,
			int left = 0, int top = 0, int right = 0, int bottom = 0);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _itemID;
	int _filledFrameIndex;
	int _animID;
	int _depthA;
	int _depthB;
	Common::Rect _dropRegion;
	Common::Rect _chest;
};

StorageRoomCheckUnlock::StorageRoomCheckUnlock(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int itemID, int filledFrameIndex, int animID, int depthA, int depthB,
		int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_itemID = itemID;
	_filledFrameIndex = filledFrameIndex;
	_depthA = depthA;
	_depthB = depthB;
	_animID = animID;
	_dropRegion = Common::Rect(left, top, right, bottom);
	_chest = Common::Rect(55, 35, 432, 189);
}

int StorageRoomCheckUnlock::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	// If we clicked on the chest, play the locked key sound we are so familiar with
	if (_chest.contains(pointLocation)) {
		_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13));
		((SceneViewWindow *)viewWindow)->getGlobalFlags().cgSRClickedOnLockedChest = 1;

		if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
			((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
	}

	return SC_FALSE;
}

int StorageRoomCheckUnlock::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (_dropRegion.contains(pointLocation) && _itemID == itemID)
		return 1;

	return 0;
}

int StorageRoomCheckUnlock::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_dropRegion.contains(pointLocation) && _itemID == itemID) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animID);
		_staticData.navFrameIndex = _filledFrameIndex;

		Location newDest = _staticData.location;

		if (globalFlags.cgTapestryFlag != 0)
			newDest.depth = _depthB;
		else
			newDest.depth = _depthA;

		((SceneViewWindow *)viewWindow)->jumpToScene(newDest);
	}

	// Key remains in inventory
	return SIC_REJECT;
}

int StorageRoomCheckUnlock::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_chest.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class DeliverLightMessage : public SceneBase {
public:
	DeliverLightMessage(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
};

DeliverLightMessage::DeliverLightMessage(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int DeliverLightMessage::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_staticData.location.timeZone != priorLocation.timeZone || _staticData.location.environment != priorLocation.environment)
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_JUMPSUIT_LIGHT_TURN_ON_MESSAGE));

	return SC_TRUE;
}

class KingsChamberGuardEncounter : public SceneBase {
public:
	KingsChamberGuardEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int timerCallback(Window *viewWindow) override;

private:
	uint32 _startingTime;
	//bool _finished;
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
		_globalFlags.cgWallExploded = 0;
		_globalFlags.cgHookPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemGrapplingHook) ? 1 : 0;
		_globalFlags.cgArrowPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBloodyArrow) ? 1 : 0;
		_globalFlags.cgHammerPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemHammer) ? 1 : 0;
		_globalFlags.cgSmithyStatus = 0;
		_globalFlags.cgSmithyGuard = 0;
		_globalFlags.cgBaileyOneWayGuard = 0;
		_globalFlags.cgBaileyTwoWayGuards = 0;
		_globalFlags.cgTapestryFlag = 0;
		_globalFlags.cgStorageRoomVisit = 0;
		_globalFlags.cgBurnedLetterPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBurnedLetter) ? 1 : 0;
		_globalFlags.cgGoldCoinsPresent = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemGoldCoins) ? 1 : 0;

		if (_globalFlags.generalWalkthroughMode == 1) {
			_globalFlags.cgSmithyStatus = 6;
			_globalFlags.cgTapestryFlag = 1;
		}
	}

	// Environment-specific
	if (environment == 4) {
		_globalFlags.scoreEnteredKeep = 1;
		_globalFlags.cgMBVisited = 1;
	} else if (environment == 6) {
		_globalFlags.cgKCVisited = 1;
	} else if (environment == 10) {
		_globalFlags.scoreEnteredTreasureRoom = 1;
		_globalFlags.cgTRVisited = 1;
	}

	return true;
}

bool SceneViewWindow::startCastleAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	if (_vm->isDemo())
		return false;

	_vm->_sound->setAmbientSound(_vm->getFilePath(1, environment, SF_AMBIENT), fade, 64);
	return true;
}

bool SceneViewWindow::checkCustomCastleAICommentDependencies(const Location &commentLocation, const AIComment &commentData) {
	switch (commentData.dependencyFlagOffsetB) {
	case 1: // Did we click on the tower door at node 3?
		return _globalFlags.cgTSTriedDoorA == 1;
	case 2: // Did we click on the tower door at node 5?
		return _globalFlags.cgTSTriedDoorB == 1;
	case 3: // Did we click on either of the tower doors?
		return _globalFlags.cgTSTriedDoorA == 1 || _globalFlags.cgTSTriedDoorB == 1;
	case 4: // Is the grappling hook in our inventory?
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemGrapplingHook);
	case 5: // Have we not been to the middle bailey?
		return _globalFlags.cgMBVisited == 0;
	case 6: // Have we not been to the keep?
		return _globalFlags.cgKCVisited == 0;
	case 7: // Have we not been across the moat?
		return _globalFlags.cgMBCrossedMoat == 0;
	case 8: // If we have not been in the keep, and the hook is in our inventory
		return _globalFlags.cgKCVisited == 0 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemGrapplingHook);
	case 9: // If we have not been in the keep and if AI table 33 is equal to 0
		return _globalFlags.cgKCVisited == 0 && _globalFlags.aiData[33] == 0;
	case 10: // If we have found the mold, did not read the journal, and do not have a key
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgSmithyStatus == 1 && _globalFlags.cgKSSmithyEntryRead == 0;
	case 11: // If we have found the mold, did not read the journal, and do not have a key
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgSmithyStatus == 1 && _globalFlags.cgKSSmithyEntryRead == 1;
	case 12: // If we have found the mold, do not have the medallion, and do not have the key
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgBSFoundMold == 1 && _globalFlags.cgSmithyStatus == 1 && _globalFlags.cgKSSmithyEntryRead == 1;
	case 13: // If we have found the mold, have a medallion, do not have a key, and AI offset 41 >= 2
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgBSFoundMold == 1 && _globalFlags.cgSmithyStatus == 1 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperMedallion) && _globalFlags.aiData[41] >= 2;
	case 14: // Is the hammer not in our invetory
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemHammer);
	case 15: // If we have not found the mold, did not read the journal, and do not have a key
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgBSFoundMold == 0 && _globalFlags.cgKSSmithyEntryRead == 0;
	case 16: // If we have not found the mold, did read the journal, and do not have a key
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgBSFoundMold == 0 && _globalFlags.cgKSSmithyEntryRead == 1;
	case 17: // If we have not been to the treasure room, and if we are not waiting for the guards
		return commentLocation.depth != 1 && _globalFlags.cgTRVisited == 0;
	case 18: // Is the letter not in our inventory?
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemBurnedLetter);
	case 19: // If we have not read either journal and not been to the treasure room
		return _globalFlags.cgTRVisited == 0 && _globalFlags.cgKSReadJournal == 0;
	case 20: // If we are in the king's study and if we have not been to the treasure room
		return _globalFlags.cgTRVisited == 0 && commentLocation.node != 11;
	case 21: // If we are inside the king's study
		return commentLocation.node != 11;
	case 22: // If the key is not in the inventory, have not opened the chest and have clicked on the locked chest
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgSRClickedOnLockedChest == 1;
	case 23: // If the key is not in the inventory, have not opened the chest, have clicked on the locked chest, read the journal entry, and not found the armoire
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgSRClickedOnLockedChest == 1 && _globalFlags.cgKSSmithyEntryRead == 1 && _globalFlags.cgFoundChestPanel == 0;
	case 24: // If we have not made the copper key, we have clicked on the chest, and have found the panel in the armoire
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgSRClickedOnLockedChest == 1 && _globalFlags.cgFoundChestPanel == 1;
	case 25: // If we have the key and have clicked on the locked chest
		return ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey) && _globalFlags.cgSRClickedOnLockedChest == 1;
	case 26: // If we have not read the letter, have not been to 1/8/3/3/0/0, have not been to the treasure room
		return _globalFlags.readBurnedLetter == 0 && _globalFlags.cgTRVisited == 0 && _globalFlags.cgViewedKeepPlans == 0 && _globalFlags.cgTapestryFlag == 0;
	case 27: // Have not been to the treasure room
		return _globalFlags.cgTRVisited == 0 && _globalFlags.cgTapestryFlag == 0;
	case 28: // Has read the burned letter, have not been to treasure room
		return _globalFlags.readBurnedLetter == 1 && _globalFlags.cgTRVisited == 0 && _globalFlags.cgTapestryFlag == 0;
	case 29: // Has seen 1/8/3/3/0/0, has not been to treasure room
		return _globalFlags.cgViewedKeepPlans == 1 && _globalFlags.cgTRVisited == 0 && _globalFlags.cgTapestryFlag == 0;
	case 30: // Has seen 1/8/3/3/0/0, has not been to treasure room, has read the burned letter
		return _globalFlags.cgViewedKeepPlans == 1 && _globalFlags.cgTRVisited == 0 && _globalFlags.readBurnedLetter == 1 && _globalFlags.cgTapestryFlag == 0;
	case 31: // If the door to CGSR has been opened, hasn't opened chest
		return _globalFlags.cgSROpenedChest == 0 && commentLocation.node != 5;
	case 32: // If the door to CGSR has been opened, has been to 1/8/3/3/0/0, has not been to treasure room
		return _globalFlags.cgViewedKeepPlans == 1 && _globalFlags.cgTRVisited == 0;
	case 33: // Has opened chest, has read burned letter, has not clicked on tapestry
		return _globalFlags.cgSROpenedChest == 1 && _globalFlags.readBurnedLetter == 1 && _globalFlags.cgTapestryFlag == 0;
	case 34: // If the door to CGSR has been opened
		return commentLocation.node != 5;
	case 35: // If we have not used the locate feature in the treasure room, have not found the sword
		return _globalFlags.cgTRFoundSword == 0;
	case 36: // If we do not have the key
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemCopperKey);
	case 37: // If we have not been in the keep and the hook is in our inventory
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemBloodyArrow);
	}

	return false;
}

SceneBase *SceneViewWindow::constructCastleSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	byte dummyFlag = 0; // a dummy flag, used as a placeholder for writing (but not reading)

	// Special scene for the trial version
	if (_vm->isTrial())
		return new TrialRecallScene(_vm, viewWindow, sceneStaticData, priorLocation);

	switch (sceneStaticData.classID) {
	case 0:
		// Default scene
		break;
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
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgWallExploded, 0, 0, 1, 0, 0);
	case 8:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgWallExploded, 0, 1, 0, 0, 0);
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
	case 19:
		return new KingsStudyGuard(_vm, viewWindow, sceneStaticData, priorLocation);
	case 20:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgHookPresent, 0, 0, 1, 0, 0);
	case 21:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgHookPresent, 0, 1, 0, 0, 0);
	case 22:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 163, 83, 236, 162, kItemGrapplingHook, 75, globalFlags.cgHookPresent);
	case 23:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgArrowPresent, 0, 0, 1, 0, 0);
	case 24:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgArrowPresent, 0, 1, 0, 0, 0);
	case 25:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 241, 7, 284, 92, kItemBloodyArrow, 57, globalFlags.cgArrowPresent);
	case 26:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgHammerPresent, 0, 0, 1, 0, 0);
		break;
	case 27:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgHammerPresent, 0, 1, 0, 0, 0);
		else
			return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 28:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 184, 111, 237, 189, kItemHammer, 3, globalFlags.cgHammerPresent);
	case 29:
		if (_vm->isDemo())
			return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgHammerPresent, 0, 0, 0, 1, 0);
		break;
	case 30:
		return new TurnDepthPreChange(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgHookPresent, 0, 0, 0, 1, 0);
	case 31:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 1, kCursorFinger, 51, 25, 149, 149);
	case 32:
		return new OneShotEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, 0, globalFlags.cgSmithyGuard, IDS_HUMAN_PRESENCE_10METERS);
	case 33:
		return new OneShotEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, _vm->isDemo() ? 2 : 6, globalFlags.cgBaileyOneWayGuard, IDS_HUMAN_PRESENCE_10METERS);
	case 34:
		return new CycleEntryVideoWarning(_vm, viewWindow, sceneStaticData, priorLocation, _vm->isDemo() ? 5 : 7, _vm->isDemo() ? 6 : 8, IDS_HUMAN_PRESENCE_10METERS);
	case 35:
		return new ClickPlayVideoSwitch(_vm, viewWindow, sceneStaticData, priorLocation, 3, kCursorFinger, globalFlags.cgTapestryFlag, 0, 0, 330, 189);
	case 36:
		return new ClickPlayVideoSwitch(_vm, viewWindow, sceneStaticData, priorLocation, 4, kCursorFinger, globalFlags.cgTapestryFlag, 0, 0, 330, 189);
	case 37:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 175, 64, 237, 126, kItemBurnedLetter, 84, globalFlags.cgBurnedLetterPresent);
	case 38:
		return new StorageRoomCheckUnlock(_vm, viewWindow, sceneStaticData, priorLocation, kItemCopperKey, 51, 1, 2, 1, 258, 100, 320, 185);
	case 39:
		return new StorageRoomDoor(_vm, viewWindow, sceneStaticData, priorLocation, 38, 0, 386, 189, 1, 9, 5, 2, 1, 1, 11, 130, 12, 0);
	case 41:
		return new OpenFirstItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 138, 32, 288, 107, 175, 65, 226, 90, 2, 1, kItemGoldCoins, 34, 35);
	case 42:
		return new SmithyBench(_vm, viewWindow, sceneStaticData, priorLocation);
	case 43:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 10, 0, 376, 189, kCursorFinger, 1, 6, 5, 1, 0, 1, TRANSITION_VIDEO, 2, -1, -1);
	case 44:
		return new BrowseBook(_vm, viewWindow, sceneStaticData, priorLocation, IDBD_DIARY1, IDBD_DIARY1_TRANS_TEXT_BASE, 0, 1, 8, 5, 2, 0, 0, TRANSITION_VIDEO, 4, -1, -1);
	case 45:
		return new BrowseBook(_vm, viewWindow, sceneStaticData, priorLocation, IDBD_DIARY2, IDBD_DIARY2_TRANS_TEXT_BASE, 1, 1, 8, 5, 2, 0, 0, TRANSITION_VIDEO, 2, -1, -1);
	case 46:
		return new PickupKingsStudyBooksA(_vm, viewWindow, sceneStaticData, priorLocation);
	case 47:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 2, kCursorFinger, 0, 75, 258, 123);
	case 48:
		return new KingsChamberGuardEncounter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 50:
		return new ClickPlaySoundSynchronous(_vm, viewWindow, sceneStaticData, priorLocation, 14, kCursorFinger, 72, 0, 372, 189);
	case 51:
		return new ClickZoom(_vm, viewWindow, sceneStaticData, priorLocation, 5, 36, 6, 12, kCursorMagnifyingGlass, 0, 0, 432, 189);
	case 52:
		return new ClickZoom(_vm, viewWindow, sceneStaticData, priorLocation, 7, 37, 8, 18, kCursorMagnifyingGlass, 0, 90, 140, 189);
	case 53:
		return new ClickZoom(_vm, viewWindow, sceneStaticData, priorLocation, 9, 38, 10, 24, kCursorMagnifyingGlass, 130, 120, 432, 189);
	case 55:
		// Valid, but not implemented.
		break;
	case 56:
		return new PlaySoundEnteringFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14, 1, 9, 0, 3, 1, 0);
	case 57:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 58:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 60:
		return new MainWallCatapultService(_vm, viewWindow, sceneStaticData, priorLocation);
	case 61:
		return new SetFlagOnEntry(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgMBCrossedMoat, 1);
	case 62:
		return new MiddleBaileyFootprintCapture(_vm, viewWindow, sceneStaticData, priorLocation);
	case 63:
		return new DisplayMessageWithEvidenceWhenEnteringNode(_vm, viewWindow, sceneStaticData, priorLocation, CASTLE_EVIDENCE_FOOTPRINT, IDS_MBT_EVIDENCE_PRESENT);
	case 64:
		return new DisplayMessageWithEvidenceWhenEnteringNode(_vm, viewWindow, sceneStaticData, priorLocation, CASTLE_EVIDENCE_SWORD, IDS_MBT_EVIDENCE_PRESENT);
	case 65:
		return new TreasureRoomSwordCapture(_vm, viewWindow, sceneStaticData, priorLocation);
	case 66:
		// Original incremented the flag each time, but it's expected that the code will never go above 1
		return new SetFlagOnEntry(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgViewedKeepPlans, 1);
	case 67:
		return new ClickChangeSceneSetFlag(_vm, viewWindow, sceneStaticData, priorLocation, 10, 0, 376, 189, kCursorPutDown, 1, 6, 5, 1, 0, 0, TRANSITION_VIDEO, 5, -1, -1, globalFlags.cgFoundChestPanel);
	case 70:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 12);
	case 71:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 72:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, dummyFlag, 12, kCursorFinger, 28, 34, 336, 189);
	case 73:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, dummyFlag, 13, kCursorFinger, 0, 0, 270, 189);
	case 74:
		return new PlaySoundExitingFromSceneDeux(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 75:
		return new DeliverLightMessage(_vm, viewWindow, sceneStaticData, priorLocation);
	case 76:
		return new SetFlagOnEntry(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgSROpenedChest, 1);
	case 77:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, globalFlags.cgTSTriedDoorB, 14, kCursorFinger, 72, 0, 372, 189);
	default:
		warning("Unknown Castle scene object %d", sceneStaticData.classID);
		break;
	}

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
