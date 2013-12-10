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
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/environ/scene_base.h"
#include "buried/environ/scene_common.h"

#include "graphics/surface.h"

namespace Buried {

class PlaceCeramicBowl : public SceneBase {
public:
	PlaceCeramicBowl(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int timerCallback(Window *viewWindow);

private:
	bool _dropped;
};

PlaceCeramicBowl::PlaceCeramicBowl(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_dropped = false;
}

int PlaceCeramicBowl::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;
	
	if (itemID != kItemCeramicBowl)
		return SIC_REJECT;

	_staticData.navFrameIndex = 112;
	viewWindow->invalidateWindow(false);
	_dropped = true;
	return SIC_ACCEPT;
}

int PlaceCeramicBowl::timerCallback(Window *viewWindow) {
	if (_dropped) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPCodeWheelStatus == 0) {
			// Play slide death animation
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(4);

			// Notify the player of his gruesome death
			((SceneViewWindow *)viewWindow)->showDeathScene(11);
			return SC_DEATH;
		} else {
			// Kill the ambient
			_vm->_sound->setAmbientSound();

			// Jump to the start of the main cavern
			DestinationScene newDest;
			newDest.destinationScene.timeZone = 2;
			newDest.destinationScene.environment = 2;
			newDest.destinationScene.node = 0;
			newDest.destinationScene.facing = 1;
			newDest.destinationScene.orientation = 1;
			newDest.destinationScene.depth = 0;
			newDest.transitionType = TRANSITION_VIDEO;
			newDest.transitionData = 3;
			newDest.transitionStartFrame = -1;
			newDest.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(newDest);
		}
	}

	return SC_TRUE;
}

class AdjustWheels : public SceneBase {
public:
	AdjustWheels(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	void preDestructor();
	int paint(Window *viewWindow, Graphics::Surface *preBuffer);
	int gdiPaint(Window *viewWindow);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	AVIFrames _leftWheelFrames;
	int _curLeftFrame;
	AVIFrames _rightWheelFrames;
	int _curRightFrame;
	Common::Rect _leftUpRegion;
	Common::Rect _leftDownRegion;
	Common::Rect _rightUpRegion;
	Common::Rect _rightDownRegion;
	bool _translateText;
};

AdjustWheels::AdjustWheels(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_curLeftFrame = ((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPCodeWheelLeftIndex;
	_curRightFrame = ((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPCodeWheelRightIndex;
	_leftUpRegion = Common::Rect(46, 0, 200, 70);
	_leftDownRegion = Common::Rect(46, 106, 200, 189);
	_rightUpRegion = Common::Rect(212, 0, 432, 66);
	_rightDownRegion = Common::Rect(212, 109, 432, 189);
	_translateText = false;

	if (!_leftWheelFrames.open(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 9)))
		error("Failed to open left wheel frames video");

	if (!_rightWheelFrames.open(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 10)))
		error("Failed to open right wheel frames video");
}

void AdjustWheels::preDestructor() {
	_leftWheelFrames.close();
	_rightWheelFrames.close();
}

int AdjustWheels::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	if (_staticData.navFrameIndex >= 0) {
		const Graphics::Surface *leftFrame = _leftWheelFrames.getFrame(_curLeftFrame);
		if (leftFrame)
			_vm->_gfx->crossBlit(preBuffer, 0, 0, 208, 189, leftFrame, 0, 0);

		const Graphics::Surface *rightFrame = _rightWheelFrames.getFrame(_curRightFrame);
		if (rightFrame)
			_vm->_gfx->crossBlit(preBuffer, 208, 0, 224, 189, rightFrame, 0, 0);
	}

	return SC_REPAINT;
}

int AdjustWheels::gdiPaint(Window *viewWindow) {
	if (_translateText && ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(168, 70, 262, 108);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int AdjustWheels::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	// TODO: Wait between frames after figuring out timing

	if (_leftUpRegion.contains(pointLocation) || _leftDownRegion.contains(pointLocation) ||
			_rightUpRegion.contains(pointLocation) || _rightDownRegion.contains(pointLocation)) {
		if (_leftDownRegion.contains(pointLocation)) {
			// Move the wheel one frame and redraw
			_curLeftFrame++;

			if (_curLeftFrame > _leftWheelFrames.getFrameCount() - 1)
				_curLeftFrame = 0;

			viewWindow->invalidateWindow(false);

			// And again for the final frame
			_curLeftFrame++;

			if (_curLeftFrame > _leftWheelFrames.getFrameCount() - 1)
				_curLeftFrame = 0;

			viewWindow->invalidateWindow(false);
		} else if (_leftUpRegion.contains(pointLocation)) {
			// Move the wheel one frame and redraw
			_curLeftFrame--;

			if (_curLeftFrame < 0)
				_curLeftFrame = _leftWheelFrames.getFrameCount() - 1;

			viewWindow->invalidateWindow(false);

			// And again for the final frame
			_curLeftFrame--;

			if (_curLeftFrame < 0)
				_curLeftFrame = _leftWheelFrames.getFrameCount() - 1;

			viewWindow->invalidateWindow(false);
		} else if (_rightDownRegion.contains(pointLocation)) {
			// Move the wheel one frame and redraw
			_curRightFrame++;

			if (_curRightFrame > _rightWheelFrames.getFrameCount() - 1)
				_curRightFrame = 0;

			viewWindow->invalidateWindow(false);

			// And again for the final frame
			_curRightFrame++;

			if (_curRightFrame > _rightWheelFrames.getFrameCount() - 1)
				_curRightFrame = 0;

			viewWindow->invalidateWindow(false);
		} else if (_rightUpRegion.contains(pointLocation)) {
			// Move the wheel one frame and redraw
			_curRightFrame--;

			if (_curRightFrame < 0)
				_curRightFrame = _rightWheelFrames.getFrameCount() - 1;

			viewWindow->invalidateWindow(false);

			// And again for the final frame
			_curRightFrame--;

			if (_curRightFrame < 0)
				_curRightFrame = _rightWheelFrames.getFrameCount() - 1;

			viewWindow->invalidateWindow(false);
		}

		((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPCodeWheelLeftIndex = _curLeftFrame;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPCodeWheelRightIndex = _curRightFrame;

		byte status = 0;
		if (_curLeftFrame == 8 && _curRightFrame == 12)
			status = 1;
		else if (_curLeftFrame == 16 && _curRightFrame == 22)
			status = 1;
		else if (_curLeftFrame == 20 && _curRightFrame == 4)
			status = 1;
		else if (_curLeftFrame == 0 && _curRightFrame == 24)
			status = 1;
		else if (_curLeftFrame == 14 && _curRightFrame == 8)
			status = 1;
		else if (_curLeftFrame == 6 && _curRightFrame == 6)
			status = 1;
		else if (_curLeftFrame == 6 && _curRightFrame == 30)
			status = 1;
		else if (_curLeftFrame == 24 && _curRightFrame == 0)
			status = 1;
		else if (_curLeftFrame == 10 && _curRightFrame == 28)
			status = 1;

		((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPCodeWheelStatus = status;

		return SC_TRUE;
	}

	// Did not click on the wheels, pop back to depth 0
	DestinationScene newDest;
	newDest.destinationScene = _staticData.location;
	newDest.destinationScene.depth = 0;
	newDest.transitionType = TRANSITION_NONE;
	newDest.transitionData = -1;
	newDest.transitionStartFrame = -1;
	newDest.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(newDest);
	return SC_TRUE;
}

int AdjustWheels::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect translateTextRegion(168, 72, 260, 106);

		if (translateTextRegion.contains(pointLocation)) {
			if (!_translateText) {
				Common::String leftText = _vm->getString(IDMYTP_WHEELS_LEFT_TRANS_TEXT_BASE + _curLeftFrame / 2);
				Common::String rightText = _vm->getString(IDMYTP_WHEELS_RIGHT_TRANS_TEXT_BASE + _curRightFrame / 2);
				Common::String finalString = leftText + rightText;

				if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1 &&
						((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPCodeWheelStatus == 1) {

					if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0))
						finalString += _vm->getString(IDS_MYTP_WALKTHROUGH_HINT_TEXT);
					else
						finalString += " (Mayan Sacred Day)";
				}

				((SceneViewWindow *)viewWindow)->displayTranslationText(finalString);
				_translateText = true;
				viewWindow->invalidateWindow(false);
			}
		} else {
			if (_translateText) {
				_translateText = false;
				viewWindow->invalidateWindow(false);
			}
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int AdjustWheels::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_leftUpRegion.contains(pointLocation) || _rightUpRegion.contains(pointLocation))
		return kCursorArrowUp;

	if (_leftDownRegion.contains(pointLocation) || _rightDownRegion.contains(pointLocation))
		return kCursorArrowDown;

	return kCursorPutDown;
}

class DateCombinationRead : public SceneBase {
public:
	DateCombinationRead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int gdiPaint(Window *viewWindow);
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _currentRegion;
};

DateCombinationRead::DateCombinationRead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_currentRegion = -1;
}

int DateCombinationRead::gdiPaint(Window *viewWindow) {
	if (_currentRegion >= 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		int left = _currentRegion * 43 + 20 + absoluteRect.left;
		Common::Rect rect(left, absoluteRect.top + 18, left + 43, absoluteRect.top + 110);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int DateCombinationRead::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect symbols(20, 18, 407, 110);

		if (symbols.contains(pointLocation)) {
			int translatedSymbolIndex = (pointLocation.x - 20) / 43;

			if (_currentRegion != translatedSymbolIndex) {
				// Update flags
				((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPCalendarListTranslated = 1;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().myTPTextTranslated = 1;

				// Display the text
				((SceneViewWindow *)viewWindow)->displayTranslationText(_vm->getString(IDMYTP_WALLS_COMBO_TRANS_TEXT_BASE + translatedSymbolIndex));

				// Reset the current region and redraw
				_currentRegion = translatedSymbolIndex;
				viewWindow->invalidateWindow(false);
			}
		} else {
			if (_currentRegion >= 0) {
				_currentRegion = -1;
				viewWindow->invalidateWindow(false);
			}
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

class ViewSingleTranslation : public SceneBase {
public:
	ViewSingleTranslation(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int translatedTextID = -1, int left = -1, int top = -1, int right = -1, int bottom = -1,
			int flagAOffset = -1, int flagBOffset = -1, int visitedFlagOffset = -1);
	int gdiPaint(Window *viewWindow);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	bool _textTranslated;
	int _textID;
	Common::Rect _clickableRegion;
	int _flagAOffset;
	int _flagBOffset;
	int _visitedFlagOffset;
};

ViewSingleTranslation::ViewSingleTranslation(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int translatedTextID, int left, int top, int right, int bottom,
		int flagAOffset, int flagBOffset, int visitedFlagOffset) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_textTranslated = false;
	_textID = translatedTextID;
	_clickableRegion = Common::Rect(left, top, right, bottom);
	_flagAOffset = flagAOffset;
	_flagBOffset = flagBOffset;
	_visitedFlagOffset = visitedFlagOffset;

	if (_visitedFlagOffset >= 0)
		((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_visitedFlagOffset, 1);
}

int ViewSingleTranslation::gdiPaint(Window *viewWindow) {
	// Draw the translated box, if applicable
	if (_textTranslated && ((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(_clickableRegion);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int ViewSingleTranslation::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_staticData.location.depth != 0) {
		// If we're not at depth zero, move to depth zero
		Location newLocation = _staticData.location;
		newLocation.depth = 0;
		((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ViewSingleTranslation::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcTranslateEnabled == 1) {
		if (_clickableRegion.contains(pointLocation)) {
			// Make sure we didn't already render the text
			if (!_textTranslated) {
				if (_flagAOffset >= 0)
					((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagAOffset, 1);
				if (_flagBOffset >= 0)
					((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_flagBOffset, 1);

				// Load and display the text
				((SceneViewWindow *)viewWindow)->displayTranslationText(_vm->getString(_textID));
				_textTranslated = true;
				viewWindow->invalidateWindow(false);
			}
		} else {
			if (_textTranslated) {
				_textTranslated = false;
				viewWindow->invalidateWindow(false);
			}
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int ViewSingleTranslation::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_staticData.location.depth != 0)
		return kCursorPutDown;

	return kCursorArrow;
}

class GenericCavernDoorMainView : public SceneBase {
public:
	GenericCavernDoorMainView(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int topZoomDepth = 0, int topLeft = -1, int topTop = -1, int topRight = -1, int topBottom = -1,
			int rightZoomDepth = 0, int rightLeft = -1, int rightTop = -1, int rightRight = -1, int rightBottom = -1,
			int offeringHeadZoomDepth = 0, int offeringHeadLeft = -1, int offeringHeadTop = -1, int offeringHeadRight = -1, int offeringHeadBottom = -1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _topZoomDepth;
	int _rightZoomDepth;
	int _offeringHeadZoomDepth;
	Common::Rect _topZoomRegion;
	Common::Rect _rightZoomRegion;
	Common::Rect _offeringHeadZoomRegion;
};

GenericCavernDoorMainView::GenericCavernDoorMainView(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int topZoomDepth, int topLeft, int topTop, int topRight, int topBottom,
		int rightZoomDepth, int rightLeft, int rightTop, int rightRight, int rightBottom,
		int offeringHeadZoomDepth, int offeringHeadLeft, int offeringHeadTop, int offeringHeadRight, int offeringHeadBottom) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_topZoomDepth = topZoomDepth;
	_rightZoomDepth = rightZoomDepth;
	_offeringHeadZoomDepth = offeringHeadZoomDepth;
	_topZoomRegion = Common::Rect(topLeft, topTop, topRight, topBottom);
	_rightZoomRegion = Common::Rect(rightLeft, rightTop, rightRight, rightBottom);
	_offeringHeadZoomRegion = Common::Rect(offeringHeadLeft, offeringHeadTop, offeringHeadRight, offeringHeadBottom);

	if (_staticData.location.node == 7)
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myMCViewedDeathGodDoor = 1;
}

int GenericCavernDoorMainView::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_staticData.location.node == 7 && (_staticData.location.timeZone != priorLocation.timeZone ||
			_staticData.location.environment != priorLocation.environment || _staticData.location.node != priorLocation.node ||
			_staticData.location.facing != priorLocation.facing || _staticData.location.orientation != priorLocation.orientation ||
			_staticData.location.depth != priorLocation.depth) && !((SceneViewWindow *)viewWindow)->isNumberInGlobalFlagTable(offsetof(GlobalFlags, evcapBaseID), offsetof(GlobalFlags, evcapNumCaptured), MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID))
		((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_PRESENT));
	return SC_TRUE;
}

int GenericCavernDoorMainView::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	// Build a default structure
	DestinationScene newDestination;
	newDestination.destinationScene = _staticData.location;
	newDestination.transitionType = TRANSITION_FADE;
	newDestination.transitionData = -1;
	newDestination.transitionStartFrame = -1;
	newDestination.transitionLength = -1;

	if (_topZoomRegion.contains(pointLocation)) {
		newDestination.destinationScene.depth = _topZoomDepth;
		((SceneViewWindow *)viewWindow)->moveToDestination(newDestination);
		return SC_TRUE;
	}

	if (_rightZoomRegion.contains(pointLocation)) {
		newDestination.destinationScene.depth = _rightZoomDepth;
		((SceneViewWindow *)viewWindow)->moveToDestination(newDestination);
		return SC_TRUE;
	}

	if (_offeringHeadZoomRegion.contains(pointLocation)) {
		newDestination.destinationScene.depth = _offeringHeadZoomDepth;
		((SceneViewWindow *)viewWindow)->moveToDestination(newDestination);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int GenericCavernDoorMainView::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_topZoomRegion.contains(pointLocation) || _rightZoomRegion.contains(pointLocation) || _offeringHeadZoomRegion.contains(pointLocation))
		return kCursorMagnifyingGlass;

	return kCursorArrow;
}

class GenericCavernDoorOfferingHead : public SceneBase {
public:
	GenericCavernDoorOfferingHead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int correctOfferingID = -1, int correctOfferingDestDepth = 0, int transitionType = -1, int transitionData = -1, int transitionStartFrame = -1, int transitionLength = -1);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	DestinationScene _correctDestination;
	int _correctOfferingID;
	Common::Rect _dropRegion;

	bool isValidItemToDrop(Window *viewWindow, int itemID);
};

GenericCavernDoorOfferingHead::GenericCavernDoorOfferingHead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int correctOfferingID, int correctOfferingDestDepth, int transitionType, int transitionData, int transitionStartFrame, int transitionLength) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_correctDestination.destinationScene = _staticData.location;
	_correctDestination.destinationScene.depth = correctOfferingDestDepth;
	_correctDestination.transitionType = transitionType;
	_correctDestination.transitionData = transitionData;
	_correctDestination.transitionStartFrame = transitionStartFrame;
	_correctDestination.transitionLength = transitionLength;
	_correctOfferingID = correctOfferingID;
	_dropRegion = Common::Rect(24, 92, 226, 154);
}

int GenericCavernDoorOfferingHead::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	// If this is walkthrough mode, only accept the correct item
	if (isValidItemToDrop(viewWindow, itemID) && _dropRegion.contains(pointLocation))
		return 1;

	return 0;
}

int GenericCavernDoorOfferingHead::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0;

	if (!isValidItemToDrop(viewWindow, itemID))
		return SIC_REJECT;

	if (_dropRegion.contains(pointLocation)) {
		switch (itemID) {
		case kItemBalconyKey:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(2);
			break;
		case kItemBloodyArrow:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(3);
			break;
		case kItemObsidianBlock:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(5);
			break;
		case kItemCoilOfRope:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(6);
			break;
		case kItemCopperKey:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(7);
			break;
		case kItemCopperMedallion:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(8);
			break;
		case kItemCeramicBowl:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(9);
			break;
		case kItemGrapplingHook:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(10);
			break;
		case kItemHammer:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(11);
			break;
		case kItemPreservedHeart:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(12);
			break;
		case kItemJadeBlock:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(13);
			break;
		case kItemLimestoneBlock:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(14);
			break;
		case kItemMetalBar:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(15);
			break;
		case kItemCavernSkull:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(16);
			break;
		case kItemEntrySkull:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(17);
			break;
		case kItemSpearSkull:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(18);
			break;
		case kItemWaterCanFull:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(19);
			break;
		case kItemWoodenPegs:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(20);
			break;
		case kItemGoldCoins:
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(21);
			break;
		}

		// Reset the offering flag
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myMCTransMadeAnOffering = 1;

		// If this was the correct offering, move to the open door scene
		if (itemID == _correctOfferingID) {
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 10), 128, false, true);
			((SceneViewWindow *)viewWindow)->moveToDestination(_correctDestination);
		}

		// These items don't get consumed
		if (itemID == kItemWaterCanFull || itemID == kItemGoldCoins)
			return SIC_REJECT;

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int GenericCavernDoorOfferingHead::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	Location newLocation = _staticData.location;
	newLocation.depth = 0;
	((SceneViewWindow *)viewWindow)->jumpToScene(newLocation);
	return SC_TRUE;
}

int GenericCavernDoorOfferingHead::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	return kCursorPutDown;
}

bool GenericCavernDoorOfferingHead::isValidItemToDrop(Window *viewWindow, int itemID) {
	// If this is walkthrough mode, only accept the correct item
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1) {
		if (itemID == _correctOfferingID || (_staticData.location.node == 8 && itemID == kItemBloodyArrow))
			return true;

		return false;
	}

	// Otherwise, any of the allowed items
	switch (itemID) {
	case kItemCavernSkull:
	case kItemEntrySkull:
	case kItemSpearSkull:
	case kItemBloodyArrow:
	case kItemCopperMedallion:
	case kItemCoilOfRope:
	case kItemCopperKey:
	case kItemJadeBlock:
	case kItemLimestoneBlock:
	case kItemObsidianBlock:
	case kItemGrapplingHook:
	case kItemPreservedHeart:
	case kItemHammer:
	case kItemGoldCoins:
	case kItemWaterCanEmpty:
	case kItemWaterCanFull:
	case kItemWoodenPegs:
	case kItemBalconyKey:
	case kItemBurnedLetter: // Can't actually drop this, though
		return true;
	}

	return false;
}

class MainCavernGlassCapture : public SceneBase {
public:
	MainCavernGlassCapture(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int locateAttempted(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _glass;
};

MainCavernGlassCapture::MainCavernGlassCapture(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_glass = Common::Rect(305, 126, 355, 156);
}

int MainCavernGlassCapture::locateAttempted(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcLocateEnabled == 1) {
		if (_glass.contains(pointLocation)) {
			// Play the animation
			((SceneViewWindow *)viewWindow)->playSynchronousAnimation(22);

			// Attempt to add it to the biochip
			if (((SceneViewWindow *)viewWindow)->addNumberToGlobalFlagTable(offsetof(GlobalFlags, evcapBaseID), offsetof(GlobalFlags, evcapNumCaptured), 12, MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID))
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ACQUIRED));
			else
				((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

			// Disable capture
			((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->disableEvidenceCapture();
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int MainCavernGlassCapture::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().bcLocateEnabled == 1) {
		if (_glass.contains(pointLocation))
			return -2;

		return -1;
	}

	return kCursorArrow;
}

bool SceneViewWindow::initializeMayanTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1) {
		GlobalFlags &flags = ((SceneViewWindow *)viewWindow)->getGlobalFlags();

		flags.myTPCodeWheelStatus = flags.generalWalkthroughMode;
		flags.myTPCodeWheelLeftIndex = flags.generalWalkthroughMode == 1 ? 8 : 0;
		flags.myTPCodeWheelRightIndex = flags.generalWalkthroughMode == 1 ? 12 : 0;
		flags.myMCDeathGodOfferings = 0;
		flags.myWGPlacedRope = flags.generalWalkthroughMode;
		flags.myWTCurrentBridgeStatus = 0;
		flags.myAGHeadAStatus = flags.generalWalkthroughMode == 1 ? 2 : 0;
		flags.myAGHeadBStatus = 0;
		flags.myAGHeadCStatus = 0;
		flags.myAGHeadDStatus = flags.generalWalkthroughMode == 1 ? 2 : 0;
		flags.myAGHeadAStatusSkullID = flags.generalWalkthroughMode == 1 ? kItemCavernSkull : 0;
		flags.myAGHeadBStatusSkullID = 0;
		flags.myAGHeadCStatusSkullID = 0;
		flags.myAGHeadDStatusSkullID = flags.generalWalkthroughMode == 1 ? kItemSpearSkull : 0;
		flags.myAGTimerHeadID = 0;
		flags.myAGTimerStartTime = 0;
		flags.myDGOfferedHeart = 0;
		flags.myAGHeadAOpenedTime = 0;
		flags.myAGHeadBOpenedTime = 0;
		flags.myAGHeadCOpenedTime = 0;
		flags.myAGHeadDOpenedTime = 0;

		flags.myPickedUpCeramicBowl = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemCeramicBowl) ? 1 : 0;
		flags.myMCPickedUpSkull = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemCavernSkull) ? 1 : 0;
		flags.myWGRetrievedJadeBlock = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemJadeBlock) ? 1 : 0;
		flags.myWTRetrievedLimestoneBlock = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemLimestoneBlock) ? 1 : 0;
		flags.myAGRetrievedEntrySkull = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemEntrySkull) ? 1 : 0;
		flags.myAGRetrievedSpearSkull = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemSpearSkull) ? 1 : 0;
		flags.myAGRetrievedCopperMedal = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemCopperMedallion) ? 1 : 0;
		flags.myAGRetrievedObsidianBlock = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemObsidianBlock) ? 1 : 0;
		flags.takenEnvironCart = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemEnvironCart) ? 1 : 0;

		if (flags.generalWalkthroughMode == 1) {
			flags.myMCPickedUpSkull = 1;
			flags.myAGRetrievedSpearSkull = 1;
			flags.myAGRetrievedCopperMedal = 1;
		}
	} else if (environment == 2) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEnteredMainCavern = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedMainCavern = 1;
	} else if (environment == 3) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedWealthGod = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedSpecRooms = 1;
	} else if (environment == 4) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedWaterGod = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedSpecRooms = 1;
	} else if (environment == 5) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedArrowGod = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedSpecRooms = 1;
	} else if (environment == 6) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedDeathGod = 1;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().myVisitedSpecRooms = 1;
	}

	return true;
}

bool SceneViewWindow::startMayanAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	bool checkFade = true;

	if (environment == 3) {
		if (oldEnvironment == 2)
			return _vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), checkFade, 64);

		return _vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), checkFade, 16); 
	} else if (environment == 4) {
		if (oldTimeZone == -2)
			_vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), fade, 64);
		else
			_vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), fade, 16);

		return _vm->_sound->setSecondaryAmbientSound(_vm->getFilePath(2, environment, 13), checkFade, 0);
	} else if (environment == 5) {
		_vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), fade, 64);
		return _vm->_sound->setSecondaryAmbientSound(_vm->getFilePath(2, environment, 12), checkFade, 128);
	}

	_vm->_sound->setAmbientSound(_vm->getFilePath(2, environment, SF_AMBIENT), fade, 64);
	return true;
}

SceneBase *SceneViewWindow::constructMayanSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 1:
		return new VideoDeath(_vm, viewWindow, sceneStaticData, priorLocation, 10, IDS_HUMAN_PRESENCE_500METERS);
	case 2:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 60, 134, 118, 180, kItemCeramicBowl, 96, offsetof(GlobalFlags, myPickedUpCeramicBowl));
	case 3:
		return new PlaceCeramicBowl(_vm, viewWindow, sceneStaticData, priorLocation);
	case 4:
		return new ClickChangeDepth(_vm, viewWindow, sceneStaticData, priorLocation, 1, kCursorMagnifyingGlass, 0, 0, 432, 189);
	case 5:
		return new AdjustWheels(_vm, viewWindow, sceneStaticData, priorLocation);
	case 6:
		return new DateCombinationRead(_vm, viewWindow, sceneStaticData, priorLocation);
	case 7:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_INNER_DOOR_TRANS_TEXT, 16, 6, 402, 110, offsetof(GlobalFlags, myTPTextTranslated), offsetof(GlobalFlags, myTPTransBreathOfItzamna));
	case 8:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_INNER_LEFT_TRANS_TEXT, 1, 6, 431, 98, offsetof(GlobalFlags, myTPTextTranslated));
	case 9:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_INNER_MIDDLE_TRANS_TEXT, 16, 8, 430, 114, offsetof(GlobalFlags, myTPTextTranslated), offsetof(GlobalFlags, myTPCalendarTopTranslated));
	case 10:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_OUTER_SOUTHLEFT_TRANS_TEXT, 4, 55, 426, 95, offsetof(GlobalFlags, myTPTextTranslated));
	case 11:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_OUTER_WEST_TRANS_TEXT, 4, 72, 420, 108, offsetof(GlobalFlags, myTPTextTranslated));
	case 12:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_OUTER_NORTH_TRANS_TEXT, 6, 38, 428, 76, offsetof(GlobalFlags, myTPTextTranslated));
	case 13:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 140, 124, 174, 158, kItemCavernSkull, 3, offsetof(GlobalFlags, myMCPickedUpSkull));
	case 14:
		return new GenericCavernDoorMainView(_vm, viewWindow, sceneStaticData, priorLocation, 1, 126, 1, 306, 30, 2, 287, 30, 379, 82, 3, 275, 84, 401, 174);
	case 15:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WG_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myWGTransDoorTop));
	case 16:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WG_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myMCTransWGOffering));
	case 17:
		return new GenericCavernDoorOfferingHead(_vm, viewWindow, sceneStaticData, priorLocation, kItemGoldCoins, 4, TRANSITION_WALK, -1, 1082, 13);
	case 18:
		return new GenericCavernDoorMainView(_vm, viewWindow, sceneStaticData, priorLocation, 1, 126, 1, 306, 30, 2, 287, 30, 379, 82, 3, 275, 84, 401, 174);
	case 19:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WATERGOD_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, offsetof(GlobalFlags, myMCTransDoor));
	case 20:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WATERGOD_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myMCTransWTOffering));
	case 21:
		return new GenericCavernDoorOfferingHead(_vm, viewWindow, sceneStaticData, priorLocation, kItemWaterCanFull, 4, TRANSITION_WALK, -1, 1125, 13);
	case 22:
		return new GenericCavernDoorMainView(_vm, viewWindow, sceneStaticData, priorLocation, 1, 126, 1, 306, 30, 2, 287, 30, 379, 82, 3, 275, 84, 401, 174);
	case 23:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_AG_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, offsetof(GlobalFlags, myMCTransDoor));
	case 24:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_AG_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myMCTransAGOffering));
	case 25:
		return new GenericCavernDoorOfferingHead(_vm, viewWindow, sceneStaticData, priorLocation, kItemBloodyArrow, 4, TRANSITION_WALK, -1, 1010, 12);
	case 26:
		return new GenericCavernDoorMainView(_vm, viewWindow, sceneStaticData, priorLocation, 1, 126, 1, 306, 30, 2, 287, 30, 379, 82, 3, 275, 84, 401, 174);
	case 27:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_DEATHGOD_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, offsetof(GlobalFlags, myMCTransDoor));
	case 28:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_DEATHGOD_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myMCTransDGOffering));
	case 31:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 194, 106, 278, 126, kItemJadeBlock, 105, offsetof(GlobalFlags, myWGRetrievedJadeBlock));
	case 32:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 140, 22, 306, 189, 2, 3, 0, 3, 1, 1, TRANSITION_WALK, -1, 264, 14, 14);
	case 33:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 158, 88, 288, 116, kItemLimestoneBlock, 84, offsetof(GlobalFlags, myWTRetrievedLimestoneBlock));
	case 34:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 80, 0, 332, 189, 2, 4, 0, 2, 1, 1, TRANSITION_WALK, -1, 401, 14, 14);
	case 50:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation,106, 0, 294, 189, 2, 5, 0, 1, 1, 1, TRANSITION_WALK, -1, 427, 13, 11);
	case 51:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 235, 144, 285, 181, kItemEntrySkull, 3, offsetof(GlobalFlags, myAGRetrievedEntrySkull));
	case 52:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 200, 138, 231, 185, kItemSpearSkull, 46, offsetof(GlobalFlags, myAGRetrievedSpearSkull));
	case 53:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 201, 4, 235, 22, kItemCopperMedallion, 45, offsetof(GlobalFlags, myAGRetrievedCopperMedal));
	case 54:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 206, 110, 280, 142, kItemObsidianBlock, 72, offsetof(GlobalFlags, myAGRetrievedObsidianBlock));
	case 60:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 11);
	case 65:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 90, 15, 346, 189, 2, 6, 0, 0, 1, 1, TRANSITION_WALK, -1, 33, 12, 13);
	case 68:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 206, 76, 246, 116, kItemEnvironCart, 53, offsetof(GlobalFlags, takenEnvironCart));
	case 69:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 10);
	case 70:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 128, offsetof(GlobalFlags, myMCStingerID), offsetof(GlobalFlags, myMCStingerChannelID), 11, 14);
	case 71:
		return new DisplayMessageWithEvidenceWhenEnteringNode(_vm, viewWindow, sceneStaticData, priorLocation, MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID, IDS_MBT_EVIDENCE_PRESENT);
	case 72:
		return new MainCavernGlassCapture(_vm, viewWindow, sceneStaticData, priorLocation);
	case 103:
		return new PlaySoundEnteringFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 12, 2, 4, 4, 2, 1, 5);
	case 120:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 10);
	case 121:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 125:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 226, 90, 256, 104, kItemCopperMedallion, 15, offsetof(GlobalFlags, myAGRetrievedCopperMedal));
	case 126:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDS_MY_AG_ALTAR_TEXT, 120, 44, 330, 72, -1, -1, offsetof(GlobalFlags, myAGVisitedAltar));
	case 127:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDS_MY_WG_ALTAR_TEXT, 118, 14, 338, 44);
	case 128:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDS_MY_WT_ALTAR_TEXT, 106, 128, 344, 162);
	}

	warning("TODO: Mayan scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
