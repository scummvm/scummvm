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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/nancy/input.h"

#include "engines/nancy/nancy.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

namespace Nancy {

void InputManager::processEvents() {
    using namespace Common;
    Common::Event event;

    _inputs &= ~(NancyInput::kLeftMouseButtonDown | NancyInput::kLeftMouseButtonUp | NancyInput::kRightMouseButtonDown | NancyInput::kRightMouseButtonUp);
    _otherKbdInput.clear();

    while (_engine->getEventManager()->pollEvent(event)) {
        switch (event.type) {
        case EVENT_KEYDOWN:
            if (event.kbd.keycode == KEYCODE_d && event.kbd.flags & Common::KBD_CTRL) {
                // Launch debug console
                _engine->launchConsole = true;
            } else if (event.kbd.keycode == KEYCODE_q && event.kbd.flags & Common::KBD_CTRL) {
                // Quit
                _engine->quitGame();
            } else {
                // Push all other keyboard events into an array and let getInput() callers handle them
                _otherKbdInput.push_back(event.kbd);
            }
            break;
        case EVENT_CUSTOM_ENGINE_ACTION_START:
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
            case kNancyActionRequestCheatMenu:
                _engine->callCheatMenu(false);
                break;
            case kNancyActionRequestEventMenu:
                _engine->callCheatMenu(true);
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
}

NancyInput InputManager::getInput() const {
    NancyInput ret;
    ret.input = _inputs;
    if (_mouseEnabled) {
        ret.mousePos = _engine->getEventManager()->getMousePos();
    } else {
        ret.eatMouseInput();
    }
    ret.otherKbdInput = _otherKbdInput;
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
	Keymap *debugKeymap = new Keymap(Keymap::kKeymapTypeGame, "nancy-debug", "Nancy Drew - Debug/Cheat Shortcuts");
	Action *act;

	act = new Action(kStandardActionInteract, U32String("Left Click Interact"));
    act->setLeftClickEvent();
	act->setCustomEngineActionEvent(kNancyActionLeftClick);
    act->addDefaultInputMapping("MOUSE_LEFT");
    act->addDefaultInputMapping("JOY_A");
	mainKeymap->addAction(act);

    act = new Action("RCLK", U32String("Right Click Interact"));
    act->setRightClickEvent();
	act->setCustomEngineActionEvent(kNancyActionRightClick);
    act->addDefaultInputMapping("MOUSE_RIGHT");
    act->addDefaultInputMapping("JOY_B");
	mainKeymap->addAction(act);

    act = new Action(kStandardActionMoveUp, U32String("Move up"));
	act->setCustomEngineActionEvent(kNancyActionMoveUp);
    act->addDefaultInputMapping("UP");
    act->addDefaultInputMapping("JOY_UP");
	mainKeymap->addAction(act);

    act = new Action(kStandardActionMoveDown, U32String("Move down"));
	act->setCustomEngineActionEvent(kNancyActionMoveDown);
    act->addDefaultInputMapping("DOWN");
    act->addDefaultInputMapping("JOY_DOWN");
	mainKeymap->addAction(act);

    act = new Action(kStandardActionMoveLeft, U32String("Move left"));
	act->setCustomEngineActionEvent(kNancyActionMoveLeft);
    act->addDefaultInputMapping("LEFT");
    act->addDefaultInputMapping("JOY_LEFT");
	mainKeymap->addAction(act);

    act = new Action(kStandardActionMoveRight, U32String("Move right"));
	act->setCustomEngineActionEvent(kNancyActionMoveRight);
    act->addDefaultInputMapping("RIGHT");
    act->addDefaultInputMapping("JOY_RIGHT");
	mainKeymap->addAction(act);

    act = new Action("FASTM", U32String("Fast move modifier"));
	act->setCustomEngineActionEvent(kNancyActionMoveFast);
    act->addDefaultInputMapping("LCTRL");
    act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	mainKeymap->addAction(act);

    // Debug shortcuts

    act = new Action("FASTC", U32String("Toggle fast conversation mode"));
	act->setCustomEngineActionEvent(kNancyActionFastConvoToggle);
    act->addDefaultInputMapping("C+S+TAB+f");
	debugKeymap->addAction(act);

    act = new Action("ENDC", U32String("Toggle end conversation mode"));
	act->setCustomEngineActionEvent(kNancyActionEndConvoToggle);
    act->addDefaultInputMapping("C+S+TAB+e");
	debugKeymap->addAction(act);

    act = new Action("MMENU", U32String("Go to main menu"));
	act->setCustomEngineActionEvent(kNancyActionRequestMainMenu);
    act->addDefaultInputMapping("C+S+TAB+F2");
	debugKeymap->addAction(act);

    act = new Action("LDSV", U32String("Go to save/load menu"));
	act->setCustomEngineActionEvent(kNancyActionRequestSaveLoad);
    act->addDefaultInputMapping("C+S+TAB+F3");
	debugKeymap->addAction(act);

    act = new Action("RLDSV", U32String("Reload last save"));
	act->setCustomEngineActionEvent(kNancyActionReloadSave);
    act->addDefaultInputMapping("C+S+TAB+F4");
	debugKeymap->addAction(act);

    act = new Action("SETUP", U32String("Go to setup menu"));
	act->setCustomEngineActionEvent(kNancyActionRequestSetupMenu);
    act->addDefaultInputMapping("C+S+TAB+F6");
	debugKeymap->addAction(act);

    act = new Action("CRED", U32String("Show credits"));
	act->setCustomEngineActionEvent(kNancyActionRequestCredits);
    act->addDefaultInputMapping("C+S+TAB+F7");
	debugKeymap->addAction(act);

    act = new Action("MAP", U32String("Go to map screen"));
	act->setCustomEngineActionEvent(kNancyActionRequestMap);
    act->addDefaultInputMapping("C+S+TAB+F8");
    act->addDefaultInputMapping("C+S+TAB+m");
	debugKeymap->addAction(act);

    act = new Action("CHEAT", U32String("Open general cheat menu"));
	act->setCustomEngineActionEvent(kNancyActionRequestCheatMenu);
    act->addDefaultInputMapping("C+S+TAB+c");
	debugKeymap->addAction(act);

    act = new Action("EVENT", U32String("Open event flags cheat menu"));
	act->setCustomEngineActionEvent(kNancyActionRequestEventMenu);
    act->addDefaultInputMapping("C+S+TAB+v");
	debugKeymap->addAction(act);

	keymaps.push_back(mainKeymap);
    keymaps.push_back(debugKeymap);
}

} // End of namespace Nancy
