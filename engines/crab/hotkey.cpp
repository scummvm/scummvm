#include "hotkey.h"
#include "stdafx.h"

using namespace pyrodactyl::input;

void HotKey::Load(rapidxml::xml_node<char> *node) {
	LoadEnum(input, "input", node);
}

bool HotKey::HandleEvents(const SDL_Event &Event) {
	if (input > IT_NONE && input < IT_TOTAL) {
		int result = gInput.Equals(input, Event);

		if (result == SDL_PRESSED)
			keydown = true;
		else if ((keydown && result == SDL_RELEASED) || result == ANALOG_PRESSED) {
			keydown = false;
			return true;
		}
	}

	return false;
}

const char *HotKey::Name() {
	if (input > IT_NONE && input < IT_TOTAL)
		return SDL_GetScancodeName(gInput.iv[input].key);

	return "";
}