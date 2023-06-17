
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
extern bool player_been_init(int16 num_scenes);

/**
 * Shuts down player_been system. Deallocates some memory
 */
extern void player_been_shutdown();

/**
 * Resets the player_been system
 */
extern void player_reset_been(void);

/**
 * Restores player_been information
 *
 * A file must already been open for reading and seeked to the right place
 * returns the number of bytes read. player_been_init must have been called
 * previously with a sufficient number of scenes to hold the record being read.
 */
extern int32 player_been_restore(Common::SeekableReadStream *handle);

/**
 * Saves player_been information
 *
 * A file must already been open for writing and seeked to the right place
 * @returns The number of bytes written
 */
extern int32 player_been_save(Common::WriteStream *handle);

/**
 * Called whenever player enters a scene
 * @param scene_num		Takes the scene to be entered
 * @returns				True if the player has been there before, or false otherwise
 */
extern bool player_enters_scene(int16 scene_num);

/**
 * Called if the apps programmer wants the player to forget about being in
 * a room
 */
extern void player_forgets_scene(int16 scene_num);

} // End of namespace M4

#endif
