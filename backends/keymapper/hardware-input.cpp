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

#include "backends/keymapper/hardware-input.h"

#ifdef ENABLE_KEYMAPPER

#include "backends/keymapper/keymapper.h"

namespace Common {

static const KeyTableEntry defaultKeys[] = {
	{"BACKSPACE", KEYCODE_BACKSPACE, "Backspace"},
	{"TAB", KEYCODE_TAB, "Tab"},
	{"CLEAR", KEYCODE_CLEAR, "Clear"},
	{"RETURN", KEYCODE_RETURN, "Return"},
	{"PAUSE", KEYCODE_PAUSE, "Pause"},
	{"ESCAPE", KEYCODE_ESCAPE, "Esc"},
	{"SPACE", KEYCODE_SPACE, "Space"},
	{"EXCLAIM", KEYCODE_EXCLAIM, "!"},
	{"QUOTEDBL", KEYCODE_QUOTEDBL, "\""},
	{"HASH", KEYCODE_HASH, "#"},
	{"DOLLAR", KEYCODE_DOLLAR, "$"},
	{"PERCENT", KEYCODE_PERCENT, "%"},
	{"AMPERSAND", KEYCODE_AMPERSAND, "&"},
	{"QUOTE", KEYCODE_QUOTE, "'"},
	{"LEFTPAREN", KEYCODE_LEFTPAREN, "("},
	{"RIGHTPAREN", KEYCODE_RIGHTPAREN, ")"},
	{"ASTERISK", KEYCODE_ASTERISK, "*"},
	{"PLUS", KEYCODE_PLUS, "+"},
	{"COMMA", KEYCODE_COMMA, ","},
	{"MINUS", KEYCODE_MINUS, "-"},
	{"PERIOD", KEYCODE_PERIOD, "."},
	{"SLASH", KEYCODE_SLASH, "/"},
	{"0", KEYCODE_0, "0"},
	{"1", KEYCODE_1, "1"},
	{"2", KEYCODE_2, "2"},
	{"3", KEYCODE_3, "3"},
	{"4", KEYCODE_4, "4"},
	{"5", KEYCODE_5, "5"},
	{"6", KEYCODE_6, "6"},
	{"7", KEYCODE_7, "7"},
	{"8", KEYCODE_8, "8"},
	{"9", KEYCODE_9, "9"},
	{"COLON", KEYCODE_COLON, ":"},
	{"SEMICOLON", KEYCODE_SEMICOLON, ";"},
	{"LESS", KEYCODE_LESS, "<"},
	{"EQUALS", KEYCODE_EQUALS, "="},
	{"GREATER", KEYCODE_GREATER, ">"},
	{"QUESTION", KEYCODE_QUESTION, "?"},
	{"AT", KEYCODE_AT, "@"},

	{"LEFTBRACKET", KEYCODE_LEFTBRACKET, "["},
	{"BACKSLASH", KEYCODE_BACKSLASH, "\\"},
	{"RIGHTBRACKET", KEYCODE_RIGHTBRACKET, "]"},
	{"CARET", KEYCODE_CARET, "^"},
	{"UNDERSCORE", KEYCODE_UNDERSCORE, "_"},
	{"BACKQUOTE", KEYCODE_BACKQUOTE, "`"},
	{"a", KEYCODE_a, "a"},
	{"b", KEYCODE_b, "b"},
	{"c", KEYCODE_c, "c"},
	{"d", KEYCODE_d, "d"},
	{"e", KEYCODE_e, "e"},
	{"f", KEYCODE_f, "f"},
	{"g", KEYCODE_g, "g"},
	{"h", KEYCODE_h, "h"},
	{"i", KEYCODE_i, "i"},
	{"j", KEYCODE_j, "j"},
	{"k", KEYCODE_k, "k"},
	{"l", KEYCODE_l, "l"},
	{"m", KEYCODE_m, "m"},
	{"n", KEYCODE_n, "n"},
	{"o", KEYCODE_o, "o"},
	{"p", KEYCODE_p, "p"},
	{"q", KEYCODE_q, "q"},
	{"r", KEYCODE_r, "r"},
	{"s", KEYCODE_s, "s"},
	{"t", KEYCODE_t, "t"},
	{"u", KEYCODE_u, "u"},
	{"v", KEYCODE_v, "v"},
	{"w", KEYCODE_w, "w"},
	{"x", KEYCODE_x, "x"},
	{"y", KEYCODE_y, "y"},
	{"z", KEYCODE_z, "z"},
	{"DELETE", KEYCODE_DELETE, "Del"},

	// Numeric keypad
	{"KP0", KEYCODE_KP0, "KP0"},
	{"KP1", KEYCODE_KP1, "KP1"},
	{"KP2", KEYCODE_KP2, "KP2"},
	{"KP3", KEYCODE_KP3, "KP3"},
	{"KP4", KEYCODE_KP4, "KP4"},
	{"KP5", KEYCODE_KP5, "KP5"},
	{"KP6", KEYCODE_KP6, "KP6"},
	{"KP7", KEYCODE_KP7, "KP7"},
	{"KP8", KEYCODE_KP8, "KP8"},
	{"KP9", KEYCODE_KP9, "KP9"},
	{"KP_PERIOD", KEYCODE_KP_PERIOD, "KP."},
	{"KP_DIVIDE", KEYCODE_KP_DIVIDE, "KP/"},
	{"KP_MULTIPLY", KEYCODE_KP_MULTIPLY, "KP*"},
	{"KP_MINUS", KEYCODE_KP_MINUS, "KP-"},
	{"KP_PLUS", KEYCODE_KP_PLUS, "KP+"},
	{"KP_ENTER", KEYCODE_KP_ENTER, "KP Enter"},
	{"KP_EQUALS", KEYCODE_KP_EQUALS, "KP="},

	// Arrows + Home/End pad
	{"UP", KEYCODE_UP, "Up"},
	{"DOWN", KEYCODE_DOWN, "Down"},
	{"RIGHT", KEYCODE_RIGHT, "Right"},
	{"LEFT", KEYCODE_LEFT, "Left"},
	{"INSERT", KEYCODE_INSERT, "Insert"},
	{"HOME", KEYCODE_HOME, "Home"},
	{"END", KEYCODE_END, "End"},
	{"PAGEUP", KEYCODE_PAGEUP, "PgUp"},
	{"PAGEDOWN", KEYCODE_PAGEDOWN, "PgDn"},

	// Function keys
	{"F1", KEYCODE_F1, "F1"},
	{"F2", KEYCODE_F2, "F2"},
	{"F3", KEYCODE_F3, "F3"},
	{"F4", KEYCODE_F4, "F4"},
	{"F5", KEYCODE_F5, "F5"},
	{"F6", KEYCODE_F6, "F6"},
	{"F7", KEYCODE_F7, "F7"},
	{"F8", KEYCODE_F8, "F8"},
	{"F9", KEYCODE_F9, "F9"},
	{"F10", KEYCODE_F10, "F10"},
	{"F11", KEYCODE_F11, "F11"},
	{"F12", KEYCODE_F12, "F12"},
	{"F13", KEYCODE_F13, "F13"},
	{"F14", KEYCODE_F14, "F14"},
	{"F15", KEYCODE_F15, "F15"},
	{"F16", KEYCODE_F16, "F16"},
	{"F17", KEYCODE_F17, "F17"},
	{"F18", KEYCODE_F18, "F18"},

	// Miscellaneous function keys
	{"HELP", KEYCODE_HELP, "Help"},
	{"PRINT", KEYCODE_PRINT, "Print"},
	{"SYSREQ", KEYCODE_SYSREQ, "SysRq"},
	{"BREAK", KEYCODE_BREAK, "Break"},
	{"MENU", KEYCODE_MENU, "Menu"},
		// Power Macintosh power key
	{"POWER", KEYCODE_POWER, "Power"},
		// Some european keyboards
	{"EURO", KEYCODE_EURO, "Euro"},
		// Atari keyboard has Undo
	{"UNDO", KEYCODE_UNDO, "Undo"},
	{"SLEEP", KEYCODE_SLEEP, "Sleep"},
	{"MUTE", KEYCODE_MUTE, "Mute"},
	{"EJECT", KEYCODE_EJECT, "Eject"},
	{"VOLUMEUP", KEYCODE_VOLUMEUP, "Volume Up"},
	{"VOLUMEDOWN", KEYCODE_VOLUMEDOWN, "Volume Down"},
	{"LEFTSOFT", KEYCODE_LEFTSOFT, "Left Soft"},
	{"RIGHTSOFT", KEYCODE_RIGHTSOFT, "Right Soft"},
	{"CALL", KEYCODE_CALL, "Call"},
	{"HANGUP", KEYCODE_HANGUP, "Hang up"},
	{"CAMERA", KEYCODE_CAMERA, "Camera"},
	{"WWW", KEYCODE_WWW, "WWW"},
	{"MAIL", KEYCODE_MAIL, "Mail"},
	{"CALCULATOR", KEYCODE_CALCULATOR, "Calculator"},
	{"CUT", KEYCODE_CUT, "Cut"},
	{"COPY", KEYCODE_COPY, "Copy"},
	{"PASTE", KEYCODE_PASTE, "Paste"},
	{"SELECT", KEYCODE_SELECT, "Select"},
	{"CANCEL", KEYCODE_CANCEL, "Cancel"},

	// Action keys
	{"AC_SEARCH", KEYCODE_AC_SEARCH, "AC Search"},
	{"AC_HOME", KEYCODE_AC_HOME, "AC Home"},
	{"AC_BACK", KEYCODE_AC_BACK, "AC Back"},
	{"AC_FORWARD", KEYCODE_AC_FORWARD, "AC Forward"},
	{"AC_STOP", KEYCODE_AC_STOP, "AC Stop"},
	{"AC_REFRESH", KEYCODE_AC_REFRESH, "AC Refresh"},
	{"AC_BOOKMARKS", KEYCODE_AC_BOOKMARKS, "AC Bookmarks"},

	// Audio keys
	{"AUDIONEXT", KEYCODE_AUDIONEXT, "Audio Next"},
	{"AUDIOPREV", KEYCODE_AUDIOPREV, "Audio Previous"},
	{"AUDIOSTOP", KEYCODE_AUDIOSTOP, "Audio Stop"},
	{"AUDIOPLAY", KEYCODE_AUDIOPLAY, "Audio Play"},
	{"AUDIOPAUSE", KEYCODE_AUDIOPAUSE, "Audio Pause"},
	{"AUDIOPLAYPAUSE", KEYCODE_AUDIOPLAYPAUSE, "Audio Play/Pause"},
	{"AUDIOMUTE", KEYCODE_AUDIOMUTE, "Audio Mute"},
	{"AUDIOREWIND", KEYCODE_AUDIOREWIND, "Audio Rewind"},
	{"AUDIOFASTFORWARD", KEYCODE_AUDIOFASTFORWARD, "Audio Fast-Forward"},

	{0, KEYCODE_INVALID, 0}
};

static const ModifierTableEntry defaultModifiers[] = {
	{ 0, "", "" },
	{ KBD_CTRL, "C+", "Ctrl+" },
	{ KBD_ALT, "A+", "Alt+" },
	{ KBD_SHIFT, "S+", "Shift+" },
	{ KBD_CTRL | KBD_ALT, "C+A+", "Ctrl+Alt+" },
	{ KBD_SHIFT | KBD_CTRL, "S+C+", "Shift+Ctrl+" },
	{ KBD_SHIFT | KBD_CTRL | KBD_ALT, "S+C+A+", "Shift+Ctrl+Alt+" },
	{ 0, 0, 0 }
};

HardwareInputSet::HardwareInputSet(bool useDefault, const KeyTableEntry *keys, const ModifierTableEntry *modifiers) {
	if (useDefault)
		addHardwareInputs(defaultKeys, defaultModifiers);
	if (keys)
		addHardwareInputs(keys, modifiers ? modifiers : defaultModifiers);
}

HardwareInputSet::~HardwareInputSet() {
	Array<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it)
		delete *it;
}

void HardwareInputSet::addHardwareInput(const HardwareInput *input) {
	assert(input);

	debug(8, "Adding hardware input [%s][%s]", input->id.c_str(), input->description.c_str());

	removeHardwareInput(input);

	_inputs.push_back(input);
}

const HardwareInput *HardwareInputSet::findHardwareInput(String id) const {
	Array<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it) {
		if ((*it)->id == id)
			return (*it);
	}
	return 0;
}

const HardwareInput *HardwareInputSet::findHardwareInput(const HardwareInputCode code) const {
	Array<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it) {
		const HardwareInput *entry = *it;
		if (entry->type == kHardwareInputTypeGeneric && entry->inputCode == code)
			return entry;
	}
	return 0;
}

const HardwareInput *HardwareInputSet::findHardwareInput(const KeyState &keystate) const {
	Array<const HardwareInput *>::const_iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it) {
		const HardwareInput *entry = *it;
		if (entry->type == kHardwareInputTypeKeyboard && entry->key == keystate)
			return entry;
	}
	return 0;
}

void HardwareInputSet::addHardwareInputs(const HardwareInputTableEntry inputs[]) {
	for (const HardwareInputTableEntry *entry = inputs; entry->hwId; ++entry)
		addHardwareInput(new HardwareInput(entry->hwId, entry->code, entry->desc));
}

void HardwareInputSet::addHardwareInputs(const KeyTableEntry keys[], const ModifierTableEntry modifiers[]) {
	const KeyTableEntry *key;
	const ModifierTableEntry *mod;

	for (mod = modifiers; mod->id; mod++) {
		for (key = keys; key->hwId; key++) {
			String fullKeyId = String::format("%s%s", mod->id, key->hwId);
			String fullKeyDesc = String::format("%s%s", mod->desc, key->desc);

			addHardwareInput(new HardwareInput(fullKeyId, KeyState(key->keycode, 0, mod->flag), fullKeyDesc));
		}
	}
}

void HardwareInputSet::removeHardwareInput(const HardwareInput *input) {
	if (!input)
		return;

	Array<const HardwareInput *>::iterator it;

	for (it = _inputs.begin(); it != _inputs.end(); ++it) {
		const HardwareInput *entry = (*it);
		bool match = false;
		if (entry->id == input->id)
			match = true;
		else if (input->type == entry->type) {
			if (input->type == kHardwareInputTypeGeneric && input->inputCode == entry->inputCode)
				match = true;
			else if (input->type == kHardwareInputTypeKeyboard && input->key == entry->key)
				match = true;
		}
		if (match) {
			debug(7, "Removing hardware input [%s] (%s) because it matches [%s] (%s)", entry->id.c_str(), entry->description.c_str(), input->id.c_str(), input->description.c_str());
			delete entry;
			_inputs.erase(it);
		}
	}
}

} //namespace Common

#endif // #ifdef ENABLE_KEYMAPPER

