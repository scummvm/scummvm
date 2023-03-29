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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_FIGHTINPUT_H
#define CRAB_FIGHTINPUT_H

#include "crab/common_header.h"
#include "crab/input.h"
#include "crab/loaders.h"

namespace Crab {

namespace pyrodactyl {
namespace input {
// The animations a sprite can play
enum FightAnimationType {
	// Idle state, you can launch new moves from this state only
	// You return to this state once a move animation is done
	FA_IDLE,

	// The moves a sprite can do
	FA_ATTACK,
	FA_BLOCK,

	// The hurt animation
	FA_HURT,

	// The death animation
	FA_DEAD
};

// The input necessary to launch a move
struct FightInput {
	// The state needed to execute this move
	FightAnimationType type;

	// The sprite state, used to have different moves trigger from the same move
	unsigned int state;

	FightInput() { Reset(); }
	void Reset() {
		type = FA_IDLE;
		state = 0;
	}

	bool operator==(const FightInput &input) { return type == input.type && state == input.state; }
	bool Idle() { return type == FA_IDLE; }

	void Load(rapidxml::xml_node<char> *node);
	FightAnimationType HandleEvents(const SDL_Event &Event);
};
} // End of namespace input
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_FIGHTINPUT_H
