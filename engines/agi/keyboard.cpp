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
	30,			// A
	48,			// B
	46,			// C
	32,			// D
	18,			// E
	33,			// F
	34,			// G
	35,			// H
	23,			// I
	36,			// J
	37,			// K
	38,			// L
	50,			// M
	49,			// N
	24,			// O
	25,			// P
	16,			// Q
	19,			// R
	31,			// S
	20,			// T
	22,			// U
	47,			// V
	17,			// W
	45,			// X
	21,			// Y
	44			// Z
};

void AgiEngine::processEvents() {
	Common::Event event;
	int key = 0;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_PREDICTIVE_DIALOG: {
			GUI::PredictiveDialog _predictiveDialog;
			_predictiveDialog.runModal();
#if 0
			strcpy(_predictiveResult, _predictiveDialog.getResult());
			if (strcmp(_predictiveResult, "")) {
				if (_game.inputMode == INPUTMODE_NORMAL) {
					//strcpy((char *)_game.inputBuffer, _predictiveResult);
					//handleKeys(KEY_ENTER);
					// TODO: repair predictive
				} else if (_game.inputMode == INPUTMODE_GETSTRING) {
					strcpy(_game.strings[_stringdata.str], _predictiveResult);
					newInputMode(INPUTMODE_NORMAL);
					//_gfx->printCharacter(_stringdata.x + strlen(_game.strings[_stringdata.str]) + 1,
					//		_stringdata.y, ' ', 15, 0);
				} else if (_game.inputMode == INPUTMODE_NONE) {
					for (int n = 0; _predictiveResult[n]; n++)
						keyEnqueue(_predictiveResult[n]);
				}
			}
#endif
			/*
			if (predictiveDialog()) {
				if (_game.inputMode == INPUT_NORMAL) {
					strcpy((char *)_game.inputBuffer, _predictiveResult);
					handleKeys(KEY_ENTER);
				} else if (_game.inputMode == INPUT_GETSTRING) {
					strcpy(_game.strings[_stringdata.str], _predictiveResult);
					newInputMode(INPUT_NORMAL);
					_gfx->printCharacter(_stringdata.x + strlen(_game.strings[_stringdata.str]) + 1,
							_stringdata.y, ' ', _game.colorFg, _game.colorBg);
				} else if (_game.inputMode == INPUT_NONE) {
					for (int n = 0; _predictiveResult[n]; n++)
						keyEnqueue(_predictiveResult[n]);
				}
			}
			*/
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			if (_game.mouseEnabled) {
				key = AGI_MOUSE_BUTTON_LEFT;
				_mouse.button = kAgiMouseButtonLeft;
				keyEnqueue(key);
				_mouse.x = event.mouse.x;
				_mouse.y = event.mouse.y;
			}
			break;
		case Common::EVENT_RBUTTONDOWN:
			if (_game.mouseEnabled) {
				key = AGI_MOUSE_BUTTON_RIGHT;
				_mouse.button = kAgiMouseButtonRight;
				keyEnqueue(key);
				_mouse.x = event.mouse.x;
				_mouse.y = event.mouse.y;
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
				_mouse.x = event.mouse.x;
				_mouse.y = event.mouse.y;

				if (!_game.mouseFence.isEmpty()) {
					if (_mouse.x < _game.mouseFence.left)
						_mouse.x = _game.mouseFence.left;
					if (_mouse.x > _game.mouseFence.right)
						_mouse.x = _game.mouseFence.right;
					if (_mouse.y < _game.mouseFence.top)
						_mouse.y = _game.mouseFence.top;
					if (_mouse.y > _game.mouseFence.bottom)
						_mouse.y = _game.mouseFence.bottom;

					g_system->warpMouse(_mouse.x, _mouse.y);
				}
			}

			break;
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONUP:
			if (_game.mouseEnabled) {
				_mouse.button = kAgiMouseButtonUp;
				_mouse.x = event.mouse.x;
				_mouse.y = event.mouse.y;
			}
			break;
		case Common::EVENT_KEYDOWN:
			if (event.kbd.hasFlags(Common::KBD_CTRL) && event.kbd.keycode == Common::KEYCODE_d) {
				_console->attach();
				break;
			}

			if ((event.kbd.ascii) && (event.kbd.ascii <= 0xFF)) {
				// No special key, directly accept it
				// Is ISO-8859-1, we need lower 128 characters only, which is plain ASCII, so no mapping required
				key = event.kbd.ascii;

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
				switch (key = event.kbd.keycode) {
				case Common::KEYCODE_LEFT:
				case Common::KEYCODE_KP4:
					if (_allowSynthetic || !event.synthetic)
						key = AGI_KEY_LEFT;
					break;
				case Common::KEYCODE_RIGHT:
				case Common::KEYCODE_KP6:
					if (_allowSynthetic || !event.synthetic)
						key = AGI_KEY_RIGHT;
					break;
				case Common::KEYCODE_UP:
				case Common::KEYCODE_KP8:
					if (_allowSynthetic || !event.synthetic)
						key = AGI_KEY_UP;
					break;
				case Common::KEYCODE_DOWN:
				case Common::KEYCODE_KP2:
					if (_allowSynthetic || !event.synthetic)
						key = AGI_KEY_DOWN;
					break;
				case Common::KEYCODE_PAGEUP:
				case Common::KEYCODE_KP9:
					if (_allowSynthetic || !event.synthetic)
						key = AGI_KEY_UP_RIGHT;
					break;
				case Common::KEYCODE_PAGEDOWN:
				case Common::KEYCODE_KP3:
					if (_allowSynthetic || !event.synthetic)
						key = AGI_KEY_DOWN_RIGHT;
					break;
				case Common::KEYCODE_HOME:
				case Common::KEYCODE_KP7:
					if (_allowSynthetic || !event.synthetic)
						key = AGI_KEY_UP_LEFT;
					break;
				case Common::KEYCODE_END:
				case Common::KEYCODE_KP1:
					if (_allowSynthetic || !event.synthetic)
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
			}
			if (key)
				keyEnqueue(key);
			break;

		case Common::EVENT_KEYUP:
			if (_egoHoldKey)
				_game.screenObjTable[SCREENOBJECTS_EGO_ENTRY].direction = 0;

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

int16 AgiEngine::getSpecialMenuControllerSlot() {
	int16 controllerSlotESC = -1;
	int16 controllerSlotSpecial = -1;

	for (uint16 curMapping = 0; curMapping < MAX_CONTROLLER_KEYMAPPINGS; curMapping++) {
		if (_game.controllerKeyMapping[curMapping].keycode == _game.specialMenuTriggerKey) {
			if (controllerSlotSpecial < 0) {
				controllerSlotSpecial = _game.controllerKeyMapping[curMapping].controllerSlot;
			}
		}
		if (_game.controllerKeyMapping[curMapping].keycode == AGI_MENU_TRIGGER_PC) {
			if (controllerSlotESC < 0) {
				controllerSlotESC = _game.controllerKeyMapping[curMapping].controllerSlot;
			}
		}
	}
	if (controllerSlotSpecial >= 0) {
		// special menu controller slot found
		if (controllerSlotSpecial != controllerSlotESC) {
			// not the same as the ESC slot (is the same in Manhunter AppleIIgs, we need to replace "pause"
			if (controllerSlotSpecial >= 10) {
				// slot needs to be at least 10
				// Atari ST SQ1 maps the special key, but doesn't trigger any menu with it
				// the controller slot in this case is 8.
				return controllerSlotSpecial;
			}
		}
	}
	return -1;
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
		// For other platforms, ESC was handled by platforms to trigger "pause game" instead
		// We need to change ESC to a platform specific code to make it work properly.
		//
		// There are exceptions though. Mixed Up Mother Goose on AppleIIgs for example actually sets up
		//  ESC for pause only. That's why we also check, if the key is actually mapped to a controller.
		// For this special case, we actually replace the pause function with a menu trigger.
		// Replacing "pause" all the time wouldn't work out as well, becaue games like KQ1 on Apple IIgs
		//  actually disable "pause" when ego has been killed, which means we wouldn't be able to access
		//  the menu anymore in that case.
		if (_menu->isAvailable()) {
			// menu is actually available
			if (_game.specialMenuTriggerKey) {
				int16 specialMenuControllerSlot = getSpecialMenuControllerSlot();

				if (specialMenuControllerSlot >= 0) {
					// menu trigger found, trigger it now
					_game.controllerOccured[specialMenuControllerSlot] = true;
					return true;
				}
			}
			// Otherwise go on and look for the ESC controller
		}
	}


	// AGI 3.149 games, The Black Cauldron and King's Quest 4 need KEY_ESCAPE to use menus
	// Games with the GF_ESCPAUSE flag need KEY_ESCAPE to pause the game
	//		(key == KEY_ESCAPE && getVersion() != 0x3149 && getGameID() != GID_BC && getGameID() != GID_KQ4 && !(getFeatures() & GF_ESCPAUSE)) )
	//		return false;

	if ((getGameID() == GID_MH1 || getGameID() == GID_MH2) && (key == AGI_KEY_ENTER) &&
			(_game.inputMode == INPUTMODE_NONE)) {
		key = 0x20; // Set Enter key to Space in Manhunter when there's no text input
	}

	debugC(3, kDebugLevelInput, "key = %04x", key);

	for (uint16 curMapping = 0; curMapping < MAX_CONTROLLER_KEYMAPPINGS; curMapping++) {
		if (_game.controllerKeyMapping[curMapping].keycode == key) {
			debugC(3, kDebugLevelInput, "event %d: key press", _game.controllerKeyMapping[curMapping].controllerSlot);
			_game.controllerOccured[_game.controllerKeyMapping[curMapping].controllerSlot] = true;
			return true;
		}
	}

	if (key == AGI_MOUSE_BUTTON_LEFT) {
		// call mouse when click is done on status bar
		// TODO
		// This should be done in a better way as in simulate ESC key
		// Sierra seems to have hardcoded it in some way, but we would have to verify, what flags
		// they checked. The previous way wasn't accurate. Mouse support for menu is missing atm anyway.
		//if ((getflag(VM_FLAG_MENUS_WORK) || (getFeatures() & GF_MENUS)) && _mouse.y <= CHAR_LINES) {
		//	newInputMode(INPUTMODE_MENU);
		//	return true;
		//}
	}

	// Show predictive dialog if the user clicks on input area
	if (key == AGI_MOUSE_BUTTON_LEFT &&
			(int)_mouse.y >= _text->promptRow_Get() * FONT_DISPLAY_HEIGHT &&
			(int)_mouse.y <= (_text->promptRow_Get() + 1) * FONT_DISPLAY_HEIGHT) {
		GUI::PredictiveDialog _predictiveDialog;
		_predictiveDialog.runModal();
#if 0
		strcpy(_predictiveResult, _predictiveDialog.getResult());
		if (strcmp(_predictiveResult, "")) {
			if (_game.inputMode == INPUTMODE_NONE) {
				for (int n = 0; _predictiveResult[n]; n++)
					keyEnqueue(_predictiveResult[n]);
			} else {
				//strcpy((char *)_game.inputBuffer, _predictiveResult);
				//handleKeys(KEY_ENTER);
				// TODO
			}
		}
#endif
		/*
		if (predictiveDialog()) {
			if (_game.inputMode == INPUT_NONE) {
				for (int n = 0; _predictiveResult[n]; n++)
					keyEnqueue(_predictiveResult[n]);
			} else {
				strcpy((char *)_game.inputBuffer, _predictiveResult);
				handleKeys(KEY_ENTER);
			}
		}
		*/
		return true;
	}

	if (_game.playerControl) {
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

		if (!(getFeatures() & GF_AGIMOUSE)) {
			// Handle mouse button events
			if (!_game.mouseHidden) {
				if (key == AGI_MOUSE_BUTTON_LEFT) {
					if (getGameID() == GID_PQ1 && _game.vars[VM_VAR_CURRENT_ROOM] == 116) {
						// WORKAROUND: Special handling for mouse clicks in the newspaper
						// screen of PQ1. Fixes bug #3018770.
						newDirection = 3;	// fake a right arrow key (next page)

					} else {
						// Click-to-walk mouse interface
						//v->flags |= fAdjEgoXY;
						// setting fAdjEgoXY here will at least break "climbing the log" in SQ2
						// in case you walked to the log by using the mouse, so don't!!!
						int16 egoDestinationX = _mouse.x;
						int16 egoDestinationY = _mouse.y;
						adjustPosToGameScreen(egoDestinationX, egoDestinationY);

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

		if (newDirection || key == AGI_KEY_STATIONARY) {
			screenObjEgo->flags &= ~fAdjEgoXY;
			screenObjEgo->direction = screenObjEgo->direction == newDirection ? 0 : newDirection;
			screenObjEgo->motionType = kMotionNormal;
			return true;
		}
	}

	return false;
}

int AgiEngine::waitKey() {
	int key = 0;

	clearKeyQueue();

	debugC(3, kDebugLevelInput, "waiting...");
	while (!(shouldQuit() || _restartGame || getflag(VM_FLAG_RESTORE_JUST_RAN))) {
		pollTimer();
		key = doPollKeyboard();
		if (key == AGI_KEY_ENTER || key == AGI_KEY_ESCAPE || key == AGI_MOUSE_BUTTON_LEFT)
			break;

		pollTimer();
		updateTimer();

		g_system->updateScreen();
	}

	// Have to clear it as original did not set this variable, and we do it in doPollKeyboard()
	// Fixes bug #2823759
	_game.keypress = 0;

	return key;
}

int AgiEngine::waitAnyKey() {
	int key = 0;

	clearKeyQueue();

	debugC(3, kDebugLevelInput, "waiting... (any key)");
	while (!(shouldQuit() || _restartGame)) {
		pollTimer();
		key = doPollKeyboard();
		if (key)
			break;
		g_system->updateScreen();
	}

	// Have to clear it as original did not set this variable, and we do it in doPollKeyboard()
	_game.keypress = 0;

	return key;
}

bool AgiEngine::isKeypress() {
	processEvents();
	return _keyQueueStart != _keyQueueEnd;
}

int AgiEngine::getKeypress() {
	int k;

	while (_keyQueueStart == _keyQueueEnd)	// block
		pollTimer();

	keyDequeue(k);

	return k;
}

void AgiEngine::clearKeyQueue() {
	while (isKeypress()) {
		getKeypress();
	}
}

} // End of namespace Agi
