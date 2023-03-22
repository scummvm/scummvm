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

#ifndef CRAB_SPRITEAI_H
#define CRAB_SPRITEAI_H

#include "crab/common_header.h"
#include "crab/movement.h"
#include "crab/PathfindingAgent.h"

// class PathfindingAgent;
namespace pyrodactyl {
namespace ai {
// States of a fighting sprite
enum AIFightState { FIGHTSTATE_GETNEXTMOVE,
					FIGHTSTATE_GETINRANGE,
					FIGHTSTATE_EXECUTEMOVE,
					FIGHTSTATE_CANTFIGHT };

// States of a fleeing sprite
enum AIFleeState { FLEESTATE_GETNEARESTEXIT,
				   FLEESTATE_RUNTOEXIT,
				   FLEESTATE_DISAPPEAR,
				   FLEESTATE_CANTFLEE };

struct SpriteAIData {
	// Data required for fighting
	struct FightData {
		// The state of the sprite
		AIFightState state;

		// Used to count down the time NPCs wait before their next move
		// Usually varies per move which is why we don't load target for it
		Timer delay;

		// The list of moves that can be performed while attacking
		std::vector<unsigned int> attack;

		FightData() { state = FIGHTSTATE_GETNEXTMOVE; }
	} fight;

	// The pattern a peaceful sprite walks in
	MovementSet walk;

	// Data required to flee
	struct FleeData {
		AIFleeState state;

		FleeData() { state = FLEESTATE_GETNEARESTEXIT; }
	} flee;

	// The next location the sprite has to reach
	// PLAYER: Used for adventure game style point-n-click movement
	// AI: Used for path-finding (usually to the player's location)
	struct Destination : public Vector2i {
		// Are we trying to reach the destination?
		bool active;

		Destination() { active = false; }
	} dest;

	SpriteAIData() {}

	void Dest(const int &x, const int &y, const bool &Active = true) {
		dest.x = x;
		dest.y = y;
		dest.active = Active;
	}
	void Dest(const Vector2i &v, const bool &Active = true) { Dest(v.x, v.y, Active); }
};
} // End of namespace ai
} // End of namespace pyrodactyl

#endif // CRAB_SPRITEAI_H
