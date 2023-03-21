#pragma once

#include "common_header.h"
#include "loaders.h"

namespace pyrodactyl
{
	namespace anim
	{
		//Types of fade effects
		enum FadeType { FADE_NONE, FADE_IN, FADE_OUT };

		//Sometimes we need to stop drawing the game for proper fade effects
		//Use DRAW_STOP to stop drawing the game until DRAW_START is called. DRAW_SAME doesn't change anything
		enum DrawType { DRAW_SAME, DRAW_STOP, DRAW_START };

		struct AnimationEffect
		{
			//What sort of effect do we apply to the image
			FadeType type;

			//The duration of the effect relative to the start of this animation
			Uint32 start, finish;

			//Warning: the only way to start drawing the game again is having another animation event with DRAW_START
			DrawType draw_game;

			AnimationEffect() { type = FADE_NONE; draw_game = DRAW_SAME; start = 0; finish = 0; }

			AnimationEffect(rapidxml::xml_node<char> *node)
			{
				if (NodeValid("effect", node))
				{
					rapidxml::xml_node<char> *effnode = node->first_node("effect");
					LoadNum(start, "start", effnode);
					LoadNum(finish, "finish", effnode);

					std::string str;
					LoadStr(str, "type", effnode);
					if (str == "fade_in") type = FADE_IN;
					else if (str == "fade_out") type = FADE_OUT;
					else type = FADE_NONE;

					LoadStr(str, "game_draw", effnode);
					if (str == "start") draw_game = DRAW_START;
					else if (str == "stop") draw_game = DRAW_STOP;
					else draw_game = DRAW_SAME;
				}
			}
		};
	}
}