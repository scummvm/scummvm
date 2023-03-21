#pragma once

#include "common_header.h"
#include "vectors.h"

namespace pyrodactyl
{
	namespace ai
	{
		//These parameters control aspects of sprites flying across the screen
		struct FlyerConstant
		{
			//How far does a flier sprite start from the camera (an offset, not the whole value)
			Vector2i start;

			//The value of the delay for fliers
			Uint32 delay_min, delay_max;

			//The velocity of fliers
			Vector2f vel;

			FlyerConstant();

			void Load(rapidxml::xml_node<char> *node);
		};

		//These values are used in various sprite related tasks
		struct SpriteConstant
		{
			//Plane width decides the maximum difference in sprite Y values that is considered on the same plane
			int plane_w;

			//Tweening constant controls the acceleration curve of every sprite
			float tweening;

			//The modifiers of x and y movement speeds
			Vector2f walk_vel_mod;

			//Data for flying sprites
			FlyerConstant fly;

			SpriteConstant();

			void Load(rapidxml::xml_node<char> *node);
		};
	}
}