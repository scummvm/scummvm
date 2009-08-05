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
 * $URL$
 * $Id$
 *
 */

#include "backends/platform/sdl/sdl.h"
#include "backends/keymapper/keymapper.h"
#include "common/keyboard.h"

#ifdef ENABLE_KEYMAPPER

using namespace Common;

struct Key {
	const char *hwId;
	KeyCode keycode;
	uint16 ascii;
	const char *desc;
	KeyType preferredAction;
	int32 modableMask;
};

static const Key keys[] = {
	{"BACKSPACE", KEYCODE_BACKSPACE, ASCII_BACKSPACE, "Backspace", kActionKeyType, ~0},
	{"TAB", KEYCODE_TAB, ASCII_TAB, "Tab", kActionKeyType, ~0},
	{"CLEAR", KEYCODE_CLEAR, 0, "Clear", kActionKeyType, ~0},
	{"RETURN", KEYCODE_RETURN, ASCII_RETURN, "Return", kActionKeyType, ~0},
	{"PAUSE", KEYCODE_PAUSE, 0, "Pause", kActionKeyType, ~0},
	{"ESCAPE", KEYCODE_ESCAPE, ASCII_ESCAPE, "Esc", kStartKeyType, ~0},
	{"SPACE", KEYCODE_SPACE, ASCII_SPACE, "Space", kActionKeyType, ~0},
	{"EXCLAIM", KEYCODE_EXCLAIM, '!', "!", kActionKeyType, ~0},
	{"QUOTEDBL", KEYCODE_QUOTEDBL, '"', "\"", kActionKeyType, ~0},
	{"HASH", KEYCODE_HASH, '#', "#", kActionKeyType, ~0},
	{"DOLLAR", KEYCODE_DOLLAR, '$', "$", kActionKeyType, ~0},
	{"AMPERSAND", KEYCODE_AMPERSAND, '&', "&", kActionKeyType, ~0},
	{"QUOTE", KEYCODE_QUOTE, '\'', "'", kActionKeyType, ~0},
	{"LEFTPAREN", KEYCODE_LEFTPAREN, '(', "(", kActionKeyType, ~0},
	{"RIGHTPAREN", KEYCODE_RIGHTPAREN, ')', ")", kActionKeyType, ~0},
	{"ASTERISK", KEYCODE_ASTERISK, '*', "*", kActionKeyType, ~0},
	{"PLUS", KEYCODE_PLUS, '+', "+", kActionKeyType, ~0},
	{"COMMA", KEYCODE_COMMA, ',', ",", kActionKeyType, ~0},
	{"MINUS", KEYCODE_MINUS, '-', "-", kActionKeyType, ~0},
	{"PERIOD", KEYCODE_PERIOD, '.', ".", kActionKeyType, ~0},
	{"SLASH", KEYCODE_SLASH, '/', "/", kActionKeyType, ~0},
	{"0", KEYCODE_0, '0', "0", kActionKeyType, ~0},
	{"1", KEYCODE_1, '1', "1", kActionKeyType, ~0},
	{"2", KEYCODE_2, '2', "2", kActionKeyType, ~0},
	{"3", KEYCODE_3, '3', "3", kActionKeyType, ~0},
	{"4", KEYCODE_4, '4', "4", kActionKeyType, ~0},
	{"5", KEYCODE_5, '5', "5", kActionKeyType, ~0},
	{"6", KEYCODE_6, '6', "6", kActionKeyType, ~0},
	{"7", KEYCODE_7, '7', "7", kActionKeyType, ~0},
	{"8", KEYCODE_8, '8', "8", kActionKeyType, ~0},
	{"9", KEYCODE_9, '9', "9", kActionKeyType, ~0},
	{"COLON", KEYCODE_COLON, ':', ":", kActionKeyType, ~0},
	{"SEMICOLON", KEYCODE_SEMICOLON, ';', ";", kActionKeyType, ~0},
	{"LESS", KEYCODE_LESS, '<', "<", kActionKeyType, ~0},
	{"EQUALS", KEYCODE_EQUALS, '=', "=", kActionKeyType, ~0},
	{"GREATER", KEYCODE_GREATER, '>', ">", kActionKeyType, ~0},
	{"QUESTION", KEYCODE_QUESTION, '?', "?", kActionKeyType, ~0},
	{"AT", KEYCODE_AT, '@', "@", kActionKeyType, ~0},

	{"LEFTBRACKET", KEYCODE_LEFTBRACKET, '[', "[", kActionKeyType, ~0},
	{"BACKSLASH", KEYCODE_BACKSLASH, '\\', "\\", kActionKeyType, ~0},
	{"RIGHTBRACKET", KEYCODE_RIGHTBRACKET, ']', "]", kActionKeyType, ~0},
	{"CARET", KEYCODE_CARET, '^', "^", kActionKeyType, ~0},
	{"UNDERSCORE", KEYCODE_UNDERSCORE, '_', "_", kActionKeyType, ~0},
	{"BACKQUOTE", KEYCODE_BACKQUOTE, '`', "`", kActionKeyType, ~0},
	{"a", KEYCODE_a, 'A', "A", kActionKeyType, ~0},
	{"b", KEYCODE_b, 'B', "B", kActionKeyType, ~0},
	{"c", KEYCODE_c, 'C', "C", kActionKeyType, ~0},
	{"d", KEYCODE_d, 'D', "D", kActionKeyType, ~0},
	{"e", KEYCODE_e, 'E', "E", kActionKeyType, ~0},
	{"f", KEYCODE_f, 'F', "F", kActionKeyType, ~0},
	{"g", KEYCODE_g, 'G', "G", kActionKeyType, ~0},
	{"h", KEYCODE_h, 'H', "H", kActionKeyType, ~0},
	{"i", KEYCODE_i, 'I', "I", kActionKeyType, ~0},
	{"j", KEYCODE_j, 'J', "J", kActionKeyType, ~0},
	{"k", KEYCODE_k, 'K', "K", kActionKeyType, ~0},
	{"l", KEYCODE_l, 'L', "L", kActionKeyType, ~0},
	{"m", KEYCODE_m, 'M', "M", kActionKeyType, ~0},
	{"n", KEYCODE_n, 'N', "N", kActionKeyType, ~0},
	{"o", KEYCODE_o, 'O', "O", kActionKeyType, ~0},
	{"p", KEYCODE_p, 'P', "P", kActionKeyType, ~0},
	{"q", KEYCODE_q, 'Q', "Q", kActionKeyType, ~0},
	{"r", KEYCODE_r, 'R', "R", kActionKeyType, ~0},
	{"s", KEYCODE_s, 'S', "S", kActionKeyType, ~0},
	{"t", KEYCODE_t, 'T', "T", kActionKeyType, ~0},
	{"u", KEYCODE_u, 'U', "U", kActionKeyType, ~0},
	{"v", KEYCODE_v, 'V', "V", kActionKeyType, ~0},
	{"w", KEYCODE_w, 'W', "W", kActionKeyType, ~0},
	{"x", KEYCODE_x, 'X', "X", kActionKeyType, ~0},
	{"y", KEYCODE_y, 'Y', "Y", kActionKeyType, ~0},
	{"z", KEYCODE_z, 'Z', "Z", kActionKeyType, ~0},
	{"DELETE", KEYCODE_DELETE, 0, "Del", kActionKeyType, ~0},

	// Numeric keypad
	{"KP0", KEYCODE_KP0, 0, "KP0", kActionKeyType, ~0},
	{"KP1", KEYCODE_KP1, 0, "KP1", kActionKeyType, ~0},
	{"KP2", KEYCODE_KP2, 0, "KP2", kActionKeyType, ~0},
	{"KP3", KEYCODE_KP3, 0, "KP3", kActionKeyType, ~0},
	{"KP4", KEYCODE_KP4, 0, "KP4", kActionKeyType, ~0},
	{"KP5", KEYCODE_KP5, 0, "KP5", kActionKeyType, ~0},
	{"KP6", KEYCODE_KP6, 0, "KP6", kActionKeyType, ~0},
	{"KP7", KEYCODE_KP7, 0, "KP7", kActionKeyType, ~0},
	{"KP8", KEYCODE_KP8, 0, "KP8", kActionKeyType, ~0},
	{"KP9", KEYCODE_KP9, 0, "KP9", kActionKeyType, ~0},
	{"KP_PERIOD", KEYCODE_KP_PERIOD, 0, "KP.", kActionKeyType, ~0},
	{"KP_DIVIDE", KEYCODE_KP_DIVIDE, 0, "KP/", kActionKeyType, ~0},
	{"KP_MULTIPLY", KEYCODE_KP_MULTIPLY, 0, "KP*", kActionKeyType, ~0},
	{"KP_MINUS", KEYCODE_KP_MINUS, 0, "KP-", kActionKeyType, ~0},
	{"KP_PLUS", KEYCODE_KP_PLUS, 0, "KP+", kActionKeyType, ~0},
	{"KP_ENTER", KEYCODE_KP_ENTER, 0, "KP Enter", kActionKeyType, ~0},
	{"KP_EQUALS", KEYCODE_KP_EQUALS, 0, "KP=", kActionKeyType, ~0},

	// Arrows + Home/End pad
	{"UP", KEYCODE_UP, 0, "Up", kDirUpKeyType, ~0},
	{"DOWN", KEYCODE_DOWN, 0, "Down", kDirDownKeyType, ~0},
	{"RIGHT", KEYCODE_RIGHT, 0, "Right", kDirRightKeyType, ~0},
	{"LEFT", KEYCODE_LEFT, 0, "Left", kDirLeftKeyType, ~0},
	{"INSERT", KEYCODE_INSERT, 0, "Insert", kActionKeyType, ~0},
	{"HOME", KEYCODE_HOME, 0, "Home", kActionKeyType, ~0},
	{"END", KEYCODE_END, 0, "End", kActionKeyType, ~0},
	{"PAGEUP", KEYCODE_PAGEUP, 0, "PgUp", kActionKeyType, ~0},
	{"PAGEDOWN", KEYCODE_PAGEDOWN, 0, "PgDn", kActionKeyType, ~0},

	// Function keys
	{"F1", KEYCODE_F1, ASCII_F1, "F1", kActionKeyType, ~0},
	{"F2", KEYCODE_F2, ASCII_F2, "F2", kActionKeyType, ~0},
	{"F3", KEYCODE_F3, ASCII_F3, "F3", kActionKeyType, ~0},
	{"F4", KEYCODE_F4, ASCII_F4, "F4", kActionKeyType, ~0},
	{"F5", KEYCODE_F5, ASCII_F5, "F5", kActionKeyType, ~0},
	{"F6", KEYCODE_F6, ASCII_F6, "F6", kActionKeyType, ~0},
	{"F7", KEYCODE_F7, ASCII_F7, "F7", kActionKeyType, ~0},
	{"F8", KEYCODE_F8, ASCII_F8, "F8", kActionKeyType, ~0},
	{"F9", KEYCODE_F9, ASCII_F9, "F9", kActionKeyType, ~0},
	{"F10", KEYCODE_F10, ASCII_F10, "F10", kActionKeyType, ~0},
	{"F11", KEYCODE_F11, ASCII_F11, "F11", kActionKeyType, ~0},
	{"F12", KEYCODE_F12, ASCII_F12, "F12", kActionKeyType, ~0},
	{"F13", KEYCODE_F13, 0, "F13", kActionKeyType, ~0},
	{"F14", KEYCODE_F14, 0, "F14", kActionKeyType, ~0},
	{"F15", KEYCODE_F15, 0, "F15", kActionKeyType, ~0},


//	// Modifier keys pressed alone
//	{"RSHIFT", KEYCODE_RSHIFT, 0, "Right Shift", kModiferKeyType, ~KBD_SHIFT},
//	{"LSHIFT", KEYCODE_LSHIFT, 0, "Left Shift", kModiferKeyType, ~KBD_SHIFT},
//	{"RCTRL", KEYCODE_RCTRL, 0, "Right Ctrl", kModiferKeyType, ~KBD_CTRL},
//	{"LCTRL", KEYCODE_LCTRL, 0, "Left Ctrl", kModiferKeyType, ~KBD_CTRL},
//	{"RALT", KEYCODE_RALT, 0, "Right Alt", kModiferKeyType, ~KBD_ALT},
//	{"LALT", KEYCODE_LALT, 0, "Left Alt", kModiferKeyType, ~KBD_ALT},


	// Miscellaneous function keys
	{"HELP", KEYCODE_HELP, 0, "Help", kActionKeyType, ~0},
	{"PRINT", KEYCODE_PRINT, 0, "Print", kActionKeyType, ~0},
	{"SYSREQ", KEYCODE_SYSREQ, 0, "SysRq", kActionKeyType, ~0},
	{"BREAK", KEYCODE_BREAK, 0, "Break", kActionKeyType, ~0},
	{"MENU", KEYCODE_MENU, 0, "Menu", kActionKeyType, ~0},
		// Power Macintosh power key
	{"POWER", KEYCODE_POWER, 0, "Power", kActionKeyType, ~0},
		// Some european keyboards
	{"EURO", KEYCODE_EURO, 0, "Euro", kActionKeyType, ~0},
		// Atari keyboard has Undo
	{"UNDO", KEYCODE_UNDO, 0, "Undo", kActionKeyType, ~0},
	{0, KEYCODE_INVALID, 0, 0, kGenericKeyType, ~0}
};

struct Mod {
	byte flag;
	const char *id;
	const char *desc;
};

static const Mod modifiers[] = {
	{ 0, "", "" },
	{ KBD_CTRL, "C+", "Ctrl+" },
	{ KBD_ALT, "A+", "Alt+" },
	{ KBD_SHIFT, "S+", "Shift+" },
	{ KBD_CTRL | KBD_ALT, "C+A+", "Ctrl+Alt+" },
	{ KBD_SHIFT | KBD_CTRL, "S+C+", "Shift+Ctrl+" },
	{ KBD_SHIFT | KBD_CTRL | KBD_ALT, "S+C+A+", "Shift+Ctrl+Alt+" },
	{ 0, 0, 0 }
};
#endif


Common::HardwareKeySet *OSystem_SDL::getHardwareKeySet() {
#ifdef ENABLE_KEYMAPPER
	HardwareKeySet *keySet = new HardwareKeySet();
	const Key *key;
	const Mod *mod;
	char fullKeyId[50];
	char fullKeyDesc[100];
	uint16 ascii;

	for (mod = modifiers; mod->id; mod++) {
		snprintf(fullKeyId, 50, "S+%s", mod->id);
		snprintf(fullKeyDesc, 100, "Shift+%s", mod->desc);

		keySet->addHardwareMod(new HardwareMod(fullKeyId, mod->flag, fullKeyDesc));
	}
	for (key = keys; key->hwId; key++) {
		ascii = key->ascii;

		snprintf(fullKeyId, 50, "%s", key->hwId);
		snprintf(fullKeyDesc, 100, "%s", key->desc);

		keySet->addHardwareKey(new HardwareKey(fullKeyId, KeyState(key->keycode, ascii, 0), fullKeyDesc, key->preferredAction));
	}

	return keySet;

#else
	return 0;
#endif
}
