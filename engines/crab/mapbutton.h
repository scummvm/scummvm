#pragma once
#include "common_header.h"

#include "menu.h"
#include "triggerset.h"

namespace pyrodactyl {
namespace ui {
class MapButton : public Button {
public:
	// The id of the location
	std::string loc;

	// Conditions needed for the location to be unlocked in world map
	pyrodactyl::event::TriggerSet unlock;

	MapButton() {}
	~MapButton() {}

	void Load(rapidxml::xml_node<char> *node) {
		Button::Load(node);

		LoadStr(loc, "id", node);
		if (NodeValid("unlock", node, false))
			unlock.Load(node->first_node("unlock"));
	}
};

typedef Menu<MapButton> MapButtonMenu;
} // End of namespace ui
} // End of namespace pyrodactyl