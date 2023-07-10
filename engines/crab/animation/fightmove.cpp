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

#include "crab/animation/fightmove.h"

namespace Crab {

using namespace pyrodactyl::anim;

void FightMove::load(rapidxml::xml_node<char> *node) {
	_frames[DIRECTION_DOWN].load(node->first_node("down"));
	_frames[DIRECTION_UP].load(node->first_node("up"));
	_frames[DIRECTION_LEFT].load(node->first_node("left"));
	_frames[DIRECTION_RIGHT].load(node->first_node("right"));

	if (nodeValid("input", node))
		_input.Load(node->first_node("input"));

	if (nodeValid("unlock", node, false))
		_unlock.Load(node->first_node("unlock"));

	if (nodeValid("effect", node))
		_eff.load(node->first_node("effect"));

	if (nodeValid("ai", node, false))
		_ai.Load(node->first_node("ai"));
}

} // End of namespace Crab
