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

#pragma once

#include "common_header.h"
#include "range.h"

namespace pyrodactyl {
namespace anim {
enum AIMoveType {
	MOVE_NONE,   // AI sprites do not use this move
	MOVE_ATTACK, // AI sprites use this move to attack you
	MOVE_DEFEND  // AI sprites use this move to dodge or defend
};

struct FightMoveAIData {
	// Can this move be used by AI to attack
	AIMoveType type;

	// The range of the move
	Range range;

	// The AI delays executing the move by this long
	unsigned int delay;

	FightMoveAIData() {
		type = MOVE_NONE;
		delay = 0;
	}

	void Load(rapidxml::xml_node<char> *node) {
		if (!LoadNum(delay, "delay", node, false))
			delay = 0;

		range.Load(node->first_node("range"));

		std::string str;
		LoadStr(str, "type", node, false);
		if (str == "attack")
			type = MOVE_ATTACK;
		else if (str == "defend")
			type = MOVE_DEFEND;
		else
			type = MOVE_NONE;
	}
};
} // End of namespace anim
} // End of namespace pyrodactyl
