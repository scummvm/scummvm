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

#include "backends/graphics/sdl/sdl-graphics.h"
#include "backends/platform/sdl/sailfish/sailfish-window.h"

/* Setting window size at anything other than full screen is unexpected
   and results in a rectangle without any decorations. So always create
   full screen window.
 */
bool SdlWindow_Sailfish::createOrUpdateWindow(int, int, uint32 flags) {
	SDL_DisplayMode dm;
	SDL_GetCurrentDisplayMode(0,&dm);
	int width, height;

	/* SDL assumes that composer takes care of rotation and so switches
	   sides in landscape rotation. But Lipstick doesn't handle rotation.
	   So put them back in correct order.
	 */
	if (dm.w < dm.h) {
		width = dm.w;
		height = dm.h;
	} else {
		width = dm.h;
		height = dm.w;
	}
	return SdlWindow::createOrUpdateWindow(width, height, flags);
}
