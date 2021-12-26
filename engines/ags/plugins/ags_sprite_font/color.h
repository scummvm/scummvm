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

#ifndef AGS_PLUGINS_AGS_SPRITE_FONT_COLOR_H
#define AGS_PLUGINS_AGS_SPRITE_FONT_COLOR_H

namespace AGS3 {
namespace Plugins {
namespace AGSSpriteFont {

#define MAX(a, b)  (((a) > (b)) ? (a) : (b))
#define DEFAULT_RGB_R_SHIFT_32  16
#define DEFAULT_RGB_G_SHIFT_32  8
#define DEFAULT_RGB_B_SHIFT_32  0
#define DEFAULT_RGB_A_SHIFT_32  24

#pragma region Color_Functions


int getr32(int c);
int getg32(int c);
int getb32(int c);
int geta32(int c);
int makeacol32(int r, int g, int b, int a);

#pragma endregion

} // namespace AGSSpriteFont
} // namespace Plugins
} // namespace AGS3

#endif
