#include "ToggleButton.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ToggleButton::Load(rapidxml::xml_node<char> *node) {
	Button::Load(node);
	LoadImgKey(on, "on", node);
	LoadImgKey(off, "off", node);

	if (NodeValid("offset", node))
		offset.Load(node->first_node("offset"));
}

void ToggleButton::Draw(const int &XOffset, const int &YOffset, Rect *clip) {
	Button::Draw(XOffset, YOffset, clip);

	if (state)
		gImageManager.Draw(x + offset.x, y + offset.y, on);
	else
		gImageManager.Draw(x + offset.x, y + offset.y, off);
}

ButtonAction ToggleButton::HandleEvents(const SDL_Event &Event, const int &XOffset, const int &YOffset) {
	ButtonAction action = Button::HandleEvents(Event, XOffset, YOffset);

	if (action == BUAC_LCLICK)
		state = !state;

	return action;
}