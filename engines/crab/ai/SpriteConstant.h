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

#ifndef CRAB_SPRITECONSTANT_H
#define CRAB_SPRITECONSTANT_H

#include "crab/common_header.h"
#include "crab/vectors.h"

namespace Crab {

namespace pyrodactyl {
namespace ai {
// These parameters control aspects of sprites flying across the screen
struct FlyerConstant {
	// How far does a flier sprite start from the camera (an offset, not the whole value)
	Vector2i start;

	// The value of the delay for fliers
	Uint32 delay_min, delay_max;

	// The velocity of fliers
	Vector2f vel;

	FlyerConstant();

	void Load(rapidxml::xml_node<char> *node);
};

// These values are used in various sprite related tasks
struct SpriteConstant {
	// Plane width decides the maximum difference in sprite Y values that is considered on the same plane
	int plane_w;

	// Tweening constant controls the acceleration curve of every sprite
	float tweening;

	// The modifiers of x and y movement speeds
	Vector2f walk_vel_mod;

	// Data for flying sprites
	FlyerConstant fly;

	SpriteConstant();

	void Load(rapidxml::xml_node<char> *node);
};
} // End of namespace ai
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_SPRITECONSTANT_H
