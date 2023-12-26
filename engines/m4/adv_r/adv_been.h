
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

#ifndef M4_ADV_R_ADV_BEEN_H
#define M4_ADV_R_ADV_BEEN_H

#include "common/serializer.h"
#include "common/stream.h"
#include "m4/m4_types.h"

namespace M4 {

struct Scene_list {
	int32 total_scenes = 0;
	int32 tail = 0;
	int16 *table = nullptr;
};

/**
 * Initializes player_been information, allocates memory for a list of scenes
 *
 * Takes the number of scenes to allocate space for
 * @returns True if successfully allocated
 */
bool player_been_init(int16 num_scenes);

/**
 * Shuts down player_been system. Deallocates some memory
 */
void player_been_shutdown();

/**
 * Resets the player_been system
 */
void player_reset_been(void);

/**
 * Saves/loads player_been information
 */
void player_been_sync(Common::Serializer &s);

/**
 * Called whenever player enters a scene
 * @param scene_num		Takes the scene to be entered
 * @returns				True if the player has been there before, or false otherwise
 */
bool player_enters_scene(int16 scene_num);

/**
 * Called if the apps programmer wants the player to forget about being in
 * a room
 */
void player_forgets_scene(int16 scene_num);

/**
 * Checks whether player has been in specified scene before
 */
bool player_been_here(int16 scene_num);

} // End of namespace M4

#endif
