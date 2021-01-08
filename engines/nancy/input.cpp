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
#include "engines/nancy/scene.h"

#include "common/events.h"
#include "common/keyboard.h"

#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"

namespace Nancy {

const int16 InputManager::mapButtonID               = 10000;
const int16 InputManager::textBoxID                 = 10002;
const int16 InputManager::textBoxScrollbarID        = 10003;
const int16 InputManager::helpButtonID              = 10004;
const int16 InputManager::menuButtonID              = 10005;
const int16 InputManager::inventoryScrollbarID      = 10006;
const int16 InputManager::inventoryItemTakeID       = 10007;
const int16 InputManager::inventoryItemReturnID     = 10008;
const int16 InputManager::orderingPuzzleID          = 10009;
const int16 InputManager::orderingPuzzleEndID       = 10010;
const int16 InputManager::rotatingLockPuzzleUpID    = 10011;
const int16 InputManager::rotatingLockPuzzleDownID  = 10012;
const int16 InputManager::rotatingLockPuzzleEndID   = 10013;
const int16 InputManager::leverPuzzleID             = 10014; // not sure abt the lever ones
const int16 InputManager::leverPuzzleEndID          = 10015;
const int16 InputManager::telephoneID               = 10016;
const int16 InputManager::telephoneEndID            = 10017;
const int16 InputManager::sliderPuzzleID            = 10018;
const int16 InputManager::sliderPuzzleEndID         = 10019;
const int16 InputManager::passwordPuzzleEndID       = 10020;

void InputManager::processEvents() {
    using namespace Common;

    isClickValidLMB = false;
    isClickValidRMB = false;

    Common::Event event;

    // TODO consider adding a keymapper
    // TODO add debug key combos
    while (_engine->getEventManager()->pollEvent(event)) {
        switch (event.type) {
            case EVENT_CUSTOM_ENGINE_ACTION_START:
                // TODO add debug shortcuts
                switch (event.customType) {
                    case kNancyActionMoveUp:
                        _inputs |= kMoveUp;
                        _engine->sceneManager->movementDirection |= SceneManager::kUp;
                        break;
                    case kNancyActionMoveDown:
                        _inputs |= kMoveDown;
                        _engine->sceneManager->movementDirection |= SceneManager::kDown;
                        break;
                    case kNancyActionMoveLeft:
                        _inputs |= kMoveLeft;
                        _engine->sceneManager->movementDirection |= SceneManager::kLeft;
                        break;
                    case kNancyActionMoveRight:
                        _inputs |= kMoveRight;
                        _engine->sceneManager->movementDirection |= SceneManager::kRight;
                        break;
                    case kNancyActionMoveFast:
                        _inputs |= kMoveFastModifier;
                        break;
                    case kNancyActionLeftClick:
                        _inputs |= kLeftMouseButton;
                        break;
                    case kNancyActionRightClick:
                        _inputs |= kRightMouseButton;
                        break;
                    default:
                        break;
                }
                break;
            case EVENT_CUSTOM_ENGINE_ACTION_END:
                switch (event.customType) {
                    case kNancyActionMoveUp:
                        _inputs &= ~kMoveUp;
                        break;
                    case kNancyActionMoveDown:
                        _inputs &= ~kMoveDown;
                        break;
                    case kNancyActionMoveLeft:
                        _inputs &= ~kMoveLeft;
                        break;
                    case kNancyActionMoveRight:
                        _inputs &= ~kMoveRight;
                        break;
                    case kNancyActionMoveFast:
                        _inputs &= ~kMoveFastModifier;
                        break;
                    case kNancyActionLeftClick:
                        _inputs &= ~kLeftMouseButton;
                        if (hoveredElementID != -1) {
                            isClickValidLMB = true;
                        }
                        break;
                    case kNancyActionRightClick:
                        _inputs &= ~kRightMouseButton;
                        if (hoveredElementID != -1) {
                            isClickValidRMB = true;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case EVENT_KEYDOWN: 
                switch (event.kbd.keycode) {
                    // Launch debug console
                    case KEYCODE_d:
                        if (event.kbd.flags & Common::KBD_CTRL) 
                            _engine->launchConsole = true;
                        break;
                    // Quit
                    case KEYCODE_q:
                        if (event.kbd.flags & Common::KBD_CTRL) {
                            _engine->quitGame();
                            break;
                        }
                        break;
                    default:
                        break;
                }
                break;
            case EVENT_MOUSEMOVE:
                // TODO add frameMousePos
                break;
            default:
                break;
        }
    }

    // Discard conflicting directions
    byte dir = _engine->sceneManager->movementDirection;
    if ((dir & SceneManager::kUp) && (dir & SceneManager::kDown)) {
        _engine->sceneManager->movementDirection &= !(SceneManager::kUp | SceneManager::kDown);
    }
    if ((dir & SceneManager::kLeft) && (dir & SceneManager::kRight)) {
        _engine->sceneManager->movementDirection &= !(SceneManager::kLeft | SceneManager::kRight);
    }
}

bool InputManager::getInput(InputManager::InputType type) {
    return _inputs & type;
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

	keymaps.push_back(mainKeymap);
    keymaps.push_back(debugKeymap);
}

} // End of namespace Nancy