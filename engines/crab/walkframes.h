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

#ifndef CRAB_WALKFRAMES_H
#define CRAB_WALKFRAMES_H

#include "animframe.h"
#include "common_header.h"
#include "movement.h"
#include "personbase.h"
#include "timer.h"

namespace pyrodactyl {
namespace anim {
enum WalkAnimType { WT_STAND,
					WT_WALK,
					WT_FIGHT,
					WT_KO,
					WT_TOTAL };

class WalkFrames {
	struct WalkFrameSet {
		AnimationFrames frames[DIRECTION_TOTAL];

		void Load(rapidxml::xml_node<char> *node) {
			frames[DIRECTION_DOWN].Load(node->first_node("down"));
			frames[DIRECTION_UP].Load(node->first_node("up"));
			frames[DIRECTION_LEFT].Load(node->first_node("left"));
			frames[DIRECTION_RIGHT].Load(node->first_node("right"));
		}
	};

	// The walking animations of the sprite
	WalkFrameSet set[WT_TOTAL];

	// The current walking animation
	WalkAnimType cur;

	// The timers used for animation playing
	Timer timer;

	// Dialog box related
	void UpdateClip(WalkAnimType type, Direction d);

public:
	WalkFrames() {
		cur = WT_STAND;
		timer.Start();
	}
	~WalkFrames() {}

	void Load(rapidxml::xml_node<char> *node);

	bool UpdateClip(Direction d, bool reset);
	void ResetClip(Direction d);

	void Type(WalkAnimType type) { cur = type; }
	WalkAnimType Type() { return cur; }
	bool Type(const Vector2f &vel, Direction &dir, const pyrodactyl::people::PersonState &pst, const bool &first_x);

	const Rect &Clip(Direction d) { return set[cur].frames[d].CurrentFrame().clip; }
	const Rect &BoxV(Direction d) { return set[cur].frames[d].CurrentFrame().box_v; }
	const TextureFlipType &Flip(Direction d) { return set[cur].frames[d].flip; }

	const ShadowOffset &Shadow(Direction d) { return set[cur].frames[d].shadow; }

	const int AnchorX(Direction d) { return set[cur].frames[d].CurrentFrame().anchor.x; }
	const int AnchorY(Direction d) { return set[cur].frames[d].CurrentFrame().anchor.y; }

	// Dialog box related
	Rect DialogClip(const pyrodactyl::people::PersonState &state);
	void UpdateClip(const pyrodactyl::people::PersonState &state);
};
} // End of namespace anim
} // End of namespace pyrodactyl

#endif // CRAB_WALKFRAMES_H
