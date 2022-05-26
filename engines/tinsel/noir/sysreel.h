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

#ifndef TINSEL_SYSREEL_H // prevent multiple includes
#define TINSEL_SYSREEL_H

#include "tinsel/dw.h"

namespace Tinsel {

enum class SysReel {
	NOTEPAD_OPENING = 4,
	NOTEPAD_OPEN = 5,
	NOTEPAD_CLOSED = 6,
	NOTEPAD_FLIPDOWN = 7,
	NOTEPAD_FLIPUP = 8,
	SCRIBBLES = 9,
	CURSOR = 11,
	INVMAIN = 15,
	SLIDER = 16,
	CONVERSATION_FRAME = 19,
	OPTIONS_MENU = 21,
	LOADSAVE_MENU = 22,
	QUIT_MENU = 23,
	SUBTITLES_MENU = 24,
	SLIDER_BG = 25,
	SLIDER_HI = 26,
	LEFT = 29,
	RIGHT = 30,
	LOADSCREEN = 31
};

class SystemReel {
public:
	SystemReel() = default;

	SCNHANDLE Get(SysReel index);
	void Set(int32 index, SCNHANDLE reel);

private:
	const static int32 MAX_SYSREELS = 0x28;

	SCNHANDLE _reels[MAX_SYSREELS];
};

} // End of namespace Tinsel

#endif
