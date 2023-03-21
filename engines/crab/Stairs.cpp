#include "stdafx.h"
#include "Stairs.h"

using namespace pyrodactyl::level;

void Stairs :: Load(rapidxml::xml_node<char> *node, const bool &echo)
{
	Shape::Load(node, echo);

	if(NodeValid("properties", node, echo))
	{
		auto pnode = node->first_node("properties");
		for(auto n = pnode->first_node("property"); n != NULL; n = n->next_sibling("property"))
		{
			std::string name;
			if(LoadStr(name, "name", n, echo))
			{
				if(name == "x")
					LoadNum(modifier.x, "value", n, echo);
				else if(name == "y")
					LoadNum(modifier.y, "value", n, echo);
			}
		}
	}
}