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
	bool pressed;
	uint8 key;
} pressedKeyCharMap[] = {
    {0x01, false, 0x48}, // up
    {0x02, false, 0x50}, // down
    {0x04, false, 0x4B}, // left
    {0x08, false, 0x4D}, // right
    {0x05, false, 0x47}, // home
    {0x09, false, 0x49}, // pageup
    {0x0A, false, 0x51}, // pagedown
    {0x06, false, 0x4F}, // end
    {0x01, true, 0x39},  // space bar
    {0x02, true, 0x1C},  // enter
    {0x04, true, 0x1D},  // ctrl
    {0x08, true, 0x38},  // alt
    {0x10, true, 0x53},  // del
    {0x20, true, 0x2A},  // left shift
    {0x20, true, 0x36},  // right shift
    {0x01, true, 0x3B},  // F1
    {0x02, true, 0x3C},  // F2
    {0x04, true, 0x3D},  // F3
    {0x08, true, 0x3E},  // F4
    {0x10, true, 0x3F},  // F5
    {0x20, true, 0x40},  // F6
    {0x40, true, 0x41},  // F7
    {0x80, true, 0x42},  // F8
    {0x01, true, 0x43},  // F9
    {0x02, true, 0x44},  // F10
    {0x04, true, 0x57},  // ?
    {0x08, true, 0x58},  // ?
    {0x00, true, 0x2A},  // left shift
    {0x00, true, 0x00},
    {0x01, false, 0x01}, // esc
    {0x00, false, 0x00}};
static_assert(ARRAYSIZE(pressedKeyCharMap) == 31, "Expected size of key char map");

ScopedKeyMapperDisable::ScopedKeyMapperDisable() {
	g_system->getEventManager()->getKeymapper()->setEnabled(false);
}

ScopedKeyMapperDisable::~ScopedKeyMapperDisable() {
	g_system->getEventManager()->getKeymapper()->setEnabled(true);
}

ScopedKeyMap::ScopedKeyMap(TwinEEngine* engine, const char *id) : _engine(engine) {
	_prevKeyMap = _engine->_input->currentKeyMap();
	_engine->_input->enabledKeyMap(cutsceneKeyMapId);
}

ScopedKeyMap::~ScopedKeyMap() {
	_engine->_input->enabledKeyMap(_prevKeyMap.c_str());
}

Input::Input(TwinEEngine *engine) : _engine(engine) {}

bool Input::isPressed(Common::KeyCode keycode, bool onlyFirstTime) const {
	if (onlyFirstTime) {
		return _pressed[keycode] == 1;
	}
	return _pressed[keycode] > 0;
}

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
	return toggleActionIfActive(TwinEActionType::CutsceneAbort) || toggleActionIfActive(TwinEActionType::UIAbort) || toggleActionIfActive(TwinEActionType::Escape);
}

bool Input::isQuickBehaviourActionActive() const {
	return isActionActive(TwinEActionType::QuickBehaviourNormal) || isActionActive(TwinEActionType::QuickBehaviourAthletic) || isActionActive(TwinEActionType::QuickBehaviourAggressive) || isActionActive(TwinEActionType::QuickBehaviourDiscreet);
}

void Input::enabledKeyMap(const char *id) {
	if (_currentKeyMap == id) {
		return;
	}

	// switching the keymap must also disable all other action keys
	memset(_pressed, 0, sizeof(_pressed));

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

void Input::readKeys() {
	skippedKey = 0;
	internalKeyCode = 0;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		uint8 localKey = 0;
		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			actionStates[event.customType] = 0;
			localKey = twineactions[event.customType].localKey;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (!_engine->cfgfile.Debug) {
				switch (event.customType) {
				case TwinEActionType::NextRoom:
				case TwinEActionType::PreviousRoom:
				case TwinEActionType::ApplyCellingGrid:
				case TwinEActionType::IncreaseCellingGridIndex:
				case TwinEActionType::DecreaseCellingGridIndex:
					break;
				default:
					localKey = twineactions[event.customType].localKey;
					actionStates[event.customType] = 1 + event.kbdRepeat;
					break;
				}
			} else {
				localKey = twineactions[event.customType].localKey;
				actionStates[event.customType] = 1 + event.kbdRepeat;
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			leftMouse = 1;
			break;
		case Common::EVENT_KEYDOWN:
			_pressed[event.kbd.keycode] = 1 + event.kbdRepeat;
			break;
		case Common::EVENT_KEYUP:
			_pressed[event.kbd.keycode] = 0;
			break;
		case Common::EVENT_RBUTTONDOWN:
			rightMouse = 1;
			break;
		default:
			break;
		}

		if (localKey == 0) {
			continue;
		}

		for (int i = 0; i < ARRAYSIZE(pressedKeyCharMap); i++) {
			if (pressedKeyCharMap[i].key == localKey) {
				if (pressedKeyCharMap[i].pressed) {
					if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
						pressedKey &= ~pressedKeyCharMap[i].high;
					} else {
						pressedKey |= pressedKeyCharMap[i].high;
					}
				} else {
					skippedKey |= pressedKeyCharMap[i].high;
				}
				break;
			}
		}
		internalKeyCode = localKey;
	}
}

void Input::getMousePositions(MouseStatusStruct *mouseData) {
	Common::Point point = g_system->getEventManager()->getMousePos();
	mouseData->x = point.x;
	mouseData->y = point.y;
	mouseData->left = leftMouse;
	mouseData->right = rightMouse;
	leftMouse = 0;
	rightMouse = 0;
}

} // namespace TwinE
