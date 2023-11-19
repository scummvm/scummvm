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

#ifndef WATCHMAKER_WINDOWS_HACKS_H
#define WATCHMAKER_WINDOWS_HACKS_H

#include "watchmaker/utils.h"
#include "watchmaker/types.h"

namespace Watchmaker {

// HACKY:
#define RGBA_MAKE(r, g, b, a) ((r << 24) | (g << 16) | (b << 8) | a)
#define RGBA_GETRED(rgba) (rgba & (0xFF << 24) )
#define RGBA_GETGREEN(rgba) (rgba & (0xFF << 16) )
#define RGBA_GETBLUE(rgba) (rgba & (0xFF << 8) )
#define RGBA_GETALPHA(rgba) (rgba & (0xFF << 0) )
#define TRUE true
#define FALSE false
typedef short WORD;
typedef int DWORD;


enum KeyCodes {
	VK_SPACE,
	VK_BACK,
	VK_DELETE,
	VK_RETURN,
	VK_TAB
};

uint32 timeGetTime();

void ResetDIKbd();
void CopyFile(char *src, char *dst, bool noOverwrite);
} // End of namespace Watchmaker

#endif // WATCHMAKER_WINDOWS_HACKS_H
