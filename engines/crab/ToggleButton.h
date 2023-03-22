#pragma once

#include "button.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
// This button has two states that can be switched between by clicking the button
// Similar to the radio button / checkbox UI element in windows/web
class ToggleButton : public Button {
	// The images corresponding to the state
	ImageKey on, off;

	// The offset at which the on/off image is drawn
	Vector2i offset;

public:
	// The state of the button - true is on, false is off
	bool state;

	ToggleButton() {
		state = false;
		on = 0;
		off = 0;
	}
	~ToggleButton() {}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const int &XOffset = 0, const int &YOffset = 0, Rect *clip = NULL);
	ButtonAction HandleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0);
};
} // End of namespace ui
} // End of namespace pyrodactyl