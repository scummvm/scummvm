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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_keyboard_state.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "common/system.h"
#include "common/keyboard.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseKeyboardState, false)

// Used in WME 1.x
// See "MSDN: Virtual-Key Codes" for more details on original WME keycodes
const keyCodeMapping wmeOriginalMapping[] = {
	{ Common::KEYCODE_BACKSPACE,    8 },
	{ Common::KEYCODE_TAB,          9 },
	{ Common::KEYCODE_RETURN,       13 },
	{ Common::KEYCODE_CAPSLOCK,     20 },
	{ Common::KEYCODE_ESCAPE,       27 },
	{ Common::KEYCODE_SPACE,        32 },

	{ Common::KEYCODE_LSHIFT,       16 },
	{ Common::KEYCODE_RSHIFT,       16 },
	{ Common::KEYCODE_LCTRL,        17 },
	{ Common::KEYCODE_RCTRL,        17 },
	// WME 1.x does not produce events for LALT & RALT

	{ Common::KEYCODE_PAUSE,        19 },
	{ Common::KEYCODE_PAGEUP,       33 },
	{ Common::KEYCODE_PAGEDOWN,     34 },
	{ Common::KEYCODE_END,          35 },
	{ Common::KEYCODE_HOME,         36 },
	{ Common::KEYCODE_LEFT,         37 },
	{ Common::KEYCODE_UP,           38 },
	{ Common::KEYCODE_RIGHT,        39 },
	{ Common::KEYCODE_DOWN,         40 },
	{ Common::KEYCODE_PRINT,        42 },
	{ Common::KEYCODE_INSERT,       45 },
	{ Common::KEYCODE_DELETE,       46 },
	{ Common::KEYCODE_SCROLLOCK,    145 },

	{ Common::KEYCODE_0,            48 },
	{ Common::KEYCODE_1,            49 },
	{ Common::KEYCODE_2,            50 },
	{ Common::KEYCODE_3,            51 },
	{ Common::KEYCODE_4,            52 },
	{ Common::KEYCODE_5,            53 },
	{ Common::KEYCODE_6,            54 },
	{ Common::KEYCODE_7,            55 },
	{ Common::KEYCODE_8,            56 },
	{ Common::KEYCODE_9,            57 },

	{ Common::KEYCODE_a,            65 },
	{ Common::KEYCODE_b,            66 },
	{ Common::KEYCODE_c,            67 },
	{ Common::KEYCODE_d,            68 },
	{ Common::KEYCODE_e,            69 },
	{ Common::KEYCODE_f,            70 },
	{ Common::KEYCODE_g,            71 },
	{ Common::KEYCODE_h,            72 },
	{ Common::KEYCODE_i,            73 },
	{ Common::KEYCODE_j,            74 },
	{ Common::KEYCODE_k,            75 },
	{ Common::KEYCODE_l,            76 },
	{ Common::KEYCODE_m,            77 },
	{ Common::KEYCODE_n,            78 },
	{ Common::KEYCODE_o,            79 },
	{ Common::KEYCODE_p,            80 },
	{ Common::KEYCODE_q,            81 },
	{ Common::KEYCODE_r,            82 },
	{ Common::KEYCODE_s,            83 },
	{ Common::KEYCODE_t,            84 },
	{ Common::KEYCODE_u,            85 },
	{ Common::KEYCODE_v,            86 },
	{ Common::KEYCODE_w,            87 },
	{ Common::KEYCODE_x,            88 },
	{ Common::KEYCODE_y,            89 },
	{ Common::KEYCODE_z,            90 },

	{ Common::KEYCODE_CLEAR,        12 },
	{ Common::KEYCODE_KP_ENTER,     13 },
	{ Common::KEYCODE_KP0,          96 },
	{ Common::KEYCODE_KP1,          97 },
	{ Common::KEYCODE_KP2,          98 },
	{ Common::KEYCODE_KP3,          99 },
	{ Common::KEYCODE_KP4,          100 },
	{ Common::KEYCODE_KP5,          101 },
	{ Common::KEYCODE_KP6,          102 },
	{ Common::KEYCODE_KP7,          103 },
	{ Common::KEYCODE_KP8,          104 },
	{ Common::KEYCODE_KP9,          105 },
	{ Common::KEYCODE_KP_MULTIPLY,  106 },
	{ Common::KEYCODE_KP_PLUS,      107 },
	{ Common::KEYCODE_KP_MINUS,     109 },
	{ Common::KEYCODE_KP_PERIOD,    110 },
	{ Common::KEYCODE_KP_DIVIDE,    111 },
	{ Common::KEYCODE_NUMLOCK,      144 },

	{ Common::KEYCODE_F1,           112 },
	{ Common::KEYCODE_F2,           113 },
	{ Common::KEYCODE_F3,           114 },
	{ Common::KEYCODE_F4,           115 },
	{ Common::KEYCODE_F5,           116 },
	{ Common::KEYCODE_F6,           117 },
	{ Common::KEYCODE_F7,           118 },
	{ Common::KEYCODE_F8,           119 },
	{ Common::KEYCODE_F9,           120 },
	{ Common::KEYCODE_F10,          121 },
	{ Common::KEYCODE_F11,          122 },
	{ Common::KEYCODE_F12,          123 },

	{ Common::KEYCODE_INVALID,      0 }
};

// Used in WME Lite & FoxTail
// See "SDL_Keycode" for more details on new WME keycodes
const keyCodeMapping wmeSdlMapping[] = {
	{ Common::KEYCODE_BACKSPACE,    8 },
	{ Common::KEYCODE_TAB,          9 },
	{ Common::KEYCODE_RETURN,       13 },
	{ Common::KEYCODE_ESCAPE,       27 },
	{ Common::KEYCODE_SPACE,        32 },
	{ Common::KEYCODE_CAPSLOCK,     1073741881 },

	{ Common::KEYCODE_LCTRL,        1073742048 },
	{ Common::KEYCODE_LSHIFT,       1073742049 },
	{ Common::KEYCODE_LALT,         1073742050 },
	{ Common::KEYCODE_RCTRL,        1073742052 },
	{ Common::KEYCODE_RSHIFT,       1073742053 },
	{ Common::KEYCODE_RALT,         1073742054 },

	{ Common::KEYCODE_DELETE,       127 },
	{ Common::KEYCODE_PRINT,        1073741894 },
	{ Common::KEYCODE_SCROLLOCK,    1073741895 },
	{ Common::KEYCODE_PAUSE,        1073741896 },
	{ Common::KEYCODE_INSERT,       1073741897 },
	{ Common::KEYCODE_HOME,         1073741898 },
	{ Common::KEYCODE_PAGEUP,       1073741899 },
	{ Common::KEYCODE_END,          1073741901 },
	{ Common::KEYCODE_PAGEDOWN,     1073741902 },
	{ Common::KEYCODE_RIGHT,        1073741903 },
	{ Common::KEYCODE_LEFT,         1073741904 },
	{ Common::KEYCODE_DOWN,         1073741905 },
	{ Common::KEYCODE_UP,           1073741906 },

	{ Common::KEYCODE_0,            48 },
	{ Common::KEYCODE_1,            49 },
	{ Common::KEYCODE_2,            50 },
	{ Common::KEYCODE_3,            51 },
	{ Common::KEYCODE_4,            52 },
	{ Common::KEYCODE_5,            53 },
	{ Common::KEYCODE_6,            54 },
	{ Common::KEYCODE_7,            55 },
	{ Common::KEYCODE_8,            56 },
	{ Common::KEYCODE_9,            57 },

	{ Common::KEYCODE_a,            97 },
	{ Common::KEYCODE_b,            98 },
	{ Common::KEYCODE_c,            99 },
	{ Common::KEYCODE_d,            100 },
	{ Common::KEYCODE_e,            101 },
	{ Common::KEYCODE_f,            102 },
	{ Common::KEYCODE_g,            103 },
	{ Common::KEYCODE_h,            104 },
	{ Common::KEYCODE_i,            105 },
	{ Common::KEYCODE_j,            106 },
	{ Common::KEYCODE_k,            107 },
	{ Common::KEYCODE_l,            108 },
	{ Common::KEYCODE_m,            109 },
	{ Common::KEYCODE_n,            110 },
	{ Common::KEYCODE_o,            111 },
	{ Common::KEYCODE_p,            112 },
	{ Common::KEYCODE_q,            113 },
	{ Common::KEYCODE_r,            114 },
	{ Common::KEYCODE_s,            115 },
	{ Common::KEYCODE_t,            116 },
	{ Common::KEYCODE_u,            117 },
	{ Common::KEYCODE_v,            118 },
	{ Common::KEYCODE_w,            119 },
	{ Common::KEYCODE_x,            120 },
	{ Common::KEYCODE_y,            121 },
	{ Common::KEYCODE_z,            122 },
	
	{ Common::KEYCODE_KP_ENTER,     13 },
	{ Common::KEYCODE_NUMLOCK,      1073741907 },
	{ Common::KEYCODE_KP_DIVIDE,    1073741908 },
	{ Common::KEYCODE_KP_MULTIPLY,  1073741909 },
	{ Common::KEYCODE_KP_MINUS,     1073741910 },
	{ Common::KEYCODE_KP_PLUS,      1073741911 },
	{ Common::KEYCODE_KP1,          1073741913 },
	{ Common::KEYCODE_KP2,          1073741914 },
	{ Common::KEYCODE_KP3,          1073741915 },
	{ Common::KEYCODE_KP4,          1073741916 },
	{ Common::KEYCODE_KP5,          1073741917 },
	{ Common::KEYCODE_KP6,          1073741918 },
	{ Common::KEYCODE_KP7,          1073741919 },
	{ Common::KEYCODE_KP8,          1073741920 },
	{ Common::KEYCODE_KP9,          1073741921 },
	{ Common::KEYCODE_KP0,          1073741922 },
	{ Common::KEYCODE_KP_PERIOD,    1073741923 },
	{ Common::KEYCODE_CLEAR,        1073741980 },

	{ Common::KEYCODE_F1,           1073741882 },
	{ Common::KEYCODE_F2,           1073741883 },
	{ Common::KEYCODE_F3,           1073741884 },
	{ Common::KEYCODE_F4,           1073741885 },
	{ Common::KEYCODE_F5,           1073741886 },
	{ Common::KEYCODE_F6,           1073741887 },
	{ Common::KEYCODE_F7,           1073741888 },
	{ Common::KEYCODE_F8,           1073741889 },
	{ Common::KEYCODE_F9,           1073741890 },
	{ Common::KEYCODE_F10,          1073741891 },
	{ Common::KEYCODE_F11,          1073741892 },
	{ Common::KEYCODE_F12,          1073741893 },

	{ Common::KEYCODE_INVALID,      0 }
};

//////////////////////////////////////////////////////////////////////////
BaseKeyboardState::BaseKeyboardState(BaseGame *inGame) : BaseScriptable(inGame) {
	init();
}

//////////////////////////////////////////////////////////////////////////
void BaseKeyboardState::init() {
	_currentPrintable = false;
	_currentCharCode = 0;
	_currentKeyData = 0;

	_currentShift = false;
	_currentAlt = false;
	_currentControl = false;

	_keyStates = new uint8[Common::KEYCODE_LAST];
	for (int i = 0; i < Common::KEYCODE_LAST; i++) {
		_keyStates[i] = false;
	}

	if (BaseEngine::instance().getTargetExecutable() < WME_LITE) {
		_mapping = wmeOriginalMapping;
		_mappingSize = ARRAYSIZE(wmeOriginalMapping);
	} else {
		_mapping = wmeSdlMapping;
		_mappingSize = ARRAYSIZE(wmeSdlMapping);
	}
}

//////////////////////////////////////////////////////////////////////////
BaseKeyboardState::~BaseKeyboardState() {
	delete[] _keyStates;
}

void BaseKeyboardState::handleKeyPress(Common::Event *event) {
	if (event->type == Common::EVENT_KEYDOWN) {
		_keyStates[event->kbd.keycode] = true;
	}
}

void BaseKeyboardState::handleKeyRelease(Common::Event *event) {
	if (event->type == Common::EVENT_KEYUP) {
		_keyStates[event->kbd.keycode] = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// IsKeyDown
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "IsKeyDown") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		uint32 vKeyCode;

		if (val->_type == VAL_STRING && strlen(val->getString()) > 0) {
			// IsKeyDown(strings) checks if a key with given ASCII code is pressed
			// Only 1st character of given string is used for the check

			// This check must be case insensitive, which means that 
			// IsKeyDown("a") & IsKeyDown("A") are either both true or both false 
			const char *str = val->getString();
			char temp = str[0];
			if (temp >= 'A' && temp <= 'Z') {
				temp += ('a' - 'A');
			}

			// Common::KeyCode is equal to ASCII code for any lowercase ASCII character
			if (temp >= ' ' && temp <= '~') {
				vKeyCode = (int)temp;
			} else {
				warning("Unhandled IsKeyDown(string): check for non-ASCII character");
				vKeyCode = 0;
			}
		} else {
			// IsKeyDown(int) checks if a key with given keycode is pressed
			// For letters, single keycode is used for upper and lower case
			// This mean that IsKeyDown(65) is true for both 'a' and Shift+'a'

			vKeyCode = Common::KEYCODE_INVALID;
			uint32 temp = (uint32)val->getInt();
			
			for (uint32 i = 0; i < _mappingSize; i++) {
				if (_mapping[i].engineKeycode == temp) {
					vKeyCode = _mapping[i].commonKeycode;
				}
			}

            if (vKeyCode == Common::KEYCODE_INVALID) {
				warning("Unknown VKEY: %d", temp);
			}

			if (BaseEngine::instance().getTargetExecutable() < WME_LITE && temp == 16) {
				stack->pushBool(_keyStates[Common::KEYCODE_LSHIFT] || _keyStates[Common::KEYCODE_RSHIFT]);
				return STATUS_OK;
			}

			if (BaseEngine::instance().getTargetExecutable() < WME_LITE && temp == 17) {
				stack->pushBool(_keyStates[Common::KEYCODE_LCTRL] || _keyStates[Common::KEYCODE_RCTRL]);
				return STATUS_OK;
			}
		}

		bool isDown = _keyStates[vKeyCode];

		stack->pushBool(isDown);
		return STATUS_OK;
	} else {
		return BaseScriptable::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseKeyboardState::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("keyboard");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Key
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Key") {
		if (_currentPrintable) {
			char key[2];
			key[0] = (char)_currentCharCode;
			key[1] = '\0';
			_scValue->setString(key);
		} else {
			_scValue->setString("");
		}

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Printable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Printable") {
		_scValue->setBool(_currentPrintable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// KeyCode
	//////////////////////////////////////////////////////////////////////////
	else if (name == "KeyCode") {
		_scValue->setInt(_currentCharCode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsShift
	//////////////////////////////////////////////////////////////////////////
	else if (name == "IsShift") {
		_scValue->setBool(_currentShift);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsAlt
	//////////////////////////////////////////////////////////////////////////
	else if (name == "IsAlt") {
		_scValue->setBool(_currentAlt);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsControl
	//////////////////////////////////////////////////////////////////////////
	else if (name == "IsControl") {
		_scValue->setBool(_currentControl);
		return _scValue;
	} else {
		return BaseScriptable::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::scSetProperty(const char *name, ScValue *value) {
	/*
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
	    setName(value->getString());
	    if (_renderer) SetWindowText(_renderer->_window, _name);
	    return STATUS_OK;
	}

	else*/ return BaseScriptable::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *BaseKeyboardState::scToString() {
	return "[keyboard state]";
}


//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::readKey(Common::Event *event) {

	Common::KeyCode code = event->kbd.keycode;

	if (event->type != Common::EVENT_KEYDOWN) {
		_currentCharCode = 0;
		_currentPrintable = false;
	}

	// use ASCII value if key pressed is an alphanumeric or punctuation key
	// keys pressed on numpad are handled in next 2 blocks
	else if (code >= Common::KEYCODE_SPACE && code < Common::KEYCODE_DELETE) {
		_currentCharCode = event->kbd.ascii;
		_currentPrintable = true;
#ifdef ENABLE_FOXTAIL
		if (BaseEngine::instance().isFoxTail()) {
			_currentCharCode = tolower(_currentCharCode);
		}
#endif
	}

	// use ASCII value for numpad '/', '*', '-', '+'
	else if (code >= Common::KEYCODE_KP_DIVIDE && code <= Common::KEYCODE_KP_PLUS) {
		_currentCharCode = event->kbd.ascii;
		_currentPrintable = true;
	}

	// if NumLock is active, use ASCII for numpad keys '0'~'9' and '.'
	// keys pressed on numpad without NumLock are considered as normal keycodes, handled in the next block
	else if ((code >= Common::KEYCODE_KP0 && code <= Common::KEYCODE_KP_PERIOD) && ((event->kbd.flags & Common::KBD_NUM) != 0)) {
		_currentCharCode = event->kbd.ascii;
		_currentPrintable = true;
	}

	// use _mapping for all other events
	else {
		_currentCharCode = 0;
		for (uint32 i = 0; i < _mappingSize; i++) {
			if (_mapping[i].commonKeycode == event->kbd.keycode) {
				_currentCharCode = _mapping[i].engineKeycode;
			}
		}

		if (!_currentCharCode && (event->kbd.flags & Common::KBD_NON_STICKY) == 0) {
			warning("Key pressed (%d '%c') is not recognized, ASCII returned (%d '%c').", event->kbd.keycode, event->kbd.keycode, event->kbd.ascii, event->kbd.ascii);
		}

		if (BaseEngine::instance().getTargetExecutable() < WME_LITE) {
			_currentPrintable = code == Common::KEYCODE_BACKSPACE || 
								code == Common::KEYCODE_TAB || 
								code == Common::KEYCODE_RETURN || 
								code == Common::KEYCODE_KP_ENTER || 
								code == Common::KEYCODE_ESCAPE;
		} else {
			_currentPrintable = false;
		}
	}

	_currentControl = event->kbd.flags & Common::KBD_CTRL;
	_currentAlt     = event->kbd.flags & Common::KBD_ALT;
	_currentShift   = event->kbd.flags & Common::KBD_SHIFT;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::persist(BasePersistenceManager *persistMgr) {
	//if (!persistMgr->getIsSaving()) cleanup();
	BaseScriptable::persist(persistMgr);

	persistMgr->transferBool(TMEMBER(_currentAlt));
	persistMgr->transferUint32(TMEMBER(_currentCharCode));
	persistMgr->transferBool(TMEMBER(_currentControl));
	persistMgr->transferUint32(TMEMBER(_currentKeyData));
	persistMgr->transferBool(TMEMBER(_currentPrintable));
	persistMgr->transferBool(TMEMBER(_currentShift));

	if (!persistMgr->getIsSaving()) {
		init();
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::isShiftDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_SHIFT);
}

//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::isControlDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_CTRL);
}

//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::isAltDown() {
	int mod = g_system->getEventManager()->getModifierState();
	return (mod & Common::KBD_ALT);
}

//////////////////////////////////////////////////////////////////////////
bool BaseKeyboardState::isCurrentPrintable() const {
	return _currentPrintable;
}

} // End of namespace Wintermute
