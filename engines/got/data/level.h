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

struct Level {
private:
	void sync(Common::Serializer &s);

public:
	byte _iconGrid[12][20] = {};            // Grid of icons
	byte _backgroundColor = 0;              // Background color
	byte _music = 0;                        // Music
	byte _actorType[16] = {};               // Type of enemies (12 max)
	byte _actorLoc[16] = {};                // Location of enemies
	byte _actorValue[16] = {};              // Pass value
	byte _palColors[3] = {};                // Change 251,253,254 to these three
	byte _actorInvis[16] = {};
	byte _extra[13] = {};

	byte _staticObject[OBJECTS_COUNT] = {}; // Static objects (treasure, keys,etc)
	int _staticX[OBJECTS_COUNT] = {};       // X coordinates of static objects
	int _staticY[OBJECTS_COUNT] = {};       // Y coordinates of static objects
	byte _newLevel[10] = {};                // Level jump for icon 200-204
	byte _newLevelLocation[10] = {};        // Grid location to jump in to
	byte _area = 0;                         // Game area (1=forest,etc)
	byte _actorDir[16] = {};                // Initial _dir
	byte _filler[3] = {};
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
