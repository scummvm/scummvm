#pragma once

#include "common_header.h"
#include "loaders.h"
#include "input.h"

namespace pyrodactyl
{
	namespace input
	{
		//The animations a sprite can play
		enum FightAnimationType
		{
			//Idle state, you can launch new moves from this state only
			//You return to this state once a move animation is done
			FA_IDLE,

			//The moves a sprite can do
			FA_ATTACK,
			FA_BLOCK,

			//The hurt animation
			FA_HURT,

			//The death animation
			FA_DEAD
		};

		//The input necessary to launch a move
		struct FightInput
		{
			//The state needed to execute this move
			FightAnimationType type;

			//The sprite state, used to have different moves trigger from the same move
			unsigned int state;

			FightInput() { Reset(); }
			void Reset() { type = FA_IDLE; state = 0; }

			bool operator== (const FightInput &input){ return type == input.type && state == input.state; }
			bool Idle() { return type == FA_IDLE; }

			void Load(rapidxml::xml_node<char> *node);
			FightAnimationType HandleEvents(const SDL_Event &Event);
		};
	}
}