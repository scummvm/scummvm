#include "stdafx.h"
#include "MusicArea.h"

using namespace pyrodactyl::level;

void MusicArea::Load(rapidxml::xml_node<char> *node, const bool &echo)
{
	Shape::Load(node, echo);

	if (NodeValid("properties", node, echo))
	{
		auto pnode = node->first_node("properties");
		for (auto n = pnode->first_node("property"); n != NULL; n = n->next_sibling("property"))
		{
			std::string name;
			if (LoadStr(name, "name", n, echo))
			{
				if (name == "music") LoadBool(track, "value", n, echo);
				else if (name == "id") LoadNum(id, "value", n, echo);
				else if (name == "loops") LoadNum(loops, "value", n, echo);
			}
		}
	}
}