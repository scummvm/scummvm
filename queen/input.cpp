/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"
#include "queen/input.h"

namespace Queen {

const char* Input::_commandKeys[LANGUAGE_COUNT] = {
	"ocmglptu", // English
	"osbgpnre", // German
	"ofdnepau", // French
	"acsdgpqu", // Italian
	"ocmglptu", // Hebrew
	"acodmthu"  // Spanish
};  

Input::Input(Language language, OSystem *system) : 
	_system(system), _fastMode(false), _keyVerb(VERB_NONE), 
	_cutawayRunning(false), _canQuit(false), _cutawayQuit(false), _talkQuit(false),
	_dialogueRunning(false), _quickSave(false), _quickLoad(false), _debugger(false),
	_inKey(0), _mouse_x(0), _mouse_y(0), _mouseButton(0), _idleTime(0) {

	switch (language) {
		case ENGLISH:
			_currentCommandKeys = _commandKeys[0];
			break;
		case GERMAN:
			_currentCommandKeys = _commandKeys[1];
			break;
		case FRENCH:
			_currentCommandKeys = _commandKeys[2];
			break;
		case ITALIAN:
			_currentCommandKeys = _commandKeys[3];
			break;
		case HEBREW:
			_currentCommandKeys = _commandKeys[4];
			break;
		case SPANISH:
			_currentCommandKeys = _commandKeys[5];
			break;
		default:
			error("Unknown language");
			break;
	}
}

void Input::delay() {
	delay(_fastMode ? DELAY_SHORT : DELAY_NORMAL);
}

void Input::delay(uint amount) { 

	OSystem::Event event;

	uint32 start = _system->get_msecs();
	uint32 cur = start;

	if (_idleTime < DELAY_SCREEN_BLANKER) {
		_idleTime += amount;
	}

	do {
		while (_system->poll_event(&event)) {
			_idleTime = 0;
			switch (event.event_code) {
				case OSystem::EVENT_KEYDOWN:
					if (event.kbd.flags == OSystem::KBD_CTRL) {
						if (event.kbd.keycode == 'd') {
							_debugger = true;
						}
						else if (event.kbd.keycode == 'f') {
							_fastMode = !_fastMode;
						}
					}
					else {
						_inKey = event.kbd.keycode;
					}
					break;

				case OSystem::EVENT_MOUSEMOVE:
					_mouse_x = event.mouse.x;
					_mouse_y = event.mouse.y;
					break;

				case OSystem::EVENT_LBUTTONDOWN:
					_mouseButton |= MOUSE_LBUTTON;
#ifdef _WIN32_WCE
					_mouse_x = event.mouse.x;
					_mouse_y = event.mouse.y;
#endif
					break;

				case OSystem::EVENT_RBUTTONDOWN:
					_mouseButton |= MOUSE_RBUTTON;
					break;

				case OSystem::EVENT_QUIT:
					_system->quit();
					break;

				default:
					break;
			}
		}

		if (amount == 0)
			break;

		{
			uint this_delay = 20; // 1?
			if (this_delay > amount)
				this_delay = amount;
			_system->delay_msecs(this_delay);
		}
		cur = _system->get_msecs();
	} while (cur < start + amount);
}

int Input::checkKeys() {

	if (_inKey)
		debug(6, "[Input::checkKeys] _inKey = %i", _inKey);

	switch (_inKey) {
		case KEY_SPACE:
			_keyVerb = VERB_SKIP_TEXT;
			break;

		case KEY_COMMA:
			_keyVerb = VERB_SCROLL_UP;
			break;

		case KEY_DOT:
			_keyVerb = VERB_SCROLL_DOWN;
			break;

		case KEY_DIGIT_1:
			_keyVerb = VERB_DIGIT_1;
			break;

		case KEY_DIGIT_2:
			_keyVerb = VERB_DIGIT_2;
			break;

		case KEY_DIGIT_3:
			_keyVerb = VERB_DIGIT_3;
			break;

		case KEY_DIGIT_4:
			_keyVerb = VERB_DIGIT_4;
			break;

		case KEY_ESCAPE:
			if (_canQuit) {
				if (_cutawayRunning) {
					debug(6, "[Input::checkKeys] Setting _cutawayQuit to true!");
					_cutawayQuit = true;
				}

				if (_dialogueRunning)
					_talkQuit = true;
			}
			break;

		case KEY_F1:	// Use Journal
			if (_cutawayRunning) {
				if (_canQuit) {
					_keyVerb = VERB_USE_JOURNAL;
					_cutawayQuit = _talkQuit = true;
				}
			}
			else {
				_keyVerb = VERB_USE_JOURNAL;
				if (_canQuit)
					_talkQuit = true;
			}
			break;

		case KEY_F11:	// Quicksave
			_quickSave = true;
			break;
			
		case KEY_F12:	// Quickload
			_quickLoad = true;
			break;

		default:
			if(_inKey == _currentCommandKeys[0])
				_keyVerb = VERB_OPEN;
			else if(_inKey == _currentCommandKeys[1])
				_keyVerb = VERB_CLOSE;
			else if(_inKey == _currentCommandKeys[2])
				_keyVerb = VERB_MOVE;
			else if(_inKey == _currentCommandKeys[3])
				_keyVerb = VERB_GIVE;
			else if(_inKey == _currentCommandKeys[4])
				_keyVerb = VERB_LOOK_AT;
			else if(_inKey == _currentCommandKeys[5])
				_keyVerb = VERB_PICK_UP;
			else if(_inKey == _currentCommandKeys[6])
				_keyVerb = VERB_TALK_TO;
			else if(_inKey == _currentCommandKeys[7])
				_keyVerb = VERB_USE;
			break;
	}
	
	int inKey = _inKey;
	_inKey = 0;	//reset
	return inKey;
}


} // End of namespace Queen

