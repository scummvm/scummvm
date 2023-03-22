#pragma once

#include "GameEventInfo.h"
#include "common_header.h"
#include "textarea.h"

namespace pyrodactyl {
namespace ui {
class DebugConsole {
	// The state of the menu
	enum { STATE_NORMAL,
		   STATE_VAR } state;

	// The overarching menu that is the starting point for all functions
	ButtonMenu menu;

	// The dialog box UI - used to check value of a variable
	ImageData bg;
	Button check, back;
	TextData value;
	TextArea text_field;

	// The variable name we're tracking
	std::string var_name;

public:
	DebugConsole() { state = STATE_NORMAL; }
	~DebugConsole() {}

	// Only restrict input when we're in variable state
	bool RestrictInput() { return (state == STATE_VAR || menu.HoverIndex() != -1); }

	void Load(const std::string &filename);
	void Draw(pyrodactyl::event::Info &info);

	void HandleEvents(const SDL_Event &Event);
	void InternalEvents();
};
} // End of namespace ui
} // End of namespace pyrodactyl