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

#ifndef __AGS_EE_AC__WALKABLEAREA_H
#define __AGS_EE_AC__WALKABLEAREA_H

void  redo_walkable_areas();
int   get_walkable_area_pixel(int x, int y);
int   get_area_scaling (int onarea, int xx, int yy);
void  scale_sprite_size(int sppic, int zoom_level, int *newwidth, int *newheight);
void  remove_walkable_areas_from_temp(int fromx, int cwidth, int starty, int endy);
int   is_point_in_rect(int x, int y, int left, int top, int right, int bottom);
Common::Bitmap *prepare_walkable_areas (int sourceChar);
int   get_walkable_area_at_location(int xx, int yy);
int   get_walkable_area_at_character (int charnum);

#endif // __AGS_EE_AC__WALKABLEAREA_H
