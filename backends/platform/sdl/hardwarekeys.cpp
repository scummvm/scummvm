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
	bool shiftable;
};

static const Key keys[] = {
	{"BACKSPACE", KEYCODE_BACKSPACE, ASCII_BACKSPACE, "Backspace", kActionKeyType, false},
	{"TAB", KEYCODE_TAB, ASCII_TAB, "Tab", kActionKeyType, false},
	{"CLEAR", KEYCODE_CLEAR, 0, "Clear", kActionKeyType, false},
	{"RETURN", KEYCODE_RETURN, ASCII_RETURN, "Return", kActionKeyType, false},
	{"PAUSE", KEYCODE_PAUSE, 0, "Pause", kActionKeyType, false},
	{"ESCAPE", KEYCODE_ESCAPE, ASCII_ESCAPE, "Esc", kStartKeyType, false},
	{"SPACE", KEYCODE_SPACE, ASCII_SPACE, "Space", kActionKeyType, false},
	{"EXCLAIM", KEYCODE_EXCLAIM, '!', "!", kActionKeyType, false},
	{"QUOTEDBL", KEYCODE_QUOTEDBL, '"', "\"", kActionKeyType, false},
	{"HASH", KEYCODE_HASH, '#', "#", kActionKeyType, false},
	{"DOLLAR", KEYCODE_DOLLAR, '$', "$", kActionKeyType, false},
	{"AMPERSAND", KEYCODE_AMPERSAND, '&', "&", kActionKeyType, false},
	{"QUOTE", KEYCODE_QUOTE, '\'', "'", kActionKeyType, false},
	{"LEFTPAREN", KEYCODE_LEFTPAREN, '(', "(", kActionKeyType, false},
	{"RIGHTPAREN", KEYCODE_RIGHTPAREN, ')', ")", kActionKeyType, false},
	{"ASTERISK", KEYCODE_ASTERISK, '*', "*", kActionKeyType, false},
	{"PLUS", KEYCODE_PLUS, '+', "+", kActionKeyType, false},
	{"COMMA", KEYCODE_COMMA, ',', ",", kActionKeyType, false},
	{"MINUS", KEYCODE_MINUS, '-', "-", kActionKeyType, false},
	{"PERIOD", KEYCODE_PERIOD, '.', ".", kActionKeyType, false},
	{"SLASH", KEYCODE_SLASH, '/', "/", kActionKeyType, false},
	{"0", KEYCODE_0, '0', "0", kActionKeyType, false},
	{"1", KEYCODE_1, '1', "1", kActionKeyType, false},
	{"2", KEYCODE_2, '2', "2", kActionKeyType, false},
	{"3", KEYCODE_3, '3', "3", kActionKeyType, false},
	{"4", KEYCODE_4, '4', "4", kActionKeyType, false},
	{"5", KEYCODE_5, '5', "5", kActionKeyType, false},
	{"6", KEYCODE_6, '6', "6", kActionKeyType, false},
	{"7", KEYCODE_7, '7', "7", kActionKeyType, false},
	{"8", KEYCODE_8, '8', "8", kActionKeyType, false},
	{"9", KEYCODE_9, '9', "9", kActionKeyType, false},
	{"COLON", KEYCODE_COLON, ':', ":", kActionKeyType, false},
	{"SEMICOLON", KEYCODE_SEMICOLON, ';', ";", kActionKeyType, false},
	{"LESS", KEYCODE_LESS, '<', "<", kActionKeyType, false},
	{"EQUALS", KEYCODE_EQUALS, '=', "=", kActionKeyType, false},
	{"GREATER", KEYCODE_GREATER, '>', ">", kActionKeyType, false},
	{"QUESTION", KEYCODE_QUESTION, '?', "?", kActionKeyType, false},
	{"AT", KEYCODE_AT, '@', "@", kActionKeyType, false},

	{"LEFTBRACKET", KEYCODE_LEFTBRACKET, '[', "[", kActionKeyType, false},
	{"BACKSLASH", KEYCODE_BACKSLASH, '\\', "\\", kActionKeyType, false},
	{"RIGHTBRACKET", KEYCODE_RIGHTBRACKET, ']', "]", kActionKeyType, false},
	{"CARET", KEYCODE_CARET, '^', "^", kActionKeyType, false},
	{"UNDERSCORE", KEYCODE_UNDERSCORE, '_', "_", kActionKeyType, false},
	{"BACKQUOTE", KEYCODE_BACKQUOTE, '`', "`", kActionKeyType, false},
	{"a", KEYCODE_a, 'a', "a", kActionKeyType, true},
	{"b", KEYCODE_b, 'b', "b", kActionKeyType, true},
	{"c", KEYCODE_c, 'c', "c", kActionKeyType, true},
	{"d", KEYCODE_d, 'd', "d", kActionKeyType, true},
	{"e", KEYCODE_e, 'e', "e", kActionKeyType, true},
	{"f", KEYCODE_f, 'f', "f", kActionKeyType, true},
	{"g", KEYCODE_g, 'g', "g", kActionKeyType, true},
	{"h", KEYCODE_h, 'h', "h", kActionKeyType, true},
	{"i", KEYCODE_i, 'i', "i", kActionKeyType, true},
	{"j", KEYCODE_j, 'j', "j", kActionKeyType, true},
	{"k", KEYCODE_k, 'k', "k", kActionKeyType, true},
	{"l", KEYCODE_l, 'l', "l", kActionKeyType, true},
	{"m", KEYCODE_m, 'm', "m", kActionKeyType, true},
	{"n", KEYCODE_n, 'n', "n", kActionKeyType, true},
	{"o", KEYCODE_o, 'o', "o", kActionKeyType, true},
	{"p", KEYCODE_p, 'p', "p", kActionKeyType, true},
	{"q", KEYCODE_q, 'q', "q", kActionKeyType, true},
	{"r", KEYCODE_r, 'r', "r", kActionKeyType, true},
	{"s", KEYCODE_s, 's', "s", kActionKeyType, true},
	{"t", KEYCODE_t, 't', "t", kActionKeyType, true},
	{"u", KEYCODE_u, 'u', "u", kActionKeyType, true},
	{"v", KEYCODE_v, 'v', "v", kActionKeyType, true},
	{"w", KEYCODE_w, 'w', "w", kActionKeyType, true},
	{"x", KEYCODE_x, 'x', "x", kActionKeyType, true},
	{"y", KEYCODE_y, 'y', "y", kActionKeyType, true},
	{"z", KEYCODE_z, 'z', "z", kActionKeyType, true},
	{"DELETE", KEYCODE_DELETE, 0, "Del", kActionKeyType, false},

	// Numeric keypad
	{"KP0", KEYCODE_KP0, 0, "KP0", kActionKeyType, false},
	{"KP1", KEYCODE_KP1, 0, "KP1", kActionKeyType, false},
	{"KP2", KEYCODE_KP2, 0, "KP2", kActionKeyType, false},
	{"KP3", KEYCODE_KP3, 0, "KP3", kActionKeyType, false},
	{"KP4", KEYCODE_KP4, 0, "KP4", kActionKeyType, false},
	{"KP5", KEYCODE_KP5, 0, "KP5", kActionKeyType, false},
	{"KP6", KEYCODE_KP6, 0, "KP6", kActionKeyType, false},
	{"KP7", KEYCODE_KP7, 0, "KP7", kActionKeyType, false},
	{"KP8", KEYCODE_KP8, 0, "KP8", kActionKeyType, false},
	{"KP9", KEYCODE_KP9, 0, "KP9", kActionKeyType, false},
	{"KP_PERIOD", KEYCODE_KP_PERIOD, 0, "KP.", kActionKeyType, false},
	{"KP_DIVIDE", KEYCODE_KP_DIVIDE, 0, "KP/", kActionKeyType, false},
	{"KP_MULTIPLY", KEYCODE_KP_MULTIPLY, 0, "KP*", kActionKeyType, false},
	{"KP_MINUS", KEYCODE_KP_MINUS, 0, "KP-", kActionKeyType, false},
	{"KP_PLUS", KEYCODE_KP_PLUS, 0, "KP+", kActionKeyType, false},
	{"KP_ENTER", KEYCODE_KP_ENTER, 0, "KP Enter", kActionKeyType, false},
	{"KP_EQUALS", KEYCODE_KP_EQUALS, 0, "KP=", kActionKeyType, false},

	// Arrows + Home/End pad
	{"UP", KEYCODE_UP, 0, "Up", kDirUpKeyType, false},
	{"DOWN", KEYCODE_DOWN, 0, "Down", kDirDownKeyType, false},
	{"RIGHT", KEYCODE_RIGHT, 0, "Right", kDirRightKeyType, false},
	{"LEFT", KEYCODE_LEFT, 0, "Left", kDirLeftKeyType, false},
	{"INSERT", KEYCODE_INSERT, 0, "Insert", kActionKeyType, false},
	{"HOME", KEYCODE_HOME, 0, "Home", kActionKeyType, false},
	{"END", KEYCODE_END, 0, "End", kActionKeyType, false},
	{"PAGEUP", KEYCODE_PAGEUP, 0, "PgUp", kActionKeyType, false},
	{"PAGEDOWN", KEYCODE_PAGEDOWN, 0, "PgDn", kActionKeyType, false},

	// Function keys
	{"F1", KEYCODE_F1, ASCII_F1, "F1", kActionKeyType, false},
	{"F2", KEYCODE_F2, ASCII_F2, "F2", kActionKeyType, false},
	{"F3", KEYCODE_F3, ASCII_F3, "F3", kActionKeyType, false},
	{"F4", KEYCODE_F4, ASCII_F4, "F4", kActionKeyType, false},
	{"F5", KEYCODE_F5, ASCII_F5, "F5", kActionKeyType, false},
	{"F6", KEYCODE_F6, ASCII_F6, "F6", kActionKeyType, false},
	{"F7", KEYCODE_F7, ASCII_F7, "F7", kActionKeyType, false},
	{"F8", KEYCODE_F8, ASCII_F8, "F8", kActionKeyType, false},
	{"F9", KEYCODE_F9, ASCII_F9, "F9", kActionKeyType, false},
	{"F10", KEYCODE_F10, ASCII_F10, "F10", kActionKeyType, false},
	{"F11", KEYCODE_F11, ASCII_F11, "F11", kActionKeyType, false},
	{"F12", KEYCODE_F12, ASCII_F12, "F12", kActionKeyType, false},
	{"F13", KEYCODE_F13, 0, "F13", kActionKeyType, false},
	{"F14", KEYCODE_F14, 0, "F14", kActionKeyType, false},
	{"F15", KEYCODE_F15, 0, "F15", kActionKeyType, false},

	// Miscellaneous function keys
	{"HELP", KEYCODE_HELP, 0, "Help", kActionKeyType, false},
	{"PRINT", KEYCODE_PRINT, 0, "Print", kActionKeyType, false},
	{"SYSREQ", KEYCODE_SYSREQ, 0, "SysRq", kActionKeyType, false},
	{"BREAK", KEYCODE_BREAK, 0, "Break", kActionKeyType, false},
	{"MENU", KEYCODE_MENU, 0, "Menu", kActionKeyType, false},
		// Power Macintosh power key
	{"POWER", KEYCODE_POWER, 0, "Power", kActionKeyType, false},
		// Some european keyboards
	{"EURO", KEYCODE_EURO, 0, "Euro", kActionKeyType, false},
		// Atari keyboard has Undo
	{"UNDO", KEYCODE_UNDO, 0, "Undo", kActionKeyType, false},
	{0, KEYCODE_INVALID, 0, 0, kGenericKeyType, false}
};

struct Mod {
	byte flag;
	const char *id;
	const char *desc;
	bool shiftable;
};

static const Mod modifiers[] = {
	{ 0, "", "", false },
	{ KBD_CTRL, "C+", "Ctrl+", false },
	{ KBD_ALT, "A+", "Alt+", false },
	{ KBD_SHIFT, "", "", true },
	{ KBD_CTRL | KBD_ALT, "C+A+", "Ctrl+Alt+", false },
	{ KBD_SHIFT | KBD_CTRL, "S+C+", "Shift+Ctrl+", true },
	{ KBD_SHIFT | KBD_CTRL | KBD_ALT, "C+A+", "Ctrl+Alt+", true },
	{ 0, 0, 0, false }
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
		for (key = keys; key->hwId; key++) {
			ascii = key->ascii;

			if (mod->shiftable && key->shiftable) {
				snprintf(fullKeyId, 50, "%s%c", mod->id, toupper(key->hwId[0]));
				snprintf(fullKeyDesc, 100, "%s%c", mod->desc, toupper(key->desc[0]));
				ascii = toupper(key->ascii);
			} else if (mod->shiftable) {
				snprintf(fullKeyId, 50, "S+%s%s", mod->id, key->hwId);
				snprintf(fullKeyDesc, 100, "Shift+%s%s", mod->desc, key->desc);
			} else {
				snprintf(fullKeyId, 50, "%s%s", mod->id, key->hwId);
				snprintf(fullKeyDesc, 100, "%s%s", mod->desc, key->desc);
			}

			keySet->addHardwareKey(new HardwareKey(fullKeyId, KeyState(key->keycode, ascii, mod->flag), fullKeyDesc, key->preferredAction ));
		}
	}

	return keySet;

#else
	return 0;
#endif
}
