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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/translation.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/input.h"

namespace Nancy {

void InputManager::processEvents() {
	using namespace Common;
	Common::Event event;

	_inputs &= ~(NancyInput::kLeftMouseButtonDown | NancyInput::kLeftMouseButtonUp | NancyInput::kRightMouseButtonDown | NancyInput::kRightMouseButtonUp);
	_otherKbdInput.clear();

	while (g_nancy->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case EVENT_KEYDOWN:
			// Push all keyboard events into an array and let getInput() callers handle them
			_otherKbdInput.push_back(event.kbd);
			_inputBeginState = g_nancy->getState();
			break;
		case EVENT_CUSTOM_ENGINE_ACTION_START:
			_inputBeginState = g_nancy->getState();

			switch (event.customType) {
			case kNancyActionLeftClick:
				_inputs |= NancyInput::kLeftMouseButtonDown;
				_inputs |= NancyInput::kLeftMouseButtonHeld;
				break;
			case kNancyActionRightClick:
				_inputs |= NancyInput::kRightMouseButtonDown;
				_inputs |= NancyInput::kRightMouseButtonHeld;
				break;
			case kNancyActionMoveUp:
				_inputs |= NancyInput::kMoveUp;
				break;
			case kNancyActionMoveDown:
				_inputs |= NancyInput::kMoveDown;
				break;
			case kNancyActionMoveLeft:
				_inputs |= NancyInput::kMoveLeft;
				break;
			case kNancyActionMoveRight:
				_inputs |= NancyInput::kMoveRight;
				break;
			case kNancyActionMoveFast:
				_inputs |= NancyInput::kMoveFastModifier;
				break;
			default:
				break;
			}

			break;
		case EVENT_CUSTOM_ENGINE_ACTION_END:
			switch (event.customType) {
			case kNancyActionLeftClick:
				_inputs |= NancyInput::kLeftMouseButtonUp;
				_inputs &= ~NancyInput::kLeftMouseButtonHeld;
				break;
			case kNancyActionRightClick:
				_inputs |= NancyInput::kRightMouseButtonUp;
				_inputs &= ~NancyInput::kRightMouseButtonHeld;
				break;
			case kNancyActionMoveUp:
				_inputs &= ~NancyInput::kMoveUp;
				break;
			case kNancyActionMoveDown:
				_inputs &= ~NancyInput::kMoveDown;
				break;
			case kNancyActionMoveLeft:
				_inputs &= ~NancyInput::kMoveLeft;
				break;
			case kNancyActionMoveRight:
				_inputs &= ~NancyInput::kMoveRight;
				break;
			case kNancyActionMoveFast:
				_inputs &= ~NancyInput::kMoveFastModifier;
				break;
			default:
				break;
			}

			break;
		default:
			break;
		}
	}

	if (_inputs == 0 && _otherKbdInput.size() == 0) {
		_inputBeginState = NancyState::kNone;
	}
}

NancyInput InputManager::getInput() const {
	NancyInput ret;

	// Filter out inputs that began in other states; e.g. if the mouse was pushed and held down
	// in a previous state, the button up event won't fire. Right now we simply block all events
	// until everything's clear, but if that causes problems the fix should be easy.
	if (_inputBeginState == g_nancy->getState()) {
		ret.input = _inputs;
		ret.otherKbdInput = _otherKbdInput;
	} else {
		ret.input = 0;
	}

	if (_mouseEnabled || g_nancy->getState() == NancyState::kCredits) {
		ret.mousePos = g_nancy->getEventManager()->getMousePos();
	} else {
		ret.eatMouseInput();
	}
	
	return ret;
}

void InputManager::forceCleanInput() {
	_inputs = 0;
	_otherKbdInput.clear();
}

void InputManager::initKeymaps(Common::KeymapArray &keymaps) {
	using namespace Common;
	using namespace Nancy;

	Keymap *mainKeymap = new Keymap(Keymap::kKeymapTypeGame, "nancy-main", "Nancy Drew");
	Action *act;

	act = new Action(kStandardActionLeftClick, _("Left Click Interact"));
	act->setLeftClickEvent();
	act->setCustomEngineActionEvent(kNancyActionLeftClick);
	act->addDefaultInputMapping("MOUSE_LEFT");
	act->addDefaultInputMapping("JOY_A");
	mainKeymap->addAction(act);

	act = new Action(kStandardActionRightClick, _("Right Click Interact"));
	act->setRightClickEvent();
	act->setCustomEngineActionEvent(kNancyActionRightClick);
	act->addDefaultInputMapping("MOUSE_RIGHT");
	act->addDefaultInputMapping("JOY_B");
	mainKeymap->addAction(act);

	act = new Action(kStandardActionMoveUp, _("Move up"));
	act->setCustomEngineActionEvent(kNancyActionMoveUp);
	act->addDefaultInputMapping("UP");
	act->addDefaultInputMapping("JOY_UP");
	mainKeymap->addAction(act);

	act = new Action(kStandardActionMoveDown, _("Move down"));
	act->setCustomEngineActionEvent(kNancyActionMoveDown);
	act->addDefaultInputMapping("DOWN");
	act->addDefaultInputMapping("JOY_DOWN");
	mainKeymap->addAction(act);

	act = new Action(kStandardActionMoveLeft, _("Move left"));
	act->setCustomEngineActionEvent(kNancyActionMoveLeft);
	act->addDefaultInputMapping("LEFT");
	act->addDefaultInputMapping("JOY_LEFT");
	mainKeymap->addAction(act);

	act = new Action(kStandardActionMoveRight, _("Move right"));
	act->setCustomEngineActionEvent(kNancyActionMoveRight);
	act->addDefaultInputMapping("RIGHT");
	act->addDefaultInputMapping("JOY_RIGHT");
	mainKeymap->addAction(act);

	act = new Action("FASTM", _("Fast move modifier"));
	act->setCustomEngineActionEvent(kNancyActionMoveFast);
	act->addDefaultInputMapping("LCTRL");
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	mainKeymap->addAction(act);

	keymaps.push_back(mainKeymap);
}

} // End of namespace Nancy
