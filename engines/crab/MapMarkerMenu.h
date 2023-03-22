#pragma once

#include "StateButton.h"
#include "common_header.h"
#include "menu.h"

namespace pyrodactyl {
namespace ui {
class MapMarkerMenu {
	// The reference map marker
	StateButton ref;

	// The menu containing all the map markers
	Menu<StateButton> menu;

	// The offset at which every map marker is drawn (used to compensate for icon design width)
	struct {
		Vector2i marker, player;
	} offset;

	// The button for the player's current position
	Button player;

public:
	MapMarkerMenu() {}
	~MapMarkerMenu() {}

	void AddButton(const std::string &name, const int &x, const int &y) {
		StateButton b;
		b.Init(ref, x, y);
		b.tooltip.text = name;
		menu.element.push_back(b);
	}

	void Clear() { menu.element.clear(); }
	void AssignPaths() { menu.AssignPaths(); }

	void SelectDest(const std::string &name) {
		for (auto &i : menu.element)
			i.State(i.tooltip.text == name);
	}

	void Erase(const std::string &name) {
		for (auto i = menu.element.begin(); i != menu.element.end(); ++i) {
			if (i->tooltip.text == name) {
				menu.element.erase(i);
				AssignPaths();
				break;
			}
		}
	}

	void Load(rapidxml::xml_node<char> *node);

	void Draw(const Element &pos, const Vector2i &player_pos, const Rect &camera);
	void HandleEvents(const Element &pos, const Vector2i &player_pos, const Rect &camera, const SDL_Event &Event);
	void InternalEvents(const Element &pos, const Vector2i &player_pos, const Rect &camera, Rect bounds);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl