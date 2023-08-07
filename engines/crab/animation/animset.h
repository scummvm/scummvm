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

#ifndef CRAB_ANIMSET_H
#define CRAB_ANIMSET_H

#include "crab/animation/fightmoves.h"
#include "crab/animation/shadow.h"
#include "crab/animation/walkframes.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
// Container for all the possible animations an object can have
struct AnimSet {
	// The frames relevant to fighting moves
	FightMoves _fight;

	// The frames relevant to walking animations
	WalkFrames _walk;

	// The bounding box of the character used for level collision
	Rect _bounds;

	// The sprite shadow
	ShadowData _shadow;

	// The camera focus point
	Vector2i _focus;

	AnimSet() {}

	void load(const Common::String &filename);

	TextureFlipType flip(const Direction &dir);
	const ShadowOffset &shadow(const Direction &dir);

	int anchorX(const Direction &dir);
	int anchorY(const Direction &dir);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ANIMSET_H
