#include "LevelExit.h"
#include "stdafx.h"

using namespace pyrodactyl::level;

void Exit::Load(rapidxml::xml_node<char> *node) {
	dim.Load(node);
	LoadStr(name, "name", node);

	if (NodeValid("properties", node)) {
		rapidxml::xml_node<char> *propertynode = node->first_node("properties");
		for (auto n = propertynode->first_node("property"); n != NULL; n = n->next_sibling("property")) {
			std::string node_name;
			LoadStr(node_name, "name", n);
			if (node_name == "entry_x") {
				if (!LoadNum(entry.x, "value", n))
					entry.x = -1;
			} else if (node_name == "entry_y") {
				if (!LoadNum(entry.y, "value", n))
					entry.y = -1;
			}
		}
	}
}