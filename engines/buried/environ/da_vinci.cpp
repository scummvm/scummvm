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

#include "buried/avi_frames.h"
#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/navarrow.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

#include "graphics/surface.h"

namespace Buried {

class SwapStillOnFlag : public SceneBase {
public:
	SwapStillOnFlag(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int flagOffset = -1, int flagValue = -1);
};

SwapStillOnFlag::SwapStillOnFlag(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int flagOffset, int flagValue) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(flagOffset) >= flagValue) {
		int curStillFrame = _staticData.navFrameIndex;
		_staticData.navFrameIndex = _staticData.miscFrameIndex;
		_staticData.miscFrameIndex = curStillFrame;
	}
}

class CapturePaintingTowerFootprint : public SceneBase {
public:
	CapturePaintingTowerFootprint(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int locateAttempted(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _footprint;
};

CapturePaintingTowerFootprint::CapturePaintingTowerFootprint(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent >= 1) {
		int curStillFrame = _staticData.navFrameIndex;
		_staticData.navFrameIndex = _staticData.miscFrameIndex;
		_staticData.miscFrameIndex = curStillFrame;
	}

	_footprint = Common::Rect(218, 112, 244, 132);
}

int CapturePaintingTowerFootprint::locateAttempted(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcLocateEnabled == 1 && _footprint.contains(pointLocation)) {
		// Play the capture animation
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent >= 1)
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(1);
		else
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(0);

		// Attempt to add it to the biochip
		if (((SceneViewWindow *)viewWindow)->addNumberToGlobalFlagTable(offsetof(GlobalFlags, evcapBaseID), offsetof(GlobalFlags, evcapNumCaptured), 12, DAVINCI_EVIDENCE_FOOTPRINT))
			((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ACQUIRED));
		else
			((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

		// Disable capture
		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->disableEvidenceCapture();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int CapturePaintingTowerFootprint::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcLocateEnabled == 1) {
		if (_footprint.contains(pointLocation)) // Cursor change
			return -2;

		// Use locate A
		return -1;
	}

	return kCursorArrow;
}

class PaintingTowerWalkOntoElevator : public SceneBase {
public:
	PaintingTowerWalkOntoElevator(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation);
};

PaintingTowerWalkOntoElevator::PaintingTowerWalkOntoElevator(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent >= 1) {
		// If the elevator is present, don't die
		int curStillFrame = _staticData.navFrameIndex;
		_staticData.navFrameIndex = _staticData.miscFrameIndex;
		_staticData.miscFrameIndex = curStillFrame;

		_staticData.destForward.destinationScene.timeZone = 5;
		_staticData.destForward.destinationScene.environment = 1;
		_staticData.destForward.destinationScene.node = 8;
		_staticData.destForward.destinationScene.facing = 0;
		_staticData.destForward.destinationScene.orientation = 1;
		_staticData.destForward.destinationScene.depth = 0;
		_staticData.destForward.transitionType = TRANSITION_WALK;
		_staticData.destForward.transitionData = 6;
		_staticData.destForward.transitionStartFrame = 56;
		_staticData.destForward.transitionLength = 16;
	}
}

int PaintingTowerWalkOntoElevator::postExitRoom(Window *viewWindow, const Location &newLocation) {
	if (newLocation.timeZone == _staticData.location.timeZone &&
			newLocation.environment == _staticData.location.environment &&
			newLocation.node == _staticData.location.node &&
			newLocation.facing == _staticData.location.facing &&
			newLocation.orientation == _staticData.location.orientation &&
			newLocation.depth == _staticData.location.depth) {
		// Notify the player of his gruesome death
		((SceneViewWindow *)viewWindow)->showDeathScene(30);
		return SC_DEATH;
	}

	// Reset the elevator since we walked down (clone2727 asks if this is possible)
	if (newLocation.timeZone == _staticData.location.timeZone && newLocation.environment == 3)
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent = 0;

	return SC_TRUE;
}

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

class PaintingTowerElevatorControls : public SceneBase {
public:
	PaintingTowerElevatorControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int gdiPaint(Window *viewWindow);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _lockHandle[2];
	Common::Rect _directionHandle[2];
	Common::Rect _transText[4];
	int _textTranslated;
};

PaintingTowerElevatorControls::PaintingTowerElevatorControls(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_lockHandle[0] = Common::Rect(152, 72, 186, 109);
	_lockHandle[1] = Common::Rect(152, 108, 186, 146);
	_directionHandle[0] = Common::Rect(252, 72, 312, 108);
	_directionHandle[1] = Common::Rect(252, 109, 312, 144);
	_transText[0] = Common::Rect(134, 50, 202, 70);
	_transText[1] = Common::Rect(136, 150, 198, 168);
	_transText[2] = Common::Rect(226, 52, 278, 70);
	_transText[3] = Common::Rect(224, 148, 288, 166);
	_textTranslated = -1;
}

int PaintingTowerElevatorControls::gdiPaint(Window *viewWindow) {
	if (_textTranslated >= 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(_transText[_textTranslated]);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int PaintingTowerElevatorControls::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_lockHandle[0].contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(4);
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverA = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTUseElevatorControls = 1;
		return SC_TRUE;
	} else if (_lockHandle[1].contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(3);
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverA = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTUseElevatorControls = 1;
		return SC_TRUE;
	} else if (_directionHandle[0].contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(6);
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverB = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTUseElevatorControls = 1;
		return SC_TRUE;
	} else if (_directionHandle[1].contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(5);
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverB = 0;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTUseElevatorControls = 1;
		return SC_TRUE;
	}

	return SC_FALSE;
}

int PaintingTowerElevatorControls::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		int newTextTrans = -1;

		for (int i = 0; i < 4; i++) {
			if (_transText[i].contains(pointLocation)) {
				((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTTransElevatorControls = 1;
				newTextTrans = i;

				if (newTextTrans != _textTranslated) {
					// Load and display the new text
					((SceneViewWindow *)viewWindow)->displayTranslationText(_vm->getString(IDDS_ELEVATOR_CONTROLS_TEXT_A + newTextTrans));
					_textTranslated = newTextTrans;
					viewWindow->invalidateWindow(false);
					break;
				}
			}
		}
	} else {
		if (_textTranslated != -1) {
			_textTranslated = -1;
			viewWindow->invalidateWindow(false);
		}
	}

	return SC_FALSE;
}

int PaintingTowerElevatorControls::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_lockHandle[0].contains(pointLocation))
		return kCursorArrowUp;
	else if (_lockHandle[1].contains(pointLocation))
		return kCursorArrowDown;
	else if (_directionHandle[0].contains(pointLocation))
		return kCursorArrowUp;
	else if (_directionHandle[1].contains(pointLocation))
		return kCursorArrowDown;

	return kCursorArrow;
}

class PaintingTowerElevatorWheel : public SceneBase {
public:
	PaintingTowerElevatorWheel(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _wheel;
};

PaintingTowerElevatorWheel::PaintingTowerElevatorWheel(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_wheel = Common::Rect(94, 0, 380, 189);
}

int PaintingTowerElevatorWheel::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_wheel.contains(pointLocation)) {
		byte lockStatus = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverA;
		byte direction = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverB;
		byte elevatorPosition = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent;

		if (lockStatus == 1) {
			if (direction == 0 && elevatorPosition == 1) {
				((SceneViewWindow *)viewWindow)->playSynchronousAnimation(7);
				((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent = 0;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTRaisedPlatform = 0;
				return SC_TRUE;
			} else if (direction == 1 && elevatorPosition == 0) {
				((SceneViewWindow *)viewWindow)->playSynchronousAnimation(8);
				((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent = 1;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTRaisedPlatform = 1;
				return SC_TRUE;
			}
		}
	}

	return SC_FALSE;
}

int PaintingTowerElevatorWheel::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_wheel.contains(pointLocation)) {
		byte lockStatus = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverA;
		byte direction = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverB;
		byte elevatorPosition = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorPresent;

		if (lockStatus == 1) {
			if (direction == 1 && elevatorPosition == 0)
				return kCursorArrowLeft;
			else if (direction == 0 && elevatorPosition == 1)
				return kCursorArrowRight;
		}
	}

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

class WheelAssemblyItemAcquire : public SceneBase {
public:
	WheelAssemblyItemAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = 0, int top = 0, int right = 0, int bottom = 0, int itemID = 0, int clearStillFrame = 0, int itemFlagOffset = 0);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	bool _itemPresent;
	Common::Rect _acquireRegion;
	int _fullFrameIndex;
	int _clearFrameIndex;
	int _itemID;
	int _itemFlagOffset;
	Common::Rect _zoomUpRegion;
};

WheelAssemblyItemAcquire::WheelAssemblyItemAcquire(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int itemID, int clearStillFrame, int itemFlagOffset) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_itemPresent = true;
	_itemID = itemID;
	_acquireRegion = Common::Rect(left, top, right, bottom);
	_fullFrameIndex = _staticData.navFrameIndex;
	_clearFrameIndex = clearStillFrame;
	_itemFlagOffset = itemFlagOffset;
	_zoomUpRegion = Common::Rect(134, 168, 200, 189);

	if (((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_itemFlagOffset) != 0) {
		_itemPresent = false;
		_staticData.navFrameIndex = _clearFrameIndex;
	}
}

int WheelAssemblyItemAcquire::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_itemPresent && _acquireRegion.contains(pointLocation)) {
		_itemPresent = false;
		_staticData.navFrameIndex = _clearFrameIndex;
		viewWindow->invalidateWindow(false);

		if (_itemFlagOffset >= 0)
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_itemFlagOffset, 1);

		Common::Point ptInventoryWindow = viewWindow->convertPointToGlobal(pointLocation);
		ptInventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->convertPointToLocal(ptInventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(_itemID, ptInventoryWindow);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int WheelAssemblyItemAcquire::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (!_itemPresent && _zoomUpRegion.contains(pointLocation)) {
		DestinationScene destData;
		destData.destinationScene = Location(5, 4, 8, 1, 1, 1);
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 15;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	}

	return SC_FALSE;
}

int WheelAssemblyItemAcquire::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;

	if (_itemID == itemID && !_itemPresent && pointLocation.x >= 0 && pointLocation.y >= 0) {
		_itemPresent = true;
		_staticData.navFrameIndex = _fullFrameIndex;

		if (_itemFlagOffset >= 0)
			((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_itemFlagOffset, 0);

		viewWindow->invalidateWindow(false);
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int WheelAssemblyItemAcquire::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_itemPresent && _acquireRegion.contains(pointLocation))
		return kCursorOpenHand;

	if (!_itemPresent && _zoomUpRegion.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

enum {
	DS_SC_DRIVE_ASSEMBLY = 1,
	DS_SC_WHEEL_ASSEMBLY = 2,
	DS_SC_PEGS = 4,
	DS_SC_COMPLETED = 8
};

class AssembleSiegeCycle : public SceneBase {
public:
	AssembleSiegeCycle(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);

private:
	Common::Rect _driveDropRegion;
	Common::Rect _wheelDropRegion;
	Common::Rect _pegDropRegion;
	Common::Rect _completedCycle;

	bool resetStillFrame(Window *viewWindow);
};

AssembleSiegeCycle::AssembleSiegeCycle(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_driveDropRegion = Common::Rect(102, 0, 170, 140);
	_wheelDropRegion = Common::Rect(264, 0, 334, 170);
	_pegDropRegion = Common::Rect(260, 10, 310, 162);
	_completedCycle = Common::Rect(102, 0, 330, 189);

	// Determine which still frame to display
	resetStillFrame(viewWindow);
}

int AssembleSiegeCycle::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_completedCycle.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSGrabbedSiegeCycle == 0) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_COMPLETED) {
			// Reset the present flag and change frame index of background
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSGrabbedSiegeCycle = 1;
			resetStillFrame(viewWindow);

			// Begin dragging
			Common::Point ptInventoryWindow = viewWindow->convertPointToGlobal(pointLocation);
			ptInventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->convertPointToLocal(ptInventoryWindow);
			((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(kItemSiegeCycle, ptInventoryWindow);
			return SC_TRUE;
		} else {
			// Otherwise, check to see if the pegs have been placed and haven't been hammer in
			if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_PEGS)
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_DS_WS_UNSTABLE_CYCLE_MESSAGE));
		}
	}

	return SC_FALSE;
}

int AssembleSiegeCycle::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_completedCycle.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSGrabbedSiegeCycle == 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_COMPLETED) != 0)
		return kCursorOpenHand;

	if (_completedCycle.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSGrabbedSiegeCycle == 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_DRIVE_ASSEMBLY) != 0
			&& (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_WHEEL_ASSEMBLY) != 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_PEGS) != 0)
		return kCursorOpenHand;

	return kCursorArrow;
}

int AssembleSiegeCycle::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	switch (itemID) {
	case kItemDriveAssembly:
		if (_driveDropRegion.contains(pointLocation) && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_DRIVE_ASSEMBLY) == 0)
			return 1;
		break;
	case kItemWheelAssembly:
		if (_wheelDropRegion.contains(pointLocation) && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_WHEEL_ASSEMBLY) == 0)
			return 1;
		break;
	case kItemWoodenPegs:
		if (_pegDropRegion.contains(pointLocation) && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_PEGS) == 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_WHEEL_ASSEMBLY) != 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_DRIVE_ASSEMBLY) != 0)
			return 1;
		break;
	case kItemHammer:
		if (_pegDropRegion.contains(pointLocation) && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_PEGS) != 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_WHEEL_ASSEMBLY) != 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_DRIVE_ASSEMBLY) != 0)
			return 1;
		break;
	}

	return 0;
}

int AssembleSiegeCycle::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;

	switch (itemID) {
	case kItemDriveAssembly:
		if (_driveDropRegion.contains(pointLocation) && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_DRIVE_ASSEMBLY) == 0) {
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus |= DS_SC_DRIVE_ASSEMBLY;
			resetStillFrame(viewWindow);
			viewWindow->invalidateWindow(false);
			return SIC_ACCEPT;
		}
		break;
	case kItemWheelAssembly:
		if (_wheelDropRegion.contains(pointLocation) && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_WHEEL_ASSEMBLY) == 0) {
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus |= DS_SC_WHEEL_ASSEMBLY;
			resetStillFrame(viewWindow);
			viewWindow->invalidateWindow(false);
			return SIC_ACCEPT;
		}
		break;
	case kItemWoodenPegs:
		if (_pegDropRegion.contains(pointLocation) && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_PEGS) == 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_WHEEL_ASSEMBLY) != 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_DRIVE_ASSEMBLY) != 0) {
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus |= DS_SC_PEGS;
			resetStillFrame(viewWindow);
			viewWindow->invalidateWindow(false);
			return SIC_ACCEPT;
		}
		break;
	case kItemHammer:
		if (_pegDropRegion.contains(pointLocation) && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_PEGS) != 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_WHEEL_ASSEMBLY) != 0 && (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus & DS_SC_DRIVE_ASSEMBLY) != 0) {
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus |= DS_SC_COMPLETED;
			resetStillFrame(viewWindow);

			// Play the hammer movie
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(3);

			viewWindow->invalidateWindow(false);
		}
		break;
	}

	return SIC_REJECT;
}

bool AssembleSiegeCycle::resetStillFrame(Window *viewWindow) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSGrabbedSiegeCycle >= 1) {
		_staticData.navFrameIndex = 213;
	} else {
		byte status = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus;
		_staticData.navFrameIndex = 105;

		if (status & DS_SC_COMPLETED) {
			_staticData.navFrameIndex = 220;
		} else {
			if ((status & DS_SC_DRIVE_ASSEMBLY) != 0 && (status & DS_SC_WHEEL_ASSEMBLY) != 0 && (status & DS_SC_PEGS) != 0) {
				_staticData.navFrameIndex = 215;
			} else {
				if ((status & DS_SC_DRIVE_ASSEMBLY) != 0 && (status & DS_SC_WHEEL_ASSEMBLY) != 0) {
					_staticData.navFrameIndex = 220;
				} else {
					if ((status & DS_SC_DRIVE_ASSEMBLY) != 0)
						_staticData.navFrameIndex = 216;
					else if ((status & DS_SC_WHEEL_ASSEMBLY) != 0)
						_staticData.navFrameIndex = 218;
				}
			}
		}
	}

	return true;
}

class SiegeCycleTopView : public SceneBase {
public:
	SiegeCycleTopView(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
};

SiegeCycleTopView::SiegeCycleTopView(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSGrabbedSiegeCycle >= 1) {
		_staticData.navFrameIndex = 214;
	} else {
		byte status = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSiegeCycleStatus;
		_staticData.navFrameIndex = 106;

		if (status & DS_SC_COMPLETED) {
			_staticData.navFrameIndex = 221;
		} else {
			if ((status & DS_SC_DRIVE_ASSEMBLY) != 0 && (status & DS_SC_WHEEL_ASSEMBLY) != 0 && (status & DS_SC_PEGS) != 0) {
				_staticData.navFrameIndex = 221;
			} else {
				if ((status & DS_SC_DRIVE_ASSEMBLY) != 0 && (status & DS_SC_WHEEL_ASSEMBLY) != 0) {
					_staticData.navFrameIndex = 221;
				} else {
					if ((status & DS_SC_DRIVE_ASSEMBLY) != 0)
						_staticData.navFrameIndex = 217;
					else if ((status & DS_SC_WHEEL_ASSEMBLY) != 0)
						_staticData.navFrameIndex = 219;
				}
			}
		}
	}
}

class UnlockCodexTowerDoor : public SceneBase {
public:
	UnlockCodexTowerDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);

private:
	Common::Rect _dropRect;
};

UnlockCodexTowerDoor::UnlockCodexTowerDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_dropRect = Common::Rect(138, 120, 198, 189);
}

int UnlockCodexTowerDoor::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemBalconyKey && _dropRect.contains(pointLocation))
		return 1;

	return 0;
}

int UnlockCodexTowerDoor::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;

	if (itemID == kItemBalconyKey && _dropRect.contains(pointLocation)) {
		// Play the unlocking animation
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(0);

		// Set the unlocked door flag
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTUnlockedDoor = 1;
	}

	// Player keeps the item
	return SIC_REJECT;
}

class CodexTowerOutsideDoor : public SceneBase {
public:
	CodexTowerOutsideDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _dropRect;
	Common::Rect _doorRect;
};

CodexTowerOutsideDoor::CodexTowerOutsideDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_dropRect = Common::Rect(130, 0, 200, 189);
	_doorRect = Common::Rect(0, 0, 166, 189);
}

int CodexTowerOutsideDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_doorRect.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTUnlockedDoor >= 1) {
			// Set the opened door flag
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverOpenedBalconyDoor = 1;

			DestinationScene destData;
			destData.destinationScene = _staticData.location;
			destData.destinationScene.depth = 1;
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 2;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;

			// Play a different video otherwise
			if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTViewedAgent3 == 0) {
				destData.transitionType = TRANSITION_VIDEO;
				destData.transitionData = 1;
			}

			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		} else {
			// Door is locked, play the door lock sound and set the tried flag
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTTriedLockedDoor = 1;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYTriedOpeningDoor = 1;
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13));

			// Check for spontaneous AI comments
			if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI))
				((SceneViewWindow *)viewWindow)->playAIComment(_staticData.location, AI_COMMENT_TYPE_SPONTANEOUS);

			((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		}
	}

	return SC_FALSE;
}

int CodexTowerOutsideDoor::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	// Little known fact that the metal bar can be used to unlock the door
	if (itemID == kItemMetalBar && _dropRect.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTUnlockedDoor == 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTViewedAgent3 == 0)
		return 1;

	return 0;
}

int CodexTowerOutsideDoor::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;

	if (itemID == kItemMetalBar && _dropRect.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTUnlockedDoor == 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTViewedAgent3 == 0) {
		// Move to the next scene
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.destinationScene.depth = 1;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionData = 2;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;
		((SceneViewWindow *)viewWindow)->moveToDestination(destData);

		// Set the unlocked door flag
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTUnlockedDoor = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCTViewedAgent3 = 1;
	}

	// Player keeps the item
	return SIC_REJECT;
}

int CodexTowerOutsideDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_doorRect.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class CourtyardCannon : public SceneBase {
public:
	CourtyardCannon(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _cannon;
};

CourtyardCannon::CourtyardCannon(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_cannon = Common::Rect(160, 10, 280, 140);
}

int CourtyardCannon::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_cannon.contains(pointLocation))
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(0);

	return SC_TRUE;
}

int CourtyardCannon::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_cannon.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class CourtyardGunDeath : public SceneBase {
public:
	CourtyardGunDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _gun;
};

CourtyardGunDeath::CourtyardGunDeath(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_gun = Common::Rect(140, 68, 294, 189);
}

int CourtyardGunDeath::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_gun.contains(pointLocation)) {
		// Play the animation
		((SceneViewWindow *)viewWindow)->playSynchronousAnimation(4);

		// Kill the player
		((SceneViewWindow *)viewWindow)->showDeathScene(31);
	}

	// Success!
	return SC_TRUE;
}

int CourtyardGunDeath::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_gun.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class ChangeBallistaDepth : public SceneBase {
public:
	ChangeBallistaDepth(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
};

ChangeBallistaDepth::ChangeBallistaDepth(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaStatus != _staticData.location.depth) {
		Location newLocation = _staticData.location;
		newLocation.depth = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaStatus;
		((SceneViewWindow *)viewWindow)->getSceneStaticData(newLocation, _staticData);
	}
}

class SpinBallista : public SceneBase {
public:
	SpinBallista(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _handle;
};

SpinBallista::SpinBallista(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_handle = Common::Rect(126, 106, 190, 162);
}

int SpinBallista::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_handle.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaStatus < 2) {
		DestinationScene destData;
		destData.destinationScene = _staticData.location;
		destData.transitionType = TRANSITION_VIDEO;
		destData.transitionStartFrame = -1;
		destData.transitionLength = -1;

		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaStatus == 1) {
			destData.transitionData = 7;
			destData.destinationScene.depth = 0;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaStatus = 0;
		} else {
			destData.transitionData = 6;
			destData.destinationScene.depth = 1;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaStatus = 1;
		}

		((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	}

	return SC_TRUE;
}

int SpinBallista::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_handle.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaStatus < 2)
		return kCursorFinger;

	return kCursorArrow;
}

class PlaceSiegeCycleOnTrack : public SceneBase {
public:
	PlaceSiegeCycleOnTrack(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);

private:
	Common::Rect _cycleRect;
	void setArrows(Window *viewWindow);
};

PlaceSiegeCycleOnTrack::PlaceSiegeCycleOnTrack(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_cycleRect = Common::Rect(0, 0, 350, 160);

	// If we placed the cycle on the track, change the still frame
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYPlacedSiegeCycle != 0) {
		_staticData.navFrameIndex = 229;
		setArrows(viewWindow);
	}
}

int PlaceSiegeCycleOnTrack::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemSiegeCycle && _cycleRect.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYPlacedSiegeCycle == 0)
		return 1;

	return 0;
}

int PlaceSiegeCycleOnTrack::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;

	if (itemID == kItemSiegeCycle && _cycleRect.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYPlacedSiegeCycle == 0) {
		_staticData.navFrameIndex = 229;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYPlacedSiegeCycle = 1;
		viewWindow->invalidateWindow(false);
		setArrows(viewWindow);
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

void PlaceSiegeCycleOnTrack::setArrows(Window *viewWindow) {
	// Can only walk to the siege cycle
	_staticData.destForward.destinationScene = Location(5, 5, 13, 0, 0, 0);
	_staticData.destForward.transitionType = TRANSITION_VIDEO;
	_staticData.destForward.transitionData = 12;
	_staticData.destForward.transitionStartFrame = -1;
	_staticData.destForward.transitionLength = -1;

	_staticData.destUp.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destUp.transitionType = -1;
	_staticData.destUp.transitionData = -1;
	_staticData.destUp.transitionStartFrame = -1;
	_staticData.destUp.transitionLength = -1;

	_staticData.destLeft.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destLeft.transitionType = -1;
	_staticData.destLeft.transitionData = -1;
	_staticData.destLeft.transitionStartFrame = -1;
	_staticData.destLeft.transitionLength = -1;

	_staticData.destRight.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destRight.transitionType = -1;
	_staticData.destRight.transitionData = -1;
	_staticData.destRight.transitionStartFrame = -1;
	_staticData.destRight.transitionLength = -1;

	_staticData.destDown.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destDown.transitionType = -1;
	_staticData.destDown.transitionData = -1;
	_staticData.destDown.transitionStartFrame = -1;
	_staticData.destDown.transitionLength = -1;

	((GameUIWindow *)viewWindow->getParent())->_navArrowWindow->updateAllArrows(_staticData);
}

class AimBallistaAwayFromTower : public SceneBase {
public:
	AimBallistaAwayFromTower(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	~AimBallistaAwayFromTower();
	void preDestructor();
	int paint(Window *viewWindow, Graphics::Surface *preBuffer);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _raiseBallista;
	Common::Rect _lowerBallista;
	Common::Rect _turnBallistaLeft;
	Common::Rect _turnBallistaRight;
	Common::Rect _ballistaHandle;
	AVIFrames *_viewFrameExtractor;
};

AimBallistaAwayFromTower::AimBallistaAwayFromTower(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_turnBallistaLeft = Common::Rect(0, 84, 44, 189);
	_turnBallistaRight = Common::Rect(45, 84, 90, 189);
	_lowerBallista = Common::Rect(368, 82, 432, 189);
	_raiseBallista = Common::Rect(304, 82, 367, 189);
	_ballistaHandle = Common::Rect(170, 116, 212, 189);

	_viewFrameExtractor = new AVIFrames(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 6));
}

AimBallistaAwayFromTower::~AimBallistaAwayFromTower() {
	preDestructor();
}

void AimBallistaAwayFromTower::preDestructor() {
	delete _viewFrameExtractor;
	_viewFrameExtractor = 0;
}

int AimBallistaAwayFromTower::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	SceneBase::paint(viewWindow, preBuffer);

	byte xPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaXPos;
	byte yPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaYPos;
	const Graphics::Surface *frame = _viewFrameExtractor->getFrame(yPos * 20 + xPos + 200);

	if (frame)
		_vm->_gfx->crossBlit(preBuffer, 120, 51, 160, 56, frame, 0, 0);

	return SC_REPAINT;
}

int AimBallistaAwayFromTower::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_ballistaHandle.contains(pointLocation)) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverShotBallista = 1;

		// Play the handle movie
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(14, 96, 110, 296, 189);

		return SC_TRUE;
	} else if (_raiseBallista.contains(pointLocation)) {
		byte &yPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaYPos;

		if (yPos == 0)
			return SC_FALSE;

		yPos--;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverUsedCrank = 1;

		// Start the spin sound
		int soundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12), 128, true, false);

		// Spin the wheel halfway
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(17, 300, 70, 432, 189);

		// Stop the spin sound
		_vm->_sound->stopSoundEffect(soundID);

		// Repaint
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	} else if (_lowerBallista.contains(pointLocation)) {
		byte &yPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaYPos;

		if (yPos >= 4)
			return SC_FALSE;

		yPos++;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverUsedCrank = 1;

		// Start the spin sound
		int soundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12), 128, true, false);

		// Spin the wheel halfway
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(18, 300, 70, 432, 189);

		// Stop the spin sound
		_vm->_sound->stopSoundEffect(soundID);

		// Repaint
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	} else if (_turnBallistaRight.contains(pointLocation)) {
		byte &xPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaXPos;

		if (xPos >= 19)
			return SC_FALSE;

		xPos++;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverUsedCrank = 1;

		// Start the spin sound
		int soundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12), 128, true, false);

		// Spin the wheel halfway
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(16, 0, 70, 100, 189);

		// Stop the spin sound
		_vm->_sound->stopSoundEffect(soundID);

		// Repaint
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	} else if (_turnBallistaLeft.contains(pointLocation)) {
		byte &xPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaXPos;

		if (xPos == 0)
			return SC_FALSE;

		xPos--;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverUsedCrank = 1;

		// Start the spin sound
		int soundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12), 128, true, false);

		// Spin the wheel halfway
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(15, 0, 70, 100, 189);

		// Stop the spin sound
		_vm->_sound->stopSoundEffect(soundID);

		// Repaint
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	}

	// Return to previous scene
	DestinationScene destData;
	destData.destinationScene = Location(5, 5, 8, 5, 1, 0);
	destData.transitionType = TRANSITION_VIDEO;
	destData.transitionData = 10;
	destData.transitionStartFrame = -1;
	destData.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	return SC_TRUE;
}

int AimBallistaAwayFromTower::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_raiseBallista.contains(pointLocation))
		return kCursorArrowUp;

	if (_lowerBallista.contains(pointLocation))
		return kCursorArrowDown;

	if (_turnBallistaRight.contains(pointLocation))
		return kCursorArrowRight;

	if (_turnBallistaLeft.contains(pointLocation))
		return kCursorArrowLeft;

	if (_ballistaHandle.contains(pointLocation))
		return kCursorFinger;

	return kCursorPutDown;
}

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

class ClickChangeSceneTranslate : public SceneBase {
public:
	ClickChangeSceneTranslate(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int cursorID = 0, int timeZone = -1, int environment = -1,
			int node = -1, int facing = -1, int orientation = -1, int depth = -1, int transitionType = -1, int transitionData = -1,
			int transitionStartFrame = -1, int transitionLength = -1, int transLeft = -1, int transTop = -1, int transRight = -1,
			int transBottom = -1, int transTextID = -1);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int gdiPaint(Window *viewWindow);
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	Common::Rect _clickRegion;
	DestinationScene _clickDestination;
	Common::Rect _translateRect;
	int _textID;
	bool _textTranslated;
};

ClickChangeSceneTranslate::ClickChangeSceneTranslate(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int cursorID, int timeZone, int environment,
		int node, int facing, int orientation, int depth, int transitionType, int transitionData,
		int transitionStartFrame, int transitionLength, int transLeft, int transTop, int transRight,
		int transBottom, int transTextID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(left, top, right, bottom);
	_cursorID = cursorID;
	_clickDestination.destinationScene.timeZone = timeZone;
	_clickDestination.destinationScene.environment = environment;
	_clickDestination.destinationScene.node = node;
	_clickDestination.destinationScene.facing = facing;
	_clickDestination.destinationScene.orientation = orientation;
	_clickDestination.destinationScene.depth = depth;
	_clickDestination.transitionType = transitionType;
	_clickDestination.transitionData = transitionData;
	_clickDestination.transitionStartFrame = transitionStartFrame;
	_clickDestination.transitionLength = transitionLength;
	_translateRect = Common::Rect(transLeft, transTop, transRight, transBottom);
	_textTranslated = false;
	_textID = transTextID;
}

int ClickChangeSceneTranslate::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		((SceneViewWindow *)viewWindow)->moveToDestination(_clickDestination);

	return SC_FALSE;
}

int ClickChangeSceneTranslate::gdiPaint(Window *viewWindow) {
	if (_textTranslated && ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(_translateRect);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int ClickChangeSceneTranslate::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		if (_translateRect.contains(pointLocation)) {
			((SceneViewWindow *)viewWindow)->displayTranslationText(_vm->getString(_textID));
			_textTranslated = true;
			viewWindow->invalidateWindow(false);
		} else {
			if (_textTranslated) {
				_textTranslated = false;
				viewWindow->invalidateWindow(false);
			}
		}
	} else {
		if (_textTranslated) {
			_textTranslated = false;
			viewWindow->invalidateWindow(false);
		}
	}

	return SC_FALSE;
}

int ClickChangeSceneTranslate::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return 0;
}

class AimBallistaToTower : public SceneBase {
public:
	AimBallistaToTower(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	~AimBallistaToTower();
	void preDestructor();
	int paint(Window *viewWindow, Graphics::Surface *preBuffer);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _raiseBallista;
	Common::Rect _lowerBallista;
	Common::Rect _turnBallistaLeft;
	Common::Rect _turnBallistaRight;
	Common::Rect _ballistaHandle;
	AVIFrames *_viewFrameExtractor;
};

AimBallistaToTower::AimBallistaToTower(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_turnBallistaLeft = Common::Rect(0, 84, 44, 189);
	_turnBallistaRight = Common::Rect(45, 84, 90, 189);
	_lowerBallista = Common::Rect(368, 82, 432, 189);
	_raiseBallista = Common::Rect(304, 82, 367, 189);
	_ballistaHandle = Common::Rect(170, 116, 212, 189);

	_viewFrameExtractor = new AVIFrames(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 6));
}

AimBallistaToTower::~AimBallistaToTower() {
	preDestructor();
}

void AimBallistaToTower::preDestructor() {
	delete _viewFrameExtractor;
	_viewFrameExtractor = 0;
}

int AimBallistaToTower::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	SceneBase::paint(viewWindow, preBuffer);

	byte xPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaXPos;
	byte yPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaYPos;
	const Graphics::Surface *frame = _viewFrameExtractor->getFrame(yPos * 20 + xPos);

	if (frame)
		_vm->_gfx->crossBlit(preBuffer, 120, 51, 160, 56, frame, 0, 0);

	return SC_REPAINT;
}

int AimBallistaToTower::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_ballistaHandle.contains(pointLocation)) {
		byte xPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaXPos;
		byte yPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaYPos;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverShotBallista = 1;

		if (xPos == 9 && yPos == 2) {
			// Play the launch movie
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(20);

			// Reset the ballista status
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaStatus = 2;
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverConnectedHook = 1;

			// Step down
			DestinationScene destData;
			destData.destinationScene = Location(5, 5, 8, 5, 1, 2);
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 11;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		} else {
			// Play the launch movie
			((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(19, 110, 108, 290, 189);
		}

		return SC_TRUE;
	} else if (_raiseBallista.contains(pointLocation)) {
		byte &yPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaYPos;

		if (yPos == 0)
			return SC_FALSE;

		yPos--;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverUsedCrank = 1;

		// Start the spin sound
		int soundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12), 128, true, false);

		// Spin the wheel halfway
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(23, 300, 70, 432, 189);

		// Stop the spin sound
		_vm->_sound->stopSoundEffect(soundID);

		// Repaint
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	} else if (_lowerBallista.contains(pointLocation)) {
		byte &yPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaYPos;

		if (yPos >= 4)
			return SC_FALSE;

		yPos++;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverUsedCrank = 1;

		// Start the spin sound
		int soundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12), 128, true, false);

		// Spin the wheel halfway
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(24, 300, 70, 432, 189);

		// Stop the spin sound
		_vm->_sound->stopSoundEffect(soundID);

		// Repaint
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	} else if (_turnBallistaRight.contains(pointLocation)) {
		byte &xPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaXPos;

		if (xPos >= 19)
			return SC_FALSE;

		xPos++;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverUsedCrank = 1;

		// Start the spin sound
		int soundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12), 128, true, false);

		// Spin the wheel halfway
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(22, 0, 70, 100, 189);

		// Stop the spin sound
		_vm->_sound->stopSoundEffect(soundID);

		// Repaint
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	} else if (_turnBallistaLeft.contains(pointLocation)) {
		byte &xPos = ((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYBallistaXPos;

		if (xPos == 0)
			return SC_FALSE;

		xPos--;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().dsCYNeverUsedCrank = 1;

		// Start the spin sound
		int soundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12), 128, true, false);

		// Spin the wheel halfway
		((SceneViewWindow *)viewWindow)->playClippedSynchronousAnimation(21, 0, 70, 100, 189);

		// Stop the spin sound
		_vm->_sound->stopSoundEffect(soundID);

		// Repaint
		viewWindow->invalidateWindow(false);
		return SC_TRUE;
	}

	// Return to previous scene
	DestinationScene destData;
	destData.destinationScene = Location(5, 5, 8, 5, 1, 1);
	destData.transitionType = TRANSITION_VIDEO;
	destData.transitionData = 11;
	destData.transitionStartFrame = -1;
	destData.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	return SC_TRUE;
}

int AimBallistaToTower::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_raiseBallista.contains(pointLocation))
		return kCursorArrowUp;

	if (_lowerBallista.contains(pointLocation))
		return kCursorArrowDown;

	if (_turnBallistaRight.contains(pointLocation))
		return kCursorArrowRight;

	if (_turnBallistaLeft.contains(pointLocation))
		return kCursorArrowLeft;

	if (_ballistaHandle.contains(pointLocation))
		return kCursorFinger;

	return kCursorPutDown;
}

class WalkDownPaintingTowerElevator : public SceneBase {
public:
	WalkDownPaintingTowerElevator(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	Common::Rect _clickRegion;
	DestinationScene _clickDestination;
};

WalkDownPaintingTowerElevator::WalkDownPaintingTowerElevator(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(48, 136, 306, 184);
	_cursorID = kCursorFinger;
	_clickDestination.destinationScene = Location(5, 3, 10, 0, 0, 0);
	_clickDestination.transitionType = TRANSITION_VIDEO;
	_clickDestination.transitionData = 9;
	_clickDestination.transitionStartFrame = -1;
	_clickDestination.transitionLength = -1;
}

int WalkDownPaintingTowerElevator::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTElevatorLeverA == 1) {
			((SceneViewWindow *)viewWindow)->moveToDestination(_clickDestination);
			((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTWalkedDownElevator = 1;
		} else {
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13), 127, false, true);
		}
	}

	return SC_FALSE;
}

int WalkDownPaintingTowerElevator::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

class ViewSiegeCyclePlans : public SceneBase {
public:
	ViewSiegeCyclePlans(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int gdiPaint(Window *viewWindow);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _transText[3];
	int _textTranslated;
};

ViewSiegeCyclePlans::ViewSiegeCyclePlans(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_transText[0] = Common::Rect(245, 8, 307, 24);
	_transText[1] = Common::Rect(132, 40, 188, 76);
	_transText[2] = Common::Rect(278, 146, 332, 178);
	_textTranslated = -1;

	// Set the scene visited flag
	((SceneViewWindow *)viewWindow)->getGlobalFlags().dsWSSeenCycleSketch = 1;
}

int ViewSiegeCyclePlans::gdiPaint(Window *viewWindow) {
	if (_textTranslated >= 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(_transText[_textTranslated]);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int ViewSiegeCyclePlans::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	// Return to depth zero
	DestinationScene destData;
	destData.destinationScene = _staticData.location;
	destData.destinationScene.depth = 0;
	destData.transitionType = TRANSITION_VIDEO;
	destData.transitionData = 16;
	destData.transitionStartFrame = -1;
	destData.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	return SC_TRUE;
}

int ViewSiegeCyclePlans::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		for (int i = 0; i < 3; i++) {
			if (_transText[i].contains(pointLocation)) {
				((SceneViewWindow *)viewWindow)->getGlobalFlags().dsPTTransElevatorControls = 1;

				Common::String text = _vm->getString(IDS_DS_WS_CYCLE_PLANS_TEXT_A + i);
				((SceneViewWindow *)viewWindow)->displayTranslationText(text);
				_textTranslated = i;
				viewWindow->invalidateWindow(false);
				break;
			}
		}
	} else {
		if (_textTranslated >= 0) {
			_textTranslated = -1;
			viewWindow->invalidateWindow(false);
		}
	}

	return SC_FALSE;
}

int ViewSiegeCyclePlans::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	return kCursorPutDown;
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

bool SceneViewWindow::checkCustomDaVinciAICommentDependencies(const Location &commentLocation, const AIComment &commentData) {
	switch (commentData.dependencyFlagOffsetB) {
	case 1: // Has not raised platform
		return _globalFlags.dsPTRaisedPlatform == 0;
	case 2: // Has raised platform
		return _globalFlags.dsPTRaisedPlatform == 1;
	case 3: // Has not been to codec tower
		return _globalFlags.dsVisitedCodexTower == 0;
	case 4: // Has not been on balcony, has not been down elevator
		return _globalFlags.dsPTBeenOnBalcony == 0 && _globalFlags.dsPTWalkedDownElevator == 0;
	case 5: // Has not raised platform, has not translated levers
		return _globalFlags.dsPTRaisedPlatform == 0 && _globalFlags.dsPTTransElevatorControls == 0;
	case 6: // Has not raised platform, has not translated levers, translation biochip is in inventory
		return _globalFlags.dsPTRaisedPlatform == 0 && _globalFlags.dsPTTransElevatorControls == 0 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemBioChipTranslate);
	case 7: // If clicked on codex door
		return _globalFlags.dsGDClickedOnCodexDoor == 1;
	case 8: // If clicked on codex door, siege cycle not in inventory
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemSiegeCycle);
	case 9: // If gear assembly has never been in inventory
		return _globalFlags.genHadDriveAssembly == 0;
	case 10: // If siege cycle has never been inventory
		return _globalFlags.genHadSiegeCycle == 0;
	case 11: // If neither wheel assembly or drive assembly have ever been in inventory, and after viewing siege cycle plans
		return _globalFlags.genHadDriveAssembly == 0 && _globalFlags.genHadWheelAssembly == 0;
	case 12: // After player has been to 5/4/8/1/1/1, siege cycle has never been in inventory
		return (_globalFlags.dsWSSiegeCycleStatus & DS_SC_DRIVE_ASSEMBLY) != 0 && (_globalFlags.dsWSSiegeCycleStatus & DS_SC_WHEEL_ASSEMBLY) != 0;
	case 13: // After player has been to 5/4/4/2/0/1 or 5/4/4/3/0/1, siege cycle has never been in inventory
		return _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.genHadSiegeCycle == 0;
	case 14: // After player has been to (5/4/4/2/0/1 or 5/4/4/3/0/1) and 5/4/8/1/1/1, before any parts are on the jig
		return _globalFlags.dsWSSeenCycleSketch == 1 && _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.dsWSSiegeCycleStatus == 0 && _globalFlags.dsWSGrabbedSiegeCycle == 0;
	case 15: // After player has been to 5/4/4/2/0/1 or 5/4/4/3/0/1, before any parts are on the jig
		return _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.dsWSSiegeCycleStatus == 0 && _globalFlags.dsWSGrabbedSiegeCycle == 0;
	case 16: // After player has been to 5/4/4/2/0/1 or 5/4/4/3/0/1, if wheel assembly and drive assembly have never been in inventory
		return _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.dsWSSiegeCycleStatus == 0 && _globalFlags.dsWSGrabbedSiegeCycle == 0 && !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemDriveAssembly) && !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemWoodenPegs);
	case 17: // After player has been to 5/4/4/2/0/1 or 5/4/4/3/0/1
		return _globalFlags.dsWSSeenBallistaSketch == 1;
	case 18: // Has tried door of codex tower, has never connected ballista hook, has not seen 5/4/4/2/0/1 or 5/4/4/3/0/1
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsCYNeverConnectedHook == 0 && _globalFlags.dsWSSeenBallistaSketch == 0;
	case 19: // Has tried door of codex tower, has not seen 5/4/4/2/0/1 or 5/4/4/3/0/1, has not been to codex tower
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsWSSeenBallistaSketch == 0 && _globalFlags.dsVisitedCodexTower == 0;
	case 20: // Has tried door of codex tower, has seen 5/4/4/2/0/1 or 5/4/4/3/0/1, has never shot ballista
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.dsCYNeverShotBallista == 0;
	case 21: // Has tried door of codex tower, has seen 5/4/4/2/0/1 or 5/4/4/3/0/1, has never used crank
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.dsCYNeverUsedCrank == 0;
	case 22: // Has never connected ballista hook to codex tower
		return _globalFlags.dsCYNeverConnectedHook == 0;
	case 23: // Has tried door of codex tower, has not seen 5/4/4/2/0/1 or 5/4/4/3/0/1, has not been to codex tower
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsWSSeenBallistaSketch == 0 && _globalFlags.dsVisitedCodexTower == 0;
	case 24: // Has tried door of codex tower, has seen 5/4/4/2/0/1 or 5/4/4/3/0/1, has not been to codex tower, siege cycle not in inventory
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.dsVisitedCodexTower == 0 && _globalFlags.dsCYBallistaStatus == 2 && !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemSiegeCycle);
	case 25: // Has tried door of codex tower, has seen 5/4/4/2/0/1 or 5/4/4/3/0/1, has not been to codex tower, has siege cycle in inventory
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.dsVisitedCodexTower == 0 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemSiegeCycle);
	case 26: // Has tried door of codex tower, has not seen 5/4/4/2/0/1 or 5/4/4/3/0/1, has not been to codex tower, has siege cycle in inventory
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsWSSeenBallistaSketch == 0 && _globalFlags.dsVisitedCodexTower == 0 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemSiegeCycle);
	case 27: // Has tried door of codex tower, has not connected ballista hook to codex tower
		return _globalFlags.dsGDClickedOnCodexDoor == 1 && _globalFlags.dsCYBallistaStatus < 2;
	case 28: // Before ever opening codex tower balcony door
		return _globalFlags.dsCYNeverOpenedBalconyDoor == 0;
	case 29: // Before ever opening codex tower balcony door, after trying unsuccessfully to open door
		return _globalFlags.dsCYNeverOpenedBalconyDoor == 0 && _globalFlags.dsCYTriedOpeningDoor == 1;
	case 30: // Before ever opening codex tower balcony door, after trying unsuccessfully to open door, balcony key in inventory
		return _globalFlags.dsCYNeverOpenedBalconyDoor == 0 && _globalFlags.dsCYTriedOpeningDoor == 1 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemBalconyKey);
	case 31: // Before ever opening codex tower balcony door, after trying unsuccessfully to open door, metal bar in inventory, balcony key not in inventory
		return _globalFlags.dsCYNeverOpenedBalconyDoor == 0 && _globalFlags.dsCYTriedOpeningDoor == 1 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemMetalBar) && !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemBalconyKey);
	case 32: // Lens filter not in ineventory
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemLensFilter);
	case 33: // Player has not found formulae, before trying to translate any codex
		return _globalFlags.dsCTCodexFormulaeFound == 0 && _globalFlags.dsCYTranslatedCodex == 0;
	case 34: // Player has not found formulae, after trying to translate any codex
		return _globalFlags.dsCTCodexFormulaeFound == 0 && _globalFlags.dsCYTranslatedCodex == 1;
	case 35: // Player has not found formulae, after trying to translate any codex, lens filter in inventory, lens filter not being used
		return _globalFlags.dsCTCodexFormulaeFound == 0 && _globalFlags.dsCYTranslatedCodex == 1 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemLensFilter) && _globalFlags.lensFilterActivated == 0;
	case 36: // After trying unsuccessfully to open the door
		return _globalFlags.dsCYTriedOpeningDoor == 1;
	case 37: // Player has not found formulae
		return _globalFlags.dsCTCodexFormulaeFound == 0;
	case 38: // Heart not in inventory
		return !((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemPreservedHeart);
	case 39: // After trying to use the codex tower elevator
		return _globalFlags.dsCYTriedElevator == 1;
	case 40: // After trying to translate any codex
		return _globalFlags.dsCYTranslatedCodex == 1;
	case 41: // Not node 8, 9, 10, or 11, player has not found codex
		return commentLocation.node != 8 && commentLocation.node != 9 && commentLocation.node != 10 && commentLocation.node != 11 && _globalFlags.dsCYFoundCodexes == 1;
	case 42: // Not node 8, 9, 10, or 11
		return commentLocation.node != 8 && commentLocation.node != 9 && commentLocation.node != 10 && commentLocation.node != 11;
	case 43: // Not node 8, 9, 10, or 11, player has not found formulae, lens filter in inventory
		return commentLocation.node != 8 && commentLocation.node != 9 && commentLocation.node != 10 && commentLocation.node != 11 && _globalFlags.dsCTCodexFormulaeFound == 0 && ((GameUIWindow *)getParent())->_inventoryWindow->isItemInInventory(kItemLensFilter);
	case 44: // After player has been to 5/4/4/2/0/1 or 5/4/4/3/0/1, before building siege cycle, drive/gear assembly not in inventory
		return _globalFlags.dsWSSeenBallistaSketch == 1 && _globalFlags.dsWSSiegeCycleStatus == 0 && _globalFlags.dsWSPickedUpWheelAssembly == 0;
	case 45: // After weeble has been spun/clicked
		return _globalFlags.dsCYWeebleClicked == 1;
	}

	return false;
}

SceneBase *SceneViewWindow::constructDaVinciSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 1:
		return new SwapStillOnFlag(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, dsPTElevatorPresent), 1);
	case 2:
		return new DisplayMessageWithEvidenceWhenEnteringNode(_vm, viewWindow, sceneStaticData, priorLocation, DAVINCI_EVIDENCE_FOOTPRINT, IDS_MBT_EVIDENCE_PRESENT);
	case 3:
		return new CapturePaintingTowerFootprint(_vm, viewWindow, sceneStaticData, priorLocation);
	case 4:
		return new PaintingTowerRetrieveKey(_vm, viewWindow, sceneStaticData, priorLocation);
	case 5:
		return new PaintingTowerElevatorControls(_vm, viewWindow, sceneStaticData, priorLocation);
	case 6:
		return new PaintingTowerElevatorWheel(_vm, viewWindow, sceneStaticData, priorLocation);
	case 7:
		return new WalkDownPaintingTowerElevator(_vm, viewWindow, sceneStaticData, priorLocation);
	case 8:
		return new PaintingTowerWalkOntoElevator(_vm, viewWindow, sceneStaticData, priorLocation);
	case 9:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 170, 80, 428, 184, kCursorFinger, 5, 1, 8, 3, 0, 0, TRANSITION_VIDEO, 0, -1, -1);
	case 10:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 11:
		return new PaintingTowerOutsideDoor(_vm, viewWindow, sceneStaticData, priorLocation);
	case 12:
		return new PaintingTowerInsideDoor(_vm, viewWindow, sceneStaticData, priorLocation);
	case 13:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 196, 0, 262, 189, 5, 3, 10, 1, 1, 1, TRANSITION_WALK, 11, 881, 20);
	case 14:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 208, 0, 306, 189, 5, 3, 0, 2, 1, 1, TRANSITION_WALK, 11, 740, 23);
	case 15:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, -1, 13, kCursorFinger, 0, 0, 384, 189);
	case 16:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 17:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, dsGDClickedOnCodexDoor), 13, kCursorFinger, 222, 0, 318, 189);
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
	case 24:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 186, 28, 292, 158, kCursorMagnifyingGlass, 5, 4, 4, 2, 1, 1, TRANSITION_VIDEO, 5, -1, -1);
	case 25:
		return new ClickChangeSceneTranslate(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 5, 4, 4, 2, 1, 0, TRANSITION_VIDEO, 6, -1, -1, 190, 88, 308, 160, IDDS_WORKSHOP_TOOLS_TEXT);
	case 26:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 44, 232, 189, kCursorMagnifyingGlass, 5, 4, 4, 3, 0, 1, TRANSITION_VIDEO, 7, -1, -1);
	case 27:
		return new ClickChangeSceneSetFlag(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 5, 4, 4, 3, 0, 0, TRANSITION_VIDEO, 8, -1, -1, offsetof(GlobalFlags, dsWSSeenBallistaSketch));
	case 28:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 112, 52, 380, 189, kCursorMagnifyingGlass, 5, 4, 4, 2, 0, 1, TRANSITION_VIDEO, 9, -1, -1);
	case 29:
		return new ClickChangeSceneSetFlag(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 5, 4, 4, 2, 0, 0, TRANSITION_VIDEO, 10, -1, -1, offsetof(GlobalFlags, dsWSSeenBallistaSketch));
	case 30:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 96, 130, 222, 164, kCursorMagnifyingGlass, 5, 4, 7, 3, 1, 1, TRANSITION_VIDEO, 11, -1, -1 );
	case 31:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 5, 4, 7, 3, 1, 0, TRANSITION_VIDEO, 12, -1, -1);
	case 32:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 158, 90, 328, 162, kItemDriveAssembly, 145, offsetof(GlobalFlags, dsWSPickedUpGearAssembly));
	case 33:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 164, 126, 276, 160, kItemWoodenPegs, 96, offsetof(GlobalFlags, dsWSPickedUpPegs));
	case 34:
		return new WheelAssemblyItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 150, 150, 276, 189, kItemWheelAssembly, 100, offsetof(GlobalFlags, dsWSPickedUpWheelAssembly));
	case 35:
		return new ViewSiegeCyclePlans(_vm, viewWindow, sceneStaticData, priorLocation);
	case 36:
		return new AssembleSiegeCycle(_vm, viewWindow, sceneStaticData, priorLocation);
	case 37:
		return new SiegeCycleTopView(_vm, viewWindow, sceneStaticData, priorLocation);
	case 38:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 130, 74, 182, 120, kItemCoilOfRope, 48, offsetof(GlobalFlags, dsGDTakenCoilOfRope));
	case 39:
		return new UnlockCodexTowerDoor(_vm, viewWindow, sceneStaticData, priorLocation);
	case 40:
		return new CodexTowerOutsideDoor(_vm, viewWindow, sceneStaticData, priorLocation);
	case 41:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 116, 0, 326, 189, 5, 2, 2, 1, 1, 1, TRANSITION_WALK, 11, 225, 15);
	case 46:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 8, kCursorFinger, 102, 124, 164, 189);
	case 51:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 36, 240, 189, kCursorMagnifyingGlass, 5, 2, 4, 0, 0, 1, TRANSITION_VIDEO, 11, -1, -1);
	case 52:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 5, 2, 4, 0, 0, 0, TRANSITION_VIDEO, 12, -1, -1);
	case 53:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 284, 46, 350, 182, kCursorMagnifyingGlass, 5, 2, 0, 2, 1, 1, TRANSITION_VIDEO, 13, -1, -1);
	case 54:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 0, 0, 432, 189, kCursorPutDown, 5, 2, 0, 2, 1, 0, TRANSITION_VIDEO, 14, -1, -1);
	case 55:
		return new ClickChangeScene(_vm, viewWindow, sceneStaticData, priorLocation, 210, 0, 330, 110, kCursorMagnifyingGlass, 5, 2, 3, 4, 1, 1, TRANSITION_VIDEO, 15, -1, -1);
	case 57:
		return new CourtyardCannon(_vm, viewWindow, sceneStaticData, priorLocation);
	case 58:
		return new ClickPlayVideoSwitch(_vm, viewWindow, sceneStaticData, priorLocation, 1, kCursorFinger, offsetof(GlobalFlags, dsCYWeebleClicked), 200, 88, 270, 189);
	case 59:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 2, kCursorFinger, 70, 136, 190, 189);
	case 60:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 5, kCursorFinger, 42, 0, 418, 100);
	case 61:
		return new ClickPlayVideo(_vm, viewWindow, sceneStaticData, priorLocation, 3, kCursorFinger, 178, 144, 288, 189);
	case 62:
		return new CourtyardGunDeath(_vm, viewWindow, sceneStaticData, priorLocation);
	case 63:
		return new ChangeBallistaDepth(_vm, viewWindow, sceneStaticData, priorLocation);
	case 64:
		return new SpinBallista(_vm, viewWindow, sceneStaticData, priorLocation);
	case 65:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 122, 8, 326, 189, 5, 5, 0, 2, 1, 1, TRANSITION_WALK, 11, 738, 18);
	case 66:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 170, 0, 432, 189, 5, 4, 0, 0, 1, 1, TRANSITION_WALK, 11, 1220, 12);
	case 67:
		return new PlaceSiegeCycleOnTrack(_vm, viewWindow, sceneStaticData, priorLocation);
	case 68:
		return new AimBallistaAwayFromTower(_vm, viewWindow, sceneStaticData, priorLocation);
	case 69:
		return new AimBallistaToTower(_vm, viewWindow, sceneStaticData, priorLocation);
	case 70:
		return new PaintingTowerCapAgent(_vm, viewWindow, sceneStaticData, priorLocation);
	case 72:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 13);
	case 74:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, -1, 13, kCursorFinger, 140, 0, 432, 189);
	case 75:
		return new ClickPlaySound(_vm, viewWindow, sceneStaticData, priorLocation, offsetof(GlobalFlags, dsCYTriedElevator), 13, kCursorFinger, 140, 130, 432, 189);
	case 76:
		return new PlaySoundEnteringScene(_vm, viewWindow, sceneStaticData, priorLocation, 12, offsetof(GlobalFlags, dsCTPlayedBallistaFalling));
	}

	warning("TODO: Da Vinci scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
