#pragma once

#include "common_header.h"
#include "vectors.h"
#include "Rectangle.h"
#include "timer.h"

namespace pyrodactyl
{
	namespace ai
	{
		struct MovementSet
		{
			struct Movement
			{
				//The position this sprite has to move to
				Rect target;

				//The time the sprite waits before it starts moving to pos
				Uint32 delay;

				Movement(rapidxml::xml_node<char> *node) { target.Load(node); LoadNum(delay, "delay", node); }
			};

			//The path followed by the sprite
			std::vector<Movement> path;

			//If true, sprite repeats the path pattern after reaching the last co-ordinate
			bool repeat;

			//The current path node we are traveling to
			unsigned int cur;

			//The time the sprite has spent waiting is calculated here
			Timer timer;

			//Is this set enabled?
			bool enabled;

			MovementSet() { cur = 0; repeat = false; enabled = false; }
			MovementSet(rapidxml::xml_node<char> *node) { cur = 0; Load(node); }

			void Load(rapidxml::xml_node<char> *node);

			bool InternalEvents(const Rect rect);
			Rect Target() { return path.at(cur).target; }
		};
	}
}