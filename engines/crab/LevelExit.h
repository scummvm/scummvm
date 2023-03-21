#pragma once

#include "common_header.h"
#include "ImageManager.h"
#include "GameEventInfo.h"
#include "Shape.h"

namespace pyrodactyl
{
	namespace level
	{
		struct Exit
		{
			std::string name;
			Shape dim;
			Vector2i entry;

			Exit() : entry(-1,-1) {}
			Exit(rapidxml::xml_node<char> *node){ Load(node); }

			void Load(rapidxml::xml_node<char> *node);
		};
	}
}