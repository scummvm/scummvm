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

#include "common/system.h"
#include "graphics/surface.h"

namespace Buried {

enum {
	WAR_GOD_HEAD_TIMER_VALUE = 3000
};

class PlaceCeramicBowl : public SceneBase {
public:
	PlaceCeramicBowl(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int timerCallback(Window *viewWindow) override;

private:
	bool _dropped;
};

PlaceCeramicBowl::PlaceCeramicBowl(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_dropped = false;
}

int PlaceCeramicBowl::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if (itemID != kItemCeramicBowl)
		return SIC_REJECT;

	_staticData.navFrameIndex = 112;
	viewWindow->invalidateWindow(false);
	_dropped = true;
	return SIC_ACCEPT;
}

int PlaceCeramicBowl::timerCallback(Window *viewWindow) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_dropped) {
		if (globalFlags.myTPCodeWheelStatus == 0) {
			// Play slide death animation
			sceneView->playSynchronousAnimation(4);

			// Notify the player of his gruesome death
			sceneView->showDeathScene(11);
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
			sceneView->moveToDestination(newDest);
		}
	}

	return SC_TRUE;
}

class AdjustWheels : public SceneBase {
public:
	AdjustWheels(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	void preDestructor() override;
	int paint(Window *viewWindow, Graphics::Surface *preBuffer) override;
	int gdiPaint(Window *viewWindow) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	_curLeftFrame = globalFlags.myTPCodeWheelLeftIndex;
	_curRightFrame = globalFlags.myTPCodeWheelRightIndex;
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
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_translateText && globalFlags.bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(168, 70, 262, 108);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int AdjustWheels::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

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

		globalFlags.myTPCodeWheelLeftIndex = _curLeftFrame;
		globalFlags.myTPCodeWheelRightIndex = _curRightFrame;

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

		globalFlags.myTPCodeWheelStatus = status;

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
	sceneView->moveToDestination(newDest);
	return SC_TRUE;
}

int AdjustWheels::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (globalFlags.bcTranslateEnabled == 1) {
		Common::Rect translateTextRegion(168, 72, 260, 106);

		if (translateTextRegion.contains(pointLocation)) {
			if (!_translateText) {
				Common::String leftText = _vm->getString(IDMYTP_WHEELS_LEFT_TRANS_TEXT_BASE + _curLeftFrame / 2);
				Common::String rightText = _vm->getString(IDMYTP_WHEELS_RIGHT_TRANS_TEXT_BASE + _curRightFrame / 2);
				Common::String finalString = leftText + rightText;

				if (globalFlags.generalWalkthroughMode == 1 &&
					globalFlags.myTPCodeWheelStatus == 1) {

					if (_vm->getVersion() >= MAKEVERSION(1, 0, 4, 0))
						finalString += _vm->getString(IDS_MYTP_WALKTHROUGH_HINT_TEXT);
					else
						finalString += " (Mayan Sacred Day)";
				}

				sceneView->displayTranslationText(finalString);
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
	int gdiPaint(Window *viewWindow) override;
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _currentRegion;
};

DateCombinationRead::DateCombinationRead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_currentRegion = -1;
}

int DateCombinationRead::gdiPaint(Window *viewWindow) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_currentRegion >= 0 && globalFlags.bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		int left = _currentRegion * 43 + 20 + absoluteRect.left;
		Common::Rect rect(left, absoluteRect.top + 18, left + 43, absoluteRect.top + 110);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int DateCombinationRead::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (globalFlags.bcTranslateEnabled == 1) {
		Common::Rect symbols(20, 18, 407, 110);

		if (symbols.contains(pointLocation)) {
			int translatedSymbolIndex = (pointLocation.x - 20) / 43;

			if (_currentRegion != translatedSymbolIndex) {
				// Update flags
				globalFlags.myTPCalendarListTranslated = 1;
				globalFlags.myTPTextTranslated = 1;

				// Display the text
				sceneView->displayTranslationText(_vm->getString(IDMYTP_WALLS_COMBO_TRANS_TEXT_BASE + translatedSymbolIndex));

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
			int translatedTextID, int left, int top, int right, int bottom,
			byte &flagA, byte &flagB, byte &visitedFlag);
	int gdiPaint(Window *viewWindow) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	bool _textTranslated;
	int _textID;
	Common::Rect _clickableRegion;
	byte &_flagA;
	byte &_flagB;
};

ViewSingleTranslation::ViewSingleTranslation(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int translatedTextID, int left, int top, int right, int bottom,
		byte &flagA, byte &flagB, byte &visitedFlag) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation),
		_flagA(flagA), _flagB(flagB) {
	_textTranslated = false;
	_textID = translatedTextID;
	_clickableRegion = Common::Rect(left, top, right, bottom);
	visitedFlag = 1;
}

int ViewSingleTranslation::gdiPaint(Window *viewWindow) {
	// Draw the translated box, if applicable
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	if (_textTranslated && sceneView->getGlobalFlags().bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(_clickableRegion);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int ViewSingleTranslation::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	if (_staticData.location.depth != 0) {
		// If we're not at depth zero, move to depth zero
		Location newLocation = _staticData.location;
		newLocation.depth = 0;
		sceneView->jumpToScene(newLocation);
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ViewSingleTranslation::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	if (sceneView->getGlobalFlags().bcTranslateEnabled == 1) {
		if (_clickableRegion.contains(pointLocation)) {
			// Make sure we didn't already render the text
			if (!_textTranslated) {
				_flagA = 1;
				_flagB = 1;

				// Load and display the text
				sceneView->displayTranslationText(_vm->getString(_textID));
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
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	_topZoomDepth = topZoomDepth;
	_rightZoomDepth = rightZoomDepth;
	_offeringHeadZoomDepth = offeringHeadZoomDepth;
	_topZoomRegion = Common::Rect(topLeft, topTop, topRight, topBottom);
	_rightZoomRegion = Common::Rect(rightLeft, rightTop, rightRight, rightBottom);
	_offeringHeadZoomRegion = Common::Rect(offeringHeadLeft, offeringHeadTop, offeringHeadRight, offeringHeadBottom);

	if (_staticData.location.node == 7)
		globalFlags.myMCViewedDeathGodDoor = 1;
}

int GenericCavernDoorMainView::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	Location &loc = _staticData.location;

	if (loc.node == 7 && (loc.timeZone != priorLocation.timeZone ||
			loc.environment != priorLocation.environment || loc.node != priorLocation.node ||
			loc.facing != priorLocation.facing || loc.orientation != priorLocation.orientation ||
			loc.depth != priorLocation.depth) && !sceneView->isNumberInGlobalFlagTable(MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID))
		sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_PRESENT));
	return SC_TRUE;
}

int GenericCavernDoorMainView::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	// Build a default structure
	DestinationScene newDestination;
	newDestination.destinationScene = _staticData.location;
	newDestination.transitionType = TRANSITION_FADE;
	newDestination.transitionData = -1;
	newDestination.transitionStartFrame = -1;
	newDestination.transitionLength = -1;

	if (_topZoomRegion.contains(pointLocation)) {
		newDestination.destinationScene.depth = _topZoomDepth;
		sceneView->moveToDestination(newDestination);
		return SC_TRUE;
	}

	if (_rightZoomRegion.contains(pointLocation)) {
		newDestination.destinationScene.depth = _rightZoomDepth;
		sceneView->moveToDestination(newDestination);
		return SC_TRUE;
	}

	if (_offeringHeadZoomRegion.contains(pointLocation)) {
		newDestination.destinationScene.depth = _offeringHeadZoomDepth;
		sceneView->moveToDestination(newDestination);
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
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

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
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if (!isValidItemToDrop(viewWindow, itemID))
		return SIC_REJECT;

	if (_dropRegion.contains(pointLocation)) {
		switch (itemID) {
		case kItemBalconyKey:
			sceneView->playSynchronousAnimation(2);
			break;
		case kItemBloodyArrow:
			sceneView->playSynchronousAnimation(3);
			break;
		case kItemObsidianBlock:
			sceneView->playSynchronousAnimation(5);
			break;
		case kItemCoilOfRope:
			sceneView->playSynchronousAnimation(6);
			break;
		case kItemCopperKey:
			sceneView->playSynchronousAnimation(7);
			break;
		case kItemCopperMedallion:
			sceneView->playSynchronousAnimation(8);
			break;
		case kItemCeramicBowl:
			sceneView->playSynchronousAnimation(9);
			break;
		case kItemGrapplingHook:
			sceneView->playSynchronousAnimation(10);
			break;
		case kItemHammer:
			sceneView->playSynchronousAnimation(11);
			break;
		case kItemPreservedHeart:
			sceneView->playSynchronousAnimation(12);
			break;
		case kItemJadeBlock:
			sceneView->playSynchronousAnimation(13);
			break;
		case kItemLimestoneBlock:
			sceneView->playSynchronousAnimation(14);
			break;
		case kItemMetalBar:
			sceneView->playSynchronousAnimation(15);
			break;
		case kItemCavernSkull:
			sceneView->playSynchronousAnimation(16);
			break;
		case kItemEntrySkull:
			sceneView->playSynchronousAnimation(17);
			break;
		case kItemSpearSkull:
			sceneView->playSynchronousAnimation(18);
			break;
		case kItemWaterCanFull:
			sceneView->playSynchronousAnimation(19);
			break;
		case kItemWoodenPegs:
			sceneView->playSynchronousAnimation(20);
			break;
		case kItemGoldCoins:
			sceneView->playSynchronousAnimation(21);
			break;
		}

		// Reset the offering flag
		sceneView->getGlobalFlags().myMCTransMadeAnOffering = 1;

		// If this was the correct offering, move to the open door scene
		if (itemID == _correctOfferingID) {
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 10), 128, false, true);
			sceneView->moveToDestination(_correctDestination);
		}

		// These items don't get consumed
		if (itemID == kItemWaterCanFull || itemID == kItemGoldCoins)
			return SIC_REJECT;

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int GenericCavernDoorOfferingHead::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	Location newLocation = _staticData.location;
	newLocation.depth = 0;
	sceneView->jumpToScene(newLocation);
	return SC_TRUE;
}

int GenericCavernDoorOfferingHead::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	return kCursorPutDown;
}

bool GenericCavernDoorOfferingHead::isValidItemToDrop(Window *viewWindow, int itemID) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	// If this is walkthrough mode, only accept the correct item
	if (globalFlags.generalWalkthroughMode == 1) {
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

class DeathGodCavernDoorOfferingHead : public SceneBase {
public:
	DeathGodCavernDoorOfferingHead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int correctOfferingDestDepth = 0, int transitionType = -1, int transitionData = -1, int transitionStartFrame = -1, int transitionLength = -1);
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	DestinationScene _correctDestination;
	Common::Rect _dropRegion;
};

DeathGodCavernDoorOfferingHead::DeathGodCavernDoorOfferingHead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int correctOfferingDestDepth, int transitionType, int transitionData, int transitionStartFrame, int transitionLength) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	_correctDestination.destinationScene = _staticData.location;
	_correctDestination.destinationScene.depth = correctOfferingDestDepth;
	_correctDestination.transitionType = transitionType;
	_correctDestination.transitionData = transitionData;
	_correctDestination.transitionStartFrame = transitionStartFrame;
	_correctDestination.transitionLength = transitionLength;
	_dropRegion = Common::Rect(50, 76, 228, 182);

	byte offerings = globalFlags.myMCDeathGodOfferings;

	if (offerings & 1) {
		if (offerings & 2) {
			if (offerings & 4)
				_staticData.navFrameIndex = 190;
			else
				_staticData.navFrameIndex = 189;
		} else if (offerings & 4) {
			_staticData.navFrameIndex = 188;
		} else {
			_staticData.navFrameIndex = 186;
		}
	} else if (offerings & 2) {
		if (offerings & 4)
			_staticData.navFrameIndex = 187;
		else
			_staticData.navFrameIndex = 185;
	} else if (offerings & 4) {
		_staticData.navFrameIndex = 184;
	}
}

int DeathGodCavernDoorOfferingHead::preExitRoom(Window *viewWindow, const Location &newLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	InventoryWindow *inventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow;

	// Put any pieces placed in the head back in the inventory
	byte &offerings = globalFlags.myMCDeathGodOfferings;

	if (offerings & 1)
		inventoryWindow->addItem(kItemObsidianBlock);
	if (offerings & 2)
		inventoryWindow->addItem(kItemJadeBlock);
	if (offerings & 4)
		inventoryWindow->addItem(kItemLimestoneBlock);

	offerings = 0;
	return SC_TRUE;
}

int DeathGodCavernDoorOfferingHead::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	// Return to depth zero
	DestinationScene newDest;
	newDest.destinationScene = _staticData.location;
	newDest.destinationScene.depth = 0;
	newDest.transitionType = TRANSITION_NONE;
	newDest.transitionData = -1;
	newDest.transitionStartFrame = -1;
	newDest.transitionLength = -1;
	sceneView->moveToDestination(newDest);
	return SC_TRUE;
}

int DeathGodCavernDoorOfferingHead::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if ((itemID == kItemJadeBlock || itemID == kItemLimestoneBlock || itemID == kItemObsidianBlock) && _dropRegion.contains(pointLocation)) {
		byte offerings = globalFlags.myMCDeathGodOfferings;

		if ((offerings & 1) != 0 && itemID == kItemObsidianBlock)
			return 0;
		if ((offerings & 2) != 0 && itemID == kItemJadeBlock)
			return 0;
		if ((offerings & 4) != 0 && itemID == kItemLimestoneBlock)
			return 0;

		return 1;
	}

	return 0;
}

int DeathGodCavernDoorOfferingHead::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if ((itemID == kItemJadeBlock || itemID == kItemLimestoneBlock || itemID == kItemObsidianBlock) && _dropRegion.contains(pointLocation)) {
		byte &offerings = globalFlags.myMCDeathGodOfferings;

		// Make sure we didn't already place the item
		if ((offerings & 1) != 0 && itemID == kItemObsidianBlock)
			return SIC_REJECT;
		if ((offerings & 2) != 0 && itemID == kItemJadeBlock)
			return SIC_REJECT;
		if ((offerings & 4) != 0 && itemID == kItemLimestoneBlock)
			return SIC_REJECT;

		// Add the item
		if (itemID == kItemObsidianBlock)
			offerings |= 1;
		else if (itemID == kItemJadeBlock)
			offerings |= 2;
		else if (itemID == kItemLimestoneBlock)
			offerings |= 4;

		// Change the image
		if (offerings & 1) {
			if (offerings & 2) {
				if (offerings & 4)
					_staticData.navFrameIndex = 190;
				else
					_staticData.navFrameIndex = 189;
			} else if (offerings & 4) {
				_staticData.navFrameIndex = 188;
			} else {
				_staticData.navFrameIndex = 186;
			}
		} else if (offerings & 2) {
			if (offerings & 4)
				_staticData.navFrameIndex = 187;
			else
				_staticData.navFrameIndex = 185;
		} else if (offerings & 4) {
			_staticData.navFrameIndex = 184;
		} else {
			_staticData.navFrameIndex = 152;
		}

		viewWindow->invalidateWindow(false);

		if ((offerings & 1) != 0 && (offerings & 2) != 0 && (offerings & 4) != 0) {
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 10), 128, false, true);
			sceneView->moveToDestination(_correctDestination);
		}

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int DeathGodCavernDoorOfferingHead::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	return kCursorPutDown;
}

class WealthGodRopeDrop : public SceneBase {
public:
	WealthGodRopeDrop(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;

private:
	Common::Rect _dropRope;
};

WealthGodRopeDrop::WealthGodRopeDrop(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (globalFlags.myWGPlacedRope == 1)
		_staticData.navFrameIndex = 121;

	_dropRope = Common::Rect(222, 149, 282, 189);
}

int WealthGodRopeDrop::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (globalFlags.myWGPlacedRope != 0) {
		Location newLocation = _staticData.location;
		newLocation.depth = 1;
		sceneView->jumpToScene(newLocation);
	}

	return SC_TRUE;
}

int WealthGodRopeDrop::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	// OK, I honestly didn't know you could use the grappling hook here
	if (_dropRope.contains(pointLocation) && (itemID == kItemCoilOfRope || itemID == kItemGrapplingHook))
		return 1;

	return 0;
}

int WealthGodRopeDrop::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if (_dropRope.contains(pointLocation) && (itemID == kItemCoilOfRope || itemID == kItemGrapplingHook)) {
		globalFlags.myWGPlacedRope = 1;
		Location newLocation = _staticData.location;
		newLocation.depth = 1;
		sceneView->jumpToScene(newLocation);
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

class WaterGodInitialWalkSetFlag : public SceneBase {
public:
	WaterGodInitialWalkSetFlag(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
};

WaterGodInitialWalkSetFlag::WaterGodInitialWalkSetFlag(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	// Set flag on entry
	globalFlags.myWTCurrentBridgeStatus = 1;
}

class WaterGodBridgeJump : public SceneBase {
public:
	WaterGodBridgeJump(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int movieFileNameID = 0, int playingStartingFrame = 0, int sequenceStartingFrame = 0, int framesPerCycle = 0,
			int jumpFudgeFrames = 0, int sequenceLength = 0, bool jumpMidCycle = false, int frameOffsetToEndOfSwing = 0);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
	int timerCallback(Window *viewWindow) override;
	int movieCallback(Window *viewWindow, VideoWindow *movie, int animationID, int status) override;

private:
	int _movieID;
	int _startingMovieFrame;
	int _playingStartingFrame;
	int _framesPerCycle;
	int _sequenceLength;
	int _jumpFudgeFrames;
	int _finalFrameIndex;
	int _soundID;
	DestinationScene _savedDestForward;
	bool _jumpMidCycle;
	int _frameOffsetToEndOfSwing;
};

WaterGodBridgeJump::WaterGodBridgeJump(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int movieFileNameID, int playingStartingFrame, int sequenceStartingFrame, int framesPerCycle,
		int jumpFudgeFrames, int sequenceLength, bool jumpMidCycle, int frameOffsetToEndOfSwing) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	_movieID = movieFileNameID;
	_playingStartingFrame = playingStartingFrame;
	_startingMovieFrame = sequenceStartingFrame;
	_framesPerCycle = framesPerCycle;
	_sequenceLength = sequenceLength;
	_jumpFudgeFrames = jumpFudgeFrames;
	_finalFrameIndex = -1;
	_soundID = -1;
	_jumpMidCycle = jumpMidCycle;
	_frameOffsetToEndOfSwing = frameOffsetToEndOfSwing;

	// Save the forward movement data for later
	_savedDestForward = _staticData.destForward;
	_staticData.destForward.destinationScene = Location(-1, -1, -1, -1, -1, -1);
	_staticData.destForward.transitionType = -1;
	_staticData.destForward.transitionData = -1;
	_staticData.destForward.transitionStartFrame = -1;
	_staticData.destForward.transitionLength = -1;

	// Set visited flag
	globalFlags.myWTSteppedOnSwings = 1;
}

int WaterGodBridgeJump::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	// Raise the ambient sound
	_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
	uint32 ambientPos = _vm->_sound->getSecondaryAmbientPosition();

	int frameStartingOffset = (ambientPos / 1838) % _sequenceLength + (_startingMovieFrame - _playingStartingFrame) % _sequenceLength;

	// Load and start the new asynchronous animation
	sceneView->startAsynchronousAnimation(_movieID, _startingMovieFrame, _playingStartingFrame + frameStartingOffset, _sequenceLength, true);
	return SC_TRUE;
}

int WaterGodBridgeJump::preExitRoom(Window *viewWindow, const Location &newLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	_finalFrameIndex = sceneView->getAsynchronousAnimationCurrentPosition();

	// Moving to another node should kill the anim
	if (newLocation.node != 4 || newLocation.timeZone != 2 || newLocation.environment != 4)
		sceneView->stopAsynchronousAnimation();

	// If we are walking into a node less than 5, kill the ambient
	if (newLocation.node <= 3)
		_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);

	return SC_TRUE;
}

int WaterGodBridgeJump::postExitRoom(Window *viewWindow, const Location &newLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	if (newLocation.facing == _staticData.location.facing && newLocation.timeZone == _staticData.location.timeZone &&
			newLocation.environment == _staticData.location.environment) {
		if (_jumpMidCycle) {
			int diff = (_finalFrameIndex - _playingStartingFrame) % (_framesPerCycle * 2);
			int diffB = (_finalFrameIndex - _playingStartingFrame - _framesPerCycle) % _framesPerCycle;
			int diffC = _framesPerCycle - (_finalFrameIndex - _playingStartingFrame) % _framesPerCycle;

			if (diff > _framesPerCycle || diffB > _framesPerCycle || diffC > _jumpFudgeFrames * 2) {
				if (_staticData.location.facing == 0)
					sceneView->showDeathScene(14);
				else
					sceneView->showDeathScene(15);

				return SC_DEATH;
			}
		} else {
			if ((_finalFrameIndex - _playingStartingFrame) % _framesPerCycle > _jumpFudgeFrames && _framesPerCycle - (_finalFrameIndex - _playingStartingFrame) % _framesPerCycle > _jumpFudgeFrames) {
				if (_staticData.location.facing == 0)
					sceneView->showDeathScene(14);
				else
					sceneView->showDeathScene(15);

				return SC_DEATH;
			}
		}
	}

	return SC_TRUE;
}

int WaterGodBridgeJump::timerCallback(Window *viewWindow) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	NavArrowWindow *navArrow = ((GameUIWindow *)viewWindow->getParent())->_navArrowWindow;

	// If we have reached the end of the starting sequence, reset the arrows
	if (_staticData.destForward.destinationScene.timeZone == -1 && sceneView->getAsynchronousAnimationCurrentPosition() >= _startingMovieFrame) {
		_staticData.destForward = _savedDestForward;
		navArrow->updateAllArrows(_staticData);
	}

	return SC_TRUE;
}

int WaterGodBridgeJump::movieCallback(Window *viewWindow, VideoWindow *movie, int animationID, int status) {
	if (status == MOVIE_LOOPING_RESTART)
		_vm->_sound->restartSecondaryAmbientSound();

	return SC_TRUE;
}

class ArrowGodHead : public SceneBase {
public:
	ArrowGodHead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int headID = 0, int clickLeft = -1, int clickTop = -1, int clickRight = -1, int clickBottom = -1,
			int emptyClosedStill = -1, int emptyOpenStill = -1, int fullClosedStill = -1, int fullOpenStill = -1,
			int emptyClosedAnim = -1, int emptyOpenAnim = -1, int fullClosedAnim = -1, int fullOpenAnim = -1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;
	int timerCallback(Window *viewWindow) override;

private:
	int _headID;
	Common::Rect _skullRegion;
	int _stillFrames[4];
	int _soundID;
	int _headAnimations[4];
	GlobalFlags &_globalFlags;

	uint32 *headOpenedTime(byte headId);
	uint32 getHeadOpenedTime(byte headId);
	void setHeadOpenedTime(byte headId, uint32 value);
	byte *headTouched(byte headId);
	void setHeadTouched(byte value);
	byte *headStatus(byte headId);
	byte getHeadStatus();
	byte getHeadStatus(byte headId);
	void setHeadStatus(byte headId, byte value);
	void setHeadStatus(byte value);
	byte *headStatusSkullId(byte headId);
	byte getHeadStatusSkullId();
	void setHeadStatusSkullId(byte value);
};

ArrowGodHead::ArrowGodHead(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int headID, int clickLeft, int clickTop, int clickRight, int clickBottom,
		int emptyClosedStill, int emptyOpenStill, int fullClosedStill, int fullOpenStill,
		int emptyClosedAnim, int emptyOpenAnim, int fullClosedAnim, int fullOpenAnim) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation),
		_globalFlags(((SceneViewWindow *)viewWindow)->getGlobalFlags()) {
	_soundID = -1;
	_headID = headID;
	_skullRegion = Common::Rect(clickLeft, clickTop, clickRight, clickBottom);
	_stillFrames[0] = emptyClosedStill;
	_stillFrames[1] = emptyOpenStill;
	_stillFrames[2] = fullClosedStill;
	_stillFrames[3] = fullOpenStill;
	_headAnimations[0] = emptyClosedAnim;
	_headAnimations[1] = emptyOpenAnim;
	_headAnimations[2] = fullClosedAnim;
	_headAnimations[3] = fullOpenAnim;
	_staticData.navFrameIndex = _stillFrames[getHeadStatus()];
}

uint32 *ArrowGodHead::headOpenedTime(byte headId) {
	switch (headId) {
	case 0:
		return &_globalFlags.myAGHeadAOpenedTime;
	case 1:
		return &_globalFlags.myAGHeadBOpenedTime;
	case 2:
		return &_globalFlags.myAGHeadCOpenedTime;
	case 3:
		return &_globalFlags.myAGHeadDOpenedTime;
	default:
		return nullptr;
	}
}

uint32 ArrowGodHead::getHeadOpenedTime(byte headId) {
	uint32 *openedTime = headOpenedTime(headId);
	return openedTime ? *openedTime : 0;
}

void ArrowGodHead::setHeadOpenedTime(byte headId, uint32 value) {
	uint32 *openedTime = headOpenedTime(headId);
	if (openedTime)
		*openedTime = value;
}

byte *ArrowGodHead::headTouched(byte headId) {
	switch (headId) {
	case 0:
		return &_globalFlags.myAGHeadATouched;
	case 1:
		return &_globalFlags.myAGHeadBTouched;
	case 2:
		return &_globalFlags.myAGHeadCTouched;
	case 3:
		return &_globalFlags.myAGHeadDTouched;
	default:
		return nullptr;
	}
}

void ArrowGodHead::setHeadTouched(byte value) {
	byte *touched = headTouched(_headID);
	if (touched)
		*touched = value;
}

byte *ArrowGodHead::headStatus(byte headId) {
	switch (headId) {
	case 0:
		return &_globalFlags.myAGHeadAStatus;
	case 1:
		return &_globalFlags.myAGHeadBStatus;
	case 2:
		return &_globalFlags.myAGHeadCStatus;
	case 3:
		return &_globalFlags.myAGHeadDStatus;
	default:
		return nullptr;
	}
}

byte ArrowGodHead::getHeadStatus() {
	return getHeadStatus(_headID);
}

byte ArrowGodHead::getHeadStatus(byte headId) {
	byte *status = headStatus(headId);
	return status ? *status : 0;
}

void ArrowGodHead::setHeadStatus(byte value) {
	setHeadStatus(_headID, value);
}

void ArrowGodHead::setHeadStatus(byte headId, byte value) {
	byte *status = headStatus(headId);
	if (status)
		*status = value;
}

byte *ArrowGodHead::headStatusSkullId(byte headId) {
	switch (headId) {
	case 0:
		return &_globalFlags.myAGHeadAStatusSkullID;
	case 1:
		return &_globalFlags.myAGHeadBStatusSkullID;
	case 2:
		return &_globalFlags.myAGHeadCStatusSkullID;
	case 3:
		return &_globalFlags.myAGHeadDStatusSkullID;
	default:
		return nullptr;
	}
}

byte ArrowGodHead::getHeadStatusSkullId() {
	byte *status = headStatusSkullId(_headID);
	return status ? *status : 0;
}

void ArrowGodHead::setHeadStatusSkullId(byte value) {
	byte *status = headStatusSkullId(_headID);
	if (status)
		*status = value;
}

int ArrowGodHead::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	byte headAStatus = _globalFlags.myAGHeadAStatus;
	byte headDStatus = _globalFlags.myAGHeadDStatus;

	if (_staticData.location.node == 0) {
		if (headAStatus == 0)
			_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
		else if (headDStatus == 0)
			_vm->_sound->adjustSecondaryAmbientSoundVolume(64, false, 0, 0);
		else
			_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
	} else if (_staticData.location.node == 2) {
		if (headAStatus == 0 || headDStatus == 0)
			_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
		else
			_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
	}

	return SC_TRUE;
}

int ArrowGodHead::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	BioChipRightWindow *bioChipRightWindow = ((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow;
	InventoryWindow *inventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow;

	// For walkthrough mode, don't allow input
	if (_globalFlags.generalWalkthroughMode == 1 && (_headID == 0 || _headID == 3))
		return SC_FALSE;

	if (_skullRegion.contains(pointLocation) && getHeadStatus() == 3) {
		byte skullIndex = getHeadStatusSkullId();
		setHeadStatusSkullId(0);
		setHeadStatus(1);
		_staticData.navFrameIndex = _stillFrames[1];
		setHeadTouched(1);

		// Begin dragging
		Common::Point ptInventoryWindow = viewWindow->convertPointToWindow(pointLocation, inventoryWindow);
		inventoryWindow->startDraggingNewItem(skullIndex, ptInventoryWindow);
		bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ArrowGodHead::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	BioChipRightWindow *bioChipRightWindow = ((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow;
	Location &loc = _staticData.location;

	// For walkthrough mode, don't allow input
	if (_globalFlags.generalWalkthroughMode == 1 && (_headID == 0 || _headID == 3))
		return SC_FALSE;

	// Did we click on the head?
	if (_skullRegion.contains(pointLocation)) {
		byte headStatus = getHeadStatus();
		setHeadTouched(1);

		if (headStatus & 1)
			headStatus--;
		else
			headStatus++;

		setHeadStatus(headStatus);

		// Play the proper movie
		int currentSoundID = -1;
		if (headStatus == 2)
			currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 14), 128, false, true);
		else
			currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 13), 128, false, true);

		if ((_headID == 1 || _headID == 2) && headStatus == 0) {
			if (loc.node == 0)
				_vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 11), 127);
			else
				_vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 11), 96);
		}

		sceneView->playSynchronousAnimation(_headAnimations[headStatus]);

		_staticData.navFrameIndex = _stillFrames[headStatus];
		viewWindow->invalidateWindow(false);

		byte headAStatus = _globalFlags.myAGHeadAStatus;
		byte headBStatus = _globalFlags.myAGHeadBStatus;
		byte headCStatus = _globalFlags.myAGHeadCStatus;
		byte headDStatus = _globalFlags.myAGHeadDStatus;

		if (loc.node == 0) {
			if (headAStatus == 0)
				_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
			else if (headDStatus == 0)
				_vm->_sound->adjustSecondaryAmbientSoundVolume(64, false, 0, 0);
			else
				_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
		} else if (loc.node == 2) {
			if (headAStatus == 0 || headDStatus == 0)
				_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
			else
				_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
		}

		_vm->_sound->stopSoundEffect(currentSoundID);

		if (loc.node == 0 && (headBStatus < 3 && headCStatus < 3))
			_vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 11), 127);
		else if (loc.node == 2 && (headBStatus < 3 && headCStatus < 3))
			_vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 11), 96);

		if (headStatus & 1)
			setHeadOpenedTime(_headID, g_system->getMillis());

		bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int ArrowGodHead::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (_globalFlags.generalWalkthroughMode == 1 && (_headID == 0 || _headID == 3))
		return 0;

	if ((itemID == kItemCavernSkull || itemID == kItemEntrySkull || itemID == kItemSpearSkull) && getHeadStatus() < 2 && _skullRegion.contains(pointLocation))
		return 1;

	return 0;
}

int ArrowGodHead::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	BioChipRightWindow *bioChipRightWindow = ((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow;

	if (_globalFlags.generalWalkthroughMode == 1 && (_headID == 0 || _headID == 3))
		return SIC_REJECT;

	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if ((itemID == kItemCavernSkull || itemID == kItemEntrySkull || itemID == kItemSpearSkull) && getHeadStatus() == 1 && _skullRegion.contains(pointLocation)) {
		setHeadStatus(2);
		setHeadTouched(1);
		setHeadStatusSkullId(itemID);

		int currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14), 128, false, true);

		sceneView->playSynchronousAnimation(_headAnimations[2]);

		_staticData.navFrameIndex = _stillFrames[2];
		viewWindow->invalidateWindow(false);

		byte headAStatus = _globalFlags.myAGHeadAStatus;
		byte headDStatus = _globalFlags.myAGHeadDStatus;

		if (_staticData.location.node == 0) {
			if (headAStatus == 0)
				_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
			else if (headDStatus == 0)
				_vm->_sound->adjustSecondaryAmbientSoundVolume(64, false, 0, 0);
			else
				_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
		} else if (_staticData.location.node == 2) {
			if (headAStatus == 0 || headDStatus == 0)
				_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
			else
				_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
		}

		_vm->_sound->stopSoundEffect(currentSoundID);

		bioChipRightWindow->sceneChanged();
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int ArrowGodHead::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_globalFlags.generalWalkthroughMode == 1 && (_headID == 0 || _headID == 3))
		return 0;

	if (_skullRegion.contains(pointLocation)) {
		if (getHeadStatus() == 3)
			return kCursorOpenHand;

		return kCursorFinger;
	}

	return 0;
}

int ArrowGodHead::timerCallback(Window *viewWindow) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	BioChipRightWindow *bioChipRightWindow = ((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow;
	Location &loc = _staticData.location;

	for (int i = 0; i < 4; i++) {
		uint32 lastStartedTimer = getHeadOpenedTime(i);

		if (lastStartedTimer > 0 && g_system->getMillis() > (lastStartedTimer + WAR_GOD_HEAD_TIMER_VALUE)) {
			setHeadOpenedTime(i, 0);

			TempCursorChange cursorChange(kCursorWait);

			if (i == _headID) {
				byte status = getHeadStatus();

				if (status & 1) {
					status--;
					setHeadStatus(status);

					int currentSoundID = -1;
					if (status == 2)
						currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 14), 128, false, true);
					else
						currentSoundID = _vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 13), 128, false, true);

					sceneView->playSynchronousAnimation(_headAnimations[status]);

					_staticData.navFrameIndex = _stillFrames[status];
					viewWindow->invalidateWindow(false);

					_vm->_sound->stopSoundEffect(currentSoundID);

					byte headAStatus = _globalFlags.myAGHeadAStatus;
					byte headBStatus = _globalFlags.myAGHeadBStatus;
					byte headCStatus = _globalFlags.myAGHeadCStatus;
					byte headDStatus = _globalFlags.myAGHeadDStatus;

					if (loc.node == 0) {
						if (headAStatus == 0)
							_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
						else if (headDStatus == 0)
							_vm->_sound->adjustSecondaryAmbientSoundVolume(64, false, 0, 0);
						else
							_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
					} else if (loc.node == 2) {
						if (headAStatus == 0 || headDStatus == 0)
							_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
						else
							_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
					}

					// Play the door closing sound, if applicable
					if (i == 1 || i == 2) {
						if (loc.node == 0 && (headBStatus == 0 || headCStatus == 0))
							_vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 11), 127);
						else if (loc.node == 2 && (headBStatus == 0 || headCStatus == 0))
							_vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 11), 96);
					}
				}

				bioChipRightWindow->sceneChanged();
			} else {
				byte status = getHeadStatus(i);

				if (status & 1) {
					status--;
					setHeadStatus(i, status);

					if (status == 2)
						_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 14), 128);
					else
						_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 13), 128);

					byte headAStatus = _globalFlags.myAGHeadAStatus;
					byte headDStatus = _globalFlags.myAGHeadDStatus;

					if (loc.node == 0) {
						if (headAStatus == 0)
							_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
						else if (headDStatus == 0)
							_vm->_sound->adjustSecondaryAmbientSoundVolume(64, false, 0, 0);
						else
							_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
					} else if (loc.node == 2) {
						if (headAStatus == 0 || headDStatus == 0)
							_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
						else
							_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
					}
				}

				bioChipRightWindow->sceneChanged();

				if (_headID == 1 || _headID == 2) {
					if (loc.node == 0)
						_vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 11), 127);
					else if (loc.node == 2)
						_vm->_sound->playSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, 11), 96);
				}
			}
		}
	}

	return SC_TRUE;
}

class ArrowGodDepthChange : public SceneBase {
public:
	ArrowGodDepthChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int postExitRoom(Window *viewWindow, const Location &newLocation) override;
	int timerCallback(Window *viewWindow) override;

private:
	bool _scheduledDepthChange;
	int _soundID;
	GlobalFlags &_globalFlags;

	bool adjustSpearVolume(Window *viewWindow);
	uint32 *headOpenedTime(byte headId);
	uint32 getHeadOpenedTime(byte headId);
	void setHeadOpenedTime(byte headId, uint32 value);
	byte *headStatus(byte headId);
	byte getHeadStatus(byte headId);
	void setHeadStatus(byte headId, byte value);
};

ArrowGodDepthChange::ArrowGodDepthChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation),
		_globalFlags(((SceneViewWindow *)viewWindow)->getGlobalFlags()) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

	_scheduledDepthChange = false;
	_soundID = -1;

	byte headAStatus = _globalFlags.myAGHeadAStatus;
	byte headBStatus = _globalFlags.myAGHeadBStatus;
	byte headCStatus = _globalFlags.myAGHeadCStatus;
	byte headDStatus = _globalFlags.myAGHeadDStatus;

	int targetDepth = 0;

	if (headBStatus > 0 || headCStatus > 0) {
		if (headBStatus > 0) {
			if (headAStatus == 0 || headDStatus == 0) {
				if (headAStatus == 0 && headDStatus == 0) {
					targetDepth = 11;
				} else if (headAStatus > 0 && headDStatus == 0) {
					targetDepth = 10;
				} else if (headAStatus == 0 && headDStatus > 0) {
					targetDepth = 9;
				}
			} else {
				targetDepth = 8;
			}
		} else if (headCStatus > 0) {
			if (headAStatus == 0 || headDStatus == 0) {
				if (headAStatus == 0 && headDStatus == 0) {
					targetDepth = 7;
				} else if (headAStatus > 0 && headDStatus == 0) {
					targetDepth = 6;
				} else if (headAStatus == 0 && headDStatus > 0) {
					targetDepth = 5;
				}
			} else {
				targetDepth = 4;
			}
		}
	} else if (headAStatus == 0 || headDStatus == 0) {
		if (headAStatus == 0 && headBStatus == 0) {
			targetDepth = 3;
		} else {
			if (headAStatus > 0 && headDStatus == 0) {
				targetDepth = 2;
			} else if (headAStatus == 0 && headDStatus > 0) {
				targetDepth = 1;
			}
		}
	}

	if (_staticData.location.depth != targetDepth) {
		Location newLocation = _staticData.location;
		newLocation.depth = targetDepth;
		sceneView->getSceneStaticData(newLocation, _staticData);
		_frameCycleCount = _staticData.cycleStartFrame;

		// Reload the frame files, if applicable
		sceneView->changeStillFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_STILLS));

		if (_staticData.cycleStartFrame >= 0)
			sceneView->changeCycleFrameMovie(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, SF_CYCLES));
	}
}

int ArrowGodDepthChange::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (((priorLocation.depth >= 8 && priorLocation.depth <= 11) && _staticData.location.depth < 8) ||
			((priorLocation.depth >= 4 && priorLocation.depth <= 7) && _staticData.location.depth < 4)) {
		SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);

		byte headBStatus = _globalFlags.myAGHeadBStatus;
		byte headCStatus = _globalFlags.myAGHeadCStatus;
		byte &headDStatus = _globalFlags.myAGHeadDStatus;
		byte doorLevelVolume = 0;

		switch (_staticData.location.node) {
		case 0:
		case 1:
			doorLevelVolume = 127;
			break;
		case 2:
			doorLevelVolume = 96;

			if (headDStatus == 2) {
				headDStatus--;
				_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13), 128);
				_scheduledDepthChange = true;
				adjustSpearVolume(viewWindow);
				sceneView->jumpToScene(_staticData.location);
				return SC_TRUE; // Original does not return here, but the status of this would be bad in that case
			}
			break;
		case 3:
			if (headCStatus == 0)
				doorLevelVolume = 127;
			else if (headBStatus == 0)
				doorLevelVolume = 64;
			break;
		}

		if (doorLevelVolume > 0)
			_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 11), doorLevelVolume);
	}

	adjustSpearVolume(viewWindow);
	return SC_TRUE;
}

int ArrowGodDepthChange::postExitRoom(Window *viewWindow, const Location &newLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	Location &loc = _staticData.location;

	if (loc.timeZone == newLocation.timeZone &&
			loc.environment == newLocation.environment &&
			loc.node == newLocation.node &&
			loc.facing == newLocation.facing &&
			loc.orientation == newLocation.orientation &&
			loc.depth == newLocation.depth &&
			!_scheduledDepthChange) {
		// Notify the player of his gruesome death
		sceneView->showDeathScene(13);
		return SC_DEATH;
	}

	return SC_TRUE;
}

uint32 *ArrowGodDepthChange::headOpenedTime(byte headId) {
	switch (headId) {
	case 0:
		return &_globalFlags.myAGHeadAOpenedTime;
	case 1:
		return &_globalFlags.myAGHeadBOpenedTime;
	case 2:
		return &_globalFlags.myAGHeadCOpenedTime;
	case 3:
		return &_globalFlags.myAGHeadDOpenedTime;
	default:
		return nullptr;
	}
}

uint32 ArrowGodDepthChange::getHeadOpenedTime(byte headId) {
	uint32 *openedTime = headOpenedTime(headId);
	return openedTime ? *openedTime : 0;
}

void ArrowGodDepthChange::setHeadOpenedTime(byte headId, uint32 value) {
	uint32 *openedTime = headOpenedTime(headId);
	if (openedTime)
		*openedTime = value;
}

byte *ArrowGodDepthChange::headStatus(byte headId) {
	switch (headId) {
	case 0:
		return &_globalFlags.myAGHeadAStatus;
	case 1:
		return &_globalFlags.myAGHeadBStatus;
	case 2:
		return &_globalFlags.myAGHeadCStatus;
	case 3:
		return &_globalFlags.myAGHeadDStatus;
	default:
		return nullptr;
	}
}

byte ArrowGodDepthChange::getHeadStatus(byte headId) {
	byte *status = headStatus(headId);
	return status ? *status : 0;
}

void ArrowGodDepthChange::setHeadStatus(byte headId, byte value) {
	byte *status = headStatus(headId);
	if (status)
		*status = value;
}

int ArrowGodDepthChange::timerCallback(Window *viewWindow) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	BioChipRightWindow *bioChipRightWindow = ((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow;
	Location &loc = _staticData.location;

	SceneBase::timerCallback(viewWindow);

	// Check to see if we moved into a death scene
	if (loc.timeZone == 2 && loc.environment == 5 &&
			loc.node == 1 && loc.facing == 3 &&
			loc.orientation == 1 && (
			loc.depth == 1 || loc.depth == 3 || loc.depth == 11 ||
			loc.depth == 7 || loc.depth == 5 || loc.depth == 9)) {
		if (loc.depth == 1)
			sceneView->playSynchronousAnimation(19);

		sceneView->showDeathScene(13);
		return SC_DEATH;
	}

	if (loc.timeZone == 2 && loc.environment == 5 &&
			loc.node == 3 && loc.facing == 3 &&
			loc.orientation == 1 && (
			loc.depth == 2  || loc.depth == 3 || loc.depth == 11 ||
			loc.depth == 10 || loc.depth == 6 || loc.depth == 7)) {
		sceneView->playSynchronousAnimation(17);
		sceneView->showDeathScene(13);
		return SC_DEATH;
	}

	// Loop through the four heads
	for (int i = 0; i < 4; i++) {
		uint32 lastStartedTimer = getHeadOpenedTime(i);

		// Check if there is a timer going for this head
		if (lastStartedTimer > 0 && (g_system->getMillis() > (lastStartedTimer + WAR_GOD_HEAD_TIMER_VALUE) ||
				i == 0 || (_globalFlags.generalWalkthroughMode == 1 && i == 1) ||
				(getHeadStatus(i) == 2 && i == 3))) {
			setHeadOpenedTime(i, 0);
			TempCursorChange cursorChange(kCursorWait);
			byte status = getHeadStatus(i);

			if (status & 1) {
				status--;
				setHeadStatus(i, status);
				_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(loc.timeZone, loc.environment, (status == 2) ? 14 : 13), 128);
				_scheduledDepthChange = true;
				adjustSpearVolume(viewWindow);
			}

			bioChipRightWindow->sceneChanged();
		}
	}

	if (_scheduledDepthChange) {
		_scheduledDepthChange = false;
		sceneView->jumpToScene(loc);
	}

	return SC_TRUE;
}

bool ArrowGodDepthChange::adjustSpearVolume(Window *viewWindow) {
	// TODO: Looks like there's a bug in the original. node == 3 should also be in here, I think
	// Need to investigate
	if (_staticData.location.node >= 0 && _staticData.location.node <= 2) {
		byte headAStatus = _globalFlags.myAGHeadAStatus;
		byte headDStatus = _globalFlags.myAGHeadDStatus;

		if (headAStatus == 0) {
			_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
		} else if (headDStatus == 0) {
			if (_staticData.location.node == 2)
				_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
			else
				_vm->_sound->adjustSecondaryAmbientSoundVolume(64, false, 0, 0);
		} else {
			_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);
		}
	}

	return true;
}

class DeathGodAltar : public SceneBase {
public:
	DeathGodAltar(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;
	int locateAttempted(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _heartPool;
	Common::Rect _puzzleBox;
	Common::Rect _blood;
};

DeathGodAltar::DeathGodAltar(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_heartPool = Common::Rect(89, 80, 159, 112);
	_puzzleBox = Common::Rect(150, 45, 260, 111);
	_blood = Common::Rect(88, 76, 162, 114);
}

int DeathGodAltar::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	Location &loc = _staticData.location;

	if (globalFlags.myDGOfferedHeart == 1) {
		if (!sceneView->isNumberInGlobalFlagTable(MAYAN_EVIDENCE_ENVIRON_CART)) {
			sceneView->playSynchronousAnimation(3);
			_staticData.navFrameIndex = 51;
			viewWindow->invalidateWindow(false);

			if ((loc.timeZone != priorLocation.timeZone || loc.environment != priorLocation.environment ||
					loc.node != priorLocation.node || loc.facing != priorLocation.facing ||
					loc.orientation != priorLocation.orientation || loc.depth != priorLocation.depth) &&
					!sceneView->isNumberInGlobalFlagTable(MAYAN_EVIDENCE_ENVIRON_CART))
				sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_PRESENT));
		}
	} else if ((loc.timeZone != priorLocation.timeZone || loc.environment != priorLocation.environment ||
			loc.node != priorLocation.node || loc.facing != priorLocation.facing ||
			loc.orientation != priorLocation.orientation || loc.depth != priorLocation.depth) &&
			!sceneView->isNumberInGlobalFlagTable(MAYAN_EVIDENCE_PHONY_BLOOD)) {
		sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_PRESENT));
	}

	return SC_TRUE;
}

int DeathGodAltar::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_puzzleBox.contains(pointLocation) && globalFlags.myDGOfferedHeart == 1 && globalFlags.takenEnvironCart == 0) {
		Location puzzleLocation = _staticData.location;
		puzzleLocation.depth = 1;
		sceneView->jumpToScene(puzzleLocation);
	}

	return SC_FALSE;
}

int DeathGodAltar::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (itemID == kItemPreservedHeart && globalFlags.myDGOfferedHeart == 0 && _heartPool.contains(pointLocation))
		return 1;

	return 0;
}

int DeathGodAltar::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (pointLocation.x == -1 && pointLocation.y == -1)
		return SIC_REJECT;

	if (itemID == kItemPreservedHeart && globalFlags.myDGOfferedHeart == 0 && _heartPool.contains(pointLocation)) {
		globalFlags.myDGOfferedHeart = 1;
		sceneView->playSynchronousAnimation(2);
		sceneView->playSynchronousAnimation(3);
		_staticData.navFrameIndex = 51;

		if (!sceneView->isNumberInGlobalFlagTable(MAYAN_EVIDENCE_ENVIRON_CART))
			sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_PRESENT));

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

int DeathGodAltar::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (globalFlags.bcLocateEnabled == 1) {
		if (globalFlags.myDGOfferedHeart == 1 &&
				globalFlags.takenEnvironCart == 0 &&
				_staticData.navFrameIndex == 51 && _puzzleBox.contains(pointLocation))
			return -2;

		if (_blood.contains(pointLocation))
			return -2;

		return -1;
	} else if (_puzzleBox.contains(pointLocation) && globalFlags.myDGOfferedHeart == 1 && globalFlags.takenEnvironCart == 0) {
		// This logic is broken in 1.04, 1.05, and 1.10. I fixed it here to match mouseUp
		return kCursorFinger;
	}

	return kCursorArrow;
}

int DeathGodAltar::locateAttempted(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	BioChipRightWindow *bioChipRightWindow = ((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow;

	if (globalFlags.bcLocateEnabled == 1 &&
			globalFlags.takenEnvironCart == 0 &&
			_puzzleBox.contains(pointLocation) && _staticData.navFrameIndex == 51 &&
			!sceneView->isNumberInGlobalFlagTable(MAYAN_EVIDENCE_ENVIRON_CART)) {
		sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_MUST_BE_REVEALED)); // All will be reveaaaaaaaaled (Yes, I used this joke twice now)
		return SC_TRUE;
	}

	if (globalFlags.bcLocateEnabled == 1) {
		if (_blood.contains(pointLocation)) {
			sceneView->playSynchronousAnimation(6);

			// Attempt to add it to the biochip
			if (sceneView->addNumberToGlobalFlagTable(MAYAN_EVIDENCE_PHONY_BLOOD))
				sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ACQUIRED));
			else
				sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

			// Disable capture
			bioChipRightWindow->disableEvidenceCapture();
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

class DeathGodPuzzleBox : public SceneBase {
public:
	DeathGodPuzzleBox(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	~DeathGodPuzzleBox();
	void preDestructor() override;
	int paint(Window *viewWindow, Graphics::Surface *preBuffer) override;
	int gdiPaint(Window *viewWindow) override;
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation) override;
	int mouseMove(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	int _puzzleIndexes[4];
	Common::Rect _clickableRegions[4];
	Common::Rect _puzzleRightHandle;
	AVIFrames _puzzleFrames[4];
	bool _translateText;

	bool isPuzzleSolved() const;
};

DeathGodPuzzleBox::DeathGodPuzzleBox(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	Location &loc = _staticData.location;
	_translateText = false;
	_puzzleIndexes[0] = _puzzleIndexes[1] = _puzzleIndexes[2] = _puzzleIndexes[3] = 0;
	_clickableRegions[0] = Common::Rect(30, 0, 111, 189);
	_clickableRegions[1] = Common::Rect(112, 0, 187, 189);
	_clickableRegions[2] = Common::Rect(188, 0, 252, 189);
	_clickableRegions[3] = Common::Rect(253, 0, 330, 189);
	_puzzleRightHandle = Common::Rect(380, 0, 432, 189);

	// Load the spinner movies
	_puzzleFrames[0].open(_vm->getFilePath(loc.timeZone, loc.environment, 4));
	_puzzleFrames[1].open(_vm->getFilePath(loc.timeZone, loc.environment, 5));
	_puzzleFrames[2].open(_vm->getFilePath(loc.timeZone, loc.environment, 6));
	_puzzleFrames[3].open(_vm->getFilePath(loc.timeZone, loc.environment, 7));
}

DeathGodPuzzleBox::~DeathGodPuzzleBox() {
	preDestructor();
}

void DeathGodPuzzleBox::preDestructor() {
	_puzzleFrames[0].close();
	_puzzleFrames[1].close();
	_puzzleFrames[2].close();
	_puzzleFrames[3].close();
}

int DeathGodPuzzleBox::paint(Window *viewWindow, Graphics::Surface *preBuffer) {
	SceneBase::paint(viewWindow, preBuffer);

	for (int i = 0; i < 4; i++) {
		const Graphics::Surface *spinnerFrame = _puzzleFrames[i].getFrame(_puzzleIndexes[i]);
		_vm->_gfx->crossBlit(preBuffer, _clickableRegions[i].left, _clickableRegions[i].top, _clickableRegions[i].width(), _clickableRegions[i].height(), spinnerFrame, 0, 0);
	}

	return SC_REPAINT;
}

int DeathGodPuzzleBox::gdiPaint(Window *viewWindow) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (_translateText && globalFlags.bcTranslateEnabled == 1) {
		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(42, 64, 324, 125);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->getScreen()->frameRect(rect, _vm->_gfx->getColor(255, 0, 0));
	}

	return SC_REPAINT;
}

int DeathGodPuzzleBox::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	BioChipRightWindow *bioChipRightWindow = ((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow;
	InventoryWindow *inventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow;

	for (int i = 0; i < 4; i++) {
		if (_clickableRegions[i].contains(pointLocation)) {
			if (pointLocation.y - _clickableRegions[i].top > _clickableRegions[i].height() / 2) {
				// Start the roll sound
				_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14), 128, false, true);

				// Spin the wheel, stop at the new value
				// FIXME: Timing
				for (int j = 0; j < 6; j++) {
					_puzzleIndexes[i]--;
					if (_puzzleIndexes[i] < 0)
						_puzzleIndexes[i] = _puzzleFrames[i].getFrameCount() - 2;
					viewWindow->invalidateWindow();
					_vm->_gfx->updateScreen();
				}

				_translateText = false;
				mouseMove(viewWindow, pointLocation);
			} else {
				// Start the roll sound
				_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14), 128, false, true);

				// Spin the wheel, stop at the new value
				// FIXME: Timing
				for (int j = 0; j < 6; j++) {
					_puzzleIndexes[i]++;
					if (_puzzleIndexes[i] > _puzzleFrames[i].getFrameCount() - 2)
						_puzzleIndexes[i] = 0;
					viewWindow->invalidateWindow();
					_vm->_gfx->updateScreen();
				}

				_translateText = false;
				mouseMove(viewWindow, pointLocation);
			}
		}
	}

	if (_puzzleRightHandle.contains(pointLocation)) {
		if (isPuzzleSolved()) {
			DestinationScene newDestination;
			newDestination.destinationScene = _staticData.location;
			newDestination.destinationScene.depth = 2;
			newDestination.transitionType = TRANSITION_VIDEO;
			newDestination.transitionStartFrame = -1;
			newDestination.transitionLength = -1;

			if (globalFlags.takenEnvironCart == 0)
				newDestination.transitionData = 4;
			else
				newDestination.transitionData = 5;

			BuriedEngine *vm = _vm;
			sceneView->moveToDestination(newDestination);

			// Play animation capturing the evidence
			// FIXME: Is this right? Shouldn't this be if takenEnvironCart == 0 only?
			sceneView->playSynchronousAnimation(7);

			// Attempt to add it to the biochip
			if (sceneView->addNumberToGlobalFlagTable(MAYAN_EVIDENCE_ENVIRON_CART))
				sceneView->displayLiveText(vm->getString(IDS_MBT_EVIDENCE_RIPPLE_DOCUMENTED));
			else
				sceneView->displayLiveText(vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

			// Disable capture
			bioChipRightWindow->disableEvidenceCapture();

			// Set the scoring flag
			globalFlags.scoreCompletedDeathGod = 1;
			globalFlags.myDGOpenedPuzzleBox = 1;

			// Play an Arthur comment
			if (globalFlags.takenEnvironCart == 0 && inventoryWindow->isItemInInventory(kItemBioChipAI))
				vm->_sound->playSoundEffect("BITDATA/MAYAN/MYDG_C01.BTA"); // Broken in 1.01

			return SC_TRUE;
		} else {
			// We did the puzzle incorrectly, so spin the wheels and kill the player
			sceneView->playPlacedSynchronousAnimation(8, 320, 0);
			sceneView->showDeathScene(12);
			return SC_DEATH;
		}
	}

	return SC_FALSE;
}

int DeathGodPuzzleBox::mouseMove(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (globalFlags.bcTranslateEnabled == 1) {
		Common::Rect translateTextRegion(42, 64, 324, 126);

		if (translateTextRegion.contains(pointLocation)) {
			if (!_translateText) {
				Common::String translatedText = _vm->getString(IDMYDG_PUZZLE_BOX_TRANS_TEXT_BASE + _puzzleIndexes[0] / 6);
				translatedText += ' ';
				translatedText += _vm->getString(IDMYDG_PUZZLE_BOX_TRANS_TEXT_BASE + 10 + _puzzleIndexes[1] / 6);
				translatedText += ' ';
				translatedText += _vm->getString(IDMYDG_PUZZLE_BOX_TRANS_TEXT_BASE + 20 + _puzzleIndexes[2] / 6);
				translatedText += ' ';
				translatedText += _vm->getString(IDMYDG_PUZZLE_BOX_TRANS_TEXT_BASE + 30 + _puzzleIndexes[3] / 6);

				sceneView->displayTranslationText(translatedText);

				_translateText = true;
				viewWindow->invalidateWindow(false);
			}
		} else {
			if (_translateText) {
				_translateText = false;
				viewWindow->invalidateWindow(false);
			}
		}
	}

	return SC_FALSE;
}

int DeathGodPuzzleBox::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 4; i++) {
		if (_clickableRegions[i].contains(pointLocation)) {
			if (pointLocation.y - _clickableRegions[i].top > _clickableRegions[i].height() / 2)
				return kCursorArrowDown;

			return kCursorArrowUp;
		}
	}

	if (_puzzleRightHandle.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

bool DeathGodPuzzleBox::isPuzzleSolved() const {
	switch (_vm->getLanguage()) {
	case Common::DE_DEU:
	case Common::ES_ESP:
	case Common::IT_ITA:
		return _puzzleIndexes[0] == 12 && _puzzleIndexes[1] == 18 && _puzzleIndexes[2] == 30 && _puzzleIndexes[3] == 24;
	case Common::EN_ANY:
		return _puzzleIndexes[0] == 18 && _puzzleIndexes[1] == 36 && _puzzleIndexes[2] == 12 && _puzzleIndexes[3] == 24;
	case Common::FR_FRA:
		return _puzzleIndexes[0] == 12 && _puzzleIndexes[1] == 18 && _puzzleIndexes[2] == 42 && _puzzleIndexes[3] == 24;
	case Common::JA_JPN:
		return _puzzleIndexes[0] == 12 && _puzzleIndexes[1] == 24 && _puzzleIndexes[2] == 30 && _puzzleIndexes[3] == 18;
	default:
		// Default to English, but warn about it
		warning("Unknown language for puzzle box");
		return _puzzleIndexes[0] == 18 && _puzzleIndexes[1] == 36 && _puzzleIndexes[2] == 12 && _puzzleIndexes[3] == 24;
	}

	return false;
}

class MainCavernGlassCapture : public SceneBase {
public:
	MainCavernGlassCapture(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int locateAttempted(Window *viewWindow, const Common::Point &pointLocation) override;
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation) override;

private:
	Common::Rect _glass;
};

MainCavernGlassCapture::MainCavernGlassCapture(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_glass = Common::Rect(305, 126, 355, 156);
}

int MainCavernGlassCapture::locateAttempted(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	BioChipRightWindow *bioChipRightWindow = ((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow;

	if (globalFlags.bcLocateEnabled == 1) {
		if (_glass.contains(pointLocation)) {
			// Play the animation
			sceneView->playSynchronousAnimation(22);

			// Attempt to add it to the biochip
			if (sceneView->addNumberToGlobalFlagTable(MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID))
				sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ACQUIRED));
			else
				sceneView->displayLiveText(_vm->getString(IDS_MBT_EVIDENCE_ALREADY_ACQUIRED));

			// Disable capture
			bioChipRightWindow->disableEvidenceCapture();
		}

		return SC_TRUE;
	}

	return SC_FALSE;
}

int MainCavernGlassCapture::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();

	if (globalFlags.bcLocateEnabled == 1) {
		if (_glass.contains(pointLocation))
			return -2;

		return -1;
	}

	return kCursorArrow;
}

class WalkVolumeChange : public SceneBase {
public:
	WalkVolumeChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			byte newVolume = 0, uint32 volumeChangeTime = 0, int stepCount = -1, int entryEffectFileNameID = -1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;

private:
	byte _newVolume;
	uint32 _volumeChangeTime;
	int _stepCount;
	int _entryEffectFileNameID;
};

WalkVolumeChange::WalkVolumeChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		byte newVolume, uint32 volumeChangeTime, int stepCount, int entryEffectFileNameID) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_newVolume = newVolume;
	_volumeChangeTime = volumeChangeTime;
	_stepCount = stepCount;
	_entryEffectFileNameID = entryEffectFileNameID;
}

int WalkVolumeChange::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_entryEffectFileNameID >= 0 && priorLocation.node != _staticData.location.node)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _entryEffectFileNameID), 128, false, true);

	return SC_TRUE;
}

int WalkVolumeChange::preExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_stepCount >= 0 && newLocation.node != _staticData.location.node)
		_vm->_sound->adjustAmbientSoundVolume(_newVolume, true, _stepCount, _volumeChangeTime);

	return SC_TRUE;
}

class AdjustSecondaryAmbientOnEntry : public SceneBase {
public:
	AdjustSecondaryAmbientOnEntry(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation) override;
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;
};

AdjustSecondaryAmbientOnEntry::AdjustSecondaryAmbientOnEntry(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
}

int AdjustSecondaryAmbientOnEntry::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_vm->_sound->adjustSecondaryAmbientSoundVolume(128, false, 0, 0);
	return SC_TRUE;
}

int AdjustSecondaryAmbientOnEntry::preExitRoom(Window *viewWindow, const Location &newLocation) {
	// Kill the ambient if moving to a different node
	if (newLocation.node != _staticData.location.node)
		_vm->_sound->adjustSecondaryAmbientSoundVolume(0, false, 0, 0);

	return SC_TRUE;
}

class WalkDualAmbientVolumeChange : public SceneBase {
public:
	WalkDualAmbientVolumeChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			byte newVolume = 0, byte secondVolume = 0, uint32 volumeChangeTime = 0, int stepCount = -1);
	int preExitRoom(Window *viewWindow, const Location &newLocation) override;

private:
	byte _newVolume;
	uint32 _volumeChangeTime;
	int _stepCount;
	byte _secondVolume;
};

WalkDualAmbientVolumeChange::WalkDualAmbientVolumeChange(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		byte newVolume, byte secondVolume, uint32 volumeChangeTime, int stepCount) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	Location &loc = _staticData.location;

	_newVolume = newVolume;
	_volumeChangeTime = volumeChangeTime;
	_stepCount = stepCount;
	_secondVolume = secondVolume;

	// If we have stepped on the far ledge, set the flag
	if (loc.timeZone == 2 && loc.environment == 4 &&
			loc.node == 5 && loc.facing == 0 &&
			loc.orientation == 1 && loc.depth == 0)
		globalFlags.myWTSteppedOnFarLedge = 1;
}

int WalkDualAmbientVolumeChange::preExitRoom(Window *viewWindow, const Location &newLocation) {
	if (_stepCount >= 0 && newLocation.node != _staticData.location.node) {
		_vm->_sound->adjustAmbientSoundVolume(_newVolume, true, _stepCount, _volumeChangeTime);
		_vm->_sound->adjustSecondaryAmbientSoundVolume(_secondVolume, true, _stepCount, _volumeChangeTime);
	}

	return SC_TRUE;
}

class SetVolumeAndFlag : public SceneBase {
public:
	SetVolumeAndFlag(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			byte newVolume, byte &flag, byte flagValue = 255);
};

SetVolumeAndFlag::SetVolumeAndFlag(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		byte newVolume, byte &flag, byte flagValue) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_vm->_sound->adjustAmbientSoundVolume(newVolume, false, 0, 0);
	flag = flagValue;
}

bool SceneViewWindow::initializeMayanTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	InventoryWindow *inventoryWindow = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow;

	if (environment == -1) {
		_globalFlags.myTPCodeWheelStatus = _globalFlags.generalWalkthroughMode;
		_globalFlags.myTPCodeWheelLeftIndex = _globalFlags.generalWalkthroughMode == 1 ? 8 : 0;
		_globalFlags.myTPCodeWheelRightIndex = _globalFlags.generalWalkthroughMode == 1 ? 12 : 0;
		_globalFlags.myMCDeathGodOfferings = 0;
		_globalFlags.myWGPlacedRope = _globalFlags.generalWalkthroughMode;
		_globalFlags.myWTCurrentBridgeStatus = 0;
		_globalFlags.myAGHeadAStatus = _globalFlags.generalWalkthroughMode == 1 ? 2 : 0;
		_globalFlags.myAGHeadBStatus = 0;
		_globalFlags.myAGHeadCStatus = 0;
		_globalFlags.myAGHeadDStatus = _globalFlags.generalWalkthroughMode == 1 ? 2 : 0;
		_globalFlags.myAGHeadAStatusSkullID = _globalFlags.generalWalkthroughMode == 1 ? kItemCavernSkull : 0;
		_globalFlags.myAGHeadBStatusSkullID = 0;
		_globalFlags.myAGHeadCStatusSkullID = 0;
		_globalFlags.myAGHeadDStatusSkullID = _globalFlags.generalWalkthroughMode == 1 ? kItemSpearSkull : 0;
		_globalFlags.myAGTimerHeadID = 0;
		_globalFlags.myAGTimerStartTime = 0;
		_globalFlags.myDGOfferedHeart = 0;
		_globalFlags.myAGHeadAOpenedTime = 0;
		_globalFlags.myAGHeadBOpenedTime = 0;
		_globalFlags.myAGHeadCOpenedTime = 0;
		_globalFlags.myAGHeadDOpenedTime = 0;

		_globalFlags.myPickedUpCeramicBowl = inventoryWindow->isItemInInventory(kItemCeramicBowl) ? 1 : 0;
		_globalFlags.myMCPickedUpSkull = inventoryWindow->isItemInInventory(kItemCavernSkull) ? 1 : 0;
		_globalFlags.myWGRetrievedJadeBlock = inventoryWindow->isItemInInventory(kItemJadeBlock) ? 1 : 0;
		_globalFlags.myWTRetrievedLimestoneBlock = inventoryWindow->isItemInInventory(kItemLimestoneBlock) ? 1 : 0;
		_globalFlags.myAGRetrievedEntrySkull = inventoryWindow->isItemInInventory(kItemEntrySkull) ? 1 : 0;
		_globalFlags.myAGRetrievedSpearSkull = inventoryWindow->isItemInInventory(kItemSpearSkull) ? 1 : 0;
		_globalFlags.myAGRetrievedCopperMedal = inventoryWindow->isItemInInventory(kItemCopperMedallion) ? 1 : 0;
		_globalFlags.myAGRetrievedObsidianBlock = inventoryWindow->isItemInInventory(kItemObsidianBlock) ? 1 : 0;
		_globalFlags.takenEnvironCart = inventoryWindow->isItemInInventory(kItemEnvironCart) ? 1 : 0;

		if (_globalFlags.generalWalkthroughMode == 1) {
			_globalFlags.myMCPickedUpSkull = 1;
			_globalFlags.myAGRetrievedSpearSkull = 1;
			_globalFlags.myAGRetrievedCopperMedal = 1;
		}
	} else if (environment == 2) {
		_globalFlags.scoreEnteredMainCavern = 1;
		_globalFlags.myVisitedMainCavern = 1;
	} else if (environment == 3) {
		_globalFlags.myVisitedWealthGod = 1;
		_globalFlags.myVisitedSpecRooms = 1;
	} else if (environment == 4) {
		_globalFlags.myVisitedWaterGod = 1;
		_globalFlags.myVisitedSpecRooms = 1;
	} else if (environment == 5) {
		_globalFlags.myVisitedArrowGod = 1;
		_globalFlags.myVisitedSpecRooms = 1;
	} else if (environment == 6) {
		_globalFlags.myVisitedDeathGod = 1;
		_globalFlags.myVisitedSpecRooms = 1;
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

bool SceneViewWindow::checkCustomMayanAICommentDependencies(const Location &commentLocation, const AIComment &commentData) {
	InventoryWindow *inventoryWindow = ((GameUIWindow *)getParent())->_inventoryWindow;

	switch (commentData.dependencyFlagOffsetB) {
	case 1: // Player hasn't translated any inscriptions
		return _globalFlags.myTPTextTranslated == 0;
	case 2: // Player hasn't translated any inscriptions, has translate biochip
		return _globalFlags.myTPTextTranslated == 0 && inventoryWindow->isItemInInventory(kItemBioChipTranslate);
	case 3: // Player hasn't translated any inscriptions, doesn't have translate biochip
		return _globalFlags.myTPTextTranslated == 0 && !inventoryWindow->isItemInInventory(kItemBioChipTranslate);
	case 4: // Has translated inscription above calendar, calendar not set to sacred day, player has never been to main cavern
		return _globalFlags.myTPCalendarTopTranslated == 1 && _globalFlags.myTPCodeWheelStatus == 0 && _globalFlags.myVisitedMainCavern == 0;
	case 5: // Has translated inscription above calendar, calendar is set to sacred day, player has never been to main cavern
		return _globalFlags.myTPCalendarTopTranslated == 1 && _globalFlags.myTPCodeWheelStatus == 1 && _globalFlags.myVisitedMainCavern == 0;
	case 6: // Player has never been to main cavern
		return _globalFlags.myVisitedMainCavern == 0;
	case 7: // Player has never been to main cavern, calendar not set to sacred day
		return _globalFlags.myVisitedMainCavern == 0 && _globalFlags.myTPCodeWheelStatus == 0;
	case 8: // Ceramic bowl not in the inventory
		return !inventoryWindow->isItemInInventory(kItemCeramicBowl);
	case 9: // If node is not 5, 6, or 8
		return commentLocation.node != 6 && commentLocation.node != 5 && commentLocation.node != 8;
	case 10: // If node is not 1, 0, 7, or 8
		return commentLocation.node != 1 && commentLocation.node != 0 && commentLocation.node != 7 && commentLocation.node != 8;
	case 11: // If node is not 0, 8, or 1
		return commentLocation.node != 0 && commentLocation.node != 8 && commentLocation.node != 1;
	case 12: // If not translated any sacred days
		return _globalFlags.myTPCalendarListTranslated == 0;
	case 13: // Not any door, no translations
		return _globalFlags.myVisitedSpecRooms == 0 && _globalFlags.myMCTransDoor == 0;
	case 14: // Not any door, no translations, has translate chip
		return _globalFlags.myVisitedSpecRooms == 0 && _globalFlags.myMCTransDoor == 0 && inventoryWindow->isItemInInventory(kItemBioChipTranslate);
	case 15: // Not any door, has translated arrow or translated water or translated wealth
		return _globalFlags.myVisitedSpecRooms == 0 && (_globalFlags.myMCTransAGOffering == 1 || _globalFlags.myMCTransWGOffering == 1 || _globalFlags.myMCTransWTOffering == 1);
	case 16: // Has translated wealth, has not been through wealth god door
		return _globalFlags.myMCTransWGOffering == 1 && _globalFlags.myVisitedWealthGod == 0;
	case 17: // Has translated water, has not been through water god door
		return _globalFlags.myMCTransWTOffering == 1 && _globalFlags.myVisitedWaterGod == 0;
	case 18: // Has translated arrow, has not been through arrow god door
		return _globalFlags.myMCTransAGOffering == 1 && _globalFlags.myVisitedArrowGod == 0;
	case 19: // Has translated death, has not been through death god door
		return _globalFlags.myMCTransDGOffering == 1 && _globalFlags.myVisitedDeathGod == 0;
	case 20: // Has not been through death god door
		return _globalFlags.myVisitedDeathGod == 0;
	case 21: // Has translated death
		return _globalFlags.myMCTransDGOffering == 1;
	case 22: // After making any offering
		return _globalFlags.myMCTransMadeAnOffering == 1;
	case 23: // Before crossing rope bridge
		return _globalFlags.myWGCrossedRopeBridge == 0;
	case 24: // If player has translated inscription above wealth god door in cavern
		return _globalFlags.myMCTransWGOffering == 1;
	case 25: // If player has not translated inscription above wealth god door in cavern
		return _globalFlags.myMCTransWGOffering == 0;
	case 26: // If player has translated inscription above wealth god door in cavern, has never been to wealth god altar room
		return _globalFlags.myMCTransWGOffering == 1 && _globalFlags.myWGSeenLowerPassage == 0;
	case 27: // Has not attached either rope or grappling hook, has never been to wealth god altar room
		return _globalFlags.myWGPlacedRope == 0 && _globalFlags.myWGSeenLowerPassage == 0;
	case 28: // Player has never stepped on swings
		return _globalFlags.myWTSteppedOnSwings == 0;
	case 29: // Player has never been on far ledge
		return _globalFlags.myWTSteppedOnFarLedge == 0;
	case 30: // Never put in heart
		return _globalFlags.myDGOfferedHeart == 0;
	case 31: // Never put in heart, no heart in inventory
		return _globalFlags.myDGOfferedHeart == 0 && inventoryWindow->isItemInInventory(kItemPreservedHeart);
	case 32: // After put in heart, puzzle box never opened
		return _globalFlags.myDGOfferedHeart == 1 && _globalFlags.myDGOpenedPuzzleBox == 0;
	case 33: // After put in heart, puzzle box never opened, player has not translated 'Itzamna' inscription over inside door of temple
		return _globalFlags.myDGOfferedHeart == 1 && _globalFlags.myDGOpenedPuzzleBox == 0 && _globalFlags.myTPTransBreathOfItzamna == 0;
	case 34: // After put in heart, puzzle box never opened, player has translated 'Itzamna' inscription over inside door of temple
		return _globalFlags.myDGOfferedHeart == 1 && _globalFlags.myDGOpenedPuzzleBox == 0 && _globalFlags.myTPTransBreathOfItzamna == 1;
	case 35: // Before interacting with any heads
		return _globalFlags.myAGHeadATouched == 0 && _globalFlags.myAGHeadBTouched == 0 && _globalFlags.myAGHeadCTouched == 0 && _globalFlags.myAGHeadDTouched == 0;
	case 36: // After interacting with any head, before jamming any head
		return (_globalFlags.myAGHeadATouched == 1 || _globalFlags.myAGHeadBTouched == 1 || _globalFlags.myAGHeadCTouched == 1 || _globalFlags.myAGHeadDTouched == 1) && _globalFlags.myAGHeadAStatus == 0 && _globalFlags.myAGHeadBStatus == 0 && _globalFlags.myAGHeadCStatus == 0 && _globalFlags.myAGHeadDStatus == 0;
	case 37: // S2 jam, S1 not jam, not altar
		return _globalFlags.myAGHeadAStatus == 0 && _globalFlags.myAGHeadBStatus == 2 && _globalFlags.myAGVisitedAltar == 0;
	case 38: // S1 jam, S2 not jam, S3 not jam, not altar
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadBStatus == 0 && _globalFlags.myAGHeadCStatus == 0 && _globalFlags.myAGHeadDStatus == 0 && _globalFlags.myAGVisitedAltar == 0;
	case 39: // S1 jam, S2 not jam, S3 not jam, S4 not jam, not altar, no skulls
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadBStatus == 0 && _globalFlags.myAGHeadCStatus == 0 && _globalFlags.myAGHeadDStatus == 0 && _globalFlags.myAGVisitedAltar == 0 && !inventoryWindow->isItemInInventory(kItemCavernSkull) && !inventoryWindow->isItemInInventory(kItemEntrySkull) && !inventoryWindow->isItemInInventory(kItemSpearSkull);
	case 40: // Before interacting with S3 or S4, not altar
		return _globalFlags.myAGHeadCTouched == 0 && _globalFlags.myAGHeadDTouched == 0 && _globalFlags.myAGVisitedAltar == 0;
	case 41: // S1 jam, S2 jam, S3 not jam, S4 not jam, after interacting with S3 and S4, not altar, no skulls
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadBStatus == 2 && _globalFlags.myAGHeadCStatus == 0 && _globalFlags.myAGHeadDStatus == 0 && _globalFlags.myAGHeadCTouched == 1 && _globalFlags.myAGHeadDTouched == 1 && _globalFlags.myAGVisitedAltar == 0 && !inventoryWindow->isItemInInventory(kItemCavernSkull) && !inventoryWindow->isItemInInventory(kItemEntrySkull) && !inventoryWindow->isItemInInventory(kItemSpearSkull);
	case 42: // S1 jam, S2 not jam, S3 not jam, S4 not jam, after interacting with S3 and S4, not altar, only 1 skull in inventory
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadBStatus == 0 && _globalFlags.myAGHeadCStatus == 0 && _globalFlags.myAGHeadDStatus == 0 && _globalFlags.myAGHeadCTouched == 1 && _globalFlags.myAGHeadDTouched == 1 && _globalFlags.myAGVisitedAltar == 0 && (inventoryWindow->isItemInInventory(kItemCavernSkull) ^ inventoryWindow->isItemInInventory(kItemEntrySkull) ^ inventoryWindow->isItemInInventory(kItemSpearSkull));
	case 43: // S1 jam, S2 jam, S3 not jam, S4 not jam, after interacting with S3 and S4, not altar, no skulls
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadBStatus == 2 && _globalFlags.myAGHeadCStatus == 0 && _globalFlags.myAGHeadDStatus == 0 && _globalFlags.myAGHeadCTouched == 1 && _globalFlags.myAGHeadDTouched == 1 && _globalFlags.myAGVisitedAltar == 0 && !inventoryWindow->isItemInInventory(kItemCavernSkull) && !inventoryWindow->isItemInInventory(kItemEntrySkull) && !inventoryWindow->isItemInInventory(kItemSpearSkull);
	case 44: // S1 jam, S2 not jam, S3 jam, S4 not jam, not altar, no skulls
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadBStatus == 0 && _globalFlags.myAGHeadCStatus == 2 && _globalFlags.myAGHeadDStatus == 0 && _globalFlags.myAGVisitedAltar == 0 && !inventoryWindow->isItemInInventory(kItemCavernSkull) && !inventoryWindow->isItemInInventory(kItemEntrySkull) && !inventoryWindow->isItemInInventory(kItemSpearSkull);
	case 45: // S1 jam, S2 not jam, S3 not jam, S4 jam, not altar, no skulls
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadBStatus == 0 && _globalFlags.myAGHeadCStatus == 0 && _globalFlags.myAGHeadDStatus == 2 && _globalFlags.myAGVisitedAltar == 0 && !inventoryWindow->isItemInInventory(kItemCavernSkull) && !inventoryWindow->isItemInInventory(kItemEntrySkull) && !inventoryWindow->isItemInInventory(kItemSpearSkull);
	case 46: // S1 jam, S2 not jam, S3 not jam, S4 jam, not altar, only 1 skull in inventory
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadBStatus == 0 && _globalFlags.myAGHeadCStatus == 0 && _globalFlags.myAGHeadDStatus == 2 && _globalFlags.myAGVisitedAltar == 0 && (inventoryWindow->isItemInInventory(kItemCavernSkull) ^ inventoryWindow->isItemInInventory(kItemEntrySkull) ^ inventoryWindow->isItemInInventory(kItemSpearSkull));
	case 47: // S1 jam, S3 jam, S4 jam, not altar
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadCStatus == 2 && _globalFlags.myAGHeadDStatus == 2 && _globalFlags.myAGVisitedAltar == 0;
	case 48: // S1 jam, S3 jam, not altar, only 1 skull in inventory
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadCStatus == 2 && _globalFlags.myAGHeadDStatus == 2 && _globalFlags.myAGVisitedAltar == 0 && (inventoryWindow->isItemInInventory(kItemCavernSkull) ^ inventoryWindow->isItemInInventory(kItemEntrySkull) ^ inventoryWindow->isItemInInventory(kItemSpearSkull));
	case 49: // S1 jam, S2 jam, S3 jam, not altar
		return _globalFlags.myAGHeadAStatus == 2 && _globalFlags.myAGHeadCStatus == 2 && _globalFlags.myAGHeadDStatus == 2 && _globalFlags.myAGVisitedAltar == 0;
	case 50: // S1 not jam, S2 jam, not altar
		return _globalFlags.myAGHeadAStatus == 0 && _globalFlags.myAGHeadBStatus == 2 && _globalFlags.myAGVisitedAltar == 0;
	}

	return false;
}

SceneBase *SceneViewWindow::constructMayanSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	SceneViewWindow *sceneView = ((SceneViewWindow *)viewWindow);
	GlobalFlags &globalFlags = sceneView->getGlobalFlags();
	byte dummyFlag = 0;	// a dummy flag, used as a placeholder for writing (but not reading)

	// Special scene for the trial version
	if (_vm->isTrial())
		return new TrialRecallScene(_vm, viewWindow, sceneStaticData, priorLocation);

	switch (sceneStaticData.classID) {
	case 0:
		// Default scene
		break;
	case 1:
		return new VideoDeath(_vm, viewWindow, sceneStaticData, priorLocation, 10, IDS_HUMAN_PRESENCE_500METERS);
	case 2:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 60, 134, 118, 180, kItemCeramicBowl, 96, globalFlags.myPickedUpCeramicBowl);
	case 3:
		return new PlaceCeramicBowl(_vm, viewWindow, sceneStaticData, priorLocation);
	case 4:
		return new ClickChangeDepth(_vm, viewWindow, sceneStaticData, priorLocation, 1, kCursorMagnifyingGlass, 0, 0, 432, 189);
	case 5:
		return new AdjustWheels(_vm, viewWindow, sceneStaticData, priorLocation);
	case 6:
		return new DateCombinationRead(_vm, viewWindow, sceneStaticData, priorLocation);
	case 7:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_INNER_DOOR_TRANS_TEXT, 16, 6, 402, 110, globalFlags.myTPTextTranslated, globalFlags.myTPTransBreathOfItzamna, dummyFlag);
	case 8:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_INNER_LEFT_TRANS_TEXT, 1, 6, 431, 98, globalFlags.myTPTextTranslated, dummyFlag, dummyFlag);
	case 9:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_INNER_MIDDLE_TRANS_TEXT, 16, 8, 430, 114, globalFlags.myTPTextTranslated, globalFlags.myTPCalendarTopTranslated, dummyFlag);
	case 10:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_OUTER_SOUTHLEFT_TRANS_TEXT, 4, 55, 426, 95, globalFlags.myTPTextTranslated, dummyFlag, dummyFlag);
	case 11:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_OUTER_WEST_TRANS_TEXT, 4, 72, 420, 108, globalFlags.myTPTextTranslated, dummyFlag, dummyFlag);
	case 12:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYTP_OUTER_NORTH_TRANS_TEXT, 6, 38, 428, 76, globalFlags.myTPTextTranslated, dummyFlag, dummyFlag);
	case 13:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 140, 124, 174, 158, kItemCavernSkull, 3, globalFlags.myMCPickedUpSkull);
	case 14:
		return new GenericCavernDoorMainView(_vm, viewWindow, sceneStaticData, priorLocation, 1, 126, 1, 306, 30, 2, 287, 30, 379, 82, 3, 275, 84, 401, 174);
	case 15:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WG_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, globalFlags.myMCTransDoor, globalFlags.myWGTransDoorTop, dummyFlag);
	case 16:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WG_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, globalFlags.myMCTransDoor, globalFlags.myMCTransWGOffering, dummyFlag);
	case 17:
		return new GenericCavernDoorOfferingHead(_vm, viewWindow, sceneStaticData, priorLocation, kItemGoldCoins, 4, TRANSITION_WALK, -1, 1082, 13);
	case 18:
		return new GenericCavernDoorMainView(_vm, viewWindow, sceneStaticData, priorLocation, 1, 126, 1, 306, 30, 2, 287, 30, 379, 82, 3, 275, 84, 401, 174);
	case 19:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WATERGOD_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, globalFlags.myMCTransDoor, dummyFlag, dummyFlag);
	case 20:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WATERGOD_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, globalFlags.myMCTransDoor, globalFlags.myMCTransWTOffering, dummyFlag);
	case 21:
		return new GenericCavernDoorOfferingHead(_vm, viewWindow, sceneStaticData, priorLocation, kItemWaterCanFull, 4, TRANSITION_WALK, -1, 1125, 13);
	case 22:
		return new GenericCavernDoorMainView(_vm, viewWindow, sceneStaticData, priorLocation, 1, 126, 1, 306, 30, 2, 287, 30, 379, 82, 3, 275, 84, 401, 174);
	case 23:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_AG_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, globalFlags.myMCTransDoor, dummyFlag, dummyFlag);
	case 24:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_AG_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, globalFlags.myMCTransDoor, globalFlags.myMCTransAGOffering, dummyFlag);
	case 25:
		return new GenericCavernDoorOfferingHead(_vm, viewWindow, sceneStaticData, priorLocation, kItemBloodyArrow, 4, TRANSITION_WALK, -1, 1010, 12);
	case 26:
		return new GenericCavernDoorMainView(_vm, viewWindow, sceneStaticData, priorLocation, 1, 126, 1, 306, 30, 2, 287, 30, 379, 82, 3, 275, 84, 401, 174);
	case 27:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_DEATHGOD_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, globalFlags.myMCTransDoor, dummyFlag, dummyFlag);
	case 28:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_DEATHGOD_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, globalFlags.myMCTransDoor, globalFlags.myMCTransDGOffering, dummyFlag);
	case 29:
		return new DeathGodCavernDoorOfferingHead(_vm, viewWindow, sceneStaticData, priorLocation, 4, TRANSITION_WALK, -1, 1045, 13);
	case 30:
		return new WealthGodRopeDrop(_vm, viewWindow, sceneStaticData, priorLocation);
	case 31:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 194, 106, 278, 126, kItemJadeBlock, 105, globalFlags.myWGRetrievedJadeBlock);
	case 32:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 140, 22, 306, 189, 2, 3, 0, 3, 1, 1, TRANSITION_WALK, -1, 264, 14, 14);
	case 33:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 158, 88, 288, 116, kItemLimestoneBlock, 84, globalFlags.myWTRetrievedLimestoneBlock);
	case 34:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 80, 0, 332, 189, 2, 4, 0, 2, 1, 1, TRANSITION_WALK, -1, 401, 14, 14);
	case 35:
		return new WaterGodInitialWalkSetFlag(_vm, viewWindow, sceneStaticData, priorLocation);
	case 36:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 0, 93, 37, 10, 73, false, 18);
	case 37:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 166, 259, 37, 10, 73);
	case 38:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 332, 425, 37, 10, 73);
	case 39:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 498, 591, 37, 10, 73);
	case 40:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 664, 757, 37, 10, 73);
	case 41:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 830, 925, 37, 10, 71, true);
	case 42:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 999, 1075, 37, 10, 73);
	case 43:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 1149, 1242, 37, 10, 73);
	case 44:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 1315, 1408, 37, 10, 73);
	case 45:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 1481, 1574, 37, 10, 73);
	case 46:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 1647, 1740, 37, 10, 73);
	case 47:
		return new WaterGodBridgeJump(_vm, viewWindow, sceneStaticData, priorLocation, 4, 1813, 1906, 37, 10, 73);
	case 50:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation,106, 0, 294, 189, 2, 5, 0, 1, 1, 1, TRANSITION_WALK, -1, 427, 13, 11);
	case 51:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 235, 144, 285, 181, kItemEntrySkull, 3, globalFlags.myAGRetrievedEntrySkull);
	case 52:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 200, 138, 231, 185, kItemSpearSkull, 46, globalFlags.myAGRetrievedSpearSkull);
	case 53:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 201, 4, 235, 22, kItemCopperMedallion, 45, globalFlags.myAGRetrievedCopperMedal);
	case 54:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 206, 110, 280, 142, kItemObsidianBlock, 72, globalFlags.myAGRetrievedObsidianBlock);
	case 55:
		return new ArrowGodHead(_vm, viewWindow, sceneStaticData, priorLocation, 0,  182, 87, 242, 189, 4, 75, 83, 79, 0, 2, 1, 3);
	case 56:
		return new ArrowGodHead(_vm, viewWindow, sceneStaticData, priorLocation, 1,  194, 89, 256, 189, 10, 76, 84, 80, 4, 6, 5, 7);
	case 57:
		return new ArrowGodHead(_vm, viewWindow, sceneStaticData, priorLocation, 2,  178, 93, 246, 189, 28, 77, 85, 81, 8, 10, 9, 11);
	case 58:
		return new ArrowGodHead(_vm, viewWindow, sceneStaticData, priorLocation, 3,  188, 92, 252, 189, 34, 78, 86, 82, 12, 14, 13, 15);
	case 59:
		return new ArrowGodDepthChange(_vm, viewWindow, sceneStaticData, priorLocation);
	case 60:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 11);
	case 65:
		return new BasicDoor(_vm, viewWindow, sceneStaticData, priorLocation, 90, 15, 346, 189, 2, 6, 0, 0, 1, 1, TRANSITION_WALK, -1, 33, 12, 13);
	case 66:
		return new DeathGodAltar(_vm, viewWindow, sceneStaticData, priorLocation);
	case 67:
		return new DeathGodPuzzleBox(_vm, viewWindow, sceneStaticData, priorLocation);
	case 68:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 206, 76, 246, 116, kItemEnvironCart, 53, globalFlags.takenEnvironCart);
	case 69:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 10);
	case 70:
		return new PlayStingers(_vm, viewWindow, sceneStaticData, priorLocation, 128, globalFlags.myMCStingerID, globalFlags.myMCStingerChannelID, 11, 14);
	case 71:
		return new DisplayMessageWithEvidenceWhenEnteringNode(_vm, viewWindow, sceneStaticData, priorLocation, MAYAN_EVIDENCE_BROKEN_GLASS_PYRAMID, IDS_MBT_EVIDENCE_PRESENT);
	case 72:
		return new MainCavernGlassCapture(_vm, viewWindow, sceneStaticData, priorLocation);
	case 75:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 40, 4500, 12, 14);
	case 76:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 64, 6333, 12);
	case 77:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 2, 1000, 2);
	case 78:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 2, 1000, 2, 14);
	case 79:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 40, 6500, 12);
	case 80:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 16, 4750, 12);
	case 81:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 30, 7750, 6);
	case 82:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 16, 2250, 18);
	case 83:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 30, 2410, 6);
	case 84:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 64, 7666, 18);
	case 85:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 255, 0, -1, 10); // First param has to be wrong
	case 86:
		return new SetVolumeAndFlag(_vm, viewWindow, sceneStaticData, priorLocation, 64, globalFlags.myWGSeenLowerPassage);
	case 87:
		return new SetVolumeAndFlag(_vm, viewWindow, sceneStaticData, priorLocation, 64, globalFlags.myWGCrossedRopeBridge);
	case 88:
		return new SetVolumeAndFlag(_vm, viewWindow, sceneStaticData, priorLocation, 64, dummyFlag);
	case 90:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 40, 3160, 12, 14);
	case 91:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 64, 4160, 12);
	case 92:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 40, 4160, 12);
	case 93:
		return new WalkVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 16, 3160, 12);
	case 100:
		return new AdjustSecondaryAmbientOnEntry(_vm, viewWindow, sceneStaticData, priorLocation);
	case 101:
		return new WalkDualAmbientVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 16, 32, 6900, 12);
	case 102:
		return new WalkDualAmbientVolumeChange(_vm, viewWindow, sceneStaticData, priorLocation, 64, 128, 6900, 12);
	case 103:
		return new PlaySoundEnteringFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 12, 2, 4, 4, 2, 1, 5);
	case 120:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 10);
	case 121:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 125:
		return new GenericItemAcquire(_vm, viewWindow, sceneStaticData, priorLocation, 226, 90, 256, 104, kItemCopperMedallion, 15, globalFlags.myAGRetrievedCopperMedal);
	case 126:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDS_MY_AG_ALTAR_TEXT, 120, 44, 330, 72, dummyFlag, dummyFlag, globalFlags.myAGVisitedAltar);
	case 127:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDS_MY_WG_ALTAR_TEXT, 118, 14, 338, 44, dummyFlag, dummyFlag, dummyFlag);
	case 128:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDS_MY_WT_ALTAR_TEXT, 106, 128, 344, 162, dummyFlag, dummyFlag, dummyFlag);
	default:
		warning("Unknown Mayan scene object %d", sceneStaticData.classID);
		break;
	}

	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
