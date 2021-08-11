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

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "SDL_syswm.h"
#include "backends/graphics/sdl/sdl-graphics.h"
#include <AppKit/NSWindow.h>

bool SdlGraphicsManager::getMacWindowScaling(float &scale) const {
#if SDL_VERSION_ATLEAST(2, 0, 0) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version); /* initialize info structure with SDL version info */
	if (!SDL_GetWindowWMInfo(_window->getSDLWindow(), &wmInfo))
		return false;

	NSWindow *nswindow = wmInfo.info.cocoa.window;
	if (!nswindow)
		return false;
	scale = [nswindow backingScaleFactor];
	return true;
#else
	return false;
#endif
}
