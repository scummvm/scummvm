#pragma once
#include "common_header.h"

#include "Item.h"
#include "SectionHeader.h"
#include "StatDrawHelper.h"

namespace pyrodactyl {
namespace item {
struct StatPreview {
	pyrodactyl::ui::SectionHeader stat, unit;

	// We increment stat draw position by this much for every new item bonus
	Vector2i inc_s, inc_u;

	bool enabled;

	StatPreview() { enabled = false; }

	void Load(rapidxml::xml_node<char> *node);
	void Draw(Item &item, pyrodactyl::stat::StatDrawHelper &helper);
};
} // End of namespace item
} // End of namespace pyrodactyl