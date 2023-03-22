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

#pragma once

#include "MusicManager.h"
#include "common_header.h"
#include "fightinput.h"
#include "fm_ai_data.h"
#include "moveeffect.h"
#include "shadow.h"
#include "triggerset.h"

namespace pyrodactyl {
namespace anim {
// A single frame of a fighting move
struct FightAnimFrame : public AnimFrame {
	// The hit box of the player WITH RESPECT TO the sprite bounding box
	//_v is the vulnerable hit box, _d is the damage hit box
	Rect box_d;

	// The displacement in the position caused by the frame
	Vector2i delta;

	// The sprite state for the duration of the frame
	unsigned int state;

	// Can we cancel/branch to another move from this frame?
	bool branch;

	FightAnimFrame() {
		state = 0;
		branch = false;
	}

	void Load(rapidxml::xml_node<char> *node, const Rect &VBOX,
			  const Uint32 &REP = 0, const int &AX = 0, const int &AY = 0);
};

// All data related to a single fighting move in a single direction
class FightAnimFrames {
	// The global vulnerable hit box for all the frames
	// If the W or H of this is 0, then use individual frame values
	Rect box_v;

public:
	// The individual frames for each direction
	std::vector<FightAnimFrame> frame;

	// The current clip
	unsigned int current_clip;

	// Should we flip the images in the frame rectangle?
	TextureFlipType flip;

	// The amount of time in milliseconds each animation frame needs to be on screen
	// If this is zero then use the value in each individual frame
	Uint32 repeat;

	// The global anchor value for all the frames
	// If this is 0, then use individual frame values
	Vector2i anchor;

	// Does this set of animation frames need a specific shadow offset?
	ShadowOffset shadow;

	FightAnimFrames() {
		Reset();
		flip = FLIP_NONE;
		repeat = 0;
	}
	void Reset() { current_clip = 0; }

	void Load(rapidxml::xml_node<char> *node);
};
} // End of namespace anim
} // End of namespace pyrodactyl
