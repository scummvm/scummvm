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

#include "common/system.h"
#include "graphics/font.h"

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

class PlayArthurOffsetTimed : public BaseOxygenTimer {
public:
	PlayArthurOffsetTimed(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int stingerVolume = 127, int lastStingerFlagOffset = -1, int effectIDFlagOffset = -1, int firstStingerFileID = -1,
			int lastStingerFileID = -1, int stingerDelay = 1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _stingerVolume;
	int _lastStingerFlagOffset;
	int _effectIDFlagOffset;
	int _firstStingerFileID;
	int _lastStingerFileID;
	int _stingerDelay;
	int _timerFlagOffset;
};

PlayArthurOffsetTimed::PlayArthurOffsetTimed(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int stingerVolume, int lastStingerFlagOffset, int effectIDFlagOffset, int firstStingerFileID,
		int lastStingerFileID, int stingerDelay) :
		BaseOxygenTimer(vm, viewWindow, sceneStaticData, priorLocation) {
	_stingerVolume = stingerVolume;
	_lastStingerFlagOffset = lastStingerFlagOffset;
	_effectIDFlagOffset = effectIDFlagOffset;
	_firstStingerFileID = firstStingerFileID;
	_lastStingerFileID = lastStingerFileID;
	_stingerDelay = stingerDelay;
}

int PlayArthurOffsetTimed::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	if (_effectIDFlagOffset >= 0 && (priorLocation.node != _staticData.location.node || priorLocation.environment != _staticData.location.environment)) {
		byte effectID = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_effectIDFlagOffset);

		if (!_vm->_sound->isSoundEffectPlaying(effectID - 1)) {
			int lastStinger = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_lastStingerFlagOffset) + 1;

			if ((lastStinger % _stingerDelay) == 0) {
				if (lastStinger <= (_lastStingerFileID - _firstStingerFileID) * _stingerDelay) {
					int fileNameIndex = _vm->computeFileNameResourceID(_staticData.location.timeZone, _staticData.location.environment, _firstStingerFileID + lastStinger / _stingerDelay - 1);
					byte newStingerID = 0;

					if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI)) {
						newStingerID = _vm->_sound->playSoundEffect(_vm->getFilePath(fileNameIndex), _stingerVolume / 2, false, true) + 1;
						byte &lastArthurComment = ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiHWLastCommentPlayed;

						if ((lastStinger / 2) != 0 && lastArthurComment < 4) {
							lastArthurComment++;
							_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, 10, lastArthurComment + 5), 128, false, true);
						}
					} else {
						newStingerID = _vm->_sound->playSoundEffect(_vm->getFilePath(fileNameIndex), _stingerVolume, false, true) + 1;
					}

					((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_effectIDFlagOffset, newStingerID);
					((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_lastStingerFlagOffset, lastStinger);
				}
			} else {
				((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_effectIDFlagOffset, 0xFF);
				((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_lastStingerFlagOffset, lastStinger);
			}
		}
	}

	return SC_TRUE;
}

class HabitatWingLockedDoor : public BaseOxygenTimer {
public:
	HabitatWingLockedDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int newFrameID = -1, int beepSoundID = -1, int voSoundID = -1, int left = 0, int top = 0, int right = 0, int bottom = 0);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _newFrameID;
	Common::Rect _clickRegion;
	int _beepSoundID;
	int _voSoundID;
};

HabitatWingLockedDoor::HabitatWingLockedDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int newFrameID, int beepSoundID, int voSoundID, int left, int top, int right, int bottom) :
		BaseOxygenTimer(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(left, top, right, bottom);
	_newFrameID = newFrameID;
	_beepSoundID = beepSoundID;
	_voSoundID = voSoundID;
}

int HabitatWingLockedDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation)) {
		int oldFrame = _staticData.navFrameIndex;
		_staticData.navFrameIndex = _newFrameID;
		viewWindow->invalidateWindow(false);

		if (_beepSoundID != -1)
			_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _beepSoundID));

		if (_voSoundID != -1)
			_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, _voSoundID));

		_staticData.navFrameIndex = oldFrame;
		viewWindow->invalidateWindow(false);

		return SC_TRUE;
	}

	return SC_FALSE;
}

int HabitatWingLockedDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

class BaseOxygenTimerInSpace : public BaseOxygenTimer {
public:
	BaseOxygenTimerInSpace(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
};

BaseOxygenTimerInSpace::BaseOxygenTimerInSpace(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		BaseOxygenTimer(vm, viewWindow, sceneStaticData, priorLocation) {
	_deathID = 40;
}

class BaseOxygenTimerCapacitance : public SceneBase {
public:
	BaseOxygenTimerCapacitance(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	virtual int postEnterRoom(Window *viewWindow, const Location &priorLocation);
	virtual int preExitRoom(Window *viewWindow, const Location &priorLocation);
	virtual int timerCallback(Window *viewWindow);

protected:
	uint32 _entryStartTime;
	bool _jumped;
};

BaseOxygenTimerCapacitance::BaseOxygenTimerCapacitance(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		SceneBase(vm, viewWindow, sceneStaticData, priorLocation) {
	_jumped = false;
}

int BaseOxygenTimerCapacitance::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	_entryStartTime = g_system->getMillis();
	return SC_TRUE;
}

int BaseOxygenTimerCapacitance::preExitRoom(Window *viewWindow, const Location &newLocation) {
	// This does the 25% warning, unlike BaseOxygenTimer

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurized == 0) {
		if (newLocation.timeZone == -2) {
			_jumped = true;
			return SC_TRUE;
		}

		int currentValue = ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer;

		if (_staticData.location.node != newLocation.node) {
			if (currentValue <= GC_AI_OT_WALK_DECREMENT) {
				if (newLocation.timeZone != -2)
					((SceneViewWindow *)viewWindow)->showDeathScene(41);
				return SC_DEATH;
			} else {
				currentValue -= GC_AI_OT_WALK_DECREMENT;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer = currentValue;

				if (currentValue < GC_AIHW_STARTING_VALUE / 4 || (currentValue % (GC_AIHW_STARTING_VALUE / 10)) == 0) {
					if (currentValue < GC_AIHW_STARTING_VALUE / 4) {
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
					((SceneViewWindow *)viewWindow)->showDeathScene(41);
				return SC_DEATH;
			} else {
				currentValue -= GC_AI_OT_TURN_DECREMENT;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer = currentValue;

				if (currentValue < GC_AIHW_STARTING_VALUE / 4 || (currentValue % (GC_AIHW_STARTING_VALUE / 10)) == 0) {
					if (currentValue < GC_AIHW_STARTING_VALUE / 4) {
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

int BaseOxygenTimerCapacitance::timerCallback(Window *viewWindow) {
	// This does the 25% warning, unlike BaseOxygenTimer

	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().generalWalkthroughMode == 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurized == 0) {
		if (_jumped)
			return SC_TRUE;

		if ((g_system->getMillis() - _entryStartTime) >= GC_AI_OT_WAIT_TIME_PERIOD) {
			int currentValue = ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer;

			if (currentValue <= GC_AI_OT_WAIT_DECREMENT) {
				((SceneViewWindow *)viewWindow)->showDeathScene(41);
				return SC_DEATH;
			} else {
				currentValue -= GC_AI_OT_WAIT_DECREMENT;
				((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer = currentValue;

				if (currentValue < GC_AIHW_STARTING_VALUE / 4 || (currentValue % (GC_AIHW_STARTING_VALUE / 10)) == 0) {
					if (currentValue < GC_AIHW_STARTING_VALUE / 4) {
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

class CapacitanceToHabitatDoorClosed : public BaseOxygenTimerCapacitance {
public:
	CapacitanceToHabitatDoorClosed(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);

private:
	Common::Rect _metalBar;
	Common::Rect _door;
};

CapacitanceToHabitatDoorClosed::CapacitanceToHabitatDoorClosed(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		BaseOxygenTimerCapacitance(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 1)
		_staticData.navFrameIndex = 7;
	else
		_staticData.navFrameIndex = 55;

	_metalBar = Common::Rect(184, 146, 264, 184);
	_door = Common::Rect(132, 14, 312, 180);
}

int CapacitanceToHabitatDoorClosed::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_metalBar.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 0) {
		_staticData.navFrameIndex = 7;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar = 1;

		Common::Point ptInventoryWindow = viewWindow->convertPointToWindow(pointLocation, ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(kItemMetalBar, ptInventoryWindow);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int CapacitanceToHabitatDoorClosed::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_door.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 0) {
			_staticData.navFrameIndex = 96;
			viewWindow->invalidateWindow(false);

			// Wait for a second (why?)
			uint32 startTime = g_system->getMillis();

			while (!_vm->shouldQuit() && g_system->getMillis() < startTime + 1000) {
				_vm->yield();
				_vm->_sound->timerCallback();
			}

			DestinationScene destData;
			destData.destinationScene = _staticData.location;
			destData.destinationScene.depth = 1;
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 1;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;

			// Move to the final destination
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
			return SC_TRUE;
		} else {
			if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurized == 0) {
				_staticData.navFrameIndex = 97;
				viewWindow->invalidateWindow(false);

				// Wait for a second (why?)
				uint32 startTime = g_system->getMillis();

				while (!_vm->shouldQuit() && g_system->getMillis() < startTime + 1000) {
					_vm->yield();
					_vm->_sound->timerCallback();
				}

				DestinationScene destData;
				destData.destinationScene = _staticData.location;
				destData.destinationScene.depth = 1;
				destData.transitionType = TRANSITION_VIDEO;
				destData.transitionData = 2;
				destData.transitionStartFrame = -1;
				destData.transitionLength = -1;

				// Move to the final destination
				((SceneViewWindow *)viewWindow)->moveToDestination(destData);
				return SC_TRUE;
			} else {
				int oldFrame = _staticData.navFrameIndex;
				_staticData.navFrameIndex = 121;
				viewWindow->invalidateWindow(false);

				_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment - 1, 12));
				_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment - 1, 13));

				_staticData.navFrameIndex = oldFrame;
				viewWindow->invalidateWindow(false);
				return SC_TRUE;
			}
		}
	}

	return SC_FALSE;
}

int CapacitanceToHabitatDoorClosed::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_metalBar.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 0)
		return kCursorOpenHand;

	if (_door.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
}

int CapacitanceToHabitatDoorClosed::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemMetalBar && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 1)
		return 1;

	return 0;
}

int CapacitanceToHabitatDoorClosed::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0; // ???

	return SIC_REJECT;
}

class CapacitanceToHabitatDoorOpen : public BaseOxygenTimerCapacitance {
public:
	CapacitanceToHabitatDoorOpen(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int postExitRoom(Window *viewWindow, const Location &newLocation);
	int mouseDown(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);
	int draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);
	int droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags);

private:
	Common::Rect _metalBar;
};

CapacitanceToHabitatDoorOpen::CapacitanceToHabitatDoorOpen(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		BaseOxygenTimerCapacitance(vm, viewWindow, sceneStaticData, priorLocation) {
	if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 1) {
		_staticData.navFrameIndex = 101;
		_staticData.destForward.transitionStartFrame = 0;
		_staticData.destForward.transitionLength = 28;
	} else {
		_staticData.navFrameIndex = 100;
		_staticData.destForward.transitionStartFrame = 53;
		_staticData.destForward.transitionLength = 28;
	}

	_metalBar = Common::Rect(184, 146, 264, 184);
}

int CapacitanceToHabitatDoorOpen::postExitRoom(Window *viewWindow, const Location &newLocation) {
	// Play the door closing sound
	if (_staticData.location.timeZone == newLocation.timeZone)
		_vm->_sound->playSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 14), 128, false, true);

	return SC_TRUE;
}

int CapacitanceToHabitatDoorOpen::mouseDown(Window *viewWindow, const Common::Point &pointLocation) {
	if (_metalBar.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 0) {
		_staticData.navFrameIndex = 101;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar = 1;
		_staticData.destForward.transitionStartFrame = 0;
		_staticData.destForward.transitionLength = 28;

		Common::Point ptInventoryWindow = viewWindow->convertPointToWindow(pointLocation, ((GameUIWindow *)viewWindow->getParent())->_inventoryWindow);
		((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->startDraggingNewItem(kItemMetalBar, ptInventoryWindow);

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SC_TRUE;
	}

	return SC_FALSE;
}

int CapacitanceToHabitatDoorOpen::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_metalBar.contains(pointLocation) && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 0)
		return kCursorOpenHand;

	return kCursorArrow;
}

int CapacitanceToHabitatDoorOpen::draggingItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (itemID == kItemMetalBar && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 1)
		return 1;

	return 0;
}

int CapacitanceToHabitatDoorOpen::droppedItem(Window *viewWindow, int itemID, const Common::Point &pointLocation, int itemFlags) {
	if (pointLocation.x == -1 && pointLocation.y == -1)
		return 0; // ???

	if (itemID == kItemMetalBar && ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 1) {
		_staticData.navFrameIndex = 100;
		((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar = 0;
		viewWindow->invalidateWindow(false);
		_staticData.destForward.transitionStartFrame = 53;
		_staticData.destForward.transitionLength = 28;

		((GameUIWindow *)viewWindow->getParent())->_bioChipRightWindow->sceneChanged();
		return SIC_ACCEPT;
	}

	return SIC_REJECT;
}

class CapacitancePanelInterface : public BaseOxygenTimerCapacitance {
public:
	CapacitancePanelInterface(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	~CapacitancePanelInterface();
	void preDestructor();
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);
	int gdiPaint(Window *viewWindow);

private:
	Common::Rect _stationRegions[15];
	int _currentSelection;
	int _currentTextIndex;
	int _lineHeight;
	Graphics::Font *_textFont;
	Common::Rect _leftTextRegion;
	Common::Rect _rightTextRegion;
};

CapacitancePanelInterface::CapacitancePanelInterface(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		BaseOxygenTimerCapacitance(vm, viewWindow, sceneStaticData, priorLocation) {
	_currentSelection = -1;
	_currentTextIndex = -1;
	_stationRegions[0] = Common::Rect(265, 110, 286, 135);
	_stationRegions[1] = Common::Rect(102, 45, 180, 134);
	_stationRegions[2] = Common::Rect(195, 106, 216, 133);
	_stationRegions[3] = Common::Rect(268, 72, 283, 87);
	_stationRegions[4] = Common::Rect(221, 46, 236, 74);
	_stationRegions[5] = Common::Rect(290, 72, 317, 108);
	_stationRegions[6] = Common::Rect(264, 55, 288, 67);
	_stationRegions[7] = Common::Rect(194, 74, 266, 84);
	_stationRegions[8] = Common::Rect(198, 62, 214, 74);
	_stationRegions[9] = Common::Rect(221, 106, 236, 134);
	_stationRegions[10] = Common::Rect(245, 46, 260, 74);
	_stationRegions[11] = Common::Rect(245, 106, 260, 134);
	_stationRegions[12] = Common::Rect(266, 92, 290, 109);
	_stationRegions[13] = Common::Rect(194, 96, 264, 106);
	_stationRegions[14] = Common::Rect(180, 85, 194, 94);
	_leftTextRegion = Common::Rect(83, 144, 211, 170);
	_rightTextRegion = Common::Rect(228, 144, 356, 170);
	_lineHeight = _vm->getLanguage() == Common::JA_JPN ? 10 : 13;
	_textFont = _vm->_gfx->createFont(_lineHeight);
}

CapacitancePanelInterface::~CapacitancePanelInterface() {
	preDestructor();
}

void CapacitancePanelInterface::preDestructor() {
	delete _textFont;
	_textFont = 0;
}

int CapacitancePanelInterface::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	byte &oxygenReserves = ((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenReserves;

	if (_currentSelection == 2) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiMRPressurized == 0 && (_stationRegions[_currentSelection].contains(pointLocation) || _rightTextRegion.contains(pointLocation))) {
			if (oxygenReserves > 0) {
				// Decrement reserves flag
				oxygenReserves--;

				// Set the machine room to pressurized
				((SceneViewWindow *)viewWindow)->getGlobalFlags().aiMRPressurized = 1;

				// Display pressurizing message
				viewWindow->invalidateWindow(false);
				_currentTextIndex = IDS_AI_PRES_PANEL_PRES_ENV_TEXT;

				// Play sound file
				_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13), 128);

				// Display pressurized text
				viewWindow->invalidateWindow(false);
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_PRES_TEXT;
				return SC_TRUE;
			} else {
				// Not enough oxygen reserves
				viewWindow->invalidateWindow(false);
				_currentTextIndex = IDS_AI_PRES_PANEL_INSUF_OXYGEN;
				return SC_TRUE;
			}
		}
	} else if (_currentSelection == 3) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurized == 0 && (_stationRegions[_currentSelection].contains(pointLocation) || _rightTextRegion.contains(pointLocation))) {
			if (oxygenReserves > 0) {
				if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRGrabbedMetalBar == 0) {
					if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurizedAttempted == 0) {
						// Display pressurizing message
						viewWindow->invalidateWindow(false);
						_currentTextIndex = IDS_AI_PRES_PANEL_PRES_ENV_TEXT;

						// Play sound file
						_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13), 128);

						// Display bulkhead message
						viewWindow->invalidateWindow(false);
						_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;

						// Play Mom audio
						// (Is this an Alien reference?)
						_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 11));

						// Update attempt flag
						((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurizedAttempted = 1;
					}
				} else {
					// Decrement reserves flag
					oxygenReserves--;

					// Set the capacitance array to pressurized
					((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurized = 1;

					// Display pressurizing message
					viewWindow->invalidateWindow(false);
					_currentTextIndex = IDS_AI_PRES_PANEL_PRES_ENV_TEXT;

					// Play sound file
					_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment, 13), 128);

					// Display pressurized text
					viewWindow->invalidateWindow(false);
					_currentTextIndex = IDS_AI_PRES_PANEL_ENV_PRES_TEXT;

					// Display oxygen text in the message window
					((SceneViewWindow *)viewWindow)->displayLiveText(_vm->getString(IDS_AI_ENTERING_PRES_ENV_TEXT));
					((SceneViewWindow *)viewWindow)->getGlobalFlags().aiOxygenTimer = GC_AIHW_STARTING_VALUE;
				}

				return SC_TRUE;
			} else {
				// Not enough oxygen reserves
				viewWindow->invalidateWindow(false);
				_currentTextIndex = IDS_AI_PRES_PANEL_INSUF_OXYGEN;
				return SC_TRUE;
			}
		}
	}

	// Check against the hotspots
	for (int i = 0; i < 15; i++) {
		if (_stationRegions[i].contains(pointLocation) && _currentSelection != i) {
			switch (i) {
			case 0:
				_staticData.navFrameIndex = 107;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_PRES_TEXT;
				return SC_TRUE;
			case 1:
				_staticData.navFrameIndex = 108;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ZERO_PRES_ENV;
				return SC_TRUE;
			case 2:
				_staticData.navFrameIndex = 109;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;

				if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiMRPressurized == 1)
					_currentTextIndex = IDS_AI_PRES_PANEL_ENV_PRES_TEXT;
				else
					_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES;
				return SC_TRUE;
			case 3:
				_staticData.navFrameIndex = 110;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;

				if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurized == 1)
					_currentTextIndex = IDS_AI_PRES_PANEL_ENV_PRES_TEXT;
				else
					_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES;
				return SC_TRUE;
			case 4:
				_staticData.navFrameIndex = 111;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;
				return SC_TRUE;
			case 5:
				_staticData.navFrameIndex = 112;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_PRES_TEXT;
				return SC_TRUE;
			case 6:
				_staticData.navFrameIndex = 113;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;
				return SC_TRUE;
			case 7:
				_staticData.navFrameIndex = 114;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;
				return SC_TRUE;
			case 8:
				_staticData.navFrameIndex = 115;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;
				return SC_TRUE;
			case 9:
				_staticData.navFrameIndex = 116;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;
				return SC_TRUE;
			case 10:
				_staticData.navFrameIndex = 117;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;
				return SC_TRUE;
			case 11:
				_staticData.navFrameIndex = 118;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;
				return SC_TRUE;
			case 12:
				_staticData.navFrameIndex = 119;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_PRES_TEXT;
				return SC_TRUE;
			case 13:
			case 14:
				_staticData.navFrameIndex = 120;
				viewWindow->invalidateWindow(false);
				_currentSelection = i;
				_currentTextIndex = IDS_AI_PRES_PANEL_ENV_DEPRES_BREACH;
				return SC_TRUE;
			}
		}
	}

	// By default, return to depth zero (zoomed out)
	DestinationScene destData;
	destData.destinationScene = _staticData.location;
	destData.destinationScene.depth = 0;
	destData.transitionType = TRANSITION_NONE;
	destData.transitionData = -1;
	destData.transitionStartFrame = -1;
	destData.transitionLength = -1;
	((SceneViewWindow *)viewWindow)->moveToDestination(destData);
	return SC_TRUE;
}

int CapacitancePanelInterface::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	for (int i = 0; i < 15; i++)
		if (_stationRegions[i].contains(pointLocation))
			return kCursorFinger;

	return kCursorPutDown;
}

int CapacitancePanelInterface::gdiPaint(Window *viewWindow) {
	if (_currentSelection >= 0) {
		uint32 color = _vm->_gfx->getColor(208, 144, 24);

		Common::String location = _vm->getString(IDS_AI_PRES_PANEL_DESC_BASE + _currentSelection);
		if (_currentSelection == 3)
			location += _vm->getString(IDS_AI_PRES_PANEL_DESC_BASE + 19);

		Common::Rect absoluteRect = viewWindow->getAbsoluteRect();
		Common::Rect rect(_leftTextRegion);
		rect.translate(absoluteRect.left, absoluteRect.top);
		_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, location, rect.left, rect.top, rect.width(), rect.height(), color, _lineHeight, kTextAlignCenter, true);

		if (_currentTextIndex >= 0) {
			rect = _rightTextRegion;
			rect.translate(absoluteRect.left, absoluteRect.top);
			_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFont, _vm->getString(_currentTextIndex), rect.left, rect.top, rect.width(), rect.height(), color, _lineHeight, kTextAlignCenter, true);
		}
	}

	return SC_FALSE;
}

class PlayArthurOffsetCapacitance : public BaseOxygenTimerCapacitance {
public:
	PlayArthurOffsetCapacitance(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int stingerVolume = 127, int lastStingerFlagOffset = -1, int effectIDFlagOffset = -1, int firstStingerFileID = -1,
			int lastStingerFileID = -1, int stingerDelay = 1, int flagOffset = -1, int newStill = -1, int newNavStart = -1, int newNavLength = -1);
	int postEnterRoom(Window *viewWindow, const Location &priorLocation);

private:
	int _stingerVolume;
	int _lastStingerFlagOffset;
	int _effectIDFlagOffset;
	int _firstStingerFileID;
	int _lastStingerFileID;
	int _stingerDelay;
};

PlayArthurOffsetCapacitance::PlayArthurOffsetCapacitance(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int stingerVolume, int lastStingerFlagOffset, int effectIDFlagOffset, int firstStingerFileID,
		int lastStingerFileID, int stingerDelay, int flagOffset, int newStill, int newNavStart, int newNavLength) :
		BaseOxygenTimerCapacitance(vm, viewWindow, sceneStaticData, priorLocation) {
	_stingerVolume = stingerVolume;
	_lastStingerFlagOffset = lastStingerFlagOffset;
	_effectIDFlagOffset = effectIDFlagOffset;
	_firstStingerFileID = firstStingerFileID;
	_lastStingerFileID = lastStingerFileID;
	_stingerDelay = stingerDelay;

	if (flagOffset >= 0 && ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(flagOffset) == 0) {
		// This is completely wrong.
		//if (newStill >= 0)
		//	_staticData.navFrameIndex;
		if (newNavStart >= 0)
			_staticData.destForward.transitionStartFrame = newNavStart;
		if (newNavLength >= 0)
			_staticData.destForward.transitionLength = newNavLength;
	}
}

int PlayArthurOffsetCapacitance::postEnterRoom(Window *viewWindow, const Location &priorLocation) {
	BaseOxygenTimerCapacitance::postEnterRoom(viewWindow, priorLocation);

	if (_effectIDFlagOffset >= 0) {
		byte effectID = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_effectIDFlagOffset);

		if (!_vm->_sound->isSoundEffectPlaying(effectID - 1)) {
			byte lastStinger = ((SceneViewWindow *)viewWindow)->getGlobalFlagByte(_lastStingerFlagOffset) + 1;

			if ((lastStinger % _stingerDelay) == 0) {
				if (lastStinger < (_lastStingerFileID - _firstStingerFileID) * _stingerDelay) {
					int fileNameIndex = _vm->computeFileNameResourceID(_staticData.location.timeZone, _staticData.location.environment, _firstStingerFileID + (lastStinger / _stingerDelay) - 1);

					if (((GameUIWindow *)viewWindow->getParent())->_inventoryWindow->isItemInInventory(kItemBioChipAI) && (lastStinger / _stingerDelay) < 3) {
						_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(fileNameIndex));

						// Play an Arthur comment if we have the chip
						switch (lastStinger / _stingerDelay) {
						case 0:
							_vm->_sound->playSynchronousSoundEffect("BITDATA/AILAB/AICR_C01.BTA", 127);
							break;
						case 1:
							_vm->_sound->playSynchronousSoundEffect("BITDATA/AILAB/AICR_C02.BTA", 127);
							break;
						case 2:
							_vm->_sound->playSynchronousSoundEffect("BITDATA/AILAB/AICR_C03.BTA", 127);
							break;
						}

						// Update the global flags
						((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_lastStingerFlagOffset, lastStinger);
					} else {
						byte newStingerID = _vm->_sound->playSoundEffect(_vm->getFilePath(fileNameIndex), _stingerVolume, false, true) + 1;

						// Update the global flags
						((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_effectIDFlagOffset, newStingerID);
						((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_lastStingerFlagOffset, lastStinger);
					}
				}
			} else {
				((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_effectIDFlagOffset, 0xFF);
				((SceneViewWindow *)viewWindow)->setGlobalFlagByte(_lastStingerFlagOffset, lastStinger);
			}
		}
	}

	return SC_TRUE;
}

class ClickChangeSceneCapacitance : public BaseOxygenTimerCapacitance {
public:
	ClickChangeSceneCapacitance(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
			int left = -1, int top = -1, int right = -1, int bottom = -1, int cursorID = 0,
			int timeZone = -1, int environment = -1, int node = -1, int facing = -1, int orientation = -1, int depth = -1,
			int transitionType = -1, int transitionData = -1, int transitionStartFrame = -1, int transitionLength = -1);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	int _cursorID;
	Common::Rect _clickRegion;
	DestinationScene _clickDestination;
};

ClickChangeSceneCapacitance::ClickChangeSceneCapacitance(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation,
		int left, int top, int right, int bottom, int cursorID,
		int timeZone, int environment, int node, int facing, int orientation, int depth,
		int transitionType, int transitionData, int transitionStartFrame, int transitionLength) :
		BaseOxygenTimerCapacitance(vm, viewWindow, sceneStaticData, priorLocation) {
	_clickRegion = Common::Rect(left, top, right, bottom);
	_cursorID = cursorID;
	_clickDestination.destinationScene = Location(timeZone, environment, node, facing, orientation, depth);
	_clickDestination.transitionType = transitionType;
	_clickDestination.transitionData = transitionData;
	_clickDestination.transitionStartFrame = transitionStartFrame;
	_clickDestination.transitionLength = transitionLength;
}

int ClickChangeSceneCapacitance::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		((SceneViewWindow *)viewWindow)->moveToDestination(_clickDestination);

	return SC_FALSE;
}

int ClickChangeSceneCapacitance::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_clickRegion.contains(pointLocation))
		return _cursorID;

	return kCursorArrow;
}

class CapacitanceDockingBayDoor : public BaseOxygenTimerCapacitance {
public:
	CapacitanceDockingBayDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation);
	int mouseUp(Window *viewWindow, const Common::Point &pointLocation);
	int specifyCursor(Window *viewWindow, const Common::Point &pointLocation);

private:
	Common::Rect _door;
};

CapacitanceDockingBayDoor::CapacitanceDockingBayDoor(BuriedEngine *vm, Window *viewWindow, const LocationStaticData &sceneStaticData, const Location &priorLocation) :
		BaseOxygenTimerCapacitance(vm, viewWindow, sceneStaticData, priorLocation) {
	_door = Common::Rect(160, 54, 276, 168);
}

int CapacitanceDockingBayDoor::mouseUp(Window *viewWindow, const Common::Point &pointLocation) {
	if (_door.contains(pointLocation)) {
		if (((SceneViewWindow *)viewWindow)->getGlobalFlags().aiCRPressurized == 1) {
			_staticData.navFrameIndex = 98;
			viewWindow->invalidateWindow(false);

			_vm->_sound->playSynchronousSoundEffect("BITDATA/AILAB/AI_LOCK.BTA");

			// Wait a second?
			uint32 startTime = g_system->getMillis();
			while (!_vm->shouldQuit() && g_system->getMillis() < startTime + 1000) {
				_vm->yield();
				_vm->_sound->timerCallback();
			}

			DestinationScene destData;
			destData.destinationScene = _staticData.location;
			destData.destinationScene.depth = 1;
			destData.transitionType = TRANSITION_VIDEO;
			destData.transitionData = 0;
			destData.transitionStartFrame = -1;
			destData.transitionLength = -1;

			// Move to the final destination
			((SceneViewWindow *)viewWindow)->moveToDestination(destData);
			return SC_TRUE;
		} else {
			int oldFrame = _staticData.navFrameIndex;
			_staticData.navFrameIndex = 99;
			viewWindow->invalidateWindow(false);

			_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment - 1, 12));
			_vm->_sound->playSynchronousSoundEffect(_vm->getFilePath(_staticData.location.timeZone, _staticData.location.environment - 1, 13));

			_staticData.navFrameIndex = oldFrame;
			viewWindow->invalidateWindow(false);
			return SC_TRUE;
		}
	}

	return SC_FALSE;
}

int CapacitanceDockingBayDoor::specifyCursor(Window *viewWindow, const Common::Point &pointLocation) {
	if (_door.contains(pointLocation))
		return kCursorFinger;

	return kCursorArrow;
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
	case 4:
		return new PlayArthurOffsetTimed(_vm, viewWindow, sceneStaticData, priorLocation, 127, offsetof(GlobalFlags, aiHWStingerID), offsetof(GlobalFlags, aiHWStingerChannelID), 4, 10, 1); // 1.01 uses a delay of 2, clone2727 likes that better
	case 5:
		return new SpaceDoorTimer(_vm, viewWindow, sceneStaticData, priorLocation, 144, 30, 268, 152, 88, -1, 1, TRANSITION_VIDEO, 4, -1, -1, -1, -1);
	case 6:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 7:
		return new HabitatWingLockedDoor(_vm, viewWindow, sceneStaticData, priorLocation, 99, 12, 13, 166, 32, 286, 182);
	case 8:
		return new HabitatWingLockedDoor(_vm, viewWindow, sceneStaticData, priorLocation, 100, 12, 13, 130, 48, 290, 189);
	case 11:
		return new BaseOxygenTimer(_vm, viewWindow, sceneStaticData, priorLocation);
	case 12:
		return new BaseOxygenTimerInSpace(_vm, viewWindow, sceneStaticData, priorLocation);
	case 20:
		return new PlayArthurOffsetCapacitance(_vm, viewWindow, sceneStaticData, priorLocation, 127, offsetof(GlobalFlags, aiCRStingerID), offsetof(GlobalFlags, aiCRStingerChannelID), 4, 11, 1);
	case 21:
		return new CapacitanceToHabitatDoorClosed(_vm, viewWindow, sceneStaticData, priorLocation);
	case 22:
		return new CapacitanceToHabitatDoorOpen(_vm, viewWindow, sceneStaticData, priorLocation);
	case 23:
		return new ClickChangeSceneCapacitance(_vm, viewWindow, sceneStaticData, priorLocation, 122, 32, 310, 140, kCursorMagnifyingGlass, 6, 2, 3, 0, 1, 1, TRANSITION_VIDEO, 3, -1, -1);
	case 24:
		return new CapacitancePanelInterface(_vm, viewWindow, sceneStaticData, priorLocation);
	case 25:
		return new CapacitanceDockingBayDoor(_vm, viewWindow, sceneStaticData, priorLocation);
	case 26:
		return new PlaySoundExitingFromScene(_vm, viewWindow, sceneStaticData, priorLocation, 14);
	case 27:
		return new PlayArthurOffsetCapacitance(_vm, viewWindow, sceneStaticData, priorLocation, 127, offsetof(GlobalFlags, aiCRStingerID), offsetof(GlobalFlags, aiCRStingerChannelID), 4, 11, 1, offsetof(GlobalFlags, aiCRGrabbedMetalBar), 73, 320, 40);
	case 28:
		return new PlayArthurOffsetCapacitance(_vm, viewWindow, sceneStaticData, priorLocation, 127, offsetof(GlobalFlags, aiCRStingerID), offsetof(GlobalFlags, aiCRStingerChannelID), 4, 11, 1, offsetof(GlobalFlags, aiCRGrabbedMetalBar), 66, 241, 25);
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
	case 75:
		return new HabitatWingLockedDoor(_vm, viewWindow, sceneStaticData, priorLocation, 51, 4, 5, 146, 0, 396, 84);
	case 93:
		return new BaseOxygenTimer(_vm, viewWindow, sceneStaticData, priorLocation);
	}

	warning("TODO: AI lab scene object %d", sceneStaticData.classID);
	return new SceneBase(_vm, viewWindow, sceneStaticData, priorLocation);
}

} // End of namespace Buried
