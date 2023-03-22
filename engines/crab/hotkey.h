#pragma once

#include "common_header.h"
#include "input.h"

namespace pyrodactyl {
namespace input {
// This class is built to integrate the input check for hotkeys bound to buttons
class HotKey {
	// The type of input the hotkey is checking for
	InputType input;

	// Has the key been pressed?
	bool keydown;

public:
	HotKey() {
		input = IT_NONE;
		keydown = false;
	}

	void Set(const InputType &val) { input = val; }
	const char *Name();

	void Load(rapidxml::xml_node<char> *node);
	bool HandleEvents(const SDL_Event &Event);
};
} // End of namespace input
} // End of namespace pyrodactyl