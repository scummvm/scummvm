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

#ifndef CRAB_MOVEEFFECT_H
#define CRAB_MOVEEFFECT_H

#include "crab/common_header.h"
#include "crab/animation/imageeffect.h"
#include "crab/music/musicparam.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
struct FightMoveEffect {
	// The image displayed on being hit
	ImageEffect _img;

	// The sound played by this move when it's activated
	pyrodactyl::music::ChunkKey _activate;

	// The sound played by this move when it hits opponent
	pyrodactyl::music::ChunkKey _hit;

	// The move the sprite hit by our current move performs - if living (hurt animation)
	int _hurt;

	// The move the sprite hit by our current move performs - if it dies as a result (dying animation)
	int _death;

	// The stun time for the enemy if this move hits a sprite
	unsigned int _stun;

	// The base damage of the move if it hits a sprite
	int _dmg;

	FightMoveEffect();

	void load(rapidxml::xml_node<char> *node);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MOVEEFFECT_H
