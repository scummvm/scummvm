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

#ifndef CRAB_TRIGGER_H
#define CRAB_TRIGGER_H
#include "crab/event/GameEventInfo.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
enum RelOp { OP_AND,
			 OP_OR };

enum TriggerType {
	TRIG_OBJ,     // Interacting with an object, status of character (hostile, coward etc), state (stand, fight, flee, KO)
	TRIG_OPINION, // Check opinion of a character (charm / intimidate / respect)
	TRIG_LOC,     // Being in a place on the map
	TRIG_ITEM,    // An item is present or not
	TRIG_RECT,    // A sprite is colliding with a rectangle
	TRIG_STAT,    // Check any stat of an object (health, attack, defense etc)
	TRIG_DIFF,    // Check the game's difficulty
	TRIG_TRAIT,   // See if a character has a certain trait
	TRIG_VAR      // A variable is present or not, or if it's a certain value
};

struct Trigger {
	TriggerType type;
	std::string target, subject, operation, val;

	// Relation to the next trigger
	RelOp rel;

	// Represents the Boolean ! operator
	bool negate;

	Trigger() {
		type = TRIG_VAR;
		rel = OP_AND;
		negate = false;
	}
	Trigger(rapidxml::xml_node<char> *node) { Load(node); }

	void Load(rapidxml::xml_node<char> *node);
	bool Evaluate(pyrodactyl::event::Info &info);

	bool Evaluate(int lhs, int rhs);
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TRIGGER_H
