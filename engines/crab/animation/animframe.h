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
#include "crab/vectors.h"
#include "crab/animation/shadow.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
struct AnimFrame {
	// Portion of sprite to show
	Rect _clip;

	// The duration for which the frame must be repeated on screen
	uint32 _repeat;

	// The anchor point of the frame
	Vector2i _anchor;

	// The vulnerable hit box for this frame
	Rect _boxV;

	AnimFrame() {
		_repeat = 0;
	}

	void load(rapidxml::xml_node<char> *node, const Rect &VBOX,
			  const uint32 &REP = 0, const int &AX = 0, const int &AY = 0);
};

class AnimationFrames {
	// The global vulnerable hit box for all the frames
	// If the W or H of this is 0, then use individual frame values
	Rect _boxV;

public:
	// The frames for the animation
	Common::Array<AnimFrame> _frame;

	// The currentClip
	uint _currentClip;

	// Should we flip the images in the frame rectangle?
	TextureFlipType _flip;

	// The global repeat value for all the frames
	// If this is 0, then use individual frame values
	uint32 _repeat;

	// The global anchor value for all the frames
	// If this is 0, then use individual frame values
	Vector2i _anchor;

	// true if animation starts at a random frame
	// used for idle animations so that every sprite doesn't animate in sync
	bool _random;

	// Does this set of animation frames need a specific shadow offset?
	ShadowOffset _shadow;

	AnimationFrames() {
		reset();
		_flip = FLIP_NONE;
		_repeat = 0;
		_random = false;
	}
	void reset() {
		_currentClip = 0;
	}

	void load(rapidxml::xml_node<char> *node);

	bool updateClip();
	const AnimFrame &currentFrame();
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ANIMFRAME_H
