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

namespace Buried {

enum {
	GC_AIHW_STARTING_VALUE = 100,
	GC_AI_OT_WALK_DECREMENT = 2,
	GC_AI_OT_TURN_DECREMENT = 1,
	GC_AI_OT_WAIT_DECREMENT = 1,
	GC_AI_OT_WAIT_TIME_PERIOD = 10000
};

class BaseOxygenTimer : public SceneBase {
public:
	BaseOxygenTimer(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	virtual int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	virtual int preExitRoom(Window *viewWindow, const Location &priorLocation);
	virtual int timerCallback(Window *viewWindow);

protected:
	uint32 _entryStartTime;
	int _deathID;
	bool _jumped;
};

BaseOxygenTimer::BaseOxygenTimer(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_deathID = 41;
	_jumped = false;
}

int BaseOxygenTimer::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_entryStartTime = g_system->getMillis();
	return SC_TRUE;
}

int BaseOxygenTimer::preExitRoom(Window *viewWindow, const Location &newLocation) {
	// NOTE: v1.01 used 25% as the low threshold instead of ~14.2%

	if (newLocation.timeZone == -2) {
		_jumped = true;
		return SC_TRUE;
	}

	int currentValue = ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer;

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 0) {
		if (_staticData.location.node != newLocation.node) {
			if (currentValue <= GC_AI_OT_WALK_DECREMENT) {
				if (newLocation.timeZone != -2)
					((SceneViewWindow *)viewWindow)->showDeathScene(_deathID);
				return SC_DEATH;
			} else {
				currentValue -= GC_AI_OT_WALK_DECREMENT;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer = currentValue;

				if (currentValue < GC_AIHW_STARTING_VALUE / 7 || (currentValue % (GC_AIHW_STARTING_VALUE / 10)) == 0) {
					if (currentValue < GC_AIHW_STARTING_VALUE / 7) {
						Common::String oxygenMessage = _vm->getString(IDS_AI_OXY_LEVEL_TEXT_TEMPLATE_LOW);
						assert(!oxygenMessage.empty());
						oxygenMessage = Common::String::format(oxygenMessage.c_str(), currentValue * 100 / GC_AIHW_STARTING_VALUE);
						((SceneViewWindow *)viewWindow)->displayLiveText(oxygenMessage);
					} else {
						Common::String oxygenMessage = _vm->getString(IDS_AI_OXY_LEVEL_TEXT_TEMPLATE_NORM);
						assert(!oxygenMessage.empty());
						oxygenMessage = Common::String::format(oxygenMessage.c_str(), currentValue * 100 / GC_AIHW_STARTING_VALUE);
						((SceneViewWindow *)viewWindow)->displayLiveText(oxygenMessage);
					}
				}
			}
		} else {
			if (currentValue <= GC_AI_OT_TURN_DECREMENT) {
				if (newLocation.timeZone != -2)
					((SceneViewWindow *)viewWindow)->showDeathScene(_deathID);
				return SC_DEATH;
			} else {
				currentValue -= GC_AI_OT_TURN_DECREMENT;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer = currentValue;

				if (currentValue < GC_AIHW_STARTING_VALUE / 7 || (currentValue % (GC_AIHW_STARTING_VALUE / 10)) == 0) {
					if (currentValue < GC_AIHW_STARTING_VALUE / 7) {
						Common::String oxygenMessage = _vm->getString(IDS_AI_OXY_LEVEL_TEXT_TEMPLATE_LOW);
						assert(!oxygenMessage.empty());
						oxygenMessage = Common::String::format(oxygenMessage.c_str(), currentValue * 100 / GC_AIHW_STARTING_VALUE);
						((SceneViewWindow *)viewWindow)->displayLiveText(oxygenMessage);
					} else {
						Common::String oxygenMessage = _vm->getString(IDS_AI_OXY_LEVEL_TEXT_TEMPLATE_NORM);
						assert(!oxygenMessage.empty());
						oxygenMessage = Common::String::format(oxygenMessage.c_str(), currentValue * 100 / GC_AIHW_STARTING_VALUE);
						((SceneViewWindow *)viewWindow)->displayLiveText(oxygenMessage);
					}
				}
			}
		}
	}

	return SC_TRUE;
}

int BaseOxygenTimer::timerCallback(Window *viewWindow) {
	// NOTE: Earlier versions (1.01) used 25% as the low threshold instead of
	// ~14.2%

	if (_jumped)
		return SC_TRUE;

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 0) {
		if ((g_system->getMillis() - _entryStartTime) >= GC_AI_OT_WAIT_TIME_PERIOD) {
			int currentValue = ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer;

			if (currentValue <= GC_AI_OT_WAIT_DECREMENT) {
				((SceneViewWindow *)viewWindow)->showDeathScene(_deathID);
				return SC_DEATH;
			} else {
				currentValue -= GC_AI_OT_WAIT_DECREMENT;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer = currentValue;

				if (currentValue < GC_AIHW_STARTING_VALUE / 7 || (currentValue % (GC_AIHW_STARTING_VALUE / 10)) == 0) {
					if (currentValue < GC_AIHW_STARTING_VALUE / 7) {
						Common::String oxygenMessage = _vm->getString(IDS_AI_OXY_LEVEL_TEXT_TEMPLATE_LOW);
						assert(!oxygenMessage.empty());
						oxygenMessage = Common::String::format(oxygenMessage.c_str(), currentValue * 100 / GC_AIHW_STARTING_VALUE);
						((SceneViewWindow *)viewWindow)->displayLiveText(oxygenMessage);
					} else {
						Common::String oxygenMessage = _vm->getString(IDS_AI_OXY_LEVEL_TEXT_TEMPLATE_NORM);
						assert(!oxygenMessage.empty());
						oxygenMessage = Common::String::format(oxygenMessage.c_str(), currentValue * 100 / GC_AIHW_STARTING_VALUE);
						((SceneViewWindow *)viewWindow)->displayLiveText(oxygenMessage);
					}
				}
			}

			_entryStartTime = g_system->getMillis();
		}
	}

	return SC_TRUE;
}

class SpaceDoorTimer : public BaseOxygenTimer {
public:
	SpaceDoorTimer(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int openFrame = -1, int closedFrame = -1, int depth = -1,
			int transitionType = -1, int transitionData = -1, int transitionStartFrame = -1, int transitionLength = -1,
			int doorFlag = -1, int doorFlagValue = 0);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	bool _clicked;
	Common::Rect _clickable;
	DestinationScene _destData;
	int _openFrame;
	int _closedFrame;
	int _doorFlag;
	int _doorFlagValue;
};

SpaceDoorTimer::SpaceDoorTimer(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int openFrame, int closedFrame, int depth,
		int transitionType, int transitionData, int transitionStartFrame, int transitionLength,
		int doorFlag, int doorFlagValue) :
		BaseOxygenTimer(vm, viewWindow, sceneStaticData, priorLocation) {
	_clicked = false;
	_openFrame = openFrame;
	_closedFrame = closedFrame;
	_doorFlag = doorFlag;
	_doorFlagValue = doorFlagValue;
	_clickable = Common::Rect(left, top, right, bottom);
	_destData.destinationScene = _staticData.location;
	_destData.destinationScene.depth = depth;
	_destData.transitionType = transitionType;
	_destData.transitionData = transitionData;
	_destData.transitionStartFrame = transitionStartFrame;
	_destData.transitionLength = transitionLength;
}

int SpaceDoorTimer::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickable.contains(pointLocation)) {
		_clicked = true;
		return SC_TRUE;
	}

	return SC_FALSE;
}

int SpaceDoorTimer::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clicked) {
		// If we are facing the habitat wing death door in walkthrough mode,
		// keep it locked.
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1 &&
				_staticData.location.timeZone == 6 && _staticData.location.environment == 1 &&
				_staticData.location.node == 3 && _staticData.location.facing == 1 &&
				_staticData.location.orientation == 2 && _staticData.location.depth == 0) {
			_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12));
			_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13));
			_clicked = false;
			return SC_TRUE;
		}

		// If we are facing the scanning room door and we have Arthur, automatically recall
		// to the future apartment
		if (_staticData.location.timeZone == 6 && _staticData.location.environment == 3 &&
				_staticData.location.node == 9 && _staticData.location.facing == 0 &&
				_staticData.location.orientation == 0 && _staticData.location.depth == 0 &&
				((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI)) {
			((SceneViewWindow *)viewWindow)->timeSuitJump(4);
			return SC_TRUE;
		}

		if (_doorFlag < 0 || ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_doorFlag) == _doorFlagValue) {
			// Change the still frame to the new one
			if (_openFrame >= 0) {
				_staticData.navFrameIndex = _openFrame;
				viewWindow->invalidateWindow(false);
				_vm->_sound->playSynchronousSoundEffect("BITDATA/AILAB/AI_LOCK.BTA"); // Broken in 1.01
			}

			((SceneViewWindow *)viewWindow)->moveToDestination(_destData);
		} else {
			// Display the closed frame
			if (_closedFrame >= 0) {
				int oldFrame = _staticData.navFrameIndex;
				_staticData.navFrameIndex = _closedFrame;
				viewWindow->invalidateWindow(false);

				_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 12));
				_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13));

				_staticData.navFrameIndex = oldFrame;
				viewWindow->invalidateWindow(false);
			}
		}

		_clicked = false;
		return SC_TRUE;
	}

	return SC_FALSE;
}

int SpaceDoorTimer::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	// If we are in walkthrough mode and are at the death door in the habitat wing,
	// don't allow you to open the door.
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 1 &&
			_staticData.location.timeZone == 6 && _staticData.location.environment == 1 &&
			_staticData.location.node == 3 && _staticData.location.facing == 1 &&
			_staticData.location.orientation == 2 && _staticData.location.depth == 0) {
		return kCursorArrow;
	}

	if (_clickable.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class UseCheeseGirlPropellant : public BaseOxygenTimer {
public:
	UseCheeseGirlPropellant(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);

private:
	Common::Rect _badPos;
};

UseCheeseGirlPropellant::UseCheeseGirlPropellant(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		BaseOxygenTimer(vm, viewWindow, sceneStaticData, priorLocation) {
	_deathID = 40;
	_badPos = Common::Rect(144, 0, 288, 189);
}

int UseCheeseGirlPropellant::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	// Display text message about zero-g and no propulsion and oxygen level
	((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_AI_IS_JUMP_IN_TEXT));
	return SC_TRUE;
}

int UseCheeseGirlPropellant::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemCheeseGirl && !_badPos.contains(pointLocation))
		return 1;

	return 0;
}

int UseCheeseGirlPropellant::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemCheeseGirl) {
		if (_badPos.contains(pointLocation)) {
			DestinationScene destData;
			destData.destinationScene.timeZone = 6;
			destData.destinationScene.environment = 10;
			destData.destinationScene.node = 1;
			destData.destinationScene.facing = 0;
			destData.destinationScene.orientation = 0;
			destData.destinationScene.depth = 0;
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 1;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		} else {
			DestinationScene destData;
			destData.destinationScene.timeZone = 6;
			destData.destinationScene.environment = 1;
			destData.destinationScene.node = 1;
			destData.destinationScene.facing = 1;
			destData.destinationScene.orientation = 2;
			destData.destinationScene.depth = 0;
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 0;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
		}

		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

class BaseOxygenTimerInSpace : public BaseOxygenTimer {
public:
	BaseOxygenTimerInSpace(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
};

BaseOxygenTimerInSpace::BaseOxygenTimerInSpace(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		BaseOxygenTimer(vm, viewWindow, sceneStaticData, priorLocation) {
	_deathID = 40;
}

bool SceneViewWindow::initializeAILabTimeZoneAndEnvironment(Window *viewWindow, int environment) {
	if (environment == -1) {
		GlobalFlags &flags = ((SceneViewWindow *)viewWindow)->getGlobalFlags();

		flags.aiHWStingerID = 0;
		flags.aiHWStingerChannelID = 0;
		flags.aiCRStingerID = 0;
		flags.aiCRStingerChannelID = 0;
		flags.aiDBStingerID = 0;
		flags.aiDBStingerChannelID = 0;
		flags.aiOxygenTimer = kAIHWStartingValue;
		flags.aiCRPressurized = flags.generalWalkthroughMode;
		flags.aiCRPressurizedAttempted = 0;
		flags.aiMRPressurized = flags.generalWalkthroughMode;
		flags.aiIceMined = 0;
		flags.aiOxygenReserves = 1;
		flags.aiSCHeardInitialSpeech = 0;
		flags.aiMRCorrectFreqSet = 4;
		flags.aiSCConversationStatus = 0;
		flags.aiSCHeardNexusDoorComment = 0;
		flags.aiSCHeardNexusDoorCode = 0;
		flags.aiNXPlayedBrainComment = 0;
		flags.aiDBPlayedSecondArthur = 0;
		flags.aiDBPlayedThirdArthur = 0;
		flags.aiDBPlayedFourthArthur = 0;
		flags.aiCRGrabbedMetalBar = ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemMetalBar) ? 1 : 0;
		flags.aiICGrabbedWaterCanister = (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemWaterCanEmpty) || ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemWaterCanFull)) ? 1 : 0;
	} else if (environment == 1) {
		((SceneViewWindow *)viewWindow)->getGlobalFlags().scoreEnteredSpaceStation = 1;
	}

	return true;
}

bool SceneViewWindow::startAILabAmbient(int oldTimeZone, int oldEnvironment, int environment, bool fade) {
	_vm->_sound->setAmbientSound(_vm->getFilePath(6, environment, SF_AMBIENT), fade, 64);
	return true;
}

SceneBase *SceneViewWindow::constructAILabSceneObject(Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) {
	// TODO

	switch (sceneStaticData.classID) {
	case 1:
		return new UseCheeseGirlPropellant(_vm, viewWindow, sceneStaticData, priorLocation);
	case 3:
		return new SpaceDoorTimer(_vm, viewWindow, sceneStaticData, priorLocation, 172, 46, 262, 136, 87, -1, 1, TRANSITION_VIDEO, 2, -1, -1, -1, -1);
	case 5:
		return new SpaceDoorTimer(_vm, viewWindow, sceneStaticData, priorLocation, 144, 30, 268, 152, 88, -1, 1, TRANSITION_VIDEO, 4, -1, -1, -1, -1);
	case 6:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 11:
		return new BaseOxygenTimer(_vm, viewWindow, sceneStaticData, priorLocation);
	case 12:
		return new BaseOxygenTimerInSpace(_vm, viewWindow, sceneStaticData, priorLocation);
	case 26:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 32:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 52:
		return new SpaceDoorTimer(_vm, viewWindow, sceneStaticData, priorLocation, 164, 40, 276, 140, -1, -1, 1, TRANSITION_VIDEO, 0, -1, -1, -1, -1);
	case 53:
		return new SpaceDoorTimer(_vm, viewWindow, sceneStaticData, priorLocation, 164, 40, 276, 140, -1, -1, 1, TRANSITION_VIDEO, 2, -1, -1, -1, -1);
	case 54:
		return new PlaySoundExitingFromSceneDeux(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 60:
		return new BaseOxygenTimer(_vm, viewWindow, sceneStaticData, priorLocation);
	case 65:
		return new SpaceDoorTimer(_vm, viewWindow, sceneStaticData, priorLocation, 164, 26, 268, 124, -1, -1, 1, TRANSITION_VIDEO, 13, -1, -1, -1, -1);
	case 66:
		return new SpaceDoorTimer(_vm, viewWindow, sceneStaticData, priorLocation, 164, 26, 268, 124, -1, -1, 1, TRANSITION_VIDEO, 16, -1, -1, -1, -1);
	case 68:
		return new PlaySoundExitingFromSceneDeux(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 70:
		return new SpaceDoorTimer(_vm, viewWindow, sceneStaticData, priorLocation, 92, 92, 212, 189, 48, -1, 1, TRANSITION_VIDEO, 0, -1, -1, -1, -1);
	case 93:
		return new BaseOxygenTimer(_vm, viewWindow, sceneStaticData, priorLocation);
	}

	warning("TODO: AI lab scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
