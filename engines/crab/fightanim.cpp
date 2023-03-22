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

#include "fightanim.h"

using namespace pyrodactyl::anim;

//------------------------------------------------------------------------
// Purpose: Load a single frame of a fighting move
//------------------------------------------------------------------------
void FightAnimFrame::Load(rapidxml::xml_node<char> *node, const Rect &VBOX, const Uint32 &rep, const int &AX, const int &AY) {
	AnimFrame::Load(node, VBOX, rep, AX, AY);

	if (NodeValid("box_d", node, false))
		box_d.Load(node->first_node("box_d"));

	if (NodeValid("shift", node, false))
		delta.Load(node->first_node("shift"));

	if (!LoadNum(state, "state", node, false))
		state = 0;

	LoadBool(branch, "branch", node, false);
}

//------------------------------------------------------------------------
// Purpose: Load a fighting move
//------------------------------------------------------------------------
void FightAnimFrames::Load(rapidxml::xml_node<char> *node) {
	LoadTextureFlipType(flip, node);

	if (!LoadNum(repeat, "repeat", node, false))
		repeat = 0;

	if (NodeValid("anchor", node, false))
		anchor.Load(node->first_node("anchor"));

	if (NodeValid("box_v", node))
		box_v.Load(node->first_node("box_v"));

	if (NodeValid("shadow", node)) {
		shadow.Load(node->first_node("shadow"));
		shadow.valid = true;
	}

	if (NodeValid("frames", node)) {
		frame.clear();
		rapidxml::xml_node<char> *framenode = node->first_node("frames");
		for (auto n = framenode->first_node("frame"); n != NULL; n = n->next_sibling("frame")) {
			FightAnimFrame faf;
			faf.Load(n, box_v, repeat, anchor.x, anchor.y);
			frame.push_back(faf);
		}
	}
}
