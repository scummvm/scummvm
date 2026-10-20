/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "controller_input.h"
#include "common/events.h"
#include "common/system.h"
#include "scooby/scooby.h"

namespace Scooby {

void ControllerInput::pollControllers(RuntimeState &state) {
	state.PreviousControllerOneInput = state.ControllerOneInput;
	state.ControllerOneInput =
		static_cast<uint8>(~static_cast<uint8>(getControllerButtons(_backButton)));
	state.ControllerOneAccumulatedChanges |= state.ControllerOneChangeBaseline ^ state.ControllerOneInput;
}

bool ControllerInput::isIntroSkipPressed() {
	// Drain the queue through the shared key-state tracking so key releases seen during the intro still
	// clear held buttons. Escape is the skip key here and is deliberately not mapped to a controller button.
	return processEvents(ControllerButtons::None, false);
}

ControllerButtons ControllerInput::getControllerButtons(ControllerButtons backButton) {
	processEvents(backButton, true);
	return getGamepadButtons() | getKeyboardAndMouseButtons(_buttons);
}

ControllerButtons ControllerInput::keyToButton(Common::KeyCode keycode) {
	switch (keycode) {
	case Common::KEYCODE_UP:
		return ControllerButtons::Up;
	case Common::KEYCODE_DOWN:
		return ControllerButtons::Down;
	case Common::KEYCODE_LEFT:
		return ControllerButtons::Left;
	case Common::KEYCODE_RIGHT:
		return ControllerButtons::Right;
	case Common::KEYCODE_z:
		return ControllerButtons::A;
	case Common::KEYCODE_x:
		return ControllerButtons::B;
	case Common::KEYCODE_c:
		return ControllerButtons::C;
	case Common::KEYCODE_RETURN:
		return ControllerButtons::Start;
	default:
		return ControllerButtons::None;
	}
}
Scooby::ControllerButtons ControllerInput::mappedActionEventToButton(uint16 action) {
	switch (action) {
	case kScoobyActionUp:
		return ControllerButtons::Up;
	case kScoobyActionDown:
		return ControllerButtons::Down;
	case kScoobyActionLeft:
		return ControllerButtons::Left;
	case kScoobyActionRight:
		return ControllerButtons::Right;
	case kScoobyActionCancel:
		return ControllerButtons::A;
	case kScoobyActionSelect:
		return ControllerButtons::B;
	case kScoobyActionToggleInventory:
		return ControllerButtons::C;
	case kScoobyActionStart:
		return ControllerButtons::Start;
	default:
		return ControllerButtons::None;
	}
}

bool ControllerInput::processEvents(ControllerButtons backButton, bool mapEscapeToBackButton) {
	bool escapePressed = false;
	Common::Event e;
	while (g_system->getEventManager()->pollEvent(e)) {
		if (e.type == Common::EVENT_KEYDOWN) {
			// if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
			// 	if (e.kbdRepeat) {
			// 		continue;
			// 	}
			//
			// 	escapePressed = true;
			// 	if (mapEscapeToBackButton) {
			// 		// Remember which button this press set, so its release clears the same button even if
			// 		// the back-button mapping changes in between (the sound test switches it to C).
			// 		_escapeButton = backButton;
			// 		_buttons |= backButton;
			// 	}
			// } else {
			// 	// _buttons |= keyToButton(e.kbd.keycode);
			// }
		} else if (e.type == Common::EVENT_KEYUP) {
			// if (e.kbd.keycode == Common::KEYCODE_ESCAPE) {
			// 	_buttons &= buttonMask(_escapeButton);
			// 	_escapeButton = ControllerButtons::None;
			// } else {
			// 	// _buttons &= buttonMask(keyToButton(e.kbd.keycode));
			// }
		} else if (e.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
			if (e.customType == kScoobyActionSkipCutscene) {
				if (e.kbdRepeat) {
					continue;
				}

				escapePressed = true;
				if (mapEscapeToBackButton) {
					// Remember which button this press set, so its release clears the same button even if
					// the back-button mapping changes in between (the sound test switches it to C).
					_escapeButton = backButton;
					_buttons |= backButton;
				}
			} else {
				_buttons |= mappedActionEventToButton(static_cast<ScoobyAction>(e.customType));
			}
		} else if (e.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
			if (e.customType == kScoobyActionSkipCutscene) {
				_buttons &= buttonMask(_escapeButton);
				_escapeButton = ControllerButtons::None;
			} else {
				_buttons &= buttonMask(mappedActionEventToButton(static_cast<ScoobyAction>(e.customType)));
			}
		}
	}

	return escapePressed;
}

ControllerButtons ControllerInput::getKeyboardAndMouseButtons(ControllerButtons backButton) {
	return backButton;
	// ControllerButtons buttons = ControllerButtons::None;
	// if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
	// 	buttons |= ControllerButtons::Up;
	// }
	// if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
	// 	buttons |= ControllerButtons::Down;
	// }
	// if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
	// 	buttons |= ControllerButtons::Left;
	// }
	// if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
	// 	buttons |= ControllerButtons::Right;
	// }
	// if (IsKeyDown(KEY_Y) || IsKeyDown(KEY_Z) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
	// 	buttons |= ControllerButtons::A;
	// }
	// if (IsKeyDown(KEY_X) || IsKeyDown(KEY_SPACE) || IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
	// 	buttons |= ControllerButtons::B;
	// }
	// if (IsKeyDown(KEY_C) || IsKeyDown(KEY_TAB) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {
	// 	buttons |= ControllerButtons::C;
	// }
	// if (IsKeyDown(KEY_ESCAPE) || IsKeyDown(KEY_BACKSPACE)) {
	// 	buttons |= backButton;
	// }
	// if (IsKeyDown(KEY_ENTER)) {
	// 	buttons |= ControllerButtons::Start;
	// }

	// return buttons;
}

ControllerButtons ControllerInput::getGamepadButtons() {
	ControllerButtons buttons = ControllerButtons::None;
	// if (!IsGamepadAvailable(0)) {
	// 	return buttons;
	// }
	//
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
	// 	buttons |= ControllerButtons::Up;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
	// 	buttons |= ControllerButtons::Down;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT)) {
	// 	buttons |= ControllerButtons::Left;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT)) {
	// 	buttons |= ControllerButtons::Right;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT)) {
	// 	buttons |= ControllerButtons::A;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN)) {
	// 	buttons |= ControllerButtons::B;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)) {
	// 	buttons |= ControllerButtons::C;
	// }
	// if (IsGamepadButtonDown(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)) {
	// 	buttons |= ControllerButtons::Start;
	// }

	return buttons;
}

} // namespace Scooby