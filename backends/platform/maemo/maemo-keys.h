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

#if defined(MAEMO)
#if defined(ENABLE_KEYMAPPER)

#ifndef PLATFORM_SDL_MAEMO_KEYS_H
#define PLATFORM_SDL_MAEMO_KEYS_H

#include "common/keyboard.h"

#include "backends/keymapper/hardware-key.h"

namespace Common {

static const ModifierTableEntry maemoModifiers[] = {
	{ 0, "", "", false },
	{ KBD_CTRL, "C+", "Ctrl+", false },
	{ KBD_SHIFT, "", "", true },
	{ KBD_SHIFT | KBD_CTRL, "S+C+", "Shift+Ctrl+", true },
	{ 0, 0, 0, false }
};

static const KeyTableEntry maemoKeys[] = {
	{"BACKSPACE", KEYCODE_BACKSPACE, ASCII_BACKSPACE, "Backspace", kActionKeyType, false},
	{"TAB", KEYCODE_TAB, ASCII_TAB, "Tab", kActionKeyType, false},
	{"CLEAR", KEYCODE_CLEAR, 0, "Clear", kActionKeyType, false},
	{"RETURN", KEYCODE_RETURN, ASCII_RETURN, "MCenter", kActionKeyType, false},
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

	{"KP_ENTER", KEYCODE_KP_ENTER, 0, "Enter", kActionKeyType, false},

	// Arrows + Home/End pad
	{"UP", KEYCODE_UP, 0, "Up", kDirUpKeyType, false},
	{"DOWN", KEYCODE_DOWN, 0, "Down", kDirDownKeyType, false},
	{"RIGHT", KEYCODE_RIGHT, 0, "Right", kDirRightKeyType, false},
	{"LEFT", KEYCODE_LEFT, 0, "Left", kDirLeftKeyType, false},

	// Function keys
	{"MENU", KEYCODE_F11, 0, "Menu", kActionKeyType, false},
	{"HOME", KEYCODE_F12, 0, "Home", kActionKeyType, false},
	{"FULLSCREEN", KEYCODE_F13, 0, "FullScreen", kActionKeyType, false},
	{"ZOOMPLUS", KEYCODE_F14, 0, "Zoom+", kActionKeyType, false},
	{"ZOOMMINUS", KEYCODE_F15, 0, "Zoom-", kActionKeyType, false},

	{0, KEYCODE_INVALID, 0, 0, kGenericKeyType, false}
};


} // namespace Common

#endif // ifndef PLATFORM_SDL_MAEMO_KEYS_H

#endif // if defined(ENABLE_KEYMAPPER)
#endif // if defined(MAEMO)
