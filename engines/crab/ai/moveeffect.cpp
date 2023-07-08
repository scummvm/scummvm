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

#include "crab/ai/moveeffect.h"

namespace Crab {

using namespace pyrodactyl::anim;

FightMoveEffect::FightMoveEffect() {
	_activate = -1;
	_hit = -1;
	_dmg = 0;
	_stun = 0;
	_hurt = -1;
	_death = -1;
}

void FightMoveEffect::load(rapidxml::xml_node<char> *node) {
	LoadNum(_stun, "stun", node);
	LoadNum(_dmg, "damage", node);
	LoadNum(_hurt, "hurt", node);
	LoadNum(_death, "death", node);

	if (NodeValid("image", node, false))
		_img.Load(node->first_node("image"));

	if (NodeValid("sound", node)) {
		rapidxml::xml_node<char> *soundnode = node->first_node("sound");

		if (!LoadNum(_activate, "activate", soundnode, false))
			_activate = -1;

		if (!LoadNum(_activate, "hit", soundnode, false))
			_activate = -1;
	}
}

} // End of namespace Crab
