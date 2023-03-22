#include "ParagraphData.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;

bool ParagraphData::Load(rapidxml::xml_node<char> *node, Rect *parent, const bool &echo) {
	if (NodeValid("line", node))
		line.Load(node->first_node("line"));

	return TextData::Load(node, parent, echo);
}

void ParagraphData::Draw(const std::string &val, const int &XOffset, const int &YOffset) {
	pyrodactyl::text::gTextManager.Draw(x + XOffset, y + YOffset, val, col, font, align, line.x, line.y);
}