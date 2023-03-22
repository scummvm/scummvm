#pragma once
#include "common_header.h"

#include "menu.h"
#include "textarea.h"

namespace pyrodactyl {
namespace ui {
class ResolutionMenu : public ButtonMenu {
	enum State {
		STATE_NORMAL,
		STATE_CHANGE
	} state;

	HoverInfo info;
	std::string def_info;
	Button change, cancel, custom;

	// Menu stores the button for each of the item in the dimension array
	std::vector<Dimension> dim;

	// The reference button for resolution
	Button ref;

	// How much the button is incremented by
	Vector2i inc;

	// The number of rows and columns
	int columns;

public:
	ResolutionMenu(void) {
		state = STATE_NORMAL;
		columns = 1;
	}
	~ResolutionMenu(void) {}

	void Load(rapidxml::xml_node<char> *node);
	void Draw();

	// Return 1 if one of resolution buttons is pressed, 2 if custom button is pressed, 0 otherwise
	int HandleEvents(const SDL_Event &Event);

	void SetInfo() { info.text = def_info + NumberToString(gScreenSettings.cur.w) + " x " + NumberToString(gScreenSettings.cur.h); }
	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl