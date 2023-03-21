#pragma once
#include "common_header.h"
#include "GameEventInfo.h"

namespace pyrodactyl
{
	namespace event
	{
		enum RelOp { OP_AND, OP_OR };

		enum TriggerType
		{
			TRIG_OBJ, //Interacting with an object, status of character (hostile, coward etc), state (stand, fight, flee, KO)
			TRIG_OPINION, //Check opinion of a character (charm / intimidate / respect)
			TRIG_LOC, //Being in a place on the map
			TRIG_ITEM, //An item is present or not
			TRIG_RECT, //A sprite is colliding with a rectangle
			TRIG_STAT, //Check any stat of an object (health, attack, defense etc)
			TRIG_DIFF, //Check the game's difficulty
			TRIG_TRAIT, //See if a character has a certain trait
			TRIG_VAR //A variable is present or not, or if it's a certain value
		};

		struct Trigger
		{
			TriggerType type;
			std::string target, subject, operation, val;

			//Relation to the next trigger
			RelOp rel;

			//Represents the Boolean ! operator
			bool negate;

			Trigger() { type = TRIG_VAR; rel = OP_AND; negate = false; }
			Trigger(rapidxml::xml_node<char> *node) { Load(node); }

			void Load(rapidxml::xml_node<char> *node);
			bool Evaluate(pyrodactyl::event::Info &info);

			bool Evaluate(int lhs, int rhs);
		};
	}
}