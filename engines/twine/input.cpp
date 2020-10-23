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

#ifndef TWINE_KEYBOARD_H
#define TWINE_KEYBOARD_H

#include "twine/input.h"
#include "common/system.h"

namespace TwinE {

Input::Input(TwinEEngine *engine) : _engine(engine) {}

bool Input::isAnyKeyPressed() const {
	return internalKeyCode != 0;
}

bool Input::isPressed(Common::KeyCode keycode) {
}

void Input::readKeys() {
	if (_engine->shouldQuit()) {
		internalKeyCode = 1;
		skippedKey = 1;
		return;
	}
	skippedKey = 0;
	internalKeyCode = 0;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		uint8 localKey = 0;
		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			actionStates[event.customType] = false;
			localKey = twineactions[event.customType].localKey;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (!cfgfile.Debug) {
				switch (event.customType) {
				case TwinEActionType::NextRoom:
				case TwinEActionType::PreviousRoom:
				case TwinEActionType::ApplyCellingGrid:
				case TwinEActionType::IncreaseCellingGridIndex:
				case TwinEActionType::DecreaseCellingGridIndex:
					break;
				default:
					localKey = twineactions[event.customType].localKey;
					actionStates[event.customType] = true;
					break;
				}
			} else {
				localKey = twineactions[event.customType].localKey;
				actionStates[event.customType] = true;
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			leftMouse = 1;
			break;
		case Common::EVENT_KEYDOWN: {
			if (event.kbd.keycode == Common::KeyCode::KEYCODE_RETURN || event.kbd.keycode == Common::KeyCode::KEYCODE_KP_ENTER) {
				_hitEnter = true;
			}
			break;
		}
		case Common::EVENT_KEYUP: {
			if (event.kbd.keycode == Common::KeyCode::KEYCODE_RETURN || event.kbd.keycode == Common::KeyCode::KEYCODE_KP_ENTER) {
				_hitEnter = false;
			}
			break;
		}
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
}; // namespace TwinE

} // namespace TwinE

#endif
