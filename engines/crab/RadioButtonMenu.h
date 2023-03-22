#pragma once

#include "RadioButton.h"
#include "menu.h"

namespace pyrodactyl {
namespace ui {
class RadioButtonMenu : public Menu<RadioButton> {
	// The description of the menu
	HoverInfo desc;

	// The selected radio button
	int select;

public:
	RadioButtonMenu() { select = 0; }
	~RadioButtonMenu() {}

	void Load(rapidxml::xml_node<char> *node) {
		if (NodeValid("desc", node))
			desc.Load(node->first_node("desc"));

		if (NodeValid("menu", node))
			Menu::Load(node->first_node("menu"));
	}

	void Draw(const int &XOffset = 0, const int &YOffset = 0) {
		desc.Draw(XOffset, YOffset);
		Menu::Draw(XOffset, YOffset);
	}

	int HandleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0) {
		int result = Menu::HandleEvents(Event, XOffset, YOffset);

		if (result >= 0) {
			select = result;

			for (int i = 0; i < element.size(); ++i)
				element.at(i).state = (i == result);
		}

		return result;
	}

	void SetUI() {
		Menu::SetUI();
		desc.SetUI();
	}
};
} // End of namespace ui
} // End of namespace pyrodactyl