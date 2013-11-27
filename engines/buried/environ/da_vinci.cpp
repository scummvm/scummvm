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
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

namespace Buried {

class PaintingTowerRetrieveKey : public SceneBase {
public:
	PaintingTowerRetrieveKey(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _key;
};

PaintingTowerRetrieveKey::PaintingTowerRetrieveKey(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTDoorLocked >= 1) {
		int curStillFrame = _staticData.navFrameIndex;
		_staticData.navFrameIndex = _staticData.miscFrameIndex;
		_staticData.miscFrameIndex = curStillFrame;
	}

	_key = Common::Rect(268, 50, 298, 88);
}

int PaintingTowerRetrieveKey::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_key.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTDoorLocked == 0) {
		// Play the unlocking movie
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(2);

		// Swap the still frame
		int curStillFrame = _staticData.navFrameIndex;
		_staticData.navFrameIndex = _staticData.miscFrameIndex;
		_staticData.miscFrameIndex = curStillFrame;

		// Add the key to the inventory
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->addItem(kItemBalconyKey);

		// Change the locked door flag
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTDoorLocked = 1;
	}

	return SC_TRUE;
}

int PaintingTowerRetrieveKey::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_key.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTDoorLocked == 0)
		return kCursorFinger;

	return kCursorArrow;
}

class PaintingTowerOutsideDoor : public SceneBase {
public:
	PaintingTowerOutsideDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _clickableArea;
};

PaintingTowerOutsideDoor::PaintingTowerOutsideDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickableArea = Common::Rect(0, 0, 236, 189);
}

int PaintingTowerOutsideDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableArea.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent >= 1) {
			DestinationScene destData;
			destData.destinationScene.timeZone = 5;
			destData.destinationScene.environment = 1;
			destData.destinationScene.node = 7;
			destData.destinationScene.facing = 3;
			destData.destinationScene.orientation = 1;
			destData.destinationScene.depth = 2;
			destData.transitionType = TRANSITION_WALK;
			destData.transitionData = 11;
			destData.transitionStartFrame = 28;
			destData.transitionLength = 12;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		} else {
			DestinationScene destData;
			destData.destinationScene.timeZone = 5;
			destData.destinationScene.environment = 1;
			destData.destinationScene.node = 7;
			destData.destinationScene.facing = 3;
			destData.destinationScene.orientation = 1;
			destData.destinationScene.depth = 1;
			destData.transitionType = TRANSITION_WALK;
			destData.transitionData = 11;
			destData.transitionStartFrame = 0;
			destData.transitionLength = 12;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		}
	}

	return SC_FALSE;
}

int PaintingTowerOutsideDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableArea.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class PaintingTowerInsideDoor : public SceneBase {
public:
	PaintingTowerInsideDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _clickableArea;
};

PaintingTowerInsideDoor::PaintingTowerInsideDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickableArea = Common::Rect(290, 0, 432, 189);
}

int PaintingTowerInsideDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableArea.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTDoorLocked >= 1) {
			DestinationScene destData;
			destData.destinationScene.timeZone = 5;
			destData.destinationScene.environment = 1;
			destData.destinationScene.node = 2;
			destData.destinationScene.facing = 2;
			destData.destinationScene.orientation = 1;
			destData.destinationScene.depth = 1;
			destData.transitionType = TRANSITION_WALK;
			destData.transitionData = 11;
			destData.transitionStartFrame = 338;
			destData.transitionLength = 22;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		} else {
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13), 127, false, true);
		}
	}

	return SC_FALSE;
}

int PaintingTowerInsideDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickableArea.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

enum {
	DS_SC_DRIVE_ASSEMBLY = 1,
	DS_SC_WHEEL_ASSEMBLY = 2,
	DS_SC_PEGS = 4,
	DS_SC_COMPLETED = 8
};

class PaintingTowerCapAgent : public SceneBase {
public:
	PaintingTowerCapAgent(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
};

PaintingTowerCapAgent::PaintingTowerCapAgent(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int PaintingTowerCapAgent::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTBeenOnBalcony = 1;

	if (!((SceneViewWindow *)viewWindow)->isNumberInGlobalFlagTable(offsetof(GlobalFlags, evcapBaseID), offsetof(GlobalFlags, evcapNumCaptured), DAVINCI_EVIDENCE_AGENT3)) {
		// Play animation of capturing the evidence
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(11);

		// Attempt to add the evidence to the biochip
		((SceneViewWindow *)viewWindow)->addNumberToGlobalFlagTable(offsetof(GlobalFlags, evcapBaseID), offsetof(GlobalFlags, evcapNumCaptured), 12, DAVINCI_EVIDENCE_AGENT3);
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ACQUIRED));

		// Turn off evidence capture
		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->disableEvidenceCapture();

		// Set flag for scoring
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreResearchAgent3DaVinci = 1;
	} else {
		if (false) { // TODO: If control down
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(12);
		}
	}

	return SC_TRUE;
}

bool SceneViewWindow::initializeDaVinciTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1) {
		GlobalFlags &flags = ((SceneViewWindow *)viewWindow)->getGlobalFlags();

		flags.dsPTElevatorPresent = flags.generalWalkthroughMode;
		flags.dsPTElevatorLeverA = flags.generalWalkthroughMode;
		flags.dsPTElevatorLeverB = flags.generalWalkthroughMode;
		flags.dsCYBallistaStatus = flags.generalWalkthroughMode;
		flags.dsWSSiegeCycleStatus = 0;
		flags.dsCTUnlockedDoor = flags.generalWalkthroughMode;
		flags.dsCTPlayedBallistaFalling = 0;
		flags.dsCYPlacedSiegeCycle = 0;
		flags.dsCYBallistaXPos = 0;
		flags.dsCYBallistaYPos = 0;

		flags.dsWSPickedUpWheelAssembly = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemWheelAssembly) ? 1 : 0;
		flags.dsWSPickedUpGearAssembly = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemDriveAssembly) ? 1 : 0;
		flags.dsWSPickedUpPegs = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemWoodenPegs) ? 1 : 0;
		flags.dsWSGrabbedSiegeCycle = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemSiegeCycle) ? 1 : 0;
		flags.dsGDTakenCoilOfRope = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemCoilOfRope) ? 1 : 0;
		flags.dsCTRetrievedLens = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemLensFilter) ? 1 : 0;
		flags.dsCTTakenHeart = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemPreservedHeart) ? 1 : 0;

		if (flags.generalWalkthroughMode == 1) {
			flags.dsWSSiegeCycleStatus = DS_SC_COMPLETED;
			flags.dsWSPickedUpWheelAssembly = 1;
			flags.dsWSPickedUpGearAssembly = 1;
			flags.dsWSPickedUpPegs = 1;
		}
	} else if (environment == 2) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEnteredCodexTower = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsVisitedCodexTower = 1;
	}

	return true;
}

bool SceneViewWindow::startDaVinciAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(5, environment, SF_AMBIENT), fade, 64);
	return true;
}

SceneBase *SceneViewWindow::constructDaVinciSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 4:
		return new PaintingTowerRetrieveKey(_vm, viewWindow, sceneStaticData, priorLocation);
	case 11:
		return new PaintingTowerOutsideDoor(_vm, viewWindow, sceneStaticData, priorLocation);
	case 12:
		return new PaintingTowerInsideDoor(_vm, viewWindow, sceneStaticData, priorLocation);
	case 13:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 196, 0, 262, 189, 5, 3, 10, 1, 1, 1, TRANSITION_WALK, 11, 881, 20);
	case 14:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 208, 0, 306, 189, 5, 3, 0, 2, 1, 1, TRANSITION_WALK, 11, 740, 23);
	case 18:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 216, 0, 324, 189, 5, 3, 2, 0, 1, 1, TRANSITION_WALK, 11, 833, 26);
	case 19:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 194, 0, 354, 189, 5, 3, 9, 1, 1, 1, TRANSITION_WALK, 11, 791, 21);
	case 20:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 102, 0, 208, 189, 5, 4, 14, 0, 1, 1, TRANSITION_WALK, 11, 1169, 28);
	case 21:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 80, 0, 250, 189, 5, 4, 15, 3, 1, 1, TRANSITION_WALK, 11, 1126, 26);
	case 22:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 2, kCursorFinger, 110, 138, 170, 189);
	case 23:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 4, kCursorFinger, 180, 122, 290, 189);
	case 32:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 158, 90, 328, 162, kItemDriveAssembly, 145, offsetof(GlobalFlags, dsWSPickedUpGearAssembly));
	case 33:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 164, 126, 276, 160, kItemWoodenPegs, 96, offsetof(GlobalFlags, dsWSPickedUpPegs));
	case 38:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 130, 74, 182, 120, kItemCoilOfRope, 48, offsetof(GlobalFlags, dsGDTakenCoilOfRope));
	case 41:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 116, 0, 326, 189, 5, 2, 2, 1, 1, 1, TRANSITION_WALK, 11, 225, 15);
	case 46:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 8, kCursorFinger, 102, 124, 164, 189);
	case 59:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 2, kCursorFinger, 70, 136, 190, 189);
	case 60:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 5, kCursorFinger, 42, 0, 418, 100);
	case 61:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 3, kCursorFinger, 178, 144, 288, 189);
	case 65:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 122, 8, 326, 189, 5, 5, 0, 2, 1, 1, TRANSITION_WALK, 11, 738, 18);
	case 66:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 170, 0, 432, 189, 5, 4, 0, 0, 1, 1, TRANSITION_WALK, 11, 1220, 12);
	case 70:
		return new PaintingTowerCapAgent(_vm, viewWindow, sceneStaticData, priorLocation);
	}

	warning("TODO: Da Vinci scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
