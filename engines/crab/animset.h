#pragma once

#include "common_header.h"
#include "fightmoves.h"
#include "shadow.h"
#include "walkframes.h"

namespace pyrodactyl {
namespace anim {
// Container for all the possible animations an object can have
struct AnimSet {
	// The frames relevant to fighting moves
	FightMoves fight;

	// The frames relevant to walking animations
	WalkFrames walk;

	// The bounding box of the character used for level collision
	Rect bounds;

	// The sprite shadow
	ShadowData shadow;

	// The camera focus point
	Vector2i focus;

	AnimSet() {}

	void Load(const std::string &filename);

	TextureFlipType Flip(const Direction &dir);
	const ShadowOffset &Shadow(const Direction &dir);

	const int AnchorX(const Direction &dir);
	const int AnchorY(const Direction &dir);
};
} // End of namespace anim
} // End of namespace pyrodactyl