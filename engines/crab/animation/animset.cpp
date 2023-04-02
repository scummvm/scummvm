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

#include "crab/animation/animset.h"

namespace Crab {

using namespace pyrodactyl::anim;

void AnimSet::Load(const std::string &filename) {
	XMLDoc conf(filename.c_str());
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.Doc()->first_node();
		if (NodeValid(node)) {
			fight.Load(node);
			walk.Load(node);

			if (NodeValid("bounds", node))
				bounds.Load(node->first_node("bounds"));

			if (NodeValid("shadow", node))
				shadow.Load(node->first_node("shadow"));

			if (NodeValid("focus", node))
				focus.Load(node->first_node("focus"));
		}
	}
}

TextureFlipType AnimSet::Flip(const Direction &dir) {
	TextureFlipType ret;
	if (fight.Flip(ret, dir))
		return ret;

	return walk.Flip(dir);
}

const ShadowOffset &AnimSet::Shadow(const Direction &dir) {
	if (fight.ValidMove())
		return fight.Shadow(dir);

	return walk.Shadow(dir);
}

const int AnimSet::AnchorX(const Direction &dir) {
	FightAnimFrame faf;
	if (fight.CurFrame(faf, dir))
		return faf.anchor.x;

	return walk.AnchorX(dir);
}

const int AnimSet::AnchorY(const Direction &dir) {
	FightAnimFrame faf;
	if (fight.CurFrame(faf, dir))
		return faf.anchor.y;

	return walk.AnchorY(dir);
}

} // End of namespace Crab
