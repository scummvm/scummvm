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

#ifndef CRAB__ANIMATION_H
#define CRAB__ANIMATION_H

#include "AnimationEffect.h"
#include "AnimationFrame.h"
#include "common_header.h"
#include "timer.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
class Animation {
	// All the frames are updated simultaneously rather than sequentially
	std::vector<AnimationFrame> frame;

	// Length of the entire animation in milliseconds
	Uint32 length;

	// Keep track of the time
	Timer timer;

public:
	Animation() { length = 0; }
	Animation(rapidxml::xml_node<char> *node);

	void Draw();
	void Reset();

	bool InternalEvents(DrawType &game_draw);
	void Start() {
		Reset();
		timer.Start();
	}
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB__ANIMATION_H
