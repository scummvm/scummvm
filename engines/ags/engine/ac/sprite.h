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

#ifndef AGS_ENGINE_AC_SPRITE_H
#define AGS_ENGINE_AC_SPRITE_H

namespace AGS3 {

void get_new_size_for_sprite(int ee, int ww, int hh, int &newwid, int &newhit);
// set any alpha-transparent pixels in the image to the appropriate
// RGB mask value so that the ->Blit calls work correctly
void set_rgb_mask_using_alpha_channel(Shared::Bitmap *image);
// from is a 32-bit RGBA image, to is a 15/16/24-bit destination image
Shared::Bitmap *remove_alpha_channel(Shared::Bitmap *from);
void pre_save_sprite(Shared::Bitmap *bitmap);
void initialize_sprite(int ee);

} // namespace AGS3

#endif
