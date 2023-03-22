#pragma once

#include "animframe.h"
#include "common_header.h"

namespace pyrodactyl {
namespace anim {
struct Range {
	bool valid;
	Rect val[DIRECTION_TOTAL];

	Range() { valid = false; }

	void Load(rapidxml::xml_node<char> *node) {
		if (NodeValid(node, false)) {
			val[DIRECTION_DOWN].Load(node->first_node("down"));
			val[DIRECTION_UP].Load(node->first_node("up"));
			val[DIRECTION_LEFT].Load(node->first_node("left"));
			val[DIRECTION_RIGHT].Load(node->first_node("right"));

			valid = true;
		} else
			valid = false;
	}
};
} // End of namespace anim
} // End of namespace pyrodactyl