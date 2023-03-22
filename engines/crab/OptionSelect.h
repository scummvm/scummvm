#pragma once
#include "common_header.h"

#include "TextData.h"
#include "button.h"

namespace pyrodactyl {
namespace ui {
class OptionSelect {
	struct
	{
		std::vector<std::string> text;
		TextData data;

		void Draw(const int &index) {
			if (index >= 0 && index < text.size())
				data.Draw(text.at(index));
		}
	} option;

	Button prev, next;
	bool usekeyboard;

public:
	int cur;

	OptionSelect() {
		cur = 0;
		usekeyboard = false;
	}
	void Load(rapidxml::xml_node<char> *node);

	void Draw();
	bool HandleEvents(const SDL_Event &Event);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl