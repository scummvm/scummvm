#pragma once

#include "common_header.h"
#include "loaders.h"

namespace pyrodactyl
{
	namespace stat
	{
		//Stat bonuses for an item
		struct Bonus
		{
			StatType type;
			int val;

			void Load(rapidxml::xml_node<char> *node)
			{
				LoadStatType(type, node);
				LoadNum(val, "val", node);
			}
		};
	}
}