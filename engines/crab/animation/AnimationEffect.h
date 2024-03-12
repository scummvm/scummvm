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

#ifndef CRAB_ANIMATIONEFFECT_H
#define CRAB_ANIMATIONEFFECT_H

#include "crab/loaders.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
// Types of fade effects
enum FadeType {
	FADE_NONE,
	FADE_IN,
	FADE_OUT
};

// Sometimes we need to stop drawing the game for proper fade effects
// Use DRAW_STOP to stop drawing the game until DRAW_START is called. DRAW_SAME doesn't change anything
enum DrawType {
	DRAW_SAME,
	DRAW_STOP,
	DRAW_START
};

struct AnimationEffect {
	// What sort of effect do we apply to the image
	FadeType _type;

	// The duration of the effect relative to the start of this animation
	uint32 _start, _finish;

	// Warning: the only way to start drawing the game again is having another animation event with DRAW_START
	DrawType _drawGame;

	AnimationEffect() {
		_type = FADE_NONE;
		_drawGame = DRAW_SAME;
		_start = 0;
		_finish = 0;
	}

	AnimationEffect(rapidxml::xml_node<char> *node) : AnimationEffect() {
		if (nodeValid("effect", node)) {
			rapidxml::xml_node<char> *effnode = node->first_node("effect");
			loadNum(_start, "start", effnode);
			loadNum(_finish, "finish", effnode);

			Common::String str;
			loadStr(str, "type", effnode);
			if (str == "fade_in")
				_type = FADE_IN;
			else if (str == "fade_out")
				_type = FADE_OUT;
			else
				_type = FADE_NONE;

			loadStr(str, "game_draw", effnode);
			if (str == "start")
				_drawGame = DRAW_START;
			else if (str == "stop")
				_drawGame = DRAW_STOP;
			else
				_drawGame = DRAW_SAME;
		}
	}
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_ANIMATIONEFFECT_H
