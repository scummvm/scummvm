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

#ifndef CRAB_FIGHTMOVE_H
#define CRAB_FIGHTMOVE_H

#include "crab/common_header.h"
#include "crab/fightanim.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
// All info for a single fighting move in all four directions
struct FightMove {
	// Frames for all four directions
	FightAnimFrames frames[DIRECTION_TOTAL];

	// The input required
	pyrodactyl::input::FightInput input;

	// The conditions to unlock this move for player use
	pyrodactyl::event::TriggerSet unlock;

	// The effects of this move - hurt animation, sound effect and so on
	FightMoveEffect eff;

	// The data needed by an AI sprite to execute this move
	FightMoveAIData ai;

	FightMove() {}
	~FightMove() {}

	void Load(rapidxml::xml_node<char> *node);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_FIGHTMOVE_H
