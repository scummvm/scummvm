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
const char *const PRESS_KEY_FOR_MORE = "Press any Key for More.";
const char *const PRESS_KEY_TO_CONTINUE = "Press any Key to Continue.";

/*----------------------------------------------------------------*/

UserInterface::UserInterface(SherlockEngine *vm) : _vm(vm) {
	_controls = new ImageFile("menu.all");
	_controlPanel = new ImageFile("controls.vgs");
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
	_cNum = 0;
	_selector = _oldSelector = -1;
	_windowBounds = Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH - 1,
		SHERLOCK_SCREEN_HEIGHT - 1);
	_windowStyle = 0;
}

UserInterface::~UserInterface() {
	delete _controls;
	delete _controlPanel;
}

void UserInterface::reset() {
	_oldKey = -1;
	_help = _oldHelp = -1;
	_oldTemp = _temp = -1;
}

/**
 * Draw the user interface onto the screen's back buffers
 */
void UserInterface::drawInterface() {
	Screen &screen = *_vm->_screen;

	screen._backBuffer2.fillRect(0, INFO_LINE, SHERLOCK_SCREEN_WIDTH, INFO_LINE + 10, INFO_BLACK);
	screen._backBuffer.transBlitFrom((*_controlPanel)[0], Common::Point(0, CONTROLS_Y));
	screen._backBuffer2.transBlitFrom((*_controlPanel)[0], Common::Point(0, CONTROLS_Y));
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
					!scene._bgShapes[_bgFound]._description.empty())) {
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
					!scene._bgShapes[_bgFound]._description.empty())) {
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

					_oldHelp = _help;
				}

				if (_bgFound != _oldBgFound || _oldBgFound == -1) {
					_infoFlag = true;
					clearInfo();

					if (_help != -1 && (scene._bgShapes[_bgFound]._description[0] != 32 &&
							scene._bgShapes[_bgFound]._description[0]))
						screen.print(Common::Point(0, INFO_LINE + 1),
						INFO_FOREGROUND, scene._bgShapes[_bgFound]._description.c_str());

					_oldBgFound = _bgFound;
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
			if (scene.checkForZones(pt, SCRIPT_ZONE) != 0 ||
					scene.checkForZones(pt, NOWALK_ZONE) != 0) {
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
	screen.slamArea(pt.x, pt.y, pt.x + frame.w, pt.y + frame.h);
	
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
		_vm->_screen->bar(Common::Rect(16, INFO_LINE, SHERLOCK_SCREEN_WIDTH - 19, 
			INFO_LINE + 10), INFO_BLACK);
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

/**
 * Creates a text window and uses it to display the in-depth description
 * of the highlighted object
 */
void UserInterface::examine() {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;
	Common::Point pt = events.mousePos();
	int canimSpeed;

	if (pt.y < (CONTROLS_Y + 9)) {
		Object &obj = scene._bgShapes[_bgFound];
		
		if (obj._lookcAnim != 0) {
			canimSpeed = ((obj._lookcAnim & 0xe0) >> 5) + 1;
			scene._cAnimFramePause = obj._lookFrames;
			_cAnimStr = obj._examine;
			_cNum = (obj._lookcAnim & 0x1f) - 1;

			scene.startCAnim(_cNum, canimSpeed);
		} else if (obj._lookPosition.y != 0) {
			// Need to walk to the object to be examined
			people.walkToCoords(obj._lookPosition, obj._lookFacing);
		}

		if (!talk._talkToAbort) {
			_cAnimStr = obj._examine;
			if (obj._lookFlag)
				_vm->setFlags(obj._lookFlag);
		}
	} else {
		// Looking at an inventory item
		_cAnimStr = inv[_selector]._examine;
		if (inv[_selector]._lookFlag)
			_vm->setFlags(inv[_selector]._lookFlag);
	}

	if (!talk._talkToAbort) {
		if (!scene._cAnimFramePause)
			printObjectDesc(_cAnimStr, true);
		else
			// description was already printed in startCAnimation
			scene._cAnimFramePause = 0;
	}
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

/**
 * Handles waiting whilst an object's description window is open. 
 */
void UserInterface::doLookControl() {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Screen &screen = *_vm->_screen;

	_key = _oldKey = -1;	
	_keyboardInput = _keycode != Common::KEYCODE_INVALID;

	if (events._released || events._rightReleased || _keyboardInput) {
		// Is an inventory object being looked at?
		if (!_invLookFlag) {
			// Is there any remaining text to display?
			if (!_descStr.empty()) {
				printObjectDesc(_descStr, false);
			} else if (!_lookHelp) {
				// Need to close the window and depress the Look button 
				Common::Point pt(MENU_POINTS[0][0], MENU_POINTS[0][1]);
				screen._backBuffer2.blitFrom((*_controls)[0]._frame, pt);
				banishWindow(true);

				_windowBounds.top = CONTROLS_Y1;
				_key = _oldKey = COMMANDS[LOOK_MODE - 1];
				_temp = _oldTemp = 0;
				_menuMode = LOOK_MODE;
				events.clearEvents();

				// Restore UI
				drawInterface();
			}
		} else {
			// Looking at an inventory object
			// Backup the user interface
			Surface tempSurface(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y1);
			tempSurface.blitFrom(screen._backBuffer2, Common::Point(0, 0),
				Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

			inv.invent(128);
			banishWindow(true);

			// Restore the ui
			screen._backBuffer2.blitFrom(tempSurface, Common::Point(0, CONTROLS_Y1));
		}

		_windowBounds.top = CONTROLS_Y1;
		_key = _oldKey = COMMANDS[LOOK_MODE - 1];
		_temp = _oldTemp = 0;
		events.clearEvents();
		_invLookFlag = false;
		_menuMode = INV_MODE;
		_windowOpen = true;
	}
}

/**
 * Handles input until one of the user interface buttons/commands is selected
 */
void UserInterface::doMainControl() {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Common::Point pt = events.mousePos();

	if ((events._pressed || events._released) && pt.y > CONTROLS_Y) {
		events.clearKeyboard();
		_key = -1;

		// Check whether the mouse is in any of the command areas
		for (_temp = 0; (_temp < 12) && (_key == -1); ++_temp) {
			Common::Rect r(MENU_POINTS[_temp][0], MENU_POINTS[_temp][1],
				MENU_POINTS[_temp][2], MENU_POINTS[_temp][3]);
			if (r.contains(pt))
				_key = COMMANDS[_temp];
		}
		--_temp;
	} else if (_keycode != Common::KEYCODE_INVALID) {
		// Keyboard control
		_keyboardInput = true;

		if (_keycode >= Common::KEYCODE_a && _keycode <= Common::KEYCODE_z) {
			const char *c = strchr(COMMANDS, _keycode);
			_temp = !c ? 12 : c - COMMANDS;
		} else {
			_temp = 12;
		}

		if (_temp == 12)
			_key = -1;
		
		if (events._rightPressed) {
			_temp = 12;
			_key = -1;
		}
	} else if (!events._released) {
		_key = -1;
	}
	
	// Check if the button being pointed to has changed
	if (_oldKey != _key && !_windowOpen) {
		// Clear the info line
		_infoFlag++;
		clearInfo();

		// If there was an old button selected, restore it
		if (_oldKey != -1) {
			_menuMode = STD_MODE;
			restoreButton(_oldTemp);
		}

		// If a new button is being pointed to, highlight it
		if (_key != -1 && _temp < 12 && !_keyboardInput)
			depressButton(_temp);

		// Save the new button selection
		_oldKey = _key;
		_oldTemp = _temp;
	}

	if (!events._pressed && !_windowOpen) {
		switch (_key) {
		case 'L':
			toggleButton(0);
			break;
		case 'M':
			toggleButton(1);
			break;
		case 'T':
			toggleButton(2);
			break;
		case 'P':
			toggleButton(3);
			break;
		case 'O':
			toggleButton(4);
			break;
		case 'C':
			toggleButton(5);
			break;
		case 'I':
			pushButton(6);
			_selector = _oldSelector = -1;
			_menuMode = INV_MODE;
			inv.invent(1);
			break;
		case 'U':
			pushButton(7);
			_selector = _oldSelector = -1;
			_menuMode = USE_MODE;
			inv.invent(2);
			break;
		case 'G':
			pushButton(8);
			_selector = _oldSelector = -1;
			_menuMode = GIVE_MODE;
			inv.invent(3);
			break;
		case 'J':
			pushButton(9);
			_menuMode = JOURNAL_MODE;
			journalControl();
			break;
		case 'F':
			pushButton(10);
			_menuMode = FILES_MODE;
			environment();
			break;
		case 'S':
			pushButton(11);
			_menuMode = SETUP_MODE;
			doControls();
			break;
		default:
			break;
		}

		_help = _oldHelp = _oldBgFound = -1;
	}
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

void UserInterface::journalControl() {
	// TODO
}

void UserInterface::environment() {
	// TODO
}

void UserInterface::doControls() {
	// TODO
}

/**
* Print the description of an object
*/
void UserInterface::printObjectDesc(const Common::String &str, bool firstTime) {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	int savedSelector;

	if (str.hasPrefix("_")) {
		_lookScriptFlag = true;
		events.setCursor(MAGNIFY);
		savedSelector = _selector;
		talk.talkTo(str.c_str() + 1);
		_lookScriptFlag = false;

		if (talk._talkToAbort) {
			events.setCursor(ARROW);
			return;
		}

		// Check if looking at an inventory object
		if (!_invLookFlag) {
			// See if this look was called by a right button click or not
			if (!_lookHelp) {
				// If it wasn't a right button click, then we need depress 
				// the look button before we close the window. So save a copy of the
				// menu area, and draw the controls onto it
				Surface tempSurface((*_controls)[0]._frame.w, (*_controls)[0]._frame.h);
				Common::Point pt(MENU_POINTS[0][0], MENU_POINTS[0][1]);

				tempSurface.blitFrom(screen._backBuffer2, Common::Point(0, 0),
					Common::Rect(pt.x, pt.y, pt.x + tempSurface.w, pt.y + tempSurface.h));
				screen._backBuffer2.transBlitFrom((*_controls)[0]._frame, pt);

				banishWindow(1);
				events.setCursor(MAGNIFY);
				_windowBounds.top = CONTROLS_Y1;
				_key = _oldKey = COMMANDS[LOOK_MODE - 1];
				_temp = _oldTemp = 0;
				_menuMode = LOOK_MODE;
				events.clearEvents();

				screen._backBuffer2.blitFrom(tempSurface, pt);
			} else {
				events.setCursor(ARROW);
				banishWindow(true);
				_windowBounds.top = CONTROLS_Y1;
				_key = _oldKey = -1;
				_temp = _oldTemp = 0;
				_menuMode = STD_MODE;
				_lookHelp = 0;
				events.clearEvents();
			}
		} else {
			// Looking at an inventory object
			_selector = _oldSelector = savedSelector;

			// Reload the inventory graphics and draw the inventory
			inv.loadInv();
			inv.putInv(2);
			inv.freeInv();
			banishWindow(1);

			_windowBounds.top = CONTROLS_Y1;
			_key = _oldKey = COMMANDS[INV_MODE - 1];
			_temp = _oldTemp = 0;
			events.clearEvents();

			_invLookFlag = 0;
			_menuMode = INV_MODE;
			_windowOpen = true;
		}

		return;
	}

	if (firstTime) {
		// Only draw the border on the first call
		_infoFlag = true;
		clearInfo();

		screen.bar(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH,
			CONTROLS_Y1 + 10), BORDER_COLOR);
		screen.bar(Common::Rect(0, CONTROLS_Y + 10, 1, SHERLOCK_SCREEN_HEIGHT - 1),
			BORDER_COLOR);
		screen.bar(Common::Rect(SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y + 10, 
			SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
		screen.bar(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - 1, SHERLOCK_SCREEN_WIDTH,
			SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	}

	// Clear background
	screen.bar(Common::Rect(2, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH - 2,
		SHERLOCK_SCREEN_HEIGHT - 2), INV_BACKGROUND);

	_windowBounds.top = CONTROLS_Y;
	events.clearEvents();

	// Loop through displaying up to five lines
	bool endOfStr = false;
	const char *msgP = str.c_str();
	for (int lineNum = 0; lineNum < 5 && !endOfStr; ++lineNum) {
		int width = 0;
		const char *lineStartP = msgP;

		// Determine how much can be displayed on the line
		do {
			width += screen.charWidth(*msgP++);
		} while (width < 300 && *msgP);
		
		if (*msgP)
			--msgP;
		else
			endOfStr = true;

		// If the line needs to be wrapped, scan backwards to find
		// the end of the previous word as a splitting point
		if (width >= 300) {
			while (*msgP != ' ')
				--msgP;
			endOfStr = false;
		}

		// Print out the line
		Common::String line(lineStartP, msgP);
		screen.gPrint(Common::Point(16, CONTROLS_Y + 12 + lineNum * 9),
			INV_FOREGROUND, line.c_str());

		if (!endOfStr)
			// Start next line at start of the nxet word after space
			++msgP;
	}

	// Handle display depending on whether all the message was shown
	if (!endOfStr) {
		makeButton(Common::Rect(46, CONTROLS_Y, 272, CONTROLS_Y + 10),
			(SHERLOCK_SCREEN_WIDTH - screen.stringWidth(PRESS_KEY_FOR_MORE)) / 2,
			PRESS_KEY_FOR_MORE);
		screen.gPrint(Common::Point((SHERLOCK_SCREEN_WIDTH -
			screen.stringWidth(PRESS_KEY_FOR_MORE)) / 2, CONTROLS_Y),
			COMMAND_FOREGROUND, "P");
		_descStr = msgP;
	} else {
		makeButton(Common::Rect(46, CONTROLS_Y, 272, CONTROLS_Y + 10),
			(SHERLOCK_SCREEN_WIDTH - screen.stringWidth(PRESS_KEY_TO_CONTINUE)) / 2,
			PRESS_KEY_FOR_MORE);
		screen.gPrint(Common::Point((SHERLOCK_SCREEN_WIDTH -
			screen.stringWidth(PRESS_KEY_TO_CONTINUE)) / 2, CONTROLS_Y),
			COMMAND_FOREGROUND, "P");
		_descStr = "";
	}

	if (firstTime) {
		if (!_windowStyle) {
			screen.slamRect(Common::Rect(0, CONTROLS_Y,
				SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
		}
		else {
			Surface tempSurface(SHERLOCK_SCREEN_WIDTH,
				(SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y) + 10);
			Common::Rect r(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);

			tempSurface.blitFrom(screen._backBuffer, Common::Point(0, CONTROLS_Y), r);
			screen._backBuffer.blitFrom(screen._backBuffer2,
				Common::Point(0, CONTROLS_Y), r);

			// Display the window
			summonWindow(tempSurface);
		}

		_selector = _oldSelector = -1;
		_windowOpen = true;
	} else {
		screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH,
			SHERLOCK_SCREEN_HEIGHT));
	}
}

/**
 * Print the previously selected object's decription
 */
void UserInterface::printObjectDesc() {
	printObjectDesc(_cAnimStr, true);
}

/**
 * Draws a button for use in the inventory, talk, and examine dialogs.
 */
void UserInterface::makeButton(const Common::Rect &bounds, int textX,
		const Common::String &str) {
	Screen &screen = *_vm->_screen;

	screen.bar(Common::Rect(bounds.left, bounds.top, bounds.right, bounds.top + 1), BUTTON_TOP);
	screen.bar(Common::Rect(bounds.left, bounds.top, bounds.left + 1, bounds.bottom), BUTTON_TOP);
	screen.bar(Common::Rect(bounds.right - 1, bounds.top, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	screen.bar(Common::Rect(bounds.left + 1, bounds.bottom - 1, bounds.right, bounds.bottom), BUTTON_BOTTOM);
	screen.bar(Common::Rect(bounds.left + 1, bounds.top + 1, bounds.right - 1, bounds.bottom - 1), BUTTON_MIDDLE);

	screen.gPrint(Common::Point(textX, bounds.top), COMMAND_HIGHLIGHTED, "%c", str[0]);
	screen.gPrint(Common::Point(textX + screen.charWidth(str[0]), bounds.top), 
		COMMAND_FOREGROUND, "%s", str.c_str() + 1);
}

/**
 * Displays a passed window by gradually displaying it up vertically
 */
void UserInterface::summonWindow(const Surface &bgSurface) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;

	if (_windowOpen)
		// A window is already open, so can't open another one
		return;

	// Gradually slide up the display of the window
	for (int idx = 1; idx <= (SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y); idx += 2) {
		screen.blitFrom(bgSurface, Common::Point(0, SHERLOCK_SCREEN_HEIGHT - idx),
			Common::Rect(0, 0, bgSurface.w, idx));
		screen.slamRect(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - idx,
			SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

		events.delay(10);
	}

	// Final display of the entire window
	screen._backBuffer.blitFrom(bgSurface, Common::Point(0, CONTROLS_Y),
		Common::Rect(0, 0, bgSurface.w, bgSurface.h));
	screen.slamArea(0, CONTROLS_Y, bgSurface.w, bgSurface.h);

	_windowOpen = true;
}

/**
 * Close a currently open window
 * @param flag	0 = slide old window down, 1 = slide old window up
 */
void UserInterface::banishWindow(bool flag) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;

	if (_windowOpen) {
		if (!flag || !_windowStyle) {
			// Slide window up
			// Only slide the window up if the window style allows it
			if (_windowStyle) {
				for (int idx = 2; idx < (SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y); idx += 2) {
					byte *pSrc = (byte *)screen._backBuffer.getBasePtr(0, CONTROLS_Y);
					Common::copy(pSrc, pSrc + (SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y + idx)
						* SHERLOCK_SCREEN_WIDTH, pSrc - SHERLOCK_SCREEN_WIDTH * 2);
					screen._backBuffer.blitFrom(screen._backBuffer2,
						Common::Point(0, CONTROLS_Y),
						Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_HEIGHT, CONTROLS_Y + idx));

					screen.slamArea(0, CONTROLS_Y + idx - 2, SHERLOCK_SCREEN_WIDTH, 
						SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y - idx + 2);
					events.delay(10);
				}

				// Restore final two old lines
				screen._backBuffer.blitFrom(screen._backBuffer2,
					Common::Point(0, SHERLOCK_SCREEN_HEIGHT - 2),
					Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - 2,
						SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
				screen.slamArea(0, SHERLOCK_SCREEN_HEIGHT - 2, SHERLOCK_SCREEN_WIDTH, 2);
			} else {
				// Restore old area to completely erase window
				screen._backBuffer.blitFrom(screen._backBuffer2,
					Common::Point(0, CONTROLS_Y),
					Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
				screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH,
					SHERLOCK_SCREEN_HEIGHT));
			}
		} else {
			for (int idx = SHERLOCK_SCREEN_HEIGHT - 1 - CONTROLS_Y1; idx >= 0; idx -= 2) {
				byte *pSrc = (byte *)screen._backBuffer.getBasePtr(0, CONTROLS_Y1 + idx);
				byte *pDest = (byte *)screen._backBuffer.getBasePtr(0, CONTROLS_Y1);

				Common::copy(pSrc, pSrc + (SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y1 - idx)
					* SHERLOCK_SCREEN_WIDTH, pDest);
				screen.slamArea(0, CONTROLS_Y1 + idx, SHERLOCK_SCREEN_WIDTH,
					SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y1 - idx);
				events.delay(10);
			}
		}

		_infoFlag = false;
		_windowOpen = false;
	}

	_menuMode = STD_MODE;
}

} // End of namespace Sherlock
