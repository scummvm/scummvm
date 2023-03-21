#pragma once

#include "common_header.h"
#include "fightanim.h"

namespace pyrodactyl
{
	namespace anim
	{
		//All info for a single fighting move in all four directions
		struct FightMove
		{
			//Frames for all four directions
			FightAnimFrames frames[DIRECTION_TOTAL];

			//The input required
			pyrodactyl::input::FightInput input;

			//The conditions to unlock this move for player use
			pyrodactyl::event::TriggerSet unlock;

			//The effects of this move - hurt animation, sound effect and so on
			FightMoveEffect eff;

			//The data needed by an AI sprite to execute this move
			FightMoveAIData ai;

			FightMove(){}
			~FightMove(){}

			void Load(rapidxml::xml_node<char> *node);
		};
	}
}