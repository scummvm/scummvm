/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "ags/plugins/ags_sprite_font/color.h"

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

int getr32(int c) {
	return ((c >> DEFAULT_RGB_R_SHIFT_32) & 0xFF);
}


int getg32(int c) {
	return ((c >> DEFAULT_RGB_G_SHIFT_32) & 0xFF);
}


int getb32(int c) {
	return ((c >> DEFAULT_RGB_B_SHIFT_32) & 0xFF);
}


int geta32(int c) {
	return ((c >> DEFAULT_RGB_A_SHIFT_32) & 0xFF);
}


int makeacol32(int r, int g, int b, int a) {
	return ((r << DEFAULT_RGB_R_SHIFT_32) |
	        (g << DEFAULT_RGB_G_SHIFT_32) |
	        (b << DEFAULT_RGB_B_SHIFT_32) |
	        (a << DEFAULT_RGB_A_SHIFT_32));
}

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3
