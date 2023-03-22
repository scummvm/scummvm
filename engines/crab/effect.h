#pragma once

#include "GameEventInfo.h"
#include "XMLDoc.h"
#include "common_header.h"
#include "loaders.h"
#include "person.h"

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