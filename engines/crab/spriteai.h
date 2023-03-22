#pragma once

#include "common_header.h"
#include "movement.h"
// #include "PathfindingAgent.h"

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