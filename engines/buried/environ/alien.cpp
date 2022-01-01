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
#include "graphics/surface.h"

namespace Buried {

class RetrieveFromPods : public SceneBase {
public:
	RetrieveFromPods(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int doorLeft = -1, int doorTop = -1, int doorRight = -1, int doorBottom = -1, int openAnim = -1, int openNormFrame = -1,
			int popAnim = -1, int openPoppedAnim = -1, int openPoppedFrame = -1, int grabLeft = -1, int grabTop = -1, int grabRight = -1,
			int grabBottom = -1, int openEmptyAnim = -1, int openEmptyFrame = -1, byte pod = 0, int itemID = -1, int returnDepth = -1,
			int popSwordAnim = -1);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

protected:
	Common::Rect _openDoor;
	Common::Rect _grabObject;
	int _itemID;
	int _openNormFrame;
	int _openPoppedFrame;
	int _openEmptyFrame;
	int _openAnim;
	int _popAnim;
	int _openPoppedAnim;
	int _openEmptyAnim;
	int _popSwordAnim;
	int _returnDepth;
	bool _doorOpen;
	byte _pod;
	GlobalFlags &_globalFlags;

	byte *podStatusFlag(byte pod);
	byte *podItemFlag();
	byte getPodStatus();
	void setPodStatus(byte status);
	void setPodStatus(byte pod, byte status);
	byte getPodItemFlag();
	void setPodItemFlag(byte value);
};

RetrieveFromPods::RetrieveFromPods(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int doorLeft, int doorTop, int doorRight, int doorBottom, int openAnim, int openNormFrame,
		int popAnim, int openPoppedAnim, int openPoppedFrame, int grabLeft, int grabTop, int grabRight,
		int grabBottom, int openEmptyAnim, int openEmptyFrame, byte pod, int itemID, int returnDepth, int popSwordAnim) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation),
		_globalFlags(((SceneViewWindow *)viewWindow)->getGlobalFlags()) {
	_openDoor = Common::Rect(doorLeft, doorTop, doorRight, doorBottom);
	_grabObject = Common::Rect(grabLeft, grabTop, grabRight, grabBottom);
	_itemID = itemID;
	_openNormFrame = openNormFrame;
	_openPoppedFrame = openPoppedFrame;
	_openEmptyFrame = openEmptyFrame;
	_openAnim = openAnim;
	_popAnim = popAnim;
	_openPoppedAnim = openPoppedAnim;
	_openEmptyAnim = openEmptyAnim;
	_returnDepth = returnDepth;
	_doorOpen = false;
	_popSwordAnim = popSwordAnim;
	_pod = pod;
}

byte *RetrieveFromPods::podStatusFlag(byte pod) {
	switch (pod) {
	case 1:
		return &_globalFlags.asRBPodAStatus;
	case 2:
		return &_globalFlags.asRBPodBStatus;
	case 3:
		return &_globalFlags.asRBPodCStatus;
	case 4:
		return &_globalFlags.asRBPodDStatus;
	case 5:
		return &_globalFlags.asRBPodEStatus;
	case 6:
		return &_globalFlags.asRBPodFStatus;
	default:
		return nullptr;
	}
}

byte RetrieveFromPods::getPodStatus() {
	byte *flag = podStatusFlag(_pod);
	return flag ? *flag : 0;
}

void RetrieveFromPods::setPodStatus(byte status) {
	byte *flag = podStatusFlag(_pod);
	if (flag)
		*flag = status;
}

void RetrieveFromPods::setPodStatus(byte pod, byte status) {
	byte *flag = podStatusFlag(pod);
	if (flag)
		*flag = status;
}

byte *RetrieveFromPods::podItemFlag() {
	switch (_pod) {
	case 1:
		return &_globalFlags.asRBPodATakenEnvironCart;
	case 2:
		return &_globalFlags.asRBPodBTakenPuzzleBox;
	case 3:
		return &_globalFlags.asRBPodCTakenCodex;
	case 4:
		return &_globalFlags.asRBPodDTakenSculpture;
	case 5:
		return &_globalFlags.asRBPodETakenSword;
	default:
		// Cheese girl in pod 6 cannot be picked up
		return nullptr;
	}
}

byte RetrieveFromPods::getPodItemFlag() {
	byte *flag = podItemFlag();
	return flag ? *flag : 0;
}

void RetrieveFromPods::setPodItemFlag(byte value) {
	byte *flag = podStatusFlag(_pod);
	if (flag)
		*flag = value;
}

int RetrieveFromPods::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_doorOpen && _grabObject.contains(pointLocation) && getPodStatus() == 1 && getPodItemFlag() == 0) {
		_staticData.navFrameIndex = _openEmptyFrame;
		setPodItemFlag(1);
		setPodStatus(2);

		// Begin dragging
		Common::Point ptInventoryWindow = viewWindow->convertPointToGlobal(pointLocation);
		ptInventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->convertPointToLocal(ptInventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(_itemID, ptInventoryWindow);

		// Update the biochips
		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int RetrieveFromPods::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (!_doorOpen && _openDoor.contains(pointLocation)) {
		_doorOpen = true;

		switch (getPodStatus()) {
		case 0:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_openAnim);
			_staticData.navFrameIndex = _openNormFrame;
			break;
		case 1:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_openPoppedAnim);
			_staticData.navFrameIndex = _openPoppedFrame;
			break;
		case 2:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_openEmptyAnim);
			_staticData.navFrameIndex = _openEmptyFrame;
			break;
		}

		return SC_TRUE;
	}

	if (_returnDepth >= 0) {
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = _returnDepth;
		destData.transitionType = TRANSITION_NONE;
		destData.transitionData = -1;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int RetrieveFromPods::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemExplosiveCharge || itemID == kItemRichardsSword) {
		if (_doorOpen && _openDoor.contains(pointLocation) && getPodStatus() == 0)
			return 1;

		return 0;
	}

	if (itemID == _itemID && _doorOpen && _grabObject.contains(pointLocation) && getPodItemFlag() == 1 && getPodStatus() == 2)
		return 1;

	return 0;
}

int RetrieveFromPods::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if ((itemID == kItemExplosiveCharge || itemID == kItemRichardsSword) && _doorOpen && _openDoor.contains(pointLocation) && getPodStatus() == 0) {
		// Play the popping movie and change the still frame
		if (itemID == kItemRichardsSword && _popSwordAnim >= 0)
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_popSwordAnim);
		else
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_popAnim);

		_staticData.navFrameIndex = _openPoppedFrame;

		setPodStatus(1);

		// If in walkthrough mode, open all the pods
		if (_globalFlags.generalWalkthroughMode == 1) {
			for (int i = 1; i <= 6; i++)
				setPodStatus(i, 1);
		}

		// Explosive charge doesn't get returned
		if (itemID == kItemExplosiveCharge)
			return SIC_ACCEPT;

		// Sword does
		return SIC_REJECT;
	}

	if (itemID == _itemID && _doorOpen) {
		if (pointLocation.x == -1 && pointLocation.y == -1) {
			_globalFlags.asTakenEvidenceThisTrip = 1;

			InventoryWindow *invWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow;
			if (invWindow->isItemInInventory(kItemEnvironCart) && invWindow->isItemInInventory(kItemMayanPuzzleBox) && invWindow->isItemInInventory(kItemCodexAtlanticus) && invWindow->isItemInInventory(kItemInteractiveSculpture) && invWindow->isItemInInventory(kItemRichardsSword))
				_globalFlags.scoreGotKrynnArtifacts = 1;
		} else if (_grabObject.contains(pointLocation) && getPodItemFlag() == 1 && getPodStatus() == 2) {
			// Change the still frame to reflect the return of the inventory item
			_staticData.navFrameIndex = _openPoppedFrame;
			viewWindow->invalidateWindow(false);

			// Reset flags
			setPodItemFlag(0);
			setPodStatus(1);
			return SIC_ACCEPT;
		}
	}

	return SIC_REJECT;
}

class DoubleZoomIn : public SceneBase {
public:
	DoubleZoomIn(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int leftA, int topA, int rightA, int bottomA, int depthA, int leftB, int topB, int rightB, int bottomB, int depthB);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _zoomRegion[2];
	int _zoomDest[2];
};

DoubleZoomIn::DoubleZoomIn(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int leftA, int topA, int rightA, int bottomA, int depthA, int leftB, int topB, int rightB, int bottomB, int depthB) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_zoomRegion[0] = Common::Rect(leftA, topA, rightA, bottomA);
	_zoomRegion[1] = Common::Rect(leftB, topB, rightB, bottomB);
	_zoomDest[0] = depthA;
	_zoomDest[1] = depthB;
}

int DoubleZoomIn::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 2; i++) {
		if (_zoomRegion[i].contains(pointLocation)) {
			DestinationScene destData;
			destData.destinationScene = _staticData.location;
			destData.destinationScene.depth = _zoomDest[i];
			destData.transitionType = TRANSITION_NONE;
			destData.transitionData = -1;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int DoubleZoomIn::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_zoomRegion[0].contains(pointLocation) || _zoomRegion[1].contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

int RetrieveFromPods::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_openDoor.contains(pointLocation) && !_doorOpen)
		return kCursorFinger;

	if (_grabObject.contains(pointLocation) && getPodStatus() == 1 && getPodItemFlag() == 0)
		return kCursorOpenHand;

	if (_returnDepth >= 0)
		return kCursorPutDown;

	return kCursorArrow;
}

class NerveNavigation : public SceneBase {
public:
	NerveNavigation(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _nerve;
	Location _forwardLocation;
};

NerveNavigation::NerveNavigation(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_nerve = Common::Rect(left, top, right, bottom);
	_forwardLocation = _staticData.destForward.destinationScene;
	_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
}

int NerveNavigation::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_nerve.contains(pointLocation)) {
		_staticData.destForward.destinationScene = _forwardLocation;
		((SceneViewWindow *)viewWindow)->moveToDestination(_staticData.destForward);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int NerveNavigation::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_nerve.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class ArmControls : public SceneBase {
public:
	ArmControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _controls[3];
	int _animIDs[3];
};

ArmControls::ArmControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_controls[0] = Common::Rect(124, 150, 140, 164);
	_controls[1] = Common::Rect(145, 146, 161, 160);
	_controls[2] = Common::Rect(155, 162, 165, 172);
	_animIDs[0] = 3;
	_animIDs[1] = 4;
	_animIDs[2] = 5;
}

int ArmControls::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 3; i++) {
		if (_controls[i].contains(pointLocation)) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_animIDs[i]);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int ArmControls::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 3; i++)
		if (_controls[i].contains(pointLocation))
			return kCursorFinger;

	return kCursorArrow;
}

class OpenAlienDoorA : public SceneBase {
public:
	OpenAlienDoorA(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
};

OpenAlienDoorA::OpenAlienDoorA(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int OpenAlienDoorA::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDangerDoorASealed != 1 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 0) {
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 1;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 16;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;

		Common::String text = _vm->getString(IDS_AS_RA_BEINGS_DETECTED_20_METERS);
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		((SceneViewWindow *)viewWindow)->displayLiveText(text);
		return SC_FALSE;
	}

	return SC_TRUE;
}

class AlienDoorAEncounter : public SceneBase {
public:
	AlienDoorAEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int timerCallback(Window *viewWindow) override;

private:
	uint32 _timerStart;
	LocationStaticData _originalSceneData;
	bool _cloaked;
	Common::Rect _nerve;
};

AlienDoorAEncounter::AlienDoorAEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_timerStart = 0;
	_cloaked = false;
	_nerve = Common::Rect(184, 160, 268, 189);

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1) {
		_originalSceneData = _staticData;
		_staticData.destUp.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destLeft.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destRight.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destDown.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	}
}

int AlienDoorAEncounter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1)
		_timerStart = g_system->getMillis();

	return SC_TRUE;
}

int AlienDoorAEncounter::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_staticData.location.depth != newLocation.depth && _staticData.location.timeZone == newLocation.timeZone && _staticData.location.node == newLocation.node)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 7), 128, false, true);

	return SC_TRUE;
}

int AlienDoorAEncounter::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemRichardsSword && ((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1 && _nerve.contains(pointLocation))
		return 1;

	return 0;
}

int AlienDoorAEncounter::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemRichardsSword && ((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1 && _nerve.contains(pointLocation)) {
		// Reset the nerve flag
		((SceneViewWindow *)viewWindow)->getGlobalFlags().asDangerDoorASealed = 1;

		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 0;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 11;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	}

	return SIC_REJECT;
}

int AlienDoorAEncounter::timerCallback(Window *viewWindow) {
	if (_timerStart != 0) {
		if (_cloaked) {
			if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 0) {
				_cloaked = false;
				_timerStart = g_system->getMillis();
			}
		} else {
			if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 1) {
				_cloaked = true;
			} else {
				if (_timerStart + 15000 < g_system->getMillis()) {
					((SceneViewWindow *)viewWindow)->playSynchronousAnimation(12);
					((SceneViewWindow *)viewWindow)->showDeathScene(50);
					return SC_DEATH;
				}
			}
		}
	}

	return SC_TRUE;
}

class AlienDoorAMoveDeath : public SceneBase {
public:
	AlienDoorAMoveDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int timerCallback(Window *viewWindow) override;
};

AlienDoorAMoveDeath::AlienDoorAMoveDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int AlienDoorAMoveDeath::timerCallback(Window *viewWindow) {
	((SceneViewWindow *)viewWindow)->showDeathScene(50);
	return SC_DEATH;
}

class AlienDoorBOpen : public SceneBase {
public:
	AlienDoorBOpen(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
};

AlienDoorBOpen::AlienDoorBOpen(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int AlienDoorBOpen::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDangerDoorASealed != 1) {
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 1;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 1) ? 15 : 13;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;

		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		return SC_FALSE;
	}

	return SC_TRUE;
}

class AlienDoorBEncounter : public SceneBase {
public:
	AlienDoorBEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
	int timerCallback(Window *viewWindow) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	uint32 _timerStart;
	LocationStaticData _originalSceneData;
	Common::Rect _nerve;
	Location _forwardLocation;
};

AlienDoorBEncounter::AlienDoorBEncounter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_timerStart = 0;
	_nerve = Common::Rect(154, 155, 256, 189);
	_forwardLocation = _staticData.destForward.destinationScene;
	_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 0) {
		_originalSceneData = _staticData;
		_staticData.destUp.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destLeft.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destRight.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destDown.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	} else {
		_staticData.navFrameIndex = 122;
	}
}

int AlienDoorBEncounter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 0) {
		_timerStart = g_system->getMillis();
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_AS_RA_BEINGS_DETECTED_20_METERS));
	}

	return SC_TRUE;
}

int AlienDoorBEncounter::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_staticData.location.depth != newLocation.depth && _staticData.location.timeZone == newLocation.timeZone && _staticData.location.node == newLocation.node)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 7), 128, false, true);

	return SC_TRUE;
}

int AlienDoorBEncounter::timerCallback(Window *viewWindow) {
	if (_timerStart != 0 && (_timerStart + 15000 < g_system->getMillis() || ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 1)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcCloakingEnabled == 0) {
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(9);
			((SceneViewWindow *)viewWindow)->showDeathScene(50);
		} else {
			_staticData = _originalSceneData;
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(8);
			_staticData.navFrameIndex = 122;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen = 1;
			_timerStart = 0;
			_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
		}
	}

	return SC_TRUE;
}

int AlienDoorBEncounter::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 1 && _nerve.contains(pointLocation)) {
		_staticData.destForward.destinationScene = _forwardLocation;
		((SceneViewWindow *)viewWindow)->moveToDestination(_staticData.destForward);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int AlienDoorBEncounter::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asDoorBGuardsSeen == 1 && _nerve.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class EncounterAmbassadorFirstZoom : public SceneBase {
public:
	EncounterAmbassadorFirstZoom(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _panel;
};

EncounterAmbassadorFirstZoom::EncounterAmbassadorFirstZoom(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_panel = Common::Rect(110, 0, 420, 62);
}

int EncounterAmbassadorFirstZoom::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asTakenEvidenceThisTrip == 1 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().asAmbassadorEncounter == 0) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().asAmbassadorEncounter = 1;

		// Here we go!
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_AS_RA_BEINGS_DETECTED_5_METERS));
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(18); // "Why are you here?"
		_staticData.navFrameIndex = 127;

		DestinationScene destData;
		destData.destinationScene = Location(7, 1, 6, 0, 1, 1);
		destData.transitionType = TRANSITION_NONE;
		destData.transitionData = -1;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	}

	return SC_TRUE;
}

int EncounterAmbassadorFirstZoom::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_panel.contains(pointLocation)) {
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 1;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 6;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int EncounterAmbassadorFirstZoom::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_panel.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

class AmbassadorEncounterPodField : public SceneBase {
public:
	AmbassadorEncounterPodField(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int timerCallback(Window *viewWindow) override;

private:
	uint32 _timerStart;
};

AmbassadorEncounterPodField::AmbassadorEncounterPodField(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_timerStart = 0;
}

int AmbassadorEncounterPodField::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_timerStart = g_system->getMillis();
	return SC_TRUE;
}

int AmbassadorEncounterPodField::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemBurnedOutCore)
		return 1;

	return 0;
}

int AmbassadorEncounterPodField::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemBurnedOutCore) {
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 2;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 20;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int AmbassadorEncounterPodField::timerCallback(Window *viewWindow) {
	if (_timerStart != 0 && (_timerStart + 30000) < g_system->getMillis()) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(19);
		((SceneViewWindow *)viewWindow)->showDeathScene(51);
		return SC_DEATH;
	}

	SceneBase::timerCallback(viewWindow);
	return SC_TRUE;
}

class AmbassadorEncounterPodWalkForward : public SceneBase {
public:
	AmbassadorEncounterPodWalkForward(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int timerCallback(Window *viewWindow) override;

private:
	uint32 _timerStart;
};

AmbassadorEncounterPodWalkForward::AmbassadorEncounterPodWalkForward(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_timerStart = 0;
}

int AmbassadorEncounterPodWalkForward::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_timerStart = g_system->getMillis();
	return SC_TRUE;
}

int AmbassadorEncounterPodWalkForward::timerCallback(Window *viewWindow) {
	if (_timerStart != 0 && (_timerStart + 15000) < g_system->getMillis()) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(21);
		((SceneViewWindow *)viewWindow)->showDeathScene(55);
		return SC_DEATH;
	}

	SceneBase::timerCallback(viewWindow);
	return SC_TRUE;
}

class AmbassadorEncounterTransportArmsOff : public SceneBase {
public:
	AmbassadorEncounterTransportArmsOff(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int timerCallback(Window *viewWindow) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	uint32 _timerStart;
	Common::Rect _transportButton;
};

AmbassadorEncounterTransportArmsOff::AmbassadorEncounterTransportArmsOff(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_timerStart = 0;
	_transportButton = Common::Rect(258, 38, 288, 82);
}

int AmbassadorEncounterTransportArmsOff::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_timerStart = g_system->getMillis();
	return SC_TRUE;
}

int AmbassadorEncounterTransportArmsOff::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_transportButton.contains(pointLocation)) {
		// Congrats, you defeated Icarus!
		_timerStart = 0;

		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreDefeatedIcarus = 1;

		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 0;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 24;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int AmbassadorEncounterTransportArmsOff::timerCallback(Window *viewWindow) {
	if (_timerStart != 0 && (_timerStart + 20000) < g_system->getMillis()) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(23);
		((SceneViewWindow *)viewWindow)->showDeathScene(54);
		return SC_DEATH;
	}

	SceneBase::timerCallback(viewWindow);
	return SC_TRUE;
}

int AmbassadorEncounterTransportArmsOff::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_transportButton.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class NormalTransporter : public SceneBase {
public:
	NormalTransporter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _clickRegion;
};

NormalTransporter::NormalTransporter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(118, 93, 153, 125);
}

int NormalTransporter::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().asAmbassadorEncounter == 1) {
			_vm->_sound->setAmbientSound();
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(1);
			_staticData.navFrameIndex = -1;
			((SceneViewWindow *)viewWindow)->showCompletionScene();
		} else {
			DestinationScene destData;
			destData.destinationScene = Location(3, 2, 4, 0, 1, 0);
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 1;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int NormalTransporter::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class EntryWithoutLensFilter : public SceneBase {
public:
	EntryWithoutLensFilter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int paint(Window *viewWindow, Graphics::Surface *preBuffer) override;
	int timerCallback(Window *viewWindow) override;

private:
	bool _transPlayed;
};

EntryWithoutLensFilter::EntryWithoutLensFilter(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_transPlayed = false;
}

int EntryWithoutLensFilter::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 8));

	Common::String text;
	if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0))
		text = _vm->getString(IDS_AS_VISIBILITY_MESSAGE);
	else
		text = "Alert: No visible light waves detected. Jumpsuit's built-in light source ineffective.";

	((SceneViewWindow *)viewWindow)->displayLiveText(text);
	return SC_TRUE;
}

int EntryWithoutLensFilter::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	preBuffer->fillRect(Common::Rect(432, 189), _vm->_gfx->getColor(0, 0, 0));
	return SC_REPAINT;
}

int EntryWithoutLensFilter::timerCallback(Window *viewWindow) {
	if (!_transPlayed && ((SceneViewWindow *)viewWindow)->getGlobalFlags().lensFilterActivated == 1) {
		_transPlayed = true;
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 0;
		destData.transitionType = TRANSITION_NONE;
		destData.transitionData = -1;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	}

	return SC_TRUE;
}

class PlayPodAudio : public SceneBase {
public:
	PlayPodAudio(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation, int untransAudio, int transSoundID);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;

private:
	int _untransSoundID;
	int _transSoundID;
};

PlayPodAudio::PlayPodAudio(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation, int untransAudio, int transSoundID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_untransSoundID = untransAudio;
	_transSoundID = transSoundID;
}

int PlayPodAudio::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		if (_transSoundID >= 0)
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _transSoundID));
	} else {
		if (_untransSoundID >= 0)
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _untransSoundID));
	}

	return SC_TRUE;
}

class InorganicPodTransDeath : public SceneBase {
public:
	InorganicPodTransDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int animID = -1, int deathScene = -1);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _clickRegion;
	int _transportAnimID;
	int _deathScene;
};

InorganicPodTransDeath::InorganicPodTransDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int animID, int deathScene) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(left, top, right, bottom);
	_transportAnimID = animID;
	_deathScene = deathScene;
}

int InorganicPodTransDeath::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		_vm->_sound->setAmbientSound();
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(_transportAnimID);
		_staticData.navFrameIndex = -1;
		((SceneViewWindow *)viewWindow)->showDeathScene(_deathScene);
	}

	return SC_FALSE;
}

int InorganicPodTransDeath::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class CheeseGirlPod : public RetrieveFromPods {
public:
	CheeseGirlPod(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;
};

CheeseGirlPod::CheeseGirlPod(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		RetrieveFromPods(vm, viewWindow, sceneStaticData, priorLocation, 128, 0, 352, 189, 20, 76, 21, 22, 77, 170, 54, 252, 156, 23, 78, 6, -1, 0, 28) {
}

int CheeseGirlPod::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_doorOpen && _grabObject.contains(pointLocation) && getPodStatus() == 1) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(23);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int CheeseGirlPod::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_openDoor.contains(pointLocation) && !_doorOpen)
		return kCursorFinger;

	// If we're over the grab region, use the finger cursor so we can click on Frank
	if (_grabObject.contains(pointLocation) && getPodStatus() == 1)
		return kCursorFinger;

	if (_returnDepth >= 0)
		return kCursorPutDown;

	return kCursorArrow;
}

class TransporterStatusRead : public SceneBase {
public:
	TransporterStatusRead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int gdiPaint(Window *viewWindow) override;
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _currentRegion;
	Common::Rect _transRegions[3];
};

TransporterStatusRead::TransporterStatusRead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_currentRegion = -1;
	_transRegions[0] = Common::Rect(184, 40, 208, 62);
	_transRegions[1] = Common::Rect(221, 55, 245, 77);
	_transRegions[2] = Common::Rect(262, 39, 286, 61);
}

int TransporterStatusRead::gdiPaint(Window *viewWindow) {
	if (_currentRegion >= 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(_transRegions[_currentRegion]);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int TransporterStatusRead::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		bool found = false;

		for (int i = 0; i < 3 && !found; i++) {
			if (_transRegions[i].contains(pointLocation)) {
				found = true;

				if (_currentRegion != i) {
					_currentRegion = i;
					viewWindow->invalidateWindow(false);
					((SceneViewWindow *)viewWindow)->displayTranslationText(_vm->getString(IDS_AS_RA_POD_A_STATUS_TEXT + i));
				}
			}
		}

		if (!found && _currentRegion >= 0) {
			_currentRegion = -1;
			viewWindow->invalidateWindow(false);
			((SceneViewWindow *)viewWindow)->displayLiveText();
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int TransporterStatusRead::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	DestinationScene destData;
	destData.destinationScene = _staticData.location;
	destData.destinationScene.depth = 0;
	destData.transitionType = TRANSITION_VIDEO;
	destData.transitionData = 7;
	destData.transitionStartFrame = -1;
	destData.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	return SC_TRUE;
}

int TransporterStatusRead::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	return kCursorPutDown;
}

bool SceneViewWindow::initializeAlienTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1) {
		_globalFlags.asInitialGuardsPass = 0;
		_globalFlags.asRBPodAStatus = 0;
		_globalFlags.asRBPodBStatus = 0;
		_globalFlags.asRBPodCStatus = 0;
		_globalFlags.asRBPodDStatus = 0;
		_globalFlags.asRBPodEStatus = 0;
		_globalFlags.asRBPodFStatus = 0;
	} else if (environment == 1) {
		_globalFlags.scoreTransportToKrynn = 1;
	}

	return true;
}

bool SceneViewWindow::startAlienAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(7, environment, SF_AMBIENT), oldTimeZone == 7 && fade, 64);
	return true;
}

SceneBase *SceneViewWindow::constructAlienSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	switch (sceneStaticData.classID) {
	case 0:
		// Default scene
		break;
	case 1:
		return new ArmControls(_vm, viewWindow, sceneStaticData, priorLocation);
	case 2:
		return new EncounterAmbassadorFirstZoom(_vm, viewWindow, sceneStaticData, priorLocation);
	case 3:
		return new TransporterStatusRead(_vm, viewWindow, sceneStaticData, priorLocation);
	case 4:
		return new OpenAlienDoorA(_vm, viewWindow, sceneStaticData, priorLocation);
	case 5:
		return new AlienDoorAEncounter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 6:
		return new AlienDoorAMoveDeath(_vm, viewWindow, sceneStaticData, priorLocation);
	case 7:
		return new AlienDoorBOpen(_vm, viewWindow, sceneStaticData, priorLocation);
	case 8:
		return new AlienDoorBEncounter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 9:
		return new AmbassadorEncounterPodField(_vm, viewWindow, sceneStaticData, priorLocation);
	case 10:
		return new AmbassadorEncounterPodWalkForward(_vm, viewWindow, sceneStaticData, priorLocation);
	case 11:
		return new AmbassadorEncounterTransportArmsOff(_vm, viewWindow, sceneStaticData, priorLocation);
	case 12:
		return new NormalTransporter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 13:
		return new EntryWithoutLensFilter(_vm, viewWindow, sceneStaticData, priorLocation);
	case 20:
		return new RetrieveFromPods(_vm, viewWindow, sceneStaticData, priorLocation, 172, 46, 272, 166, 0, 61, 1, 2, 62, 198, 78, 248, 116, 3, 63, 1, kItemEnvironCart, -1, 29);
	case 21:
		return new DoubleZoomIn(_vm, viewWindow, sceneStaticData, priorLocation, 240, 88, 300, 178, 1, 100, 0, 160, 98, 2);
	case 22:
		return new RetrieveFromPods(_vm, viewWindow, sceneStaticData, priorLocation, 150, 0, 394, 189, 4, 64, 5, 6, 65, 190, 74, 312, 142, 7, 66, 2, kItemMayanPuzzleBox, 0, 25);
	case 23:
		return new RetrieveFromPods(_vm, viewWindow, sceneStaticData, priorLocation, 140, 8, 274, 189, 8, 67, 9, 10, 68, 176, 42, 232, 124, 11, 69, 3, kItemCodexAtlanticus, 0, 26);
	case 24:
		return new RetrieveFromPods(_vm, viewWindow, sceneStaticData, priorLocation, 100, 0, 280, 189, 12, 70, 13, 14, 71, 146, 60, 252, 156, 15, 72, 4, kItemInteractiveSculpture, -1, 27);
	case 25:
		return new DoubleZoomIn(_vm, viewWindow, sceneStaticData, priorLocation, 256, 0, 322, 100, 1, 106, 84, 172, 189, 2);
	case 26:
		return new RetrieveFromPods(_vm, viewWindow, sceneStaticData, priorLocation, 134, 0, 276, 189, 16, 73, 17, 18, 74, 190, 4, 224, 166, 19, 75, 5, kItemRichardsSword, 0);
	case 27:
		return new CheeseGirlPod(_vm, viewWindow, sceneStaticData, priorLocation);
	case 30:
		return new PlayPodAudio(_vm, viewWindow, sceneStaticData, priorLocation, 9, 10);
	case 31:
		return new PlayPodAudio(_vm, viewWindow, sceneStaticData, priorLocation, 11, 12);
	case 32:
		return new InorganicPodTransDeath(_vm, viewWindow, sceneStaticData, priorLocation, 92, 88, 158, 128, 2, 52);
	case 33:
		return new InorganicPodTransDeath(_vm, viewWindow, sceneStaticData, priorLocation, 92, 88, 158, 128, 26, 53);
	case 40:
		return new NerveNavigation(_vm, viewWindow, sceneStaticData, priorLocation, 262, 122, 302, 189);
	case 41:
		return new NerveNavigation(_vm, viewWindow, sceneStaticData, priorLocation, 170, 144, 250, 180);
	case 42:
		return new NerveNavigation(_vm, viewWindow, sceneStaticData, priorLocation, 180, 160, 270, 189);
	case 50:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 127, globalFlags.asRBLastStingerID, globalFlags.asRBStingerID, 10, 14);
	default:
		warning("Unknown Alien scene object %d", sceneStaticData.classID);
		break;
	}

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
