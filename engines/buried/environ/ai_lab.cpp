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
	case 54:
		return new PlaySoundExitingFromSceneDeux(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 60:
		return new BaseOxygenTimer(_vm, viewWindow, sceneStaticData, priorLocation);
	case 68:
		return new PlaySoundExitingFromSceneDeux(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 93:
		return new BaseOxygenTimer(_vm, viewWindow, sceneStaticData, priorLocation);
	}

	warning("TODO: AI lab scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
