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

void AnimSet::load(const Common::String &filename) {
	XMLDoc conf(filename);
	if (conf.ready()) {
		rapidxml::xml_node<char> *node = conf.doc()->first_node();
		if (NodeValid(node)) {
			_fight.Load(node);
			_walk.Load(node);

			if (NodeValid("bounds", node))
				_bounds.Load(node->first_node("bounds"));

			if (NodeValid("shadow", node))
				_shadow.Load(node->first_node("shadow"));

			if (NodeValid("focus", node))
				_focus.Load(node->first_node("focus"));
		}
	}
}

TextureFlipType AnimSet::flip(const Direction &dir) {
	TextureFlipType ret;
	if (_fight.Flip(ret, dir))
		return ret;

	return _walk.Flip(dir);
}

const ShadowOffset &AnimSet::shadow(const Direction &dir) {
	if (_fight.ValidMove())
		return _fight.Shadow(dir);

	return _walk.Shadow(dir);
}

int AnimSet::anchorX(const Direction &dir) {
	FightAnimFrame faf;
	if (_fight.CurFrame(faf, dir))
		return faf._anchor.x;

	return _walk.AnchorX(dir);
}

int AnimSet::anchorY(const Direction &dir) {
	FightAnimFrame faf;
	if (_fight.CurFrame(faf, dir))
		return faf._anchor.y;

	return _walk.AnchorY(dir);
}

} // End of namespace Crab
