#include "stdafx.h"

#include "ClipButton.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ClipButton::Load(rapidxml::xml_node<char> *node, const bool &echo) {
	Button::Load(node, echo);

	if (NodeValid("clip", node, false))
		clip.Load(node->first_node("clip"));
	else {
		clip.x = 0;
		clip.y = 0;
		clip.w = gImageManager.GetTexture(img.normal).W();
		clip.h = gImageManager.GetTexture(img.normal).H();
	}
}

void ClipButton::Draw(const int &XOffset, const int &YOffset) {
	Button::Draw(XOffset, YOffset, &clip);
}