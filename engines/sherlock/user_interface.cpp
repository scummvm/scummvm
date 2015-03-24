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

#include "sherlock/user_interface.h"
#include "sherlock/sherlock.h"

namespace Sherlock {

// Main user interface menu control locations
const int MENU_POINTS[12][4] = { 
	{ 13, 153, 72, 165 },
	{ 13, 169, 72, 181 },
	{ 13, 185, 72, 197 },
	{ 88, 153, 152, 165 },
	{ 88, 169, 152, 181 },
	{ 88, 185, 152, 197 },
	{ 165, 153, 232, 165 },
	{ 165, 169, 232, 181 },
	{ 165, 185, 233, 197 },
	{ 249, 153, 305, 165 },
	{ 249, 169, 305, 181 },
	{ 249, 185, 305, 197 } 
};

// Inventory control locations */
const int INVENTORY_POINTS[8][3] = { 
	{ 4, 50, 28 },
	{ 52, 99, 76 },
	{ 101, 140, 122 },
	{ 142, 187, 165 },
	{ 189, 219, 197 },
	{ 221, 251, 233 },
	{ 253, 283, 265 },
	{ 285, 315, 293 } 
};

const char COMMANDS[13] = "LMTPOCIUGJFS";

/*----------------------------------------------------------------*/

UserInterface::UserInterface(SherlockEngine *vm) : _vm(vm) {
	_bgFound = 0;
	_oldBgFound = -1;
	_keycode = Common::KEYCODE_INVALID;
	_helpStyle = 0;
	_menuCounter = 0;
	_menuMode = STD_MODE;
	_help = _oldHelp = 0;
	_lookHelp = 0;
	_key = _oldKey = 0;
	_temp = _oldTemp = 0;
	_invLookFlag = 0;
	_windowOpen = false;
	_oldLook = false;
	_keyboardInput = false;
	_invMode = 0;
	_pause = false;

	_controls = new ImageFile("menu.all");
}

UserInterface::~UserInterface() {
	delete _controls;
}

/**
 * Main input handler for the user interface
 */
void UserInterface::handleInput() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;

	if (_menuCounter)
		whileMenuCounter();

	Common::Point pt = events.mousePos();
	_bgFound = scene.findBgShape(Common::Rect(pt.x, pt.y, pt.x + 1, pt.y + 1));
	_keycode = Common::KEYCODE_INVALID;

	// Check kbd and set the mouse released flag if Enter or space is pressed.
	// Otherwise, the pressed key is stored for later use
	if (events.kbHit()) {
		Common::KeyState keyState = events.getKey();

		if (keyState.keycode == Common::KEYCODE_x && keyState.flags & Common::KBD_ALT) {
			_vm->quitGame();
			return;
		} else if (keyState.keycode == Common::KEYCODE_SPACE ||
				keyState.keycode == Common::KEYCODE_RETURN) {
			events._pressed = events._oldButtons = 0;
			_keycode = Common::KEYCODE_INVALID;
		}
	}

	// Do button highlighting check
	if (!_vm->_scriptMoreFlag) {	// Don't if scripts are running
		if (((events._rightPressed || events._rightReleased) && _helpStyle) ||
				(!_helpStyle && !_menuCounter)) {
			// Handle any default commands if we're in STD_MODE
			if (_menuMode == STD_MODE) {
				if (pt.y < CONTROLS_Y &&
					(events._rightPressed || (!_helpStyle && !events._released)) &&
					(_bgFound != -1) && (_bgFound < 1000) &&
					(scene._bgShapes[_bgFound]._defaultCommand ||
					scene._bgShapes[_bgFound]._description[0])) {
					// If there is no default command, so set it to Look
					if (scene._bgShapes[_bgFound]._defaultCommand)
						_help = scene._bgShapes[_bgFound]._defaultCommand - 1;
					else
						_help = 0;

					// Reset 'help' if it is an invalid command
					if (_help > 5)
						_help = -1;
				} else if (pt.y < CONTROLS_Y &&
					((events._rightReleased && _helpStyle) || (events._released && !_helpStyle)) &&
					(_bgFound != -1 && _bgFound < 1000) &&
					(scene._bgShapes[_bgFound]._defaultCommand || 
					scene._bgShapes[_bgFound]._description[0])) {
					// If there is no default command, set it to Look
					if (scene._bgShapes[_bgFound]._defaultCommand)
						_menuMode = (MenuMode)scene._bgShapes[_bgFound]._defaultCommand;
					else
						_menuMode = LOOK_MODE;
					events._released = true;
					events._pressed = events._oldButtons = false;
					_help = _oldHelp = -1;
				
					if (_menuMode == LOOK_MODE) {
						// Set the flag to tell the game that this was a right-click
						// call to look and should exit without the look button being pressed
						_lookHelp = true;
					}
				} else {
					_help = -1;
				}

				// Check if highlighting a different button than last time
				if (_help != _oldHelp) {
					// If another button was highlighted previously, restore it
					if (_oldHelp != -1)
						restoreButton(_oldHelp);

					// If we're highlighting a new button, then draw it pressed
					if (_help != -1)
						depressButton(_help);

					_help = _oldHelp;
				}

				if (_bgFound != _oldBgFound) {
					_infoFlag = true;
					clearInfo();

					if (_help != -1 && (scene._bgShapes[_bgFound]._description[0] != 32 &&
							scene._bgShapes[_bgFound]._description[0]))
						screen.print(Common::Point(0, INFO_LINE + 1),
						INFO_FOREGROUND, INFO_BACKGROUND, "%s",
						scene._bgShapes[_bgFound]._description);
				}
			} else {
				// We're not in STD_MODE
				// If there isn't a window open, then revert back to STD_MODE
				if (!_windowOpen && events._rightReleased) {
					// Restore all buttons
					for (int idx = 0; idx < 12; ++idx)
						restoreButton(idx);

					_menuMode = STD_MODE;
					_key = _oldKey = -1;
					_temp = _oldTemp = _lookHelp = _invLookFlag = 0;
					events.clearEvents();
				}
			}
		}
	}

	// Reset the old bgshape number if the mouse button is released, so that
	// it can e re-highlighted when we come back here
	if ((events._rightReleased && _helpStyle) || (events._released && !_helpStyle))
		_oldBgFound = -1;

	// Do routines that should be done before input processing
	switch (_menuMode) {
	case LOOK_MODE:
		if (!_windowOpen) {
			if (events._released && _bgFound >= 0 && _bgFound < 1000) {
				if (!scene._bgShapes[_bgFound]._examine.empty())
					examine();
			} else {
				lookScreen(pt);
			}
		}
		break;

	case MOVE_MODE:
	case OPEN_MODE:
	case CLOSE_MODE:
	case PICKUP_MODE:
		lookScreen(pt);
		break;

	case TALK_MODE:
		if (!_windowOpen) {
			bool personFound;

			if (_bgFound >= 1000) {
				personFound = false;
				if (!events._released)
					lookScreen(pt);
			} else {
				personFound = scene._bgShapes[_bgFound]._aType == PERSON && _bgFound != -1;
			}

			if (events._released && personFound)
				talk.talk(_bgFound);
			else if (personFound)
				lookScreen(pt);
			else if (_bgFound < 1000)
				clearInfo();
		}
		break;

	case USE_MODE:
	case GIVE_MODE:
	case INV_MODE:
		if (_invMode == 1 || _invMode == 2 || _invMode == 3) {
			if (pt.y < CONTROLS_Y)
				lookInv();
			else
				lookScreen(pt);
		}
		break;

	default:
		break;
	}

	//
	// Do input processing
	//
	if (events._pressed || events._released || events._rightPressed || 
			_keycode != Common::KEYCODE_INVALID || _pause) {
		if (((events._released && (_helpStyle || _help == -1)) || (events._rightReleased && !_helpStyle)) &&
				(pt.y <= CONTROLS_Y) && (_menuMode == STD_MODE)) {
			// The mouse was clicked in the playing area with no action buttons down.
			// Check if the mouse was clicked in a script zone. If it was,
			// then execute the script. Otherwise, walk to the given position
			if (scene.checkForZones(pt, SCRIPT_ZONE) != 0) {
				// Mouse clicked in script zone
				events._pressed = events._released = false;
			} else {
				people._walkDest = pt;
				people._allowWalkAbort = false;
				people.goAllTheWay();
			}

			if (_oldKey != -1) {
				restoreButton(_oldTemp);
				_oldKey = -1;
			}
		}

		// Handle action depending on selected mode
		switch (_menuMode) {
		case LOOK_MODE:
			if (_windowOpen)
				doLookControl();
			break;

		case MOVE_MODE:
			doMiscControl(ALLOW_MOVEMENT);
			break;

		case TALK_MODE:
			if (_windowOpen)
				doTalkControl();
			break;

		case OPEN_MODE:
			doMiscControl(ALLOW_OPEN);
			break;

		case CLOSE_MODE:
			doMiscControl(ALLOW_CLOSE);
			break;

		case PICKUP_MODE:
			doPickControl();
			break;

		case USE_MODE:
		case GIVE_MODE:
		case INV_MODE:
			doInvControl();
			break;

		case FILES_MODE:
			doEnvControl();
			break;

		default:
			break;
		}

		// As long as there isn't an open window, do main input processing.
		// Windows are opened when in TALK, USE, INV, and GIVE modes
		if ((!_windowOpen && !_menuCounter && pt.y > CONTROLS_Y) || 
				_keycode != Common::KEYCODE_INVALID) {
			if (events._pressed || events._released || _pause ||
					_keycode != Common::KEYCODE_INVALID)
				doMainControl();
		}

		if (pt.y < CONTROLS_Y && events._pressed && _oldTemp != (_menuMode - 1) && _oldKey != -1)
			restoreButton(_oldTemp);
	}
}

/**
 * Draws the image for a user interface button in the down/pressed state.
 */
void UserInterface::depressButton(int num) {
	Screen &screen = *_vm->_screen;
	Common::Point pt(MENU_POINTS[num][0], MENU_POINTS[num][1]);

	Graphics::Surface &s = (*_controls)[num]._frame;
	screen._backBuffer.transBlitFrom(s, pt);
	screen.slamArea(pt.x, pt.y, pt.x + s.w, pt.y + s.h);
}

/**
 * Draws the image for the given user interface button in the up
 * (not selected) position
 */
void UserInterface::restoreButton(int num) {
	Screen &screen = *_vm->_screen;
	Common::Point pt(MENU_POINTS[num][0], MENU_POINTS[num][1]);
	Graphics::Surface &frame = (*_controls)[num]._frame;

	screen._backBuffer.blitFrom(screen._backBuffer2, pt,
		Common::Rect(pt.x, pt.y, pt.x + 90, pt.y + 19));
	screen._backBuffer.blitFrom(screen._backBuffer, pt,
		Common::Rect(pt.x, pt.y, pt.x + frame.w, pt.y + frame.h));
	
	if (!_menuCounter) {
		_infoFlag++;
		clearInfo();
	}
}

/**
 * If he mouse button is pressed, then calls depressButton to draw the button
 * as pressed; if not, it will show it as released with a call to "restoreButton".
 */
void UserInterface::pushButton(int num) {
	Events &events = *_vm->_events;
	_oldKey = -1;

	if (!events._released) {
		if (_oldHelp != -1)
			restoreButton(_oldHelp);
		if (_help != -1)
			restoreButton(_help);

		depressButton(num);
		events.wait(6);
	}

	restoreButton(num);
}

/**
 * By the time this method has been called, the graphics for the button change
 * have already been drawn. This simply takes care of switching the mode around
 * accordingly
 */
void UserInterface::toggleButton(int num) {
	Screen &screen = *_vm->_screen;
	
	if (_menuMode != (num + 1)) {
		_menuMode = (MenuMode)(num + 1);
		_oldKey = COMMANDS[num];
		_oldTemp = num;

		if (_keyboardInput) {
			if (_oldHelp != -1 && _oldHelp != num)
				restoreButton(_oldHelp);
			if (_help != -1 && _help != num)
				restoreButton(_help);

			_keyboardInput = false;

			Graphics::Surface &s = (*_controls)[num]._frame;
			Common::Point pt(MENU_POINTS[num][0], MENU_POINTS[num][1]);
			screen._backBuffer.transBlitFrom(s, pt);
			screen.slamArea(pt.x, pt.y, pt.x + s.w, pt.y + s.h);
		}
	} else {
		_menuMode = STD_MODE;
		_oldKey = -1;
		restoreButton(num);
	}
}


/**
 * Clears the info line of the screen
 */
void UserInterface::clearInfo() {
	if (_infoFlag) {
		_vm->_screen->fillRect(16, INFO_LINE, SHERLOCK_SCREEN_WIDTH - 20, INFO_LINE + 9,
			INFO_BLACK);
		_infoFlag = false;
		_oldLook = -1;
	}
}

/**
 * Handles counting down whilst checking for input, then clears the info line.
 */
void UserInterface::whileMenuCounter() {
	if (!(--_menuCounter) || _vm->_events->checkInput()) {
		_menuCounter = 0;
		++_infoFlag;
		clearInfo();
	}
}

void UserInterface::examine() {
	// TODO
}

void UserInterface::lookScreen(const Common::Point &pt) {
	// TODO
}

void UserInterface::lookInv() {
	// TODO
}

void UserInterface::doEnvControl() {
	// TODO
}

void UserInterface::doInvControl() {
	// TODO
}

void UserInterface::doLookControl() {
	// TODO
}

void UserInterface::doMainControl() {
	// TODO
}

void UserInterface::doMiscControl(int allowed) {
	// TODO
}

void UserInterface::doPickControl() {
	// TODO
}

void UserInterface::doTalkControl() {
	// TODO
}

} // End of namespace Sherlock
