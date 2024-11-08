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

extern const int16 RIPLEY_SERIES_DIRS[];
extern const int16 RIPLEY_SHADOWS_DIRS[6];

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
	// Strangerke : I turned the callback from private to public as it's called by room 803. TODO: Check if it's correct, then update the description accordingly
public:
	/**
	 * This is called when PLAYER walker code sends system message back to C (used by MAIN PLAYER WALKER ONLY)
	 */
	static void player_walker_callback(frac16 myMessage, machine *sender);

public:
	~Walker() override {}

	bool walk_load_walker_and_shadow_series() override;
	machine *walk_initialize_walker() override;

	static void unloadSprites();

	bool ripley_said(const char *const list[][2]);
};

void enable_player();
void disable_player();

} // namespace Riddle
} // namespace M4

#endif
