#pragma once

#include "common_header.h"
#include "imageeffect.h"
#include "musicparam.h"

namespace pyrodactyl {
namespace anim {
struct FightMoveEffect {
	// The image displayed on being hit
	ImageEffect img;

	// The sound played by this move when it's activated
	pyrodactyl::music::ChunkKey activate;

	// The sound played by this move when it hits opponent
	pyrodactyl::music::ChunkKey hit;

	// The move the sprite hit by our current move performs - if living (hurt animation)
	int hurt;

	// The move the sprite hit by our current move performs - if it dies as a result (dying animation)
	int death;

	// The stun time for the enemy if this move hits a sprite
	unsigned int stun;

	// The base damage of the move if it hits a sprite
	int dmg;

	FightMoveEffect();

	void Load(rapidxml::xml_node<char> *node);
};
} // End of namespace anim
} // End of namespace pyrodactyl