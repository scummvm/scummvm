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
	{ 4, 50, 29 },
	{ 52, 99, 77 },
	{ 101, 140, 123 },
	{ 142, 187, 166 },
	{ 189, 219, 198 },
	{ 221, 251, 234 },
	{ 253, 283, 266 },
	{ 285, 315, 294 } 
};

const int SETUP_POINTS[12][4]  = { 
	{ 4, 154, 101, 53 },		// Exit
	{ 4, 165, 101, 53 },		// Music Toggle
	{ 219, 165, 316, 268 },		// Voice Toggle
	{ 103, 165, 217, 160 },		// Sound Effects Toggle
	{ 219, 154, 316, 268 },		// Help Button Left/Right
	{ 103, 154, 217, 160 },		// New Font Style
	{ 4, 187, 101, 53 },		// Joystick Toggle
	{ 103, 187, 217, 160 },		// Calibrate Joystick
	{ 219, 176, 316, 268 },		// Fade Style
	{ 103, 176, 217, 160 },		// Window Open Style
	{ 4, 176, 101, 53 }, 		// Portraits Toggle
	{ 219, 187, 316, 268 }		// _key Pad Accel. Toggle
};



const char COMMANDS[13] = "LMTPOCIUGJFS";
const char INVENTORY_COMMANDS[9] = { "ELUG-+,." };
const char *const PRESS_KEY_FOR_MORE = "Press any Key for More.";
const char *const PRESS_KEY_TO_CONTINUE = "Press any Key to Continue.";

const char *const MOPEN[] = { 
	"This cannot be opened", "It is already open", "It is locked", "Wait for Watson", " ", "." 
};
const char *const MCLOSE[] = { 
	"This cannot be closed", "It is already closed", "The safe door is in the way" 
};
const char *const MMOVE[] = { 
	"This cannot be moved", "It is bolted to the floor", "It is too heavy", "The other crate is in the way"
};
const char *const MPICK[] = {
	"Nothing of interest here", "It is bolted down", "It is too big to carry", "It is too heavy",
	"I think a girl would be more your type", "Those flowers belong to Penny", "She's far too young for you!", 
	"I think a girl would be more your type!", "Government property for official use only" 
};
const char *const MUSE[] = { 
	"You can't do that", "It had no effect", "You can't reach it", "OK, the door looks bigger! Happy?", 
	"Doors don't smoke" 
};

const char *const SETUP_STRS0[2] = { "off", "on" };
const char *const SETUP_STRS1[2] = { "Directly", "by Pixel" };
const char *const SETUP_STRS2[2] = { "Left", "Right" };
const char *const SETUP_STRS3[2] = { "Appear", "Slide" };
const char *const SETUP_STRS4[2] = { "Slow", "Fast" };
const char *const SETUP_STRS5[2] = { "Left", "Right" };
const char *const SETUP_NAMES[12] = {
	"Exit", "M", "V", "S", "B", "New Font Style", "J", "Calibrate Joystick", "F", "W", "P", "K"
};

/*----------------------------------------------------------------*/

/**
 * Draws the interface for the settings window
 */
void Settings::drawInteface(bool flag) {
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	UserInterface &ui = *_vm->_ui;
	Common::String tempStr;

	if (!flag) {
		screen._backBuffer1.fillRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y1 + 1), BORDER_COLOR);
		screen._backBuffer1.fillRect(Common::Rect(0, CONTROLS_Y1 + 1, 2, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
		screen._backBuffer1.fillRect(Common::Rect(SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y1 + 1, SHERLOCK_SCREEN_WIDTH,
			SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
		screen._backBuffer1.hLine(0, SHERLOCK_SCREEN_HEIGHT - 1, SHERLOCK_SCREEN_WIDTH - 1, BORDER_COLOR);
		screen._backBuffer1.fillRect(Common::Rect(2, CONTROLS_Y1 + 1, SHERLOCK_SCREEN_WIDTH - 2,
			SHERLOCK_SCREEN_HEIGHT - 2), INV_BACKGROUND);
	}

	screen.makeButton(Common::Rect(SETUP_POINTS[0][0], SETUP_POINTS[0][1], SETUP_POINTS[0][2], SETUP_POINTS[0][1] + 10),
		SETUP_POINTS[0][3] - screen.stringWidth("Exit") / 2, "Exit");

	tempStr = Common::String::format("Music %s", SETUP_STRS0[sound._music]);
	screen.makeButton(Common::Rect(SETUP_POINTS[1][0], SETUP_POINTS[1][1], SETUP_POINTS[1][2], SETUP_POINTS[1][1] + 10), 
		SETUP_POINTS[1][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Voices %s", SETUP_STRS0[sound._voices]);
	screen.makeButton(Common::Rect(SETUP_POINTS[2][0], SETUP_POINTS[2][1], SETUP_POINTS[2][2], SETUP_POINTS[2][1] + 10), 
		SETUP_POINTS[2][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Sound Effects %s", SETUP_STRS0[sound._digitized]);
	screen.makeButton(Common::Rect(SETUP_POINTS[3][0], SETUP_POINTS[3][1], SETUP_POINTS[3][2], SETUP_POINTS[3][1] + 10), 
		SETUP_POINTS[3][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Auto Help %s", SETUP_STRS5[ui._helpStyle]);
	screen.makeButton(Common::Rect(SETUP_POINTS[4][0], SETUP_POINTS[4][1], SETUP_POINTS[4][2], SETUP_POINTS[4][1] + 10), 
		SETUP_POINTS[4][3] - screen.stringWidth(tempStr) / 2, tempStr);
	screen.makeButton(Common::Rect(SETUP_POINTS[5][0], SETUP_POINTS[5][1], SETUP_POINTS[5][2], SETUP_POINTS[5][1] + 10), 
		SETUP_POINTS[5][3] - screen.stringWidth("New Font Style") / 2, "New Font Style");

	// WORKAROUND: We don't support the joystick in ScummVM, so draw the next two buttons as disabled
	tempStr = "Joystick Off";
	screen.makeButton(Common::Rect(SETUP_POINTS[6][0], SETUP_POINTS[6][1], SETUP_POINTS[6][2], SETUP_POINTS[6][1] + 10), 
		SETUP_POINTS[6][3] - screen.stringWidth(tempStr) / 2, tempStr);
	screen.buttonPrint(Common::Point(SETUP_POINTS[6][3], SETUP_POINTS[6][1]), COMMAND_NULL, false, tempStr);

	tempStr = "Calibrate Joystick";
	screen.makeButton(Common::Rect(SETUP_POINTS[7][0], SETUP_POINTS[7][1], SETUP_POINTS[7][2], SETUP_POINTS[7][1] + 10), 
		SETUP_POINTS[7][3] - screen.stringWidth(tempStr) / 2, tempStr);
	screen.buttonPrint(Common::Point(SETUP_POINTS[7][3], SETUP_POINTS[7][1]), COMMAND_NULL, false, tempStr);

	tempStr = Common::String::format("Fade %s", screen._fadeStyle ? "by Pixel" : "Directly");
	screen.makeButton(Common::Rect(SETUP_POINTS[8][0], SETUP_POINTS[8][1], SETUP_POINTS[8][2], SETUP_POINTS[8][1] + 10), 
		SETUP_POINTS[8][3] - screen.stringWidth(tempStr) / 2, tempStr);
	
	tempStr = Common::String::format("Windows %s", ui._windowStyle ? "Slide" : "Appear");
	screen.makeButton(Common::Rect(SETUP_POINTS[9][0], SETUP_POINTS[9][1], SETUP_POINTS[9][2], SETUP_POINTS[9][1] + 10), 
		SETUP_POINTS[9][3] - screen.stringWidth(tempStr) / 2, tempStr);

	tempStr = Common::String::format("Portraits %s", SETUP_STRS0[people._portraitsOn]);
	screen.makeButton(Common::Rect(SETUP_POINTS[10][0], SETUP_POINTS[10][1], SETUP_POINTS[10][2], SETUP_POINTS[10][1] + 10),
		SETUP_POINTS[10][3] - screen.stringWidth(tempStr) / 2, tempStr);
	tempStr = Common::String::format("Key Pad %s", _vm->_keyPadSpeed ? "Fast" : "Slow");

	screen.makeButton(Common::Rect(SETUP_POINTS[11][0], SETUP_POINTS[11][1], SETUP_POINTS[11][2], SETUP_POINTS[11][1] + 10),
		SETUP_POINTS[11][3] - screen.stringWidth(tempStr) / 2, tempStr);

	// Show the window immediately, or slide it on-screen 
	if (!flag) {
		if (!ui._windowStyle) {
			screen.slamRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
		} else {
			ui.summonWindow(true, CONTROLS_Y1);
		}

		ui._windowOpen = true;
	} else {
		screen.slamRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
	}
}

int Settings::drawButtons(const Common::Point &pt, int _key) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	UserInterface &ui = *_vm->_ui;
	int found = -1;
	byte color;
	Common::String tempStr;

	for (int idx = 0; idx < 12; ++idx) {
		if ((pt.x > SETUP_POINTS[idx][0] && pt.x < SETUP_POINTS[idx][2] && pt.y > SETUP_POINTS[idx][1]
				&& pt.y < (SETUP_POINTS[idx][1] + 10) && (events._released || events._released)) 
				|| (_key == SETUP_NAMES[idx][0])) {
			found = idx;
			color = COMMAND_HIGHLIGHTED;
		} else {
			color = COMMAND_FOREGROUND;
		}

		// Print the button text
		switch (idx) {
		case 1: 
			tempStr = Common::String::format("Music %s", SETUP_STRS0[sound._music]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 2: 
			tempStr = Common::String::format("Voices %s", SETUP_STRS0[sound._voices]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 3: 
			tempStr = Common::String::format("Sound Effects %s", SETUP_STRS0[sound._digitized]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 4: 
			tempStr = Common::String::format("Auto Help %s", SETUP_STRS2[ui._helpStyle]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 6: 
			tempStr = "Joystick Off";
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), COMMAND_NULL, true, tempStr);
			break;
		case 7:
			tempStr = "Calibrate Joystick";
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), COMMAND_NULL, true, tempStr);
			break;
		case 8: 
			tempStr = Common::String::format("Fade %s", SETUP_STRS1[screen._fadeStyle]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 9: 
			tempStr = Common::String::format("Windows %s", SETUP_STRS3[ui._windowStyle]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 10: 
			tempStr = Common::String::format("Portraits %s", SETUP_STRS0[people._portraitsOn]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		case 11: 
			tempStr = Common::String::format("Key Pad %s", SETUP_STRS4[_vm->_keyPadSpeed]);
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, tempStr);
			break;
		default: 
			screen.buttonPrint(Common::Point(SETUP_POINTS[idx][3], SETUP_POINTS[idx][1]), color, true, SETUP_NAMES[idx]);
			break;
		}
	}

	return found;
}


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
	_temp1 = 0;
	_invLookFlag = 0;
	_windowOpen = false;
	_oldLook = false;
	_keyboardInput = false;
	_pause = false;
	_cNum = 0;
	_selector = _oldSelector = -1;
	_windowBounds = Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH - 1,
		SHERLOCK_SCREEN_HEIGHT - 1);
	_windowStyle = 1;	// Sliding windows
	_find = 0;
	_oldUse = 0;
	_endKeyActive = true;
	_lookScriptFlag = false;
	_infoFlag = false;
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
void UserInterface::drawInterface(int bufferNum) {
	Screen &screen = *_vm->_screen;

	if (bufferNum & 1)
		screen._backBuffer1.transBlitFrom((*_controlPanel)[0], Common::Point(0, CONTROLS_Y));
	if (bufferNum & 2)
		screen._backBuffer2.transBlitFrom((*_controlPanel)[0], Common::Point(0, CONTROLS_Y));
	if (bufferNum == 3)
		screen._backBuffer2.fillRect(0, INFO_LINE, SHERLOCK_SCREEN_WIDTH, INFO_LINE + 10, INFO_BLACK);
}

/**
 * Main input handler for the user interface
 */
void UserInterface::handleInput() {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
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
	// Otherwise, the pressed _key is stored for later use
	if (events.kbHit()) {
		Common::KeyState keyState = events.getKey();
		_keycode = keyState.ascii;

		if (keyState.keycode == Common::KEYCODE_x && keyState.flags & Common::KBD_ALT) {
			_vm->quitGame();
			events.pollEvents();
			return;
		} else if (keyState.keycode == Common::KEYCODE_SPACE ||
				keyState.keycode == Common::KEYCODE_RETURN) {
			events._pressed = false;
			events._oldButtons = 0;
			_keycode = Common::KEYCODE_INVALID;
		}
	}

	// Do button highlighting check
	if (!talk._scriptMoreFlag) {	// Don't if scripts are running
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

					if (_help != -1 && !scene._bgShapes[_bgFound]._description.empty()
							&& scene._bgShapes[_bgFound]._description[0] != ' ')
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
				personFound = _bgFound != -1 && scene._bgShapes[_bgFound]._aType == PERSON;
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
		if (inv._invMode == 1 || inv._invMode == 2 || inv._invMode == 3) {
			if (pt.y > CONTROLS_Y)
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
			doMiscControl(ALLOW_MOVE);
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
	screen._backBuffer1.transBlitFrom(s, pt);
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

	screen._backBuffer1.blitFrom(screen._backBuffer2, pt,
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
			screen._backBuffer1.transBlitFrom(s, pt);
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
		_vm->_screen->vgaBar(Common::Rect(16, INFO_LINE, SHERLOCK_SCREEN_WIDTH - 19,
			INFO_LINE + 10), INFO_BLACK);
		_infoFlag = false;
		_oldLook = -1;
	}
}

/**
 * Clear any active text window
 */
void UserInterface::clearWindow() {
	if (_windowOpen) {
		_vm->_screen->vgaBar(Common::Rect(3, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2,
			SHERLOCK_SCREEN_HEIGHT - 2), INV_BACKGROUND);
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
			people.walkToCoords(Common::Point(obj._lookPosition.x, obj._lookPosition.y * 100), obj._lookFacing);
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

	if (_invLookFlag) {
		// Dont close the inventory window when starting an examine display, since it's
		// window will slide up to replace the inventory display
		_windowOpen = false;
		_menuMode = LOOK_MODE;
	}

	if (!talk._talkToAbort) {
		if (!scene._cAnimFramePause)
			printObjectDesc(_cAnimStr, true);
		else
			// description was already printed in startCAnimation
			scene._cAnimFramePause = 0;
	}
}

/**
 * Print the name of an object in the scene
 */
void UserInterface::lookScreen(const Common::Point &pt) {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Common::Point mousePos = events.mousePos();
	int temp;
	Common::String tempStr;
	int x, width;

	// Don't display anything for right button command
	if ((events._rightPressed || events._rightPressed) && !events._pressed)
		return;

	if (mousePos.y < CONTROLS_Y && (temp = _bgFound) != -1) {
		if (temp != _oldLook) {
			_infoFlag = true;
			clearInfo();

			if (temp < 1000)
				tempStr = scene._bgShapes[temp]._description;
			else
				tempStr = scene._bgShapes[temp - 1000]._description;

			_infoFlag = true;
			clearInfo();

			// Only print description if there is one
			if (!tempStr.empty() && tempStr[0] != ' ') {
				// If inventory is active and an item is selected for a Use or Give action
				if ((_menuMode == INV_MODE || _menuMode == USE_MODE || _menuMode == GIVE_MODE) &&
						(inv._invMode == 2 || inv._invMode == 3)) {
					int width1 = 0, width2 = 0;

					if (inv._invMode == 2) {
						// Using an object
						x = width = screen.stringWidth("Use ");

						if (temp < 1000 && scene._bgShapes[temp]._aType != PERSON)
							// It's not a person, so make it lowercase
							tempStr.setChar(tolower(tempStr[0]), 0);

						x += screen.stringWidth(tempStr);

						// If we're using an inventory object, add in the width
						// of the object name and the " on "
						if (_selector != -1) {
							width1 = screen.stringWidth(inv[_selector]._name);
							x += width1;
							width2 = screen.stringWidth(" on ");
							x += width2;
						}

						// If the line will be too long, keep cutting off characters
						// until the string will fit
						while (x > 280) {
							x -= screen.charWidth(tempStr.lastChar());
							tempStr.deleteLastChar();
						}

						int xStart = (SHERLOCK_SCREEN_WIDTH - x) / 2;
						screen.print(Common::Point(xStart, INFO_LINE + 1),
							INFO_FOREGROUND, "Use ");

						if (_selector != -1) {
							screen.print(Common::Point(xStart + width, INFO_LINE + 1),
								TALK_FOREGROUND, inv[_selector]._name.c_str());
							screen.print(Common::Point(xStart + width + width1, INFO_LINE + 1),
								INFO_FOREGROUND, " on ");
							screen.print(Common::Point(xStart + width + width1 + width2, INFO_LINE + 1),
								INFO_FOREGROUND, tempStr.c_str());
						} else {
							screen.print(Common::Point(xStart + width, INFO_LINE + 1),
								INFO_FOREGROUND, tempStr.c_str());
						}
					} else if (temp >= 0 && temp < 1000 && _selector != -1 &&
							scene._bgShapes[temp]._aType == PERSON) {
						// Giving an object to a person
						width1 = screen.stringWidth(inv[_selector]._name);
						x = width = screen.stringWidth("Give ");
						x += width1;
						width2 = screen.stringWidth(" to ");
						x += width2;
						x += screen.stringWidth(tempStr);

						// Ensure string will fit on-screen
						while (x > 280) {
							x -= screen.charWidth(tempStr.lastChar());
							tempStr.deleteLastChar();
						}

						int xStart = (SHERLOCK_SCREEN_WIDTH - x) / 2;
						screen.print(Common::Point(xStart, INFO_LINE + 1), 
							INFO_FOREGROUND, "Give ");
						screen.print(Common::Point(xStart + width, INFO_LINE + 1), 
							TALK_FOREGROUND, inv[_selector]._name.c_str());
						screen.print(Common::Point(xStart + width + width1, INFO_LINE + 1), 
							INFO_FOREGROUND, " to ");
						screen.print(Common::Point(xStart + width + width1 + width2, INFO_LINE + 1), 
							INFO_FOREGROUND, tempStr.c_str());
					}
				} else {
					screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, tempStr.c_str());
				}

				_infoFlag = true;
				_oldLook = temp;
			}
		}
	} else {
		clearInfo();
	}
}

/**
 * Gets the item in the inventory the mouse is on and display's it's description
 */
void UserInterface::lookInv() {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Screen &screen = *_vm->_screen;
	Common::Point mousePos = events.mousePos();

	if (mousePos.x > 15 && mousePos.x < 314 && mousePos.y > (CONTROLS_Y1 + 11)
			&& mousePos.y < (SHERLOCK_SCREEN_HEIGHT - 2)) {
		int temp = (mousePos.x - 6) / 52 + inv._invIndex;
		if (temp < inv._holdings) {
			if (temp < inv._holdings) {
				clearInfo();
				screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND,
					inv[temp]._description.c_str());
				_infoFlag = true;
				_oldLook = temp;
			}
		} else {
			clearInfo();
		}
	} else {
		clearInfo();
	}
}

/**
 * Handles input when the file list window is being displayed
 */
void UserInterface::doEnvControl() {
	Events &events = *_vm->_events;
	SaveManager &saves = *_vm->_saves;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	Common::Point mousePos = events.mousePos();
	static const char ENV_COMMANDS[7] = "ELSUDQ";
	byte color;

	_key = _oldKey = -1;
	_keyboardInput = false;
	int found = saves.getHighlightedButton();

	if (events._pressed || events._released)
	{
		events.clearKeyboard();

		// Check for a filename entry being highlighted
		int found1 = 0;
		if ((events._pressed || events._released) && mousePos.y > (CONTROLS_Y + 10))
		{
			for (_selector = 0; (_selector < 5) && !found1; ++_selector)
				if (mousePos.y > (CONTROLS_Y + 11 + _selector * 10) && mousePos.y < (CONTROLS_Y + 21 + _selector * 10))
					found1 = 1;

			if (_selector + saves._savegameIndex - 1 < MAX_SAVEGAME_SLOTS + (saves._envMode != 1))
				_selector = _selector + saves._savegameIndex - 1;
			else
				_selector = -1;

			if (!found1)
				_selector = -1;
		}

		// Handle selecting buttons, if any
		saves.highlightButtons(found);

		if (found == 0 || found == 5)
			saves._envMode = SAVEMODE_NONE;
	}

	if (_keycode) {
		_key = toupper(_keycode);

		// Escape _key will close the dialog
		if (_key == Common::KEYCODE_ESCAPE)
			_key = 'E';

		if (_key == 'E' || _key == 'L' || _key == 'S' || _key == 'U' || _key == 'D' || _key == 'Q') {
			const char *chP = strchr(ENV_COMMANDS, _key);
			int btnIndex = !chP ? -1 : chP - ENV_COMMANDS;
			saves.highlightButtons(btnIndex);
			_keyboardInput = true;

			if (_key == 'E' || _key == 'Q') {
				saves._envMode = SAVEMODE_NONE;
			} else if (_key >= '1' && _key <= '9') {
				_keyboardInput = true;
				_selector = _key - '1';
				if (_selector >= MAX_SAVEGAME_SLOTS + (saves._envMode == 1 ? 0 : 1))
					_selector = -1;

				if (saves.checkGameOnScreen(_selector))
					_oldSelector = _selector;
			} else {
				_selector = -1;
			}
		}
	}

	if (_selector != _oldSelector)  {
		if (_oldSelector != -1 && _oldSelector >= saves._savegameIndex && _oldSelector < (saves._savegameIndex + 5)) {
			screen.print(Common::Point(6, CONTROLS_Y + 12 + (_oldSelector - saves._savegameIndex) * 10), 
				INV_FOREGROUND, "%d.", _oldSelector + 1);
			screen.print(Common::Point(24, CONTROLS_Y + 12 + (_oldSelector - saves._savegameIndex) * 10), 
				INV_FOREGROUND, "%s", saves._savegames[_oldSelector].c_str());
		}

		if (_selector != -1) {
			screen.print(Common::Point(6, CONTROLS_Y + 12 + (_selector - saves._savegameIndex) * 10), 
				TALK_FOREGROUND, "%d.", _selector + 1);
			screen.print(Common::Point(24, CONTROLS_Y + 12 + (_selector - saves._savegameIndex) * 10), 
				TALK_FOREGROUND, "%s", saves._savegames[_selector].c_str());
		}

		_oldSelector = _selector;
	}

	if (events._released || _keyboardInput) {
		if ((found == 0 && events._released) || _key == 'E') {
			banishWindow();
			_windowBounds.top = CONTROLS_Y1;
			
			events._pressed = events._released = _keyboardInput = false;
			_keycode = Common::KEYCODE_INVALID;
		} else if ((found == 1 && events._released) || _key == 'L') {
			saves._envMode = SAVEMODE_LOAD;
			if (_selector != -1) {
				saves.loadGame(_selector + 1);
			}
		} else if ((found == 2 && events._released) || _key == 'S') {
			saves._envMode = SAVEMODE_SAVE;
			if (_selector != -1) {
				if (saves.checkGameOnScreen(_selector))
					_oldSelector = _selector;

				if (saves.getFilename(_selector)) {
					saves.saveGame(_selector + 1, saves._savegames[_selector]);

					banishWindow(1);
					_windowBounds.top = CONTROLS_Y1;
					_key = _oldKey = -1;
					_keycode = Common::KEYCODE_INVALID;
					_keyboardInput = false;
				} else {
					if (!talk._talkToAbort) {
						screen._backBuffer1.fillRect(Common::Rect(6, CONTROLS_Y + 11 + (_selector - saves._savegameIndex) * 10, 
							SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y + 20 + (_selector - saves._savegameIndex) * 10), INV_BACKGROUND);
						screen.gPrint(Common::Point(6, CONTROLS_Y + 11 + (_selector - saves._savegameIndex) * 10), INV_FOREGROUND, 
							"%d.", _selector + 1);
						screen.gPrint(Common::Point(24, CONTROLS_Y + 11 + (_selector - saves._savegameIndex) * 10), INV_FOREGROUND, 
							"%s", saves._savegames[_selector].c_str());
						
						screen.slamArea(6, CONTROLS_Y + 11 + (_selector - saves._savegameIndex) * 10, 311, 10);
						_selector = _oldSelector = -1;
					}
				}
			}
		} else if (((found == 3 && events._released) || _key == 'U') && saves._savegameIndex) {
			bool moreKeys;
			do {
				saves._savegameIndex--;
				screen._backBuffer1.fillRect(Common::Rect(3, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2,
					SHERLOCK_SCREEN_HEIGHT - 1), INV_BACKGROUND);

				for (int idx = saves._savegameIndex; idx < (saves._savegameIndex + 5); ++idx) {
					color = INV_FOREGROUND;
					if (idx == _selector && idx >= saves._savegameIndex && idx < (saves._savegameIndex + 5))
						color = TALK_FOREGROUND;

					screen.gPrint(Common::Point(6, CONTROLS_Y + 11 + (idx - saves._savegameIndex) * 10), color, "%d.", idx + 1);
					screen.gPrint(Common::Point(24, CONTROLS_Y + 11 + (idx - saves._savegameIndex) * 10), color, "%s", saves._savegames[idx].c_str());
				}

				screen.slamRect(Common::Rect(3, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2, SHERLOCK_SCREEN_HEIGHT));

				color = !saves._savegameIndex ? COMMAND_NULL : COMMAND_FOREGROUND;
				screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), color, true, "Up");
				color = (saves._savegameIndex == MAX_SAVEGAME_SLOTS - 5) ? COMMAND_NULL : COMMAND_FOREGROUND;
				screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), color, true, "Down");

				// Check for there are more pending U keys pressed
				moreKeys = false;
				if (events.kbHit()) {
					Common::KeyState keyState = events.getKey();

					_key = toupper(keyState.keycode);
					moreKeys = _key == 'U';
				}
			} while ((saves._savegameIndex) && moreKeys);
		} else if (((found == 4 && events._released) || _key == 'D') && saves._savegameIndex < (MAX_SAVEGAME_SLOTS - 5)) {
			bool moreKeys;
			do {
				saves._savegameIndex++;
				screen._backBuffer1.fillRect(Common::Rect(3, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2, 
					SHERLOCK_SCREEN_HEIGHT - 1), INV_BACKGROUND);

				for (int idx = saves._savegameIndex; idx < (saves._savegameIndex + 5); ++idx) {
					if (idx == _selector && idx >= saves._savegameIndex && idx < (saves._savegameIndex + 5))
						color = TALK_FOREGROUND;
					else
						color = INV_FOREGROUND;

					screen.gPrint(Common::Point(6, CONTROLS_Y + 11 + (idx - saves._savegameIndex) * 10), color,
						"%d.", idx + 1);
					screen.gPrint(Common::Point(24, CONTROLS_Y + 11 + (idx - saves._savegameIndex) * 10), color, 
						"%s", saves._savegames[idx].c_str());
				}
				
				screen.slamRect(Common::Rect(3, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2, SHERLOCK_SCREEN_HEIGHT));

				color = (!saves._savegameIndex) ? COMMAND_NULL : COMMAND_FOREGROUND;
				screen.buttonPrint(Common::Point(ENV_POINTS[3][2], CONTROLS_Y), color, true, "Up");

				color = (saves._savegameIndex == MAX_SAVEGAME_SLOTS - 5) ? COMMAND_NULL : COMMAND_FOREGROUND;
				screen.buttonPrint(Common::Point(ENV_POINTS[4][2], CONTROLS_Y), color, true, "Down");

				// Check for there are more pending D keys pressed
				moreKeys = false;
				if (events.kbHit()) {
					Common::KeyState keyState;
					_key = toupper(keyState.keycode);

					moreKeys = _key == 'D';
				}
			} while (saves._savegameIndex < (MAX_SAVEGAME_SLOTS - 5) && moreKeys);
		} else if ((found == 5 && events._released) || _key == 'Q') {
			clearWindow();
			screen.print(Common::Point(0, CONTROLS_Y + 20), INV_FOREGROUND, "Are you sure you wish to Quit ?");
			screen.vgaBar(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y + 10), BORDER_COLOR);

			screen.makeButton(Common::Rect(112, CONTROLS_Y, 150, CONTROLS_Y + 10), 136 - screen.stringWidth("Yes") / 2, "Yes");
			screen.makeButton(Common::Rect(161, CONTROLS_Y, 209, CONTROLS_Y + 10), 184 - screen.stringWidth("No") / 2, "No");
			screen.slamArea(112, CONTROLS_Y, 97, 10);

			do {
				scene.doBgAnim();

				if (talk._talkToAbort)
					return;

				events.pollEventsAndWait();
				events.setButtonState();
				if (events.kbHit()) {
					Common::KeyState keyState = events.getKey();
					_key = toupper(keyState.keycode);

					if (_key == 'X' && (keyState.flags & Common::KBD_ALT) != 0) {
						_vm->quitGame();
						events.pollEvents();
						return;
					}

					if (_key == Common::KEYCODE_ESCAPE)
						_key = 'N';

					if (_key == Common::KEYCODE_RETURN || _key == Common::KEYCODE_SPACE) {
						events._pressed = false;
						events._released = true;
						events._oldButtons = 0;
						_keycode = Common::KEYCODE_INVALID;
					}
				}

				if (events._pressed || events._released) {
					if (mousePos.x > 112 && mousePos.x < 159 && mousePos.y > CONTROLS_Y && mousePos.y < (CONTROLS_Y + 9))
						color = COMMAND_HIGHLIGHTED;
					else
						color = COMMAND_FOREGROUND;
					screen.buttonPrint(Common::Point(136, CONTROLS_Y), color, true, "Yes");

					if (mousePos.x > 161 && mousePos.x < 208 && mousePos.y > CONTROLS_Y && mousePos.y < (CONTROLS_Y + 9))
						color = COMMAND_HIGHLIGHTED;
					else
						color = COMMAND_FOREGROUND;
					screen.buttonPrint(Common::Point(184, CONTROLS_Y), color, true, "No");
				}

				if (mousePos.x > 112 && mousePos.x < 159 && mousePos.y > CONTROLS_Y && mousePos.y < (CONTROLS_Y + 9) && events._released)
					_key = 'Y';

				if (mousePos.x > 161 && mousePos.x < 208 && mousePos.y > CONTROLS_Y && mousePos.y < (CONTROLS_Y + 9) && events._released)
					_key = 'N';
			} while (!_vm->shouldQuit() && _key != 'Y' && _key != 'N');

			if (_key == 'Y') {
				_vm->quitGame();
				events.pollEvents();
				return;
			} else {
				screen.buttonPrint(Common::Point(184, CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "No");
				banishWindow(1);
				_windowBounds.top = CONTROLS_Y1;
				_key = -1;
			}
		} else {
			if (_selector != -1) {
				// Are we already in Load mode?
				if (saves._envMode == SAVEMODE_LOAD) {
					saves.loadGame(_selector + 1);
				} else if (saves._envMode == SAVEMODE_SAVE || _selector == MAX_SAVEGAME_SLOTS) {
					// We're alreaady in save mode, or pointed to an empty save slot
					if (saves.checkGameOnScreen(_selector))
						_oldSelector = _selector;

					if (saves.getFilename(_selector)) {
						saves.saveGame(_selector + 1, saves._savegames[_selector]);
						banishWindow();
						_windowBounds.top = CONTROLS_Y1;
						_key = _oldKey = -1;
						_keycode = Common::KEYCODE_INVALID;
						_keyboardInput = false;
					} else {
						if (!talk._talkToAbort) {
							screen._backBuffer1.fillRect(Common::Rect(6, CONTROLS_Y + 11 + (_selector - saves._savegameIndex) * 10, 
								317, CONTROLS_Y + 20 + (_selector - saves._savegameIndex) * 10), INV_BACKGROUND);
							screen.gPrint(Common::Point(6, CONTROLS_Y + 11 + (_selector - saves._savegameIndex) * 10), 
								INV_FOREGROUND, "%d.", _selector + 1);
							screen.gPrint(Common::Point(24, CONTROLS_Y + 11 + (_selector - saves._savegameIndex) * 10), 
								INV_FOREGROUND, "%s", saves._savegames[_selector].c_str());
							screen.slamArea(6, CONTROLS_Y + 11 + (_selector - saves._savegameIndex) * 10, 311, 10);
							_selector = _oldSelector = -1;
						}
					}
				}
			}
		}
	}
}

/**
 * Handle input whilst the inventory is active
 */
void UserInterface::doInvControl() {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	int colors[8];
	Common::Point mousePos = events.mousePos();

	_key = _oldKey = -1;
	_keyboardInput = false;

	// Check whether any inventory slot is highlighted
	int found = -1;
	Common::fill(&colors[0], &colors[8], (int)COMMAND_FOREGROUND);
	for (int idx = 0; idx < 8; ++idx) {
		Common::Rect r(INVENTORY_POINTS[idx][0], CONTROLS_Y1,
			INVENTORY_POINTS[idx][1], CONTROLS_Y1 + 10);
		if (r.contains(mousePos)) {
			found = idx;
			break;
		}
	}

	if (events._pressed || events._released) {
		events.clearKeyboard();

		if (found != -1)
			// If a slot highlighted, set it's color
			colors[found] = COMMAND_HIGHLIGHTED;
		screen.buttonPrint(Common::Point(INVENTORY_POINTS[0][2], CONTROLS_Y1), colors[0], true, "Exit");

		if (found >= 0 && found <= 3) {
			screen.buttonPrint(Common::Point(INVENTORY_POINTS[1][2], CONTROLS_Y1), colors[1], true, "Look");
			screen.buttonPrint(Common::Point(INVENTORY_POINTS[2][2], CONTROLS_Y1), colors[2], true, "Use");
			screen.buttonPrint(Common::Point(INVENTORY_POINTS[3][2], CONTROLS_Y1), colors[3], true, "Give");
			inv._invMode = (InvMode)found;
			_selector = -1;
		}

		if (inv._invIndex) {
			screen.print(Common::Point(INVENTORY_POINTS[4][2], CONTROLS_Y1 + 1), colors[4], "^^");
			screen.print(Common::Point(INVENTORY_POINTS[5][2], CONTROLS_Y1 + 1), colors[5], "^");
		}

		if ((inv._holdings - inv._invIndex) > 6) {
			screen.print(Common::Point(INVENTORY_POINTS[6][2], CONTROLS_Y1 + 1), colors[6], "_");
			screen.print(Common::Point(INVENTORY_POINTS[7][2], CONTROLS_Y1 + 1), colors[7], "__");
		}

		bool flag = false;
		if (inv._invMode == 1 || inv._invMode == 2 || inv._invMode == 3) {
			Common::Rect r(15, CONTROLS_Y1 + 11, 314, SHERLOCK_SCREEN_HEIGHT - 2);
			if (r.contains(mousePos)) {
				_selector = (mousePos.x - 6) / 52 + inv._invIndex;
				if (_selector < inv._holdings)
					flag = true;
			}
		}

		if (!flag && mousePos.y >(CONTROLS_Y1 + 11))
			_selector = -1;
	}

	if (_keycode != Common::KEYCODE_INVALID) {
		_key = toupper(_keycode);

		if (_key == Common::KEYCODE_ESCAPE)
			// Escape will also 'E'xit out of inventory display
			_key = Common::KEYCODE_e;

		if (_key == 'E' || _key == 'L' || _key == 'U' || _key == 'G'
				|| _key == '-' || _key == '+') {
			int temp = inv._invMode;

			const char *chP = strchr(INVENTORY_COMMANDS, _key);
			inv._invMode = !chP ? INVMODE_INVALID : (InvMode)(chP - INVENTORY_COMMANDS);
			inv.invCommands(true);

			inv._invMode = (InvMode)temp;
			_keyboardInput = true;
			if (_key == 'E')
				inv._invMode = INVMODE_EXIT;
			_selector = -1;
		} else {
			_selector = -1;
		}
	}

	if (_selector != _oldSelector) {
		if (_oldSelector != -1) {
			// Un-highlight
			if (_oldSelector >= inv._invIndex && _oldSelector < (inv._invIndex + 6))
				inv.highlight(_oldSelector, BUTTON_MIDDLE);
		}

		if (_selector != -1)
			inv.highlight(_selector, 235);

		_oldSelector = _selector;
	}

	if (events._released || _keyboardInput) {
		if ((found == 0 && events._released) || _key == 'E') {
			inv.freeInv();
			_infoFlag = true;
			clearInfo();
			banishWindow(false);
			_key = -1;
			events.clearEvents();
			events.setCursor(ARROW);
		} else if ((found == 1 && events._released) || (_key == 'L')) {
			inv._invMode = INVMODE_LOOK;
		} else if ((found == 2 && events._released) || (_key == 'U')) {
			inv._invMode = INVMODE_USE;
		} else if ((found == 3 && events._released) || (_key == 'G')) {
			inv._invMode = INVMODE_GIVE;
		} else if (((found == 4 && events._released) || _key == ',') && inv._invIndex) {
			if (inv._invIndex >= 6)
				inv._invIndex -= 6;
			else
				inv._invIndex = 0;

			screen.print(Common::Point(INVENTORY_POINTS[4][2], CONTROLS_Y1 + 1),
				COMMAND_HIGHLIGHTED, "^^");
			inv.freeGraphics();
			inv.loadGraphics();
			inv.putInv(1);
			inv.invCommands(true);
		} else if (((found == 5 && events._released) || _key == Common::KEYCODE_MINUS
				|| _key == Common::KEYCODE_KP_MINUS) && inv._invIndex > 0) {
			--inv._invIndex;
			screen.print(Common::Point(INVENTORY_POINTS[4][2], CONTROLS_Y1 + 1), COMMAND_HIGHLIGHTED, "^");
			inv.freeGraphics();
			inv.loadGraphics();
			inv.putInv(1);
			inv.invCommands(true);
		} else if (((found == 6 && events._released) || _key == Common::KEYCODE_PLUS
				|| _key == Common::KEYCODE_KP_PLUS) &&  (inv._holdings - inv._invIndex) > 6) {
			++inv._invIndex;
			screen.print(Common::Point(INVENTORY_POINTS[6][2], CONTROLS_Y1 + 1), COMMAND_HIGHLIGHTED, "_");
			inv.freeGraphics();
			inv.loadGraphics();
			inv.putInv(1);
			inv.invCommands(true);
		} else if (((found == 7 && events._released) || _key == '.') && (inv._holdings - inv._invIndex) > 6) {
			inv._invIndex += 6;
			if ((inv._holdings - 6) < inv._invIndex)
				inv._invIndex = inv._holdings - 6;

			screen.print(Common::Point(INVENTORY_POINTS[7][2], CONTROLS_Y1 + 1), COMMAND_HIGHLIGHTED, "_");
			inv.freeGraphics();
			inv.loadGraphics();
			inv.putInv(1);
			inv.invCommands(true);
		} else {
			// If something is being given, make sure it's to a person
			if (inv._invMode == 3) {
				if (_bgFound != -1 && scene._bgShapes[_bgFound]._aType == PERSON)
					_find = _bgFound;
				else
					_find = -1;
			} else {
				_find = _bgFound;
			}

			if ((mousePos.y < CONTROLS_Y1) && (inv._invMode == 1) && (_find >= 0) && (_find < 1000)) {
				if (!scene._bgShapes[_find]._examine.empty() &&
						scene._bgShapes[_find]._examine[0] >= ' ')
					inv.doInvJF();
			} else if (_selector != -1 || _find >= 0) {
				// Selector is the inventory object that was clicked on, or selected.
				// If it's -1, then no inventory item is highlighted yet. Otherwise,
				// an object in the scene has been clicked.

				if (_selector != -1 && inv._invMode == INVMODE_LOOK 
						&& mousePos.y >(CONTROLS_Y1 + 11))
					inv.doInvJF();

				if (talk._talkToAbort)
					return;

				// Now check for the Use and Give actions. If inv_mode is 3, 
				// that means GIVE is in effect, _selector is the object being
				// given, and _find is the target.
				// The same applies to USE, except if _selector is -1, then USE
				// is being tried on an object in the scene without an inventory
				// object being highlighted first.

				if ((inv._invMode == 2 || (_selector != -1 && inv._invMode == 3)) && _find >= 0) {
					events._pressed = events._released = false;
					_infoFlag = true;
					clearInfo();

					int temp = _selector;	// Save the selector
					_selector = -1;

					inv.putInv(1);
					_selector = temp;		// Restore it
					temp = inv._invMode;
					inv._invMode = INVMODE_USE55;
					inv.invCommands(true);

					_infoFlag = true;
					clearInfo();
					banishWindow(false);
					_key = -1;

					inv.freeInv();

					if (_selector >= 0)
						// Use/Give inv object with scene object
						checkUseAction(&scene._bgShapes[_find]._use[0], inv[_selector]._name, MUSE, _find, temp - 2);
					else
						// Now inv object has been highlighted
						checkUseAction(&scene._bgShapes[_find]._use[0], "*SELF*", MUSE, _find, temp - 2);
						
					_selector = _oldSelector = -1;
				}
			}
		}
	}
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
			} else {
				events.setCursor(ARROW);
				banishWindow(true);
				_windowBounds.top = CONTROLS_Y1;
				_key = _oldKey = -1;
				_temp = _oldTemp = 0;
				_menuMode = STD_MODE;
				events.clearEvents();
			}
		} else {
			// Looking at an inventory object
			// Backup the user interface
			Surface tempSurface(SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y1);
			tempSurface.blitFrom(screen._backBuffer2, Common::Point(0, 0),
				Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

			inv.drawInventory(128);
			banishWindow(true);

			// Restore the ui
			screen._backBuffer2.blitFrom(tempSurface, Common::Point(0, CONTROLS_Y1));

			_windowBounds.top = CONTROLS_Y1;
			_key = _oldKey = COMMANDS[LOOK_MODE - 1];
			_temp = _oldTemp = 0;
			events.clearEvents();
			_invLookFlag = false;
			_menuMode = INV_MODE;
			_windowOpen = true;
		}
	}
}

/**
 * Handles input until one of the user interface buttons/commands is selected
 */
void UserInterface::doMainControl() {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	SaveManager &saves = *_vm->_saves;
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
			inv.drawInventory(1);
			break;
		case 'U':
			pushButton(7);
			_selector = _oldSelector = -1;
			_menuMode = USE_MODE;
			inv.drawInventory(2);
			break;
		case 'G':
			pushButton(8);
			_selector = _oldSelector = -1;
			_menuMode = GIVE_MODE;
			inv.drawInventory(3);
			break;
		case 'J':
			pushButton(9);
			_menuMode = JOURNAL_MODE;
			journalControl();
			break;
		case 'F':
			pushButton(10);
			_menuMode = FILES_MODE;

			// Create a thumbnail of the current screen before the files dialog is shown, in case
			// the user saves the game
			saves.createThumbnail();

			// Display the dialog
			saves.drawInterface();
			
			_selector = _oldSelector = -1;
			_windowOpen = true;
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

/**
 * Handles the input for the MOVE, OPEN, and CLOSE commands
 */
void UserInterface::doMiscControl(int allowed) {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;
	Talk &talk = *_vm->_talk;

	if (events._released) {
		_temp = _bgFound;
		if (_bgFound != -1) {
			// Only allow pointing to objects, not people
			if (_bgFound < 1000) {
				events.clearEvents();
				Object &obj = scene._bgShapes[_bgFound];

				switch (allowed) {
				case ALLOW_OPEN:
					checkAction(obj._aOpen, MOPEN, _temp);
					if (_menuMode && !talk._talkToAbort) {
						_menuMode = STD_MODE;
						restoreButton(OPEN_MODE - 1);
						_key = _oldKey = -1;
					}
					break;

				case ALLOW_CLOSE:
					checkAction(obj._aClose, MCLOSE, _temp);
					if (_menuMode != TALK_MODE && !talk._talkToAbort) {
						_menuMode = STD_MODE;
						restoreButton(CLOSE_MODE - 1);
						_key = _oldKey = -1;
					}
					break;

				case ALLOW_MOVE:
					checkAction(obj._aMove, MMOVE, _temp);
					if (_menuMode != TALK_MODE && !talk._talkToAbort) {
						_menuMode = STD_MODE;
						restoreButton(MOVE_MODE - 1);
						_key = _oldKey = -1;
					}
					break;

				default:
					break;
				}
			}
		}
	}
}

/**
 * Handles input for picking up items
 */
void UserInterface::doPickControl() {
	Events &events = *_vm->_events;
	Scene &scene = *_vm->_scene;

	if (events._released) {
		if ((_temp = _bgFound) != -1) {
			events.clearEvents();

			// Don't allow characters to be picked up
			if (_bgFound < 1000) {
				scene._bgShapes[_bgFound].pickUpObject(MPICK);

				if (_menuMode != TALK_MODE) {
					_key = _oldKey = -1;
					_menuMode = STD_MODE;
					restoreButton(PICKUP_MODE - 1);
				}
			}
		}
	}
}

/**
 * Handles input when in talk mode. It highlights the buttons and available statements,
 * and handles allowing the user to click on them
 */
void UserInterface::doTalkControl() {
	Events &events = *_vm->_events;
	Journal &journal = *_vm->_journal;
	People &people = *_vm->_people;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Talk &talk = *_vm->_talk;
	Common::Point mousePos = events.mousePos();
	int select;

	_key = _oldKey = -1;
	_keyboardInput = false;

	if (events._pressed || events._released) {
		events.clearKeyboard();

		// Handle button printing
		if (mousePos.x > 99 && mousePos.x < 138 && mousePos.y > CONTROLS_Y && mousePos.y < (CONTROLS_Y + 10) && !_endKeyActive)
			screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Exit");
		else if (_endKeyActive)
			screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_FOREGROUND, true, "Exit");
		
		if (mousePos.x > 140 && mousePos.x < 170 && mousePos.y > CONTROLS_Y && mousePos.y < (CONTROLS_Y + 10) && talk._moreTalkUp)
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Up");
		else if (talk._moreTalkUp)
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_FOREGROUND, true, "Up");

		if (mousePos.x > 181&& mousePos.x < 220 && mousePos.y > CONTROLS_Y && mousePos.y < (CONTROLS_Y + 10) && talk._moreTalkDown)
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_HIGHLIGHTED, true, "Down");
		else if (talk._moreTalkDown)
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_FOREGROUND, true, "Down");

		bool found = false;
		for (_selector = talk._talkIndex; _selector < (int)talk._statements.size() && !found; ++_selector) {
			if (mousePos.y > talk._statements[_selector]._talkPos.top &&
					mousePos.y < talk._statements[_selector]._talkPos.bottom)
				found = true;
		}
		--_selector;
		if (!found)
			_selector = -1;
	}

	if (_keycode != Common::KEYCODE_INVALID) {
		_key = toupper(_keycode);
		if (_key == Common::KEYCODE_ESCAPE)
			_key = 'E';

		// Check for number press indicating reply line
		if (_key >= '1' && _key <= ('1' + (int)talk._statements.size() - 1)) {
			for (uint idx = 0; idx < talk._statements.size(); ++idx) {
				if (talk._statements[idx]._talkMap == (_key - '1')) {
					// Found the given statement
					_selector = idx;
					_key = -1;
					_keyboardInput = true;
					break;
				}
			}
		} else if (_key == 'E' || _key == 'U' || _key == 'D') {
			_keyboardInput = true;
		} else {
			_selector = -1;
		}
	}

	if (_selector != _oldSelector) {
		// Remove highlighting from previous line, if any
		if (_oldSelector != -1) {
			if (!((talk._talkHistory[talk._converseNum][_oldSelector] >> (_oldSelector & 7)) & 1))
				talk.talkLine(_oldSelector, talk._statements[_oldSelector]._talkMap, INV_FOREGROUND,
					talk._statements[_oldSelector]._talkPos.top, true);
			else
				talk.talkLine(_oldSelector, talk._statements[_oldSelector]._talkMap, TALK_NULL,
					talk._statements[_oldSelector]._talkPos.top, true);
		}

		// Add highlighting to new line, if any
		if (_selector != -1)
			talk.talkLine(_selector, talk._statements[_selector]._talkMap, TALK_FOREGROUND,
				talk._statements[_selector]._talkPos.top, true);
	}

	if (events._released || _keyboardInput) {
		if (((Common::Rect(99, CONTROLS_Y, 138, CONTROLS_Y + 10).contains(mousePos) && events._released) 
				|| _key == 'E') && _endKeyActive) {
			talk.freeTalkVars();
			talk.pullSequence();
			banishWindow();
			_windowBounds.top = CONTROLS_Y1;
		} else if (((Common::Rect(140, CONTROLS_Y, 179, CONTROLS_Y + 10).contains(mousePos) && events._released) 
				|| _key == 'U') && talk._moreTalkUp) {
			while (talk._statements[--talk._talkIndex]._talkMap == -1)
				;
			screen._backBuffer1.fillRect(Common::Rect(5, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2,
				SHERLOCK_SCREEN_HEIGHT - 1), INV_BACKGROUND);
			talk.displayTalk(false);

			screen.slamRect(Common::Rect(5, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH - 5, SHERLOCK_SCREEN_HEIGHT - 2));
		} else if (((Common::Rect(181, CONTROLS_Y, 220, CONTROLS_Y + 10).contains(mousePos) && events._released)
				|| _key == 'D') && talk._moreTalkDown) {
			do {
				++talk._talkIndex;
			} while (talk._talkIndex < (int)talk._statements.size() && talk._statements[talk._talkIndex]._talkMap == -1);

			screen._backBuffer1.fillRect(Common::Rect(5, CONTROLS_Y + 11, SHERLOCK_SCREEN_WIDTH - 2,
				SHERLOCK_SCREEN_HEIGHT - 1), INV_BACKGROUND);
			talk.displayTalk(false);

			screen.slamRect(Common::Rect(5, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH - 5, SHERLOCK_SCREEN_HEIGHT - 2));
		} else if (_selector != -1) {
			screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, true, "Exit");
			screen.buttonPrint(Common::Point(159, CONTROLS_Y), COMMAND_NULL, true, "Up");
			screen.buttonPrint(Common::Point(200, CONTROLS_Y), COMMAND_NULL, true, "Down");

			// If the reply is new, add it to the journal
			if (!talk._talkHistory[talk._converseNum][_selector]) {
				journal.record(talk._converseNum, _selector);

				// Add any Holmes point to Holmes' total, if any
				if (talk._statements[_selector]._quotient)
					people._holmesQuotient += talk._statements[_selector]._quotient;
			}

			// Flag the response as having been used
			talk._talkHistory[talk._converseNum][_selector] = true;
		
			clearWindow();
			screen.print(Common::Point(16, CONTROLS_Y + 12), TALK_FOREGROUND, "Sherlock Holmes");
			talk.talkLine(_selector + 128, talk._statements[_selector]._talkMap, COMMAND_FOREGROUND, CONTROLS_Y + 21, true);

			switch (talk._statements[_selector]._portraitSide & 3) {
			case 0:
			case 1:
				people._portraitSide = 20;
				break;
			case 2:
				people._portraitSide = 220;
				break;
			case 3:
				people._portraitSide = 120;
				break;
			}

			// Check for flipping Holmes
			if (talk._statements[_selector]._portraitSide & REVERSE_DIRECTION)
				people._holmesFlip = true;

			talk._speaker = 0;
			people.setTalking(0);

			if (!talk._statements[_selector]._voiceFile.empty() && sound._voices) {
				sound.playSound(talk._statements[_selector]._voiceFile);

				// Set voices as an indicator for waiting
				sound._voices = 2;
				sound._speechOn = *sound._soundIsOn;
			} else {
				sound._speechOn = false;
			}

			// Set the _scriptCurrentIndex so if the statement is irrupted, the entire
			// reply will be shown when it's restarted
			talk._scriptCurrentIndex = 0;
			talk.waitForMore(talk._statements[_selector]._statement.size());
			if (talk._talkToAbort)
				return;

			people.clearTalking();
			if (talk._talkToAbort)
				return;

			while (!_vm->shouldQuit()) {
				talk._scriptSelect = _selector;
				talk._speaker = talk._talkTo;
				talk.doScript(talk._statements[_selector]._reply);

				if (!talk._talkToAbort) {
					if (!talk._talkStealth)
						clearWindow();

					if (!talk._statements[_selector]._modified.empty()) {
						for (uint idx = 0; idx < talk._statements[_selector]._modified.size(); ++idx) {
							_vm->setFlags(talk._statements[_selector]._modified[idx]);
						}

						talk.setTalkMap();
					}

					// Check for another linked talk file
					Common::String linkFilename = talk._statements[_selector]._linkFile;
					if (!linkFilename.empty() && !talk._scriptMoreFlag) {
						talk.freeTalkVars();
						talk.loadTalkFile(linkFilename);

						// Find the first new statement
						select = _selector = _oldSelector = -1;
						for (uint idx = 0; idx < talk._statements.size() && select == -1; ++idx) {
							if (!talk._statements[idx]._talkMap)
								select = talk._talkIndex = idx;
						}

						// See if the new statement is a stealth reply
						talk._talkStealth = talk._statements[select]._statement.hasPrefix("^") ? 2 : 0;

						// Is the new talk file a standard file, reply first file, or a stealth file
						if (!talk._statements[select]._statement.hasPrefix("*") &&
								!talk._statements[select]._statement.hasPrefix("^")) {
							// Not a reply first file, so display the new selections
							if (_endKeyActive)
								screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_FOREGROUND, true, "Exit");
							else
								screen.buttonPrint(Common::Point(119, CONTROLS_Y), COMMAND_NULL, true, "Exit");

							talk.displayTalk(true);
							events.setCursor(ARROW);
							break;
						} else {
							_selector = select;

							if (!talk._talkHistory[talk._converseNum][_selector])
								journal.record(talk._converseNum, _selector);

							talk._talkHistory[talk._converseNum][_selector] = true;
						}
					} else {
						talk.freeTalkVars();
						talk.pullSequence();
						banishWindow();
						_windowBounds.top = CONTROLS_Y1;
						break;
					}
				} else {
					break;
				}
			}

			events._pressed = events._released = false;
			events._oldButtons = 0;
			talk._talkStealth = 0;

			// If a script was pushed onto the script stack, restore it
			if (!talk._scriptStack.empty()) {
				ScriptStackEntry stackEntry = talk._scriptStack.pop();
				talk._scriptName = stackEntry._name;
				talk._scriptSaveIndex = stackEntry._currentIndex;
				talk._scriptSelect = stackEntry._select;
			}
		}
	}
}

void UserInterface::journalControl() {
	Events &events = *_vm->_events;
	Journal &journal = *_vm->_journal;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	int found;
	bool doneFlag = false;

	// Draw the journal screen
	journal.drawInterface();

	// Handle journal events
	do {
		found = _key = -1;
		events.setButtonState();
		
		// Handle keypresses
		if (events.kbHit()) {
			Common::KeyState keyState = events.getKey();
			if (keyState.keycode == Common::KEYCODE_x && (keyState.flags & Common::KBD_ALT)) {
				_vm->quitGame();
				return;
			} else if (keyState.keycode == Common::KEYCODE_e || keyState.keycode == Common::KEYCODE_ESCAPE) {
				doneFlag = true;
			} else {
				_key = toupper(keyState.keycode);
			}
		}

		if (!doneFlag)
			doneFlag = journal.handleEvents(_key);
	} while (!_vm->shouldQuit() && !doneFlag);

	// Finish up
	_infoFlag = _keyboardInput = false;
	_keycode = Common::KEYCODE_INVALID;
	_windowOpen = false;
	_windowBounds.top = CONTROLS_Y1;
	_key = -1;

	// Reset the palette
	screen.setPalette(screen._cMap);

	screen._backBuffer1.blitFrom(screen._backBuffer2);
	scene.updateBackground();
	screen.slamArea(0, 0, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	// TODO
}

/**
 * Handles input when the settings window is being shown
 */
void UserInterface::doControls() {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Sound &sound = *_vm->_sound;
	Talk &talk = *_vm->_talk;
	UserInterface &ui = *_vm->_ui;
	int found;
	bool updateConfig = false;

	Settings settings(_vm);
	settings.drawInteface(false);

	do {
		if (_menuCounter)
			whileMenuCounter();

		found = -1;
		_key = -1;

		scene.doBgAnim();
		if (talk._talkToAbort)
			return;

		events.setButtonState();
		Common::Point pt = events.mousePos();

		if (events._pressed || events._released || events.kbHit()) {
			clearInfo();
			_key = -1;

			if (events.kbHit()) {
				Common::KeyState keyState = events.getKey();
				_key = toupper(keyState.keycode);

				if (_key == Common::KEYCODE_RETURN || _key == Common::KEYCODE_SPACE) {
					events._pressed = false;
					events._oldButtons = 0;
					_keycode = Common::KEYCODE_INVALID;
					events._released = true;
				}
			}

			// Handle highlighting button under mouse
			found = settings.drawButtons(pt, _key);
		}

		if ((found == 0 && events._released) || (_key == 'E' || _key == Common::KEYCODE_ESCAPE))
			// Exit
			break;

		if ((found == 1 && events._released) || _key == 'M') {
			// Toggle music
			if (sound._music) {
				sound.stopSound();
				sound._music = false;
			} else {
				sound._music = true;
				sound.startSong();
			}

			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 2 && events._released) || _key == 'V') {
			sound._voices = !sound._voices;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 3 && events._released) || _key == 'S') {
			// Toggle sound effects
			sound._digitized = !sound._digitized;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 4 && events._released) || _key == 'A') {
			// Help button style
			ui._helpStyle ^= 1;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 5 && events._released) || _key == 'N') {
			// New font style
			int fontNum = screen.fontNumber() + 1;
			if (fontNum == 3)
				fontNum = 0;

			screen.setFont(fontNum);
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 6 && events._released) || _key == 'J') {
			// Toggle joystick - not implemented under ScummVM
		}

		if ((found == 7 && events._released) || _key == 'C') {
			// Calibrate joystick - No implementation in ScummVM
		}

		if ((found == 8 && events._released) || _key == 'F') {
			// Toggle fade style
			screen._fadeStyle = !screen._fadeStyle;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 9 && events._released) || _key == 'W') {
			// Window style
			ui._windowStyle ^= 1;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 10 && events._released) || _key == 'P') {
			// Toggle portraits being shown
			people._portraitsOn = !people._portraitsOn;
			updateConfig = true;
			settings.drawInteface(true);
		}

		if ((found == 11 && events._released) || _key == 'K') {
			// Toggle keypad acceleration speed
			_vm->_keyPadSpeed ^= 1;
			updateConfig = true;
			settings.drawInteface(true);
		}
	} while (!_vm->shouldQuit());

	banishWindow();
	
	if (updateConfig)
		_vm->saveConfig();
	
	_keycode = Common::KEYCODE_INVALID;
	_keyboardInput = false;
	_windowBounds.top = CONTROLS_Y1;
	_key = -1;
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

	Surface &bb = *screen._backBuffer;
	if (firstTime) {
		// Only draw the border on the first call
		_infoFlag = true;
		clearInfo();

		bb.fillRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH,
			CONTROLS_Y1 + 10), BORDER_COLOR);
		bb.fillRect(Common::Rect(0, CONTROLS_Y + 10, 1, SHERLOCK_SCREEN_HEIGHT - 1),
			BORDER_COLOR);
		bb.fillRect(Common::Rect(SHERLOCK_SCREEN_WIDTH - 2, CONTROLS_Y + 10,
			SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
		bb.fillRect(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - 1, SHERLOCK_SCREEN_WIDTH,
			SHERLOCK_SCREEN_HEIGHT), BORDER_COLOR);
	}

	// Clear background
	bb.fillRect(Common::Rect(2, CONTROLS_Y + 10, SHERLOCK_SCREEN_WIDTH - 2,
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
		screen.makeButton(Common::Rect(46, CONTROLS_Y, 272, CONTROLS_Y + 10),
			(SHERLOCK_SCREEN_WIDTH - screen.stringWidth(PRESS_KEY_FOR_MORE)) / 2,
			PRESS_KEY_FOR_MORE);
		screen.gPrint(Common::Point((SHERLOCK_SCREEN_WIDTH -
			screen.stringWidth(PRESS_KEY_FOR_MORE)) / 2, CONTROLS_Y),
			COMMAND_FOREGROUND, "P");
		_descStr = msgP;
	} else {
		screen.makeButton(Common::Rect(46, CONTROLS_Y, 272, CONTROLS_Y + 10),
			(SHERLOCK_SCREEN_WIDTH - screen.stringWidth(PRESS_KEY_TO_CONTINUE)) / 2,
			PRESS_KEY_TO_CONTINUE);
		screen.gPrint(Common::Point((SHERLOCK_SCREEN_WIDTH -
			screen.stringWidth(PRESS_KEY_TO_CONTINUE)) / 2, CONTROLS_Y),
			COMMAND_FOREGROUND, "P");
		_descStr = "";
	}

	if (firstTime) {
		if (!_windowStyle) {
			screen.slamRect(Common::Rect(0, CONTROLS_Y,
				SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
		} else {
			// Display the window
			summonWindow();
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
 * Displays a passed window by gradually scrolling it vertically on-screen
 */
void UserInterface::summonWindow(const Surface &bgSurface, bool slideUp) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;

	if (_windowOpen)
		// A window is already open, so can't open another one
		return;

	if (slideUp) {
		// Gradually slide up the display of the window
		for (int idx = 1; idx <= bgSurface.h; idx += 2) {
			screen._backBuffer->blitFrom(bgSurface, Common::Point(0, SHERLOCK_SCREEN_HEIGHT - idx),
				Common::Rect(0, 0, bgSurface.w, idx));
			screen.slamRect(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - idx,
				SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));

			events.delay(10);
		}
	} else {
		// Gradually slide down the display of the window
		for (int idx = 1; idx <= bgSurface.h; idx += 2) {
			screen._backBuffer->blitFrom(bgSurface, 
				Common::Point(0, SHERLOCK_SCREEN_HEIGHT - bgSurface.h),
				Common::Rect(0, bgSurface.h - idx, bgSurface.w, bgSurface.h));
			screen.slamRect(Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - bgSurface.h,
				SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT - bgSurface.h + idx));

			events.delay(10);
		}
	}

	// Final display of the entire window
	screen._backBuffer->blitFrom(bgSurface, Common::Point(0, 
		SHERLOCK_SCREEN_HEIGHT - bgSurface.h),
		Common::Rect(0, 0, bgSurface.w, bgSurface.h));
	screen.slamArea(0, SHERLOCK_SCREEN_HEIGHT - bgSurface.h, bgSurface.w, bgSurface.h);

	_windowOpen = true;
}

/**
 * Slide the window stored in the back buffer onto the screen
 */
void UserInterface::summonWindow(bool slideUp, int height) {
	Screen &screen = *_vm->_screen;

	// Extract the window that's been drawn on the back buffer
	Surface tempSurface(SHERLOCK_SCREEN_WIDTH,
		(SHERLOCK_SCREEN_HEIGHT - height));
	Common::Rect r(0, height, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT);
	tempSurface.blitFrom(screen._backBuffer1, Common::Point(0, 0), r);

	// Remove drawn window with original user interface
	screen._backBuffer1.blitFrom(screen._backBuffer2,
		Common::Point(0, height), r);

	// Display the window gradually on-screen
	summonWindow(tempSurface, slideUp);
}

/**
 * Close a currently open window
 * @param flag	0 = slide old window down, 1 = slide prior UI back up
 */
void UserInterface::banishWindow(bool slideUp) {
	Events &events = *_vm->_events;
	Screen &screen = *_vm->_screen;

	if (_windowOpen) {
		if (slideUp || !_windowStyle) {
			// Slide window down
			// Only slide the window if the window style allows it
			if (_windowStyle) {
				for (int idx = 2; idx < (SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y); idx += 2) {
					// Shift the window down by 2 lines
					byte *pSrc = (byte *)screen._backBuffer1.getBasePtr(0, CONTROLS_Y + idx - 2);
					byte *pSrcEnd = (byte *)screen._backBuffer1.getBasePtr(0, SHERLOCK_SCREEN_HEIGHT - 2);
					byte *pDest = (byte *)screen._backBuffer1.getBasePtr(0, SHERLOCK_SCREEN_HEIGHT);
					Common::copy_backward(pSrc, pSrcEnd, pDest);

					// Restore lines from the ui in the secondary back buffer
					screen._backBuffer1.blitFrom(screen._backBuffer2,
						Common::Point(0, CONTROLS_Y),
						Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, CONTROLS_Y + idx));

					screen.slamArea(0, CONTROLS_Y + idx - 2, SHERLOCK_SCREEN_WIDTH, 
						SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y - idx + 2);
					events.delay(10);
				}

				// Restore final two old lines
				screen._backBuffer1.blitFrom(screen._backBuffer2,
					Common::Point(0, SHERLOCK_SCREEN_HEIGHT - 2),
					Common::Rect(0, SHERLOCK_SCREEN_HEIGHT - 2,
						SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
				screen.slamArea(0, SHERLOCK_SCREEN_HEIGHT - 2, SHERLOCK_SCREEN_WIDTH, 2);
			} else {
				// Restore old area to completely erase window
				screen._backBuffer1.blitFrom(screen._backBuffer2,
					Common::Point(0, CONTROLS_Y),
					Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
				screen.slamRect(Common::Rect(0, CONTROLS_Y, SHERLOCK_SCREEN_WIDTH,
					SHERLOCK_SCREEN_HEIGHT));
			}
		} else {
			// Slide the original user interface up to cover the dialog
			for (int idx = 1; idx < (SHERLOCK_SCREEN_HEIGHT - CONTROLS_Y1); idx += 2) {
				byte *pSrc = (byte *)screen._backBuffer2.getBasePtr(0, CONTROLS_Y1);
				byte *pSrcEnd = (byte *)screen._backBuffer2.getBasePtr(0, CONTROLS_Y1 + idx);
				byte *pDest = (byte *)screen._backBuffer1.getBasePtr(0, SHERLOCK_SCREEN_HEIGHT - idx);
				Common::copy(pSrc, pSrcEnd, pDest);

				screen.slamArea(0, SHERLOCK_SCREEN_HEIGHT - idx, SHERLOCK_SCREEN_WIDTH,
					SHERLOCK_SCREEN_HEIGHT);
				events.delay(10);
			}

			// Show entire final area
			screen._backBuffer1.blitFrom(screen._backBuffer2, Common::Point(0, CONTROLS_Y1),
				Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
			screen.slamRect(Common::Rect(0, CONTROLS_Y1, SHERLOCK_SCREEN_WIDTH, SHERLOCK_SCREEN_HEIGHT));
		}

		_infoFlag = false;
		_windowOpen = false;
	}

	_menuMode = STD_MODE;
}

/**
 * Checks to see whether a USE action is valid on the given object
 */
void UserInterface::checkUseAction(const UseType *use, const Common::String &invName,
		const char *const messages[], int objNum, int giveMode) {
	Events &events = *_vm->_events;
	Inventory &inv = *_vm->_inventory;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	bool printed = messages == nullptr;

	if (objNum >= 1000) {
		// Holmes was specified, so do nothing
		_infoFlag = true;
		clearInfo();
		_infoFlag = true;
		
		// Display error message
		_menuCounter = 30;
		screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, "You can't do that to yourself.");
		return;
	}

	// Scan for target item
	int targetNum = -1;
	if (giveMode) {
		for (int idx = 0; idx < 4 && targetNum == -1; ++idx) {
			if ((scumm_stricmp(use[idx]._target.c_str(), "*GIVE*") == 0 || scumm_stricmp(use[idx]._target.c_str(), "*GIVEP*") == 0)
					&& scumm_stricmp(use[idx]._names[0].c_str(), invName.c_str()) == 0) {
				// Found a match
				targetNum = idx;
				if (scumm_stricmp(use[idx]._target.c_str(), "*GIVE*") == 0)
					inv.deleteItemFromInventory(invName);
			}
		}
	} else {
		for (int idx = 0; idx < 4 && targetNum == -1; ++idx) {
			if (scumm_stricmp(use[idx]._target.c_str(), invName.c_str()) == 0)
				targetNum = idx;
		}
	}

	if (targetNum != -1) {
		// Found a target, so do the action
		const UseType &action = use[targetNum];
		
		events.setCursor(WAIT);

		if (action._useFlag)
			_vm->setFlags(action._useFlag);

		if (action._cAnimNum != 99) {
			if (action._cAnimNum == 0)
				scene.startCAnim(9, action._cAnimSpeed);
			else
				scene.startCAnim(action._cAnimNum - 1, action._cAnimSpeed);
		}

		if (!talk._talkToAbort) {
			Object &obj = scene._bgShapes[objNum];
			for (int idx = 0; idx < 4 && !talk._talkToAbort; ++idx) {
				if (obj.checkNameForCodes(action._names[idx], messages)) {
					if (!talk._talkToAbort)
						printed = true;
				}
			}

			// Print "Done..." as an ending, unless flagged for leaving scene or otherwise flagged
			if (scene._goToScene != 1 && !printed && !talk._talkToAbort) {
				_infoFlag++;
				clearInfo();
				screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, "Done...");
				_menuCounter = 25;
			}
		}
	} else {
		// Couldn't find target, so print error
		_infoFlag = true;
		clearInfo();

		if (giveMode) {
			screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, "No, thank you.");
		} else if (messages == nullptr) {
			screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, "You can't do that.");
		} else {
			screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, messages[0]);
		}

		_infoFlag = true;
		_menuCounter = 30;
	}

	events.setCursor(ARROW);
}

/**
 * Called for OPEN, CLOSE, and MOVE actions are being done
 */
void UserInterface::checkAction(ActionType &action, const char *const messages[], int objNum) {
	Events &events = *_vm->_events;
	People &people = *_vm->_people;
	Scene &scene = *_vm->_scene;
	Screen &screen = *_vm->_screen;
	Talk &talk = *_vm->_talk;
	bool printed = false;
	bool doCAnim = true;
	int cAnimNum;
	Common::Point pt(-1, -1);
	int dir = -1;

	if (objNum >= 1000)
		// Ignore actions done on characters
		return;

	if (!action._cAnimSpeed) {
		// Invalid action, to print error message
		_infoFlag = true;
		clearInfo();
		screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, messages[action._cAnimNum]);
		_infoFlag = true;

		// Set how long to show the message
		_menuCounter = 30;
	} else {
		Object &obj = scene._bgShapes[objNum];

		if (action._cAnimNum == 0)
			// Really a 10
			cAnimNum = 9;
		else
			cAnimNum = action._cAnimNum - 1;
	
		if (action._cAnimNum != 99) {
			CAnim &anim = scene._cAnim[cAnimNum];

			if (action._cAnimNum != 99) {
				if (action._cAnimSpeed & REVERSE_DIRECTION) {
					pt = anim._teleportPos;
					dir = anim._teleportDir;
				} else {
					pt = anim._goto;
					dir = anim._gotoDir;
				}
			}
		} else {
			pt = Common::Point(-1, -1);
			dir = -1;
		}

		// Has a value, so do action
		// Show wait cursor whilst walking to object and doing action
		events.setCursor(WAIT);

		for (int nameIdx = 0; nameIdx < 4; ++nameIdx) {
			if (action._names[nameIdx].hasPrefix("*") && action._names[nameIdx].size() >= 2 
					&& toupper(action._names[nameIdx][1]) == 'W') {
				if (obj.checkNameForCodes(Common::String(action._names[nameIdx].c_str() + 2), messages)) {
					if (!talk._talkToAbort)
						printed = true;
				}
			}
		}

		for (int nameIdx = 0; nameIdx < 4; ++nameIdx) {
			if (action._names[nameIdx].hasPrefix("*") && action._names[nameIdx].size() >= 2) {
				char ch = toupper(action._names[nameIdx][1]);

				if (ch == 'T' || ch == 'B') {
					printed = true;
					if (pt.x != -1)
						// Holmes needs to walk to object before the action is done
						people.walkToCoords(pt, dir);

					if (!talk._talkToAbort) {
						// Ensure Holmes is on the exact intended location
						people[AL]._position = pt;
						people[AL]._sequenceNumber = dir;
						people.gotoStand(people[AL]);

						talk.talkTo(action._names[nameIdx].c_str() + 2);
						if (ch == 'T')
							doCAnim = false;
					}
				}
			}
		}

		if (doCAnim && !talk._talkToAbort) {
			if (pt.x != -1)
				// Holmes needs to walk to object before the action is done
				people.walkToCoords(pt, dir);
		}

		for (int nameIdx = 0; nameIdx < 4; ++nameIdx) {
			if (action._names[nameIdx].hasPrefix("*") && action._names[nameIdx].size() >= 2
					&& toupper(action._names[nameIdx][1]) == 'F') {
				if (obj.checkNameForCodes(action._names[nameIdx].c_str() + 2, messages)) {
					if (!talk._talkToAbort)
						printed = true;
				}
			}
		}

		if (doCAnim && !talk._talkToAbort && action._cAnimNum != 99)
			scene.startCAnim(cAnimNum, action._cAnimSpeed);

		if (!talk._talkToAbort) {
			for (int nameIdx = 0; nameIdx < 4 && !talk._talkToAbort; ++nameIdx) {
				if (obj.checkNameForCodes(action._names[nameIdx], messages)) {
					if (!talk._talkToAbort)
						printed = true;
				}
			}

			// Unless we're leaving the scene, print a "Done" message unless the printed flag has been set
			if (scene._goToScene != 1 && !printed && !talk._talkToAbort) {
				_infoFlag = true;
				clearInfo();
				screen.print(Common::Point(0, INFO_LINE + 1), INFO_FOREGROUND, "Done...");

				// Set how long to show the message
				_menuCounter = 30;	
			}
		}
	}

	// Reset cursor back to arrow
	events.setCursor(ARROW);
}


} // End of namespace Sherlock
