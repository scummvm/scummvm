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


#include "crab/animation/walkframes.h"

namespace Crab {

using namespace pyrodactyl::anim;
using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Loader Function
//------------------------------------------------------------------------
void WalkFrames::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("stand", node))
		_set[WT_STAND].load(node->first_node("stand"));

	if (nodeValid("walk", node))
		_set[WT_WALK].load(node->first_node("walk"));

	if (nodeValid("fight", node))
		_set[WT_FIGHT].load(node->first_node("fight"));

	if (nodeValid("ko", node))
		_set[WT_KO].load(node->first_node("ko"));
}

//------------------------------------------------------------------------
// Purpose: Used for walking inside levels
//------------------------------------------------------------------------
bool WalkFrames::updateClip(Direction d, bool reset) {
	if (_timer.Ticks() > _set[_cur].frames[d].currentFrame()._repeat || reset) {
		_timer.Start();
		return _set[_cur].frames[d].updateClip();
	}

	return false;
}

void WalkFrames::resetClip(Direction d) {
	_set[_cur].frames[d].reset();
	_timer.Start();
}

//------------------------------------------------------------------------
// Purpose: Used inside dialog box
//------------------------------------------------------------------------
void WalkFrames::updateClip(WalkAnimType type, Direction d) {
	if (!_timer.Started())
		_timer.Start();

	if (_timer.Ticks() > _set[type].frames[d].currentFrame()._repeat) {
		_set[type].frames[d].updateClip();
		_timer.Start();
	}
}

Rect WalkFrames::dialogClip(const PersonState &state) {
	if (state == PST_FIGHT)
		return _set[WT_FIGHT].frames[DIRECTION_DOWN].currentFrame()._clip;
	else if (state == PST_KO)
		return _set[WT_KO].frames[DIRECTION_DOWN].currentFrame()._clip;

	return _set[WT_STAND].frames[DIRECTION_DOWN].currentFrame()._clip;
}

void WalkFrames::updateClip(const PersonState &state) {
	if (state == PST_FIGHT)
		updateClip(WT_FIGHT, DIRECTION_DOWN);
	else if (state == PST_KO)
		updateClip(WT_KO, DIRECTION_DOWN);
	else
		updateClip(WT_STAND, DIRECTION_DOWN);
}

//------------------------------------------------------------------------
// Purpose: Decide direction and type of animation, return whether it has changed
//------------------------------------------------------------------------
bool WalkFrames::type(const Vector2f &vel, Direction &dir, const pyrodactyl::people::PersonState &pst, const bool &firstX) {
	Direction prevDir = dir;
	WalkAnimType prevType = _cur;

	if (pst == PST_KO) {
		_cur = WT_KO;
	} else if (firstX) {
		// If we prioritize the X direction, X velocity is checked first for direction and then Y velocity
		if (vel.x > 0) {
			dir = DIRECTION_RIGHT;
			_cur = WT_WALK;
		} else if (vel.x < 0) {
			dir = DIRECTION_LEFT;
			_cur = WT_WALK;
		} else if (vel.y > 0) {
			dir = DIRECTION_DOWN;
			_cur = WT_WALK;
		} else if (vel.y < 0) {
			dir = DIRECTION_UP;
			_cur = WT_WALK;
		} else {
			_cur = WT_STAND;
		}
	} else {
		// If we prioritize the Y direction, Y velocity is checked first for direction and then Y velocity
		if (vel.y > 0) {
			dir = DIRECTION_DOWN;
			_cur = WT_WALK;
		} else if (vel.y < 0) {
			dir = DIRECTION_UP;
			_cur = WT_WALK;
		} else if (vel.x > 0) {
			dir = DIRECTION_RIGHT;
			_cur = WT_WALK;
		} else if (vel.x < 0) {
			dir = DIRECTION_LEFT;
			_cur = WT_WALK;
		} else {
			_cur = WT_STAND;
		}
	}

	if (prevDir != dir || prevType != _cur)
		return true;

	return false;
}

} // End of namespace Crab
