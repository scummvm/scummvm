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

#ifndef CRAB_ANIMFRAME_H
#define CRAB_ANIMFRAME_H

#include "crab/Rectangle.h"
#include "crab/common_header.h"
#include "crab/shadow.h"
#include "crab/vectors.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
struct AnimFrame {
	// Portion of sprite to show
	Rect clip;

	// The duration for which the frame must be repeated on screen
	Uint32 repeat;

	// The anchor point of the frame
	Vector2i anchor;

	// The vulnerable hit box for this frame
	Rect box_v;

	AnimFrame() { repeat = 0; }

	void Load(rapidxml::xml_node<char> *node, const Rect &VBOX,
			  const Uint32 &REP = 0, const int &AX = 0, const int &AY = 0);
};

class AnimationFrames {
	// The global vulnerable hit box for all the frames
	// If the W or H of this is 0, then use individual frame values
	Rect box_v;

public:
	// The frames for the animation
	std::vector<AnimFrame> frame;

	// The current clip
	unsigned int current_clip;

	// Should we flip the images in the frame rectangle?
	TextureFlipType flip;

	// The global repeat value for all the frames
	// If this is 0, then use individual frame values
	Uint32 repeat;

	// The global anchor value for all the frames
	// If this is 0, then use individual frame values
	Vector2i anchor;

	// true if animation starts at a random frame
	// used for idle animations so that every sprite doesn't animate in sync
	bool random;

	// Does this set of animation frames need a specific shadow offset?
	ShadowOffset shadow;

	AnimationFrames() {
		Reset();
		flip = FLIP_NONE;
		repeat = 0;
		random = false;
	}
	void Reset() { current_clip = 0; }

	void Load(rapidxml::xml_node<char> *node);

	bool UpdateClip();
	const AnimFrame &CurrentFrame();
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ANIMFRAME_H
