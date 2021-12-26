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

#ifndef AGS_ENGINE_AC_WALKABLE_AREA_H
#define AGS_ENGINE_AC_WALKABLE_AREA_H

namespace AGS3 {

void  redo_walkable_areas();
int   get_walkable_area_pixel(int x, int y);
int   get_area_scaling(int onarea, int xx, int yy);
void  scale_sprite_size(int sppic, int zoom_level, int *newwidth, int *newheight);
void  remove_walkable_areas_from_temp(int fromx, int cwidth, int starty, int endy);
int   is_point_in_rect(int x, int y, int left, int top, int right, int bottom);
// IMPORTANT: this function returns *global pointer*, do not delete the returned bitmap! -- subject to future refactor
Shared::Bitmap *prepare_walkable_areas(int sourceChar);
int   get_walkable_area_at_location(int xx, int yy);
int   get_walkable_area_at_character(int charnum);

} // namespace AGS3

#endif
