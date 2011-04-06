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

#include "backends/platform/webos/webos.h"
#include "backends/platform/webos/keyboard.h"
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
	{"RETURN", KEYCODE_RETURN, ASCII_RETURN, "Return", kActionKeyType, false},
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
	{"EQUALS", KEYCODE_EQUALS, '=', "=", kActionKeyType, false},
	{"QUESTION", KEYCODE_QUESTION, '?', "?", kActionKeyType, false},
	{"AT", KEYCODE_AT, '@', "@", kActionKeyType, false},

	{"UNDERSCORE", KEYCODE_UNDERSCORE, '_', "_", kActionKeyType, false},
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

	// Miscellaneous function keys
	{ "FORWARD", (KeyCode) KEYCODE_FORWARD, ASCII_FORWARD, "Forward", kActionKeyType, false },

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
	{ KBD_SHIFT, "", "", true },
	{ KBD_SHIFT | KBD_CTRL, "S+C+", "Shift+Ctrl+", true },
	{ 0, 0, 0, false }
};
#endif

Common::HardwareKeySet *OSystem_SDL_WebOS::getHardwareKeySet() {
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
