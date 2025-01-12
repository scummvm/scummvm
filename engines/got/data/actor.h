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

#include "common/stream.h"
#include "graphics/managed_surface.h"

#ifndef GOT_DATA_ACTOR_H
#define GOT_DATA_ACTOR_H

namespace Got {

#define DIRECTION_COUNT 4
#define FRAME_COUNT 4

struct ACTOR { // Size=256
	// First part loaded from disk  (size=40)
	byte move = 0;               // Movement pattern (0=none)
	byte width = 0;              // Physical width
	byte height = 0;             // Physical height
	byte directions = 0;         // 1,2 or 4 (1=uni-directional)
	byte frames = 0;             // # frames per direction
	byte frame_speed = 0;        // # cycles between frame changes
	byte frame_sequence[4] = {}; // Sequence
	byte speed = 0;              // Move every Nth cycle
	byte size_x = 0;             // Non-physical padding on X coordinate
	byte size_y = 0;             // Non-physical padding on Y coordinate
	byte strength = 0;           // Hit strength
	byte health = 0;             //
	byte num_moves = 0;          // # of moves every <speed> cycles
	byte shot_type = 0;          // Actor # of shot
	byte shot_pattern = 0;       // Func number to decide to shoot
	byte shots_allowed = 0;      // # shots allowed on screen
	byte solid = 0;              // 1=solid (not ghost,etc)
	byte flying = 0;             //
	byte rating = 0;             // rnd(100) < rating = jewel
	byte type = 0;               // Actor (0=thor,1=hammer,2=enemy,3=shot)
	char name[9] = {};           // Actors name
	byte func_num = 0;           // Special function when thor touches
	byte func_pass = 0;          // Value to pass to func
	uint16 magic_hurts = 0;      // Bitwise magic hurts flags
	byte future1[4] = {};

	// The rest is dynamic    //size=216
	// Direction/frame surfaces
	Graphics::ManagedSurface pic[DIRECTION_COUNT][FRAME_COUNT];

	byte frame_count = 0; // Count to switch frames
	byte dir = 0;         // Direction of travel
	byte last_dir = 0;    // Last direction of travel
	int x = 0;            // Actual X coordinate
	int y = 0;            // Actual Y coordinate
	int center = 0;       // Center of object
	int last_x[2] = {};   // Last X coordinate on each page
	int last_y[2] = {};   // Last Y coordinate on each page
	byte used = 0;        // 1=active, 0=not active
	byte next = 0;        // Next frame to be shown
	byte speed_count = 0; // Count down to movement
	byte vunerable = 0;   // Count down to vulnerability
	byte shot_cnt = 0;    // Count down to another shot
	byte num_shots = 0;   // # of shots currently on screen
	byte creator = 0;     // Which actor # created this actor
	byte pause = 0;       // Pause must be 0 to move
	byte actor_num = 0;
	byte move_count = 0;
	byte dead = 0;
	byte toggle = 0;
	byte center_x = 0;
	byte center_y = 0;
	byte show = 0;        // Display or not (for blinking)
	byte temp1 = 0;
	byte temp2 = 0;
	byte counter = 0;
	byte move_counter = 0;
	byte edge_counter = 0;
	byte temp3 = 0;
	byte temp4 = 0;
	byte temp5 = 0;
	byte hit_thor = 0;
	int rand = 0;
	byte init_dir = 0;
	byte pass_value = 0;
	byte shot_actor = 0;
	byte magic_hit = 0;
	byte temp6 = 0;
	int i1 = 0, i2 = 0, i3 = 0, i4 = 0, i5 = 0, i6 = 0;
	byte init_health = 0;
	byte talk_counter = 0;
	byte etype = 0;
	byte future2[25] = {};

	void loadFixed(Common::SeekableReadStream *src);
	void loadFixed(const byte *src);

	/**
     * Copies the fixed portion and pics from a source actor.
     */
	void copyFixedAndPics(const ACTOR &src);

	ACTOR &operator=(const ACTOR &src);

	int getPos() const {
		return ((x + 7) / 16) + (((y + 8) / 16) * 20);
	}
};

} // namespace Got

#endif
