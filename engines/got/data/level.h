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

#ifndef GOT_DATA_LEVEL_H
#define GOT_DATA_LEVEL_H

#include "common/serializer.h"

namespace Got {

#define OBJECTS_COUNT 30

struct LEVEL { // size=512
private:
	void sync(Common::Serializer &s);

public:
	byte icon[12][20] = {};    // 0   grid of icons
	byte bg_color = 0;         // 240 background color
	byte type = 0;             // 241 music
	byte actor_type[16] = {};  // 242 type of enemies (12 max)
	byte actor_loc[16] = {};   // 254 location of enemies
	byte actor_value[16] = {}; // pass value
	byte pal_colors[3] = {};   // change 251,253,254 to these three
	byte actor_invis[16] = {};
	byte extra[13] = {};

	byte static_obj[OBJECTS_COUNT] = {}; // 302 static objects (treasure, keys,etc)
	int static_x[OBJECTS_COUNT] = {};    // 332 X coor of static objects
	int static_y[OBJECTS_COUNT] = {};    // 392 Y coor of static objects
	byte new_level[10] = {};             // 452 level jump for icon 200-204
	byte new_level_loc[10] = {};         // 462 grid location to jump in to
	byte area = 0;                       // 472 game area (1=forest,etc)
	byte actor_dir[16] = {};             // initial dir
	byte future[3] = {};                 // 473

	/**
     * Loads level data from the global sd_data for the given level
     */
	void load(int level);

	/**
     * Saves the level data to the global sd_data
     */
	void save(int level);
};

} // namespace Got

#endif
