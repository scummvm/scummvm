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

#ifndef GOT_GFX_IMAGE_H
#define GOT_GFX_IMAGE_H

#include "got/data/defines.h"

namespace Got {

//extern uint make_mask(MASK_IMAGE *new_image, uint page_start,
//	byte *Image, int image_width, int image_height);
extern void setup_actor(Actor *actr, char num, char dir, int x, int y);
extern void make_actor_mask(Actor *actr);
extern int load_standard_actors();
extern void show_enemies();
extern int load_enemy(int type);
extern int actor_visible(int invis_num);
extern void setup_magic_item(int item);
extern void load_new_thor();

} // namespace Got

#endif
