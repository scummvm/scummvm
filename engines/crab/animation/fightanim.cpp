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

#include "crab/animation/fightanim.h"

namespace Crab {

using namespace pyrodactyl::anim;

//------------------------------------------------------------------------
// Purpose: Load a single frame of a fighting move
//------------------------------------------------------------------------
void FightAnimFrame::load(rapidxml::xml_node<char> *node, const Rect &VBOX, const uint32 &rep, const int &AX, const int &AY) {
	AnimFrame::load(node, VBOX, rep, AX, AY);

	if (nodeValid("box_d", node, false))
		_boxD.load(node->first_node("box_d"));

	if (nodeValid("shift", node, false))
		_delta.load(node->first_node("shift"));

	if (!loadNum(_state, "state", node, false))
		_state = 0;

	loadBool(_branch, "branch", node, false);
}

//------------------------------------------------------------------------
// Purpose: Load a fighting move
//------------------------------------------------------------------------
void FightAnimFrames::load(rapidxml::xml_node<char> *node) {
	loadTextureFlipType(_flip, node);

	if (!loadNum(_repeat, "repeat", node, false))
		_repeat = 0;

	if (nodeValid("anchor", node, false))
		_anchor.load(node->first_node("anchor"));

	if (nodeValid("box_v", node))
		_boxV.load(node->first_node("box_v"));

	if (nodeValid("shadow", node)) {
		_shadow.load(node->first_node("shadow"));
		_shadow._valid = true;
	}

	if (nodeValid("frames", node)) {
		_frame.clear();
		rapidxml::xml_node<char> *framenode = node->first_node("frames");
		for (auto n = framenode->first_node("frame"); n != NULL; n = n->next_sibling("frame")) {
			FightAnimFrame faf;
			faf.load(n, _boxV, _repeat, _anchor.x, _anchor.y);
			_frame.push_back(faf);
		}
	}
}

} // End of namespace Crab
