#pragma once

#include "common_header.h"
#include "Shape.h"

namespace pyrodactyl
{
	namespace level
	{
		class Stairs : public Shape
		{
		public:
			//The modifier by which the velocity of the character walking over the stairs is changed
			Vector2f modifier;

			Stairs() : modifier(1.0f, 1.0f){}
			~Stairs(){}

			void Load(rapidxml::xml_node<char> *node, const bool &echo = true);
		};
	}
}