#include "TraitButton.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;

void TraitButton::Init(const TraitButton &ref, const int &XOffset, const int &YOffset) {
	StateButton::Init(ref, XOffset, YOffset);
	offset = ref.offset;
}

void TraitButton::Load(rapidxml::xml_node<char> *node, const bool &echo) {
	StateButton::Load(node, echo);

	if (NodeValid("offset", node))
		offset.Load(node->first_node("offset"), echo);
}

void TraitButton::Draw(const int &XOffset, const int &YOffset, Rect *clip) {
	if (trait_img != 0)
		gImageManager.Draw(x + offset.x, y + offset.y, trait_img);

	StateButton::Draw(XOffset, YOffset, clip);
}

void TraitButton::Cache(const pyrodactyl::people::Trait &trait) {
	trait_img = trait.img;
	caption.text = trait.name;
}

void TraitButton::Empty() {
	trait_img = 0;
	caption.text = "";
}