#include "stdafx.h"
#include "TextData.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;

bool TextData::Load(rapidxml::xml_node<char> *node, Rect *parent, const bool &echo)
{
	if (NodeValid(node, echo))
	{
		Element::Load(node, parent, echo);
		LoadFontKey(font, "font", node);
		LoadNum(col, "color", node);
		LoadAlign(align, node, false);
		LoadBool(background, "background", node, false);
		return true;
	}

	return false;
}

void TextData::Draw(const std::string &val, const int &XOffset, const int &YOffset)
{
	pyrodactyl::text::gTextManager.Draw(x + XOffset, y + YOffset, val, col, font, align, background);
}

void TextData::DrawColor(const std::string &val, const int &color, const int &XOffset, const int &YOffset)
{
	pyrodactyl::text::gTextManager.Draw(x + XOffset, y + YOffset, val, color, font, align, background);
}