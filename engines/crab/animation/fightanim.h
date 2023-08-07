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

#ifndef CRAB_FIGHTANIM_H
#define CRAB_FIGHTANIM_H

#include "crab/music/MusicManager.h"
#include "crab/common_header.h"
#include "crab/input/fightinput.h"
#include "crab/animation/fm_ai_data.h"
#include "crab/ai/moveeffect.h"
#include "crab/animation/shadow.h"
#include "crab/event/triggerset.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
// A single frame of a fighting move
struct FightAnimFrame : public AnimFrame {
	// The hit box of the player WITH RESPECT TO the sprite bounding box
	//boxV is the vulnerable hit box, boxD is the damage hit box
	Rect _boxD;

	// The displacement in the position caused by the frame
	Vector2i _delta;

	// The sprite state for the duration of the frame
	uint _state;

	// Can we cancel/branch to another move from this frame?
	bool _branch;

	FightAnimFrame() {
		_state = 0;
		_branch = false;
	}

	void load(rapidxml::xml_node<char> *node, const Rect &VBOX,
			  const uint32 &REP = 0, const int &AX = 0, const int &AY = 0);
};

// All data related to a single fighting move in a single direction
class FightAnimFrames {
	// The global vulnerable hit box for all the frames
	// If the W or H of this is 0, then use individual frame values
	Rect _boxV;

public:
	// The individual frames for each direction
	Common::Array<FightAnimFrame> _frame;

	// The currentClip
	uint _currentClip;

	// Should we flip the images in the frame rectangle?
	TextureFlipType _flip;

	// The amount of time in milliseconds each animation frame needs to be on screen
	// If this is zero then use the value in each individual frame
	uint32 _repeat;

	// The global anchor value for all the frames
	// If this is 0, then use individual frame values
	Vector2i _anchor;

	// Does this set of animation frames need a specific shadow offset?
	ShadowOffset _shadow;

	FightAnimFrames() {
		reset();
		_flip = FLIP_NONE;
		_repeat = 0;
	}
	void reset() {
		_currentClip = 0;
	}

	void load(rapidxml::xml_node<char> *node);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_FIGHTANIM_H
