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

#ifndef CRAB_EFFECT_H
#define CRAB_EFFECT_H

#include "crab/GameEventInfo.h"
#include "crab/XMLDoc.h"
#include "crab/common_header.h"
#include "crab/loaders.h"
#include "crab/person.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
enum EventResultType {
	ER_NONE,   // Do nothing
	ER_MAP,    // Change the map visible to player
	ER_DEST,   // Add or remove a destination on world map
	ER_IMG,    // Change the character button image
	ER_TRAIT,  // Add or remove a trait from a character
	ER_LEVEL,  // Change level
	ER_MOVE,   // Move sprite
	ER_PLAYER, // Switch the player sprite
	ER_SAVE,   // Save game
	ER_SYNC,   // Sync the level
	ER_QUIT    // Quit to main menu
};

struct EventResult {
	EventResultType type;
	std::string val;
	int x, y;

	EventResult() : val("") {
		type = ER_NONE;
		x = -1;
		y = -1;
	}
};

struct EventSeqInfo {
	bool cur;
	std::string loc, val;

	EventSeqInfo() { cur = false; }
	EventSeqInfo(const bool &Cur) { cur = Cur; }
};

enum EffectType {
	EFF_VAR,     // variable operations like adding, removing etc
	EFF_JOURNAL, // Add an objective to the player quest book
	EFF_OBJ,     // Change status (hostile, coward etc), state (stand, fight, flee, KO etc) of a character
	EFF_ITEM,    // Add/remove an item in the player's inventory
	EFF_LIKE,    // Change opinion of a character (charm)
	EFF_FEAR,    // Change opinion of a character (intimidate)
	EFF_RESPECT, // Change opinion of a character (respect)
	EFF_HEALTH,  // Change health of a character
	EFF_SOUND,   // Manipulate the game music
	EFF_MONEY,   // Set the money variable (not its value, just that which variable is the current money variable)
	EFF_END,     // End of the event sequence, remove it from active sequences
	// EFFECT DIVISION HERE
	EFF_MOVE,   // Make a character move
	EFF_MAP,    // Change the world map
	EFF_DEST,   // Add a destination to the world map
	EFF_IMG,    // Change the player button image
	EFF_TRAIT,  // Add or remove a trait from a character
	EFF_LEVEL,  // Load a new level
	EFF_PLAYER, // Swap the player sprite
	EFF_SAVE,   // Auto save the game
	EFF_QUIT    // Quit to main menu
};

struct Effect {
	EffectType type;
	std::string subject, operation, val;

	Effect() { type = EFF_VAR; }
	~Effect() {}

	void Load(rapidxml::xml_node<char> *node);
	bool Execute(pyrodactyl::event::Info &info, const std::string &player_id,
				 std::vector<EventResult> &result, std::vector<EventSeqInfo> &end_seq);

	void ChangeOpinion(pyrodactyl::event::Info &info, pyrodactyl::people::OpinionType type);
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_EFFECT_H
