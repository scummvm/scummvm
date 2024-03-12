
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

#ifndef M4_RIDDLE_WALKER_H
#define M4_RIDDLE_WALKER_H

#include "common/array.h"
#include "m4/adv_r/adv_walk.h"

namespace M4 {
namespace Riddle {

// These are the walker types
enum {
	WALKER_PLAYER = 0,
	WALKER_ALT    = 1
};

// These are the shadow types
enum {
	SHADOW_PLAYER = 0,
	SHADOW_ALT    = 1
};

class Walker : public M4::Walker {
private:
	/**
	 * This is called when PLAYER walker code sends system message back to C (used by MAIN PLAYER WALKER ONLY)
	 */
	static void player_walker_callback(frac16 myMessage, machine *sender);

public:
	~Walker() override {}

	bool walk_load_walker_and_shadow_series() override;
	machine *walk_initialize_walker() override;

	void reset_walker_sprites();
	static void unloadSprites();
};

void enable_player();
void disable_player();
void wilbur_abduct(int trigger);
void player_walk_to(int32 x, int32 y, int32 facing_x, int32 facing_y, int trigger = -1);
void player_walk_to(int32 x, int32 y, int trigger = -1);

} // namespace Riddle
} // namespace M4

#endif
