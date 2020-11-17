/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "twine/input.h"
#include "backends/keymapper/keymapper.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "twine/actor.h"
#include "twine/twine.h"

namespace TwinE {

const char *mainKeyMapId = "mainKeyMap";
const char *uiKeyMapId = "uiKeyMap";
const char *cutsceneKeyMapId = "cutsceneKeyMap";

/** Pressed key char map - scanCodeTab2 */
static const struct KeyProperties {
	uint8 high;
	bool cursor;
	uint8 key;
} pressedKeyCharMap[] = {
    {0x01, true, 0x48}, // up
    {0x02, true, 0x50}, // down
    {0x04, true, 0x4B}, // left
    {0x08, true, 0x4D}, // right
    {0x01, false, 0x39},  // space bar
    {0x02, false, 0x1C},  // enter
    {0x04, false, 0x1D},  // ctrl
    {0x08, false, 0x38},  // alt
    {0x10, false, 0x53},  // del
    {0x20, false, 0x2A},  // left shift
    {0x20, false, 0x36}  // right shift
};

ScopedKeyMap::ScopedKeyMap(TwinEEngine* engine, const char *id) : _engine(engine) {
	_prevKeyMap = _engine->_input->currentKeyMap();
	_engine->_input->enableKeyMap(id);
}

ScopedKeyMap::~ScopedKeyMap() {
	_engine->_input->enableKeyMap(_prevKeyMap.c_str());
}

Input::Input(TwinEEngine *engine) : _engine(engine) {}

bool Input::isActionActive(TwinEActionType actionType, bool onlyFirstTime) const {
	if (onlyFirstTime) {
		return actionStates[actionType] == 1;
	}
	return actionStates[actionType] > 0;
}

bool Input::toggleActionIfActive(TwinEActionType actionType) {
	if (actionStates[actionType] > 0) {
		actionStates[actionType] = 0;
		return true;
	}
	return false;
}

bool Input::toggleAbortAction() {
	bool abortState = false;
	abortState |= toggleActionIfActive(TwinEActionType::CutsceneAbort);
	abortState |= toggleActionIfActive(TwinEActionType::UIAbort);
	abortState |= toggleActionIfActive(TwinEActionType::Escape);
	return abortState;
}

bool Input::isQuickBehaviourActionActive() const {
	return isActionActive(TwinEActionType::QuickBehaviourNormal) || isActionActive(TwinEActionType::QuickBehaviourAthletic) || isActionActive(TwinEActionType::QuickBehaviourAggressive) || isActionActive(TwinEActionType::QuickBehaviourDiscreet);
}

bool Input::isMoveOrTurnActionActive() const {
	return isActionActive(TwinEActionType::TurnLeft) || isActionActive(TwinEActionType::TurnRight) || isActionActive(TwinEActionType::MoveBackward) || isActionActive(TwinEActionType::MoveForward);
}

bool Input::isHeroActionActive() const {
	return isActionActive(TwinEActionType::ExecuteBehaviourAction) || isActionActive(TwinEActionType::SpecialAction);
}

void Input::enableKeyMap(const char *id) {
	if (_currentKeyMap == id) {
		return;
	}

	Common::Keymapper *keymapper = g_system->getEventManager()->getKeymapper();
	const Common::KeymapArray &keymaps = keymapper->getKeymaps();
	for (Common::Keymap *keymap : keymaps) {
		const Common::String& keymapId = keymap->getId();
		if (keymapId == mainKeyMapId || keymapId == uiKeyMapId || keymapId == cutsceneKeyMapId) {
			keymap->setEnabled(keymapId == id);
		}
	}
	_currentKeyMap = id;
	debug("enable keymap %s", id);
}

// TODO: get rid of this table
static constexpr const struct ActionMapping {
	TwinEActionType action;
	uint8 localKey;
} twineactions[] = {
    {Pause, 0x19},
    {NextRoom, 0x13},
    {PreviousRoom, 0x21},
    {ApplyCellingGrid, 0x14},
    {IncreaseCellingGridIndex, 0x22},
    {DecreaseCellingGridIndex, 0x30},
    {DebugGridCameraPressUp, 0x2E},
    {DebugGridCameraPressDown, 0x2C},
    {DebugGridCameraPressLeft, 0x1F},
    {DebugGridCameraPressRight, 0x2D},
	{DebugMenu, 0x00},
	{DebugMenuActivate, 0x00},
    {QuickBehaviourNormal, 0x3B},
    {QuickBehaviourAthletic, 0x3C},
    {QuickBehaviourAggressive, 0x3D},
    {QuickBehaviourDiscreet, 0x3E},
    {ExecuteBehaviourAction, 0x39},
    {BehaviourMenu, 0x1D},
    {OptionsMenu, 0x40},
    {RecenterScreenOnTwinsen, 0x1C},
    {UseSelectedObject, 0x1C},
    {ThrowMagicBall, 0x38},
    {MoveForward, 0x48},
    {MoveBackward, 0x50},
    {TurnRight, 0x4D},
    {TurnLeft, 0x4B},
    {UseProtoPack, 0x24},
    {OpenHolomap, 0x23},
    {InventoryMenu, 0x36},
    {SpecialAction, 0x11},
    {Escape, 0x01},
    {UIEnter, 0x00},
    {UIAbort, 0x00},
    {UILeft, 0x00},
    {UIRight, 0x00},
    {UIUp, 0x00},
    {UIDown, 0x00},
    {UINextPage, 0x00},
    {CutsceneAbort, 0x00}};

static_assert(ARRAYSIZE(twineactions) == TwinEActionType::Max, "Unexpected action mapping array size");

uint8 Input::processCustomEngineEventStart(const Common::Event &event) {
	if (!_engine->cfgfile.Debug) {
		switch (event.customType) {
		case TwinEActionType::DebugGridCameraPressUp:
		case TwinEActionType::DebugGridCameraPressDown:
		case TwinEActionType::DebugGridCameraPressLeft:
		case TwinEActionType::DebugGridCameraPressRight:
		case TwinEActionType::DebugMenu:
		case TwinEActionType::DebugMenuActivate:
		case TwinEActionType::NextRoom:
		case TwinEActionType::PreviousRoom:
		case TwinEActionType::ApplyCellingGrid:
		case TwinEActionType::IncreaseCellingGridIndex:
		case TwinEActionType::DecreaseCellingGridIndex:
			break;
		default:
			actionStates[event.customType] = 1 + event.kbdRepeat;
			return twineactions[event.customType].localKey;
		}
	} else {
		actionStates[event.customType] = 1 + event.kbdRepeat;
		return twineactions[event.customType].localKey;
	}
	return 0;
}

uint8 Input::processCustomEngineEventEnd(const Common::Event &event) {
	actionStates[event.customType] = 0;
	return twineactions[event.customType].localKey;
}

void Input::readKeys() {
	cursorKeys = 0;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		uint8 localKey = 0;
		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			localKey = processCustomEngineEventEnd(event);
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			localKey = processCustomEngineEventStart(event);
			break;
		default:
			break;
		}

		if (localKey == 0) {
			continue;
		}

		for (int i = 0; i < ARRAYSIZE(pressedKeyCharMap); i++) {
			if (pressedKeyCharMap[i].key == localKey) {
				if (pressedKeyCharMap[i].cursor) {
					cursorKeys |= pressedKeyCharMap[i].high;
				} else {
					if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
						pressedKey &= ~pressedKeyCharMap[i].high;
					} else {
						pressedKey |= pressedKeyCharMap[i].high;
					}
				}
				break;
			}
		}
	}
}

void Input::getMousePositions(MouseStatusStruct *mouseData) {
	Common::Point point = g_system->getEventManager()->getMousePos();
	mouseData->x = point.x;
	mouseData->y = point.y;
}

bool Input::isMouseHovering(int32 left, int32 top, int32 right, int32 bottom) const {
	Common::Point point = g_system->getEventManager()->getMousePos();
	return point.x >= left && point.x <= right && point.y >= top && point.y <= bottom;
}

} // namespace TwinE
