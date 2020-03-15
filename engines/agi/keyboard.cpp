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

#include "common/events.h"
#include "gui/predictivedialog.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/text.h"

namespace Agi {

//
// IBM-PC keyboard scancodes
//
const uint8 scancodeTable[26] = {
	30,         // A
	48,         // B
	46,         // C
	32,         // D
	18,         // E
	33,         // F
	34,         // G
	35,         // H
	23,         // I
	36,         // J
	37,         // K
	38,         // L
	50,         // M
	49,         // N
	24,         // O
	25,         // P
	16,         // Q
	19,         // R
	31,         // S
	20,         // T
	22,         // U
	47,         // V
	17,         // W
	45,         // X
	21,         // Y
	44          // Z
};

void AgiEngine::processScummVMEvents() {
	Common::Event event;
	int key = 0;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_PREDICTIVE_DIALOG:
			showPredictiveDialog();
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (_game.mouseEnabled) {
				key = AGI_MOUSE_BUTTON_LEFT;
				_mouse.button = kAgiMouseButtonLeft;
				keyEnqueue(key);
				_mouse.pos.x = event.mouse.x;
				_mouse.pos.y = event.mouse.y;
			}
			break;
		case Common::EVENT_RBUTTONDOWN:
			if (_game.mouseEnabled) {
				key = AGI_MOUSE_BUTTON_RIGHT;
				_mouse.button = kAgiMouseButtonRight;
				keyEnqueue(key);
				_mouse.pos.x = event.mouse.x;
				_mouse.pos.y = event.mouse.y;
			}
			break;
		case Common::EVENT_WHEELUP:
			if (_game.mouseEnabled) {
				key = AGI_MOUSE_WHEEL_UP;
				keyEnqueue(key);
			}
			break;
		case Common::EVENT_WHEELDOWN:
			if (_game.mouseEnabled) {
				key = AGI_MOUSE_WHEEL_DOWN;
				keyEnqueue(key);
			}
			break;
		case Common::EVENT_MOUSEMOVE:
			if (_game.mouseEnabled) {
				_mouse.pos.x = event.mouse.x;
				_mouse.pos.y = event.mouse.y;

				if (!_game.mouseFence.isEmpty()) {
					if (_mouse.pos.x < _game.mouseFence.left)
						_mouse.pos.x = _game.mouseFence.left;
					if (_mouse.pos.x > _game.mouseFence.right)
						_mouse.pos.x = _game.mouseFence.right;
					if (_mouse.pos.y < _game.mouseFence.top)
						_mouse.pos.y = _game.mouseFence.top;
					if (_mouse.pos.y > _game.mouseFence.bottom)
						_mouse.pos.y = _game.mouseFence.bottom;

					g_system->warpMouse(_mouse.pos.x, _mouse.pos.y);
				}
			}

			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			if (_game.mouseEnabled) {
				_mouse.button = kAgiMouseButtonUp;
				_mouse.pos.x = event.mouse.x;
				_mouse.pos.y = event.mouse.y;
			}
			break;
		case Common::EVENT_KEYDOWN:
			key = event.kbd.ascii;
			if (event.kbd.keycode >= Common::KEYCODE_KP0 && event.kbd.keycode <= Common::KEYCODE_KP9) {
				if (!(event.kbd.flags & Common::KBD_NUM)) {
					// HACK: Num-Lock not enabled
					// We shouldn't get a valid ascii code in these cases. We fix it here, so that cursor keys
					// on the numpad work properly.
					key = 0;
				}
			}

			if ((key) && (key <= 0xFF)) {
				// No special key, directly accept it
				// Is ISO-8859-1, we need lower 128 characters only, which is plain ASCII, so no mapping required
				if (Common::isAlpha(key)) {
					// Key is A-Z.
					// Map Ctrl-A to 1, Ctrl-B to 2, etc.
					if (event.kbd.flags & Common::KBD_CTRL) {
						key = toupper(key) - 'A' + 1;
					} else if (event.kbd.flags & Common::KBD_ALT) {
						// Map Alt-A, Alt-B etc. to special scancode values according to an internal scancode table.
						key = scancodeTable[toupper(key) - 'A'] << 8;
					}
				}
			} else {
				key = 0;
				switch (event.kbd.keycode) {
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_KP4:
					if (_allowSynthetic || !event.kbdRepeat)
						key = AGI_KEY_LEFT;
					break;
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_KP6:
					if (_allowSynthetic || !event.kbdRepeat)
						key = AGI_KEY_RIGHT;
					break;
				case Common::KEYCODE_UP:
				case Common::KEYCODE_KP8:
					if (_allowSynthetic || !event.kbdRepeat)
						key = AGI_KEY_UP;
					break;
				case Common::KEYCODE_DOWN:
				case Common::KEYCODE_KP2:
					if (_allowSynthetic || !event.kbdRepeat)
						key = AGI_KEY_DOWN;
					break;
				case Common::KEYCODE_PAGEUP:
				case Common::KEYCODE_KP9:
					if (_allowSynthetic || !event.kbdRepeat)
						key = AGI_KEY_UP_RIGHT;
					break;
				case Common::KEYCODE_PAGEDOWN:
				case Common::KEYCODE_KP3:
					if (_allowSynthetic || !event.kbdRepeat)
						key = AGI_KEY_DOWN_RIGHT;
					break;
				case Common::KEYCODE_HOME:
				case Common::KEYCODE_KP7:
					if (_allowSynthetic || !event.kbdRepeat)
						key = AGI_KEY_UP_LEFT;
					break;
				case Common::KEYCODE_END:
				case Common::KEYCODE_KP1:
					if (_allowSynthetic || !event.kbdRepeat)
						key = AGI_KEY_DOWN_LEFT;
					break;
				case Common::KEYCODE_KP5:
					key = AGI_KEY_STATIONARY;
					break;
				case Common::KEYCODE_F1:
					key = AGI_KEY_F1;
					break;
				case Common::KEYCODE_F2:
					key = AGI_KEY_F2;
					break;
				case Common::KEYCODE_F3:
					key = AGI_KEY_F3;
					break;
				case Common::KEYCODE_F4:
					key = AGI_KEY_F4;
					break;
				case Common::KEYCODE_F5:
					key = AGI_KEY_F5;
					break;
				case Common::KEYCODE_F6:
					key = AGI_KEY_F6;
					break;
				case Common::KEYCODE_F7:
					key = AGI_KEY_F7;
					break;
				case Common::KEYCODE_F8:
					key = AGI_KEY_F8;
					break;
				case Common::KEYCODE_F9:
					key = AGI_KEY_F9;
					break;
				case Common::KEYCODE_F10:
					key = AGI_KEY_F10;
					break;
				case Common::KEYCODE_F11:
					key = AGI_KEY_F11;
					break;
				case Common::KEYCODE_F12:
					key = AGI_KEY_F12;
					break;
				case Common::KEYCODE_KP_ENTER:
					key = AGI_KEY_ENTER;
					break;
				default:
					break;
				}

				switch (event.kbd.keycode) {
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_UP:
				case Common::KEYCODE_DOWN:
				case Common::KEYCODE_HOME:
				case Common::KEYCODE_END:
				case Common::KEYCODE_PAGEUP:
				case Common::KEYCODE_PAGEDOWN:
				case Common::KEYCODE_KP4:
				case Common::KEYCODE_KP6:
				case Common::KEYCODE_KP8:
				case Common::KEYCODE_KP2:
				case Common::KEYCODE_KP9:
				case Common::KEYCODE_KP3:
				case Common::KEYCODE_KP7:
				case Common::KEYCODE_KP1:
					_keyHoldModeLastKey = event.kbd.keycode;
					break;
				default:
					break;
				}
			}
			if (key)
				keyEnqueue(key);
			break;

		case Common::EVENT_KEYUP:
			if (_keyHoldMode) {
				// Original AGI actually created direction events in here
				// but only in case the last pressed cursor key was released, in other cases it did nothing.
				// So when you pressed and held down left and then pressed up, and then released left,
				// direction wouldn't be changed at all.
				//
				// We don't create direction events in here, that's why we create a stationary event instead,
				// which will result in a direction change to 0 in handleController().
				switch (event.kbd.keycode) {
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_UP:
				case Common::KEYCODE_DOWN:
				case Common::KEYCODE_HOME:
				case Common::KEYCODE_END:
				case Common::KEYCODE_PAGEUP:
				case Common::KEYCODE_PAGEDOWN:
				case Common::KEYCODE_KP4:
				case Common::KEYCODE_KP6:
				case Common::KEYCODE_KP8:
				case Common::KEYCODE_KP2:
				case Common::KEYCODE_KP9:
				case Common::KEYCODE_KP3:
				case Common::KEYCODE_KP7:
				case Common::KEYCODE_KP1:
					if (_keyHoldModeLastKey == event.kbd.keycode) {
						keyEnqueue(AGI_KEY_STATIONARY);
					}
					break;
				default:
					break;
				}
			}
			break;

		default:
			break;
		}
	}
}

/**
 * Raw key grabber.
 * poll_keyboard() is the raw key grabber (above the gfx driver, that is).
 * It handles console keys and insulates AGI from the console. In the main
 * loop, handle_keys() handles keyboard input and ego movement.
 */
int AgiEngine::doPollKeyboard() {
	int key = 0;

	// If a key is ready, rip it
	if (isKeypress()) {
		key = getKeypress();

		debugC(3, kDebugLevelInput, "key %02x pressed", key);
	}

	return key;
}

bool AgiEngine::handleMouseClicks(uint16 &key) {
	// No mouse click? -> exit
	if (key != AGI_MOUSE_BUTTON_LEFT)
		return false;

	if (!cycleInnerLoopIsActive()) {
		// Only do this, when no inner loop is currently active
		Common::Rect displayLineRect = _gfx->getFontRectForDisplayScreen(0, 0, FONT_COLUMN_CHARACTERS, 1);
//		Common::Rect displayLineRect(_gfx->getDisplayScreenWidth(), _gfx->getDisplayFontHeight());

		if (displayLineRect.contains(_mouse.pos)) {
			// Mouse is inside first line of the screen
			if (getFlag(VM_FLAG_MENUS_ACCESSIBLE) && _menu->isAvailable()) {
				_menu->delayedExecuteViaMouse();
				key = 0; // eat event
				return true;
			}
		}

		if (_text->promptIsEnabled()) {
			// Prompt is currently enabled
			int16 promptRow = _text->promptRow_Get();

			displayLineRect.moveTo(0, promptRow * _gfx->getDisplayFontHeight());

			if (displayLineRect.contains(_mouse.pos)) {
				// and user clicked within the line of the prompt
				showPredictiveDialog();

				key = 0; // eat event
				return true;
			}
		}
	}

	if (cycleInnerLoopIsActive()) {
		// inner loop active, check what kind of loop it is. Then process / forward it
		switch (_game.cycleInnerLoopType) {
		case CYCLE_INNERLOOP_GETSTRING:
		case CYCLE_INNERLOOP_GETNUMBER: {
			// process in here
			int16 stringRow, stringColumn, stringMaxLen;

			_text->stringPos_Get(stringRow, stringColumn);
			stringMaxLen = _text->stringGetMaxLen();

			Common::Rect displayRect = _gfx->getFontRectForDisplayScreen(stringColumn, stringRow, stringMaxLen, 1);
			if (displayRect.contains(_mouse.pos)) {
				// user clicked inside the input space
				showPredictiveDialog();

				key = 0; // eat event
				return true;
			}
			break;
		}
		case CYCLE_INNERLOOP_INVENTORY:
			// TODO: forward
			break;

		case CYCLE_INNERLOOP_MENU_VIA_KEYBOARD:
			_menu->mouseEvent(key);
			key = 0; // eat event
			break;

		case CYCLE_INNERLOOP_SYSTEMUI_SELECTSAVEDGAMESLOT:
			// TODO: forward
			break;

		default:
			break;
		}
	}
	return false;
}

bool AgiEngine::handleController(uint16 key) {
	ScreenObjEntry *screenObjEgo = &_game.screenObjTable[SCREENOBJECTS_EGO_ENTRY];

	if (key == 0) // nothing pressed
		return false;

	// This previously skipped processing, when ESC was pressed and called menu directly.
	// This original approach was bad, because games check different flags before actually allowing the
	//  user to enter the menu. We checked a few common flags, like for example the availability of the prompt.
	//  But this stopped the user being able to enter the menu, when the original interpreter actually allowed it.
	//  We now instead implement this feature using another way for those platforms.
	if (key == AGI_KEY_ESCAPE) {
		// Escape pressed, user probably wants to trigger the menu
		// For PC, just passing ASCII code for ESC will normally trigger a controller
		//  and the scripts will then trigger the menu
		switch (getPlatform()) {
		case Common::kPlatformAmiga:
		case Common::kPlatformApple2GS:
		case Common::kPlatformAtariST:
			// For these platforms, the button ESC normally triggered "pause"
			// But users could at the same time trigger the menu by clicking on the status line
			// We check, if menu is currently available and supposed to be accessible.
			// If yes, we do a delayed trigger now, otherwise we continue processing the key just like normal.
			//
			// This is probably the solution with the highest compatibility.
			// Several games also look for special keys see AGI_MENU_TRIGGER_*
			// And then there's also Mixed Up Mother Goose, which actually hooks the ESC key for the regular menu
			//
			// We risk in here of course, that we let the user access the menu, when it shouldn't be possible.
			// I'm not 100% sure if those other interpreters really only check VM_FLAG_MENUS_ACCESSIBLE
			// Needs further investigation.
			if (getFlag(VM_FLAG_MENUS_ACCESSIBLE) && _menu->isAvailable()) {
				// menu is supposed to be accessible and is also available
				_menu->delayedExecuteViaKeyboard();
				return true;
			}
		default:
			break;
		}
		// Otherwise go on and look for the ESC controller
	}

	if ((getGameID() == GID_MH1 || getGameID() == GID_MH2) && (key == AGI_KEY_ENTER) &&
	        (!_text->promptIsEnabled())) {
		key = 0x20; // Set Enter key to Space in Manhunter when prompt is disabled
	}

	debugC(3, kDebugLevelInput, "key = %04x", key);

	for (uint16 curMapping = 0; curMapping < MAX_CONTROLLER_KEYMAPPINGS; curMapping++) {
		if (_game.controllerKeyMapping[curMapping].keycode == key) {
			debugC(3, kDebugLevelInput, "event %d: key press", _game.controllerKeyMapping[curMapping].controllerSlot);
			_game.controllerOccured[_game.controllerKeyMapping[curMapping].controllerSlot] = true;
			return true;
		}
	}

	int16 newDirection = 0;

	switch (key) {
	case AGI_KEY_UP:
		newDirection = 1;
		break;
	case AGI_KEY_DOWN:
		newDirection = 5;
		break;
	case AGI_KEY_LEFT:
		newDirection = 7;
		break;
	case AGI_KEY_RIGHT:
		newDirection = 3;
		break;
	case AGI_KEY_UP_RIGHT:
		newDirection = 2;
		break;
	case AGI_KEY_DOWN_RIGHT:
		newDirection = 4;
		break;
	case AGI_KEY_UP_LEFT:
		newDirection = 8;
		break;
	case AGI_KEY_DOWN_LEFT:
		newDirection = 6;
		break;
	default:
		break;
	}

	if (_game.playerControl) {
		if (!(getFeatures() & GF_AGIMOUSE)) {
			// Handle mouse button events
			if (!_game.mouseHidden) {
				if (key == AGI_MOUSE_BUTTON_LEFT) {
					if (getGameID() == GID_PQ1 && getVar(VM_VAR_CURRENT_ROOM) == 116) {
						// WORKAROUND: Special handling for mouse clicks in the newspaper
						// screen of PQ1. Fixes bug #3018770.
						newDirection = 3;   // fake a right arrow key (next page)

					} else {
						// Click-to-walk mouse interface
						//v->flags |= fAdjEgoXY;
						// setting fAdjEgoXY here will at least break "climbing the log" in SQ2
						// in case you walked to the log by using the mouse, so don't!!!
						int16 egoDestinationX = _mouse.pos.x;
						int16 egoDestinationY = _mouse.pos.y;
						_gfx->translateDisplayPosToGameScreen(egoDestinationX, egoDestinationY);

						screenObjEgo->motionType = kMotionEgo;
						if (egoDestinationX < (screenObjEgo->xSize / 2)) {
							screenObjEgo->move_x = -1;
						} else {
							screenObjEgo->move_x = egoDestinationX - (screenObjEgo->xSize / 2);
						}
						screenObjEgo->move_y        = egoDestinationY;
						screenObjEgo->move_stepSize = screenObjEgo->stepSize;
						return true;
					}
				}
			}
		}
	}

	if (newDirection || key == AGI_KEY_STATIONARY) {
		// TODO: not sure, what original AGI did with AdjEgoXY
		screenObjEgo->flags &= ~fAdjEgoXY;
		if (screenObjEgo->direction == newDirection) {
			setVar(VM_VAR_EGO_DIRECTION, 0);
		} else {
			setVar(VM_VAR_EGO_DIRECTION, newDirection);
		}
		if (_game.playerControl) {
			screenObjEgo->motionType = kMotionNormal;
		}
		return true;
	}

	return false;
}

bool AgiEngine::showPredictiveDialog() {
	GUI::PredictiveDialog predictiveDialog;

	runDialog(predictiveDialog);

	Common::String predictiveResult(predictiveDialog.getResult());
	uint16 predictiveResultLen = predictiveResult.size();
	if (predictiveResult.size()) {
		// User actually entered something
		for (int16 resultPos = 0; resultPos < predictiveResultLen; resultPos++) {
			keyEnqueue(predictiveResult[resultPos]);
		}
		if (!cycleInnerLoopIsActive()) {
			if (_text->promptIsEnabled()) {
				// add ENTER, when the input is probably meant for the prompt
				keyEnqueue(AGI_KEY_ENTER);
			}
		} else {
			switch (_game.cycleInnerLoopType) {
			case CYCLE_INNERLOOP_GETSTRING:
			case CYCLE_INNERLOOP_GETNUMBER:
				// add ENTER, when the input is probably meant for GetString/GetNumber
				keyEnqueue(AGI_KEY_ENTER);
				break;
			default:
				break;
			}
		}
		return true;
	}
	return false;
}

int AgiEngine::waitKey() {
	int key = 0;

	clearKeyQueue();

	debugC(3, kDebugLevelInput, "waiting...");
	while (!(shouldQuit() || _restartGame || getFlag(VM_FLAG_RESTORE_JUST_RAN))) {
		wait(10);
		key = doPollKeyboard();
		if (key == AGI_KEY_ENTER || key == AGI_KEY_ESCAPE || key == AGI_MOUSE_BUTTON_LEFT)
			break;
	}
	return key;
}

int AgiEngine::waitAnyKey() {
	int key = 0;

	clearKeyQueue();

	debugC(3, kDebugLevelInput, "waiting... (any key)");
	while (!(shouldQuit() || _restartGame)) {
		wait(10);
		key = doPollKeyboard();
		if (key)
			break;
	}
	return key;
}

bool AgiEngine::isKeypress() {
	processScummVMEvents();
	return _keyQueueStart != _keyQueueEnd;
}

int AgiEngine::getKeypress() {
	int k;

	while (_keyQueueStart == _keyQueueEnd)  // block
		wait(10);

	keyDequeue(k);

	return k;
}

void AgiEngine::clearKeyQueue() {
	while (isKeypress()) {
		getKeypress();
	}
}

} // End of namespace Agi
