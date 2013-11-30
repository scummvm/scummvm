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
	case 15:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WG_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myWGTransDoorTop));
	case 16:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WG_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myMCTransWGOffering));
	case 19:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WATERGOD_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, offsetof(GlobalFlags, myMCTransDoor));
	case 20:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_WATERGOD_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myMCTransWTOffering));
	case 23:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_AG_DOOR_TOP_TRANS_TEXT, 12, 128, 426, 156, offsetof(GlobalFlags, myMCTransDoor));
	case 24:
		return new ViewSingleTranslation(_vm, viewWindow, sceneStaticData, priorLocation, IDMYMC_AG_DOOR_RIGHT_TRANS_TEXT, 46, 1, 315, 188, offsetof(GlobalFlags, myMCTransDoor), offsetof(GlobalFlags, myMCTransAGOffering));
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
