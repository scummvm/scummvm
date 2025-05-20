/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BAGEL_MFC_KEYBOARD_H
#define BAGEL_MFC_KEYBOARD_H

#include "common/events.h"

namespace Bagel {
namespace MFC {

enum {
	VK_UP = Common::KEYCODE_UP,
	VK_DOWN = Common::KEYCODE_DOWN,                             // go to next page of text
	VK_LEFT = Common::KEYCODE_LEFT,
	VK_RIGHT = Common::KEYCODE_RIGHT,
	VK_NUMPAD1 = Common::KEYCODE_KP1,
	VK_NUMPAD2 = Common::KEYCODE_KP2,
	VK_NUMPAD3 = Common::KEYCODE_KP3,
	VK_NUMPAD4 = Common::KEYCODE_KP4,
	VK_NUMPAD5 = Common::KEYCODE_KP5,
	VK_NUMPAD6 = Common::KEYCODE_KP6,
	VK_NUMPAD7 = Common::KEYCODE_KP7,
	VK_NUMPAD8 = Common::KEYCODE_KP8,
	VK_NUMPAD9 = Common::KEYCODE_KP9,
	VK_PRIOR = Common::KEYCODE_PAGEUP,
	VK_NEXT = Common::KEYCODE_PAGEDOWN,
	VK_HOME = Common::KEYCODE_HOME,
	VK_END = Common::KEYCODE_END,
	VK_BACK = Common::KEYCODE_BACKSPACE,
	VK_SCROLL = Common::KEYCODE_SCROLLOCK,
	VK_F1 = Common::KEYCODE_F1,
	VK_F2 = Common::KEYCODE_F2,
	VK_F3 = Common::KEYCODE_F3,
	VK_F4 = Common::KEYCODE_F4,
	VK_F8 = Common::KEYCODE_F8,
	VK_RETURN = Common::KEYCODE_RETURN,
	VK_ESCAPE = Common::KEYCODE_ESCAPE,
	VK_SPACE = Common::KEYCODE_SPACE
};

} // namespace MFC
} // namespace Bagel

#endif
