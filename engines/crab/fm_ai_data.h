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