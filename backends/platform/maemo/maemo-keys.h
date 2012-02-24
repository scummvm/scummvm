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

#include "backends/keymapper/hardware-input.h"

namespace Common {

static const ModifierTableEntry maemoModifiers[] = {
	{ 0, "", "", false },
	{ KBD_CTRL, "C+", "Ctrl+", false },
	{ KBD_SHIFT, "", "", true },
	{ KBD_SHIFT | KBD_CTRL, "S+C+", "Shift+Ctrl+", true },
	{ 0, 0, 0, false }
};

static const KeyTableEntry maemoKeys[] = {
	{"BACKSPACE", KEYCODE_BACKSPACE, ASCII_BACKSPACE, "Backspace", false},
	{"TAB", KEYCODE_TAB, ASCII_TAB, "Tab", false},
	{"CLEAR", KEYCODE_CLEAR, 0, "Clear", false},
	{"RETURN", KEYCODE_RETURN, ASCII_RETURN, "MCenter", false},
	{"ESCAPE", KEYCODE_ESCAPE, ASCII_ESCAPE, "Esc", false},
	{"SPACE", KEYCODE_SPACE, ASCII_SPACE, "Space", false},
	{"EXCLAIM", KEYCODE_EXCLAIM, '!', "!", false},
	{"QUOTEDBL", KEYCODE_QUOTEDBL, '"', "\"", false},
	{"HASH", KEYCODE_HASH, '#', "#", false},
	{"DOLLAR", KEYCODE_DOLLAR, '$', "$", false},
	{"AMPERSAND", KEYCODE_AMPERSAND, '&', "&", false},
	{"QUOTE", KEYCODE_QUOTE, '\'', "'", false},
	{"LEFTPAREN", KEYCODE_LEFTPAREN, '(', "(", false},
	{"RIGHTPAREN", KEYCODE_RIGHTPAREN, ')', ")", false},
	{"ASTERISK", KEYCODE_ASTERISK, '*', "*", false},
	{"PLUS", KEYCODE_PLUS, '+', "+", false},
	{"COMMA", KEYCODE_COMMA, ',', ",", false},
	{"MINUS", KEYCODE_MINUS, '-', "-", false},
	{"PERIOD", KEYCODE_PERIOD, '.', ".", false},
	{"SLASH", KEYCODE_SLASH, '/', "/", false},
	{"0", KEYCODE_0, '0', "0", false},
	{"1", KEYCODE_1, '1', "1", false},
	{"2", KEYCODE_2, '2', "2", false},
	{"3", KEYCODE_3, '3', "3", false},
	{"4", KEYCODE_4, '4', "4", false},
	{"5", KEYCODE_5, '5', "5", false},
	{"6", KEYCODE_6, '6', "6", false},
	{"7", KEYCODE_7, '7', "7", false},
	{"8", KEYCODE_8, '8', "8", false},
	{"9", KEYCODE_9, '9', "9", false},
	{"COLON", KEYCODE_COLON, ':', ":", false},
	{"SEMICOLON", KEYCODE_SEMICOLON, ';', ";", false},
	{"LESS", KEYCODE_LESS, '<', "<", false},
	{"EQUALS", KEYCODE_EQUALS, '=', "=", false},
	{"GREATER", KEYCODE_GREATER, '>', ">", false},
	{"QUESTION", KEYCODE_QUESTION, '?', "?", false},
	{"AT", KEYCODE_AT, '@', "@", false},

	{"LEFTBRACKET", KEYCODE_LEFTBRACKET, '[', "[", false},
	{"BACKSLASH", KEYCODE_BACKSLASH, '\\', "\\", false},
	{"RIGHTBRACKET", KEYCODE_RIGHTBRACKET, ']', "]", false},
	{"CARET", KEYCODE_CARET, '^', "^", false},
	{"UNDERSCORE", KEYCODE_UNDERSCORE, '_', "_", false},
	{"BACKQUOTE", KEYCODE_BACKQUOTE, '`', "`", false},
	{"a", KEYCODE_a, 'a', "a", true},
	{"b", KEYCODE_b, 'b', "b", true},
	{"c", KEYCODE_c, 'c', "c", true},
	{"d", KEYCODE_d, 'd', "d", true},
	{"e", KEYCODE_e, 'e', "e", true},
	{"f", KEYCODE_f, 'f', "f", true},
	{"g", KEYCODE_g, 'g', "g", true},
	{"h", KEYCODE_h, 'h', "h", true},
	{"i", KEYCODE_i, 'i', "i", true},
	{"j", KEYCODE_j, 'j', "j", true},
	{"k", KEYCODE_k, 'k', "k", true},
	{"l", KEYCODE_l, 'l', "l", true},
	{"m", KEYCODE_m, 'm', "m", true},
	{"n", KEYCODE_n, 'n', "n", true},
	{"o", KEYCODE_o, 'o', "o", true},
	{"p", KEYCODE_p, 'p', "p", true},
	{"q", KEYCODE_q, 'q', "q", true},
	{"r", KEYCODE_r, 'r', "r", true},
	{"s", KEYCODE_s, 's', "s", true},
	{"t", KEYCODE_t, 't', "t", true},
	{"u", KEYCODE_u, 'u', "u", true},
	{"v", KEYCODE_v, 'v', "v", true},
	{"w", KEYCODE_w, 'w', "w", true},
	{"x", KEYCODE_x, 'x', "x", true},
	{"y", KEYCODE_y, 'y', "y", true},
	{"z", KEYCODE_z, 'z', "z", true},
	{"DELETE", KEYCODE_DELETE, 0, "Del", false},

	{"KP_ENTER", KEYCODE_KP_ENTER, 0, "Enter", false},

	// Arrows + Home/End pad
	{"UP", KEYCODE_UP, 0, "Up", false},
	{"DOWN", KEYCODE_DOWN, 0, "Down", false},
	{"RIGHT", KEYCODE_RIGHT, 0, "Right", false},
	{"LEFT", KEYCODE_LEFT, 0, "Left", false},

	// Function keys
	{"MENU", KEYCODE_F11, 0, "Menu", false},
	{"HOME", KEYCODE_F12, 0, "Home", false},
	{"FULLSCREEN", KEYCODE_F13, 0, "FullScreen", false},
	{"ZOOMPLUS", KEYCODE_F14, 0, "Zoom+", false},
	{"ZOOMMINUS", KEYCODE_F15, 0, "Zoom-", false},

	{0, KEYCODE_INVALID, 0, 0, false}
};

} // namespace Common

#endif // ifndef PLATFORM_SDL_MAEMO_KEYS_H

#endif // if defined(ENABLE_KEYMAPPER)
#endif // if defined(MAEMO)
