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

#include "common/keyboard.h"

namespace ZVision {

uint8 VKkey(Common::KeyCode scumm_key) {
	if (scumm_key >= Common::KEYCODE_a && scumm_key <= Common::KEYCODE_z)
		return 0x41 + scumm_key - Common::KEYCODE_a;
	if (scumm_key >= Common::KEYCODE_0 && scumm_key <= Common::KEYCODE_9)
		return 0x30 + scumm_key - Common::KEYCODE_0;
	if (scumm_key >= Common::KEYCODE_F1 && scumm_key <= Common::KEYCODE_F15)
		return 0x70 + scumm_key - Common::KEYCODE_F1;
	if (scumm_key >= Common::KEYCODE_KP0 && scumm_key <= Common::KEYCODE_KP9)
		return 0x60 + scumm_key - Common::KEYCODE_KP0;

	switch (scumm_key) {
	case Common::KEYCODE_BACKSPACE:
		return 0x8;
	case Common::KEYCODE_TAB:
		return 0x9;
	case Common::KEYCODE_CLEAR:
		return 0xC;
	case Common::KEYCODE_RETURN:
		return 0xD;
	case Common::KEYCODE_CAPSLOCK:
		return 0x14;
	case Common::KEYCODE_ESCAPE:
		return 0x1B;
	case Common::KEYCODE_SPACE:
		return 0x20;
	case Common::KEYCODE_PAGEUP:
		return 0x21;
	case Common::KEYCODE_PAGEDOWN:
		return 0x22;
	case Common::KEYCODE_END:
		return 0x23;
	case Common::KEYCODE_HOME:
		return 0x24;
	case Common::KEYCODE_LEFT:
		return 0x25;
	case Common::KEYCODE_UP:
		return 0x26;
	case Common::KEYCODE_RIGHT:
		return 0x27;
	case Common::KEYCODE_DOWN:
		return 0x28;
	case Common::KEYCODE_PRINT:
		return 0x2A;
	case Common::KEYCODE_INSERT:
		return 0x2D;
	case Common::KEYCODE_DELETE:
		return 0x2E;
	case Common::KEYCODE_HELP:
		return 0x2F;
	case Common::KEYCODE_KP_MULTIPLY:
		return 0x6A;
	case Common::KEYCODE_KP_PLUS:
		return 0x6B;
	case Common::KEYCODE_KP_MINUS:
		return 0x6D;
	case Common::KEYCODE_KP_PERIOD:
		return 0x6E;
	case Common::KEYCODE_KP_DIVIDE:
		return 0x6F;
	case Common::KEYCODE_NUMLOCK:
		return 0x90;
	case Common::KEYCODE_SCROLLOCK:
		return 0x91;
	case Common::KEYCODE_LSHIFT:
		return 0xA0;
	case Common::KEYCODE_RSHIFT:
		return 0xA1;
	case Common::KEYCODE_LCTRL:
		return 0xA2;
	case Common::KEYCODE_RCTRL:
		return 0xA3;
	case Common::KEYCODE_MENU:
		return 0xA5;
	case Common::KEYCODE_LEFTBRACKET:
		return 0xDB;
	case Common::KEYCODE_RIGHTBRACKET:
		return 0xDD;
	case Common::KEYCODE_SEMICOLON:
		return 0xBA;
	case Common::KEYCODE_BACKSLASH:
		return 0xDC;
	case Common::KEYCODE_QUOTE:
		return 0xDE;
	case Common::KEYCODE_SLASH:
		return 0xBF;
	case Common::KEYCODE_TILDE:
		return 0xC0;
	case Common::KEYCODE_COMMA:
		return 0xBC;
	case Common::KEYCODE_PERIOD:
		return 0xBE;
	case Common::KEYCODE_MINUS:
		return 0xBD;
	case Common::KEYCODE_PLUS:
		return 0xBB;
	default:
		return 0;
	}

	return 0;
}

}
