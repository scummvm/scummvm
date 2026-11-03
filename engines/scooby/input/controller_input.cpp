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

void ControllerInput::pollControllers(RuntimeState &state, InputScreen screen) {
	state.previousControllerOneInput = state.controllerOneInput;
	// Managed-only mouse support: merge the controller presses produced by mouse activity (mouse_input.h).
	_mouse.beginPoll();
	ControllerButtons buttons = getControllerButtons(_backButton);
	buttons |= _mouse.sampleControllerButtons(state, screen);
	state.controllerOneInput = static_cast<uint8>(~static_cast<uint8>(buttons));
	state.controllerOneAccumulatedChanges |= state.controllerOneChangeBaseline ^ state.controllerOneInput;
}

bool ControllerInput::isIntroSkipPressed() {
	// Drain the queue through the shared key-state tracking so key releases seen during the intro still
	// clear held buttons. Escape is the skip key here and is deliberately not mapped to a controller button.
	bool escapePressed = processEvents(ControllerButtons::None, false);
	// Managed-only mouse support: a left click skips like Escape.
	bool clicked = _mouse.takeIntroSkipClick();
	return escapePressed || clicked;
}

ControllerButtons ControllerInput::getControllerButtons(ControllerButtons backButton) {
	processEvents(backButton, true);
	return _buttons;
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
		if (e.type == Common::EVENT_MOUSEMOVE || e.type == Common::EVENT_LBUTTONDOWN ||
				   e.type == Common::EVENT_RBUTTONDOWN || e.type == Common::EVENT_WHEELUP ||
				   e.type == Common::EVENT_WHEELDOWN) {
			// Managed-only mouse support (mouse_input.h).
			_mouse.handleEvent(e);
		} else if (e.type == Common::EVENT_CUSTOM_ENGINE_ACTION_START) {
			// Managed-only mouse support: keyboard or pad use hides the pointer (mouse_input.h).
			_mouse.handleKeyboardOrPadInput();
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

} // namespace Scooby
