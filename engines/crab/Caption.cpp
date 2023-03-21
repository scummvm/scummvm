#include "stdafx.h"

#include "Caption.h"

using namespace pyrodactyl::text;
using namespace pyrodactyl::ui;

void Caption :: Init(const Caption &c, const int &XOffset, const int &YOffset)
{
	*this = c;
	x += XOffset;
	y += YOffset;
}

void Caption :: Load(rapidxml::xml_node<char> *node, Rect *parent)
{
	if(TextData::Load(node,parent,false))
	{
		LoadStr(text, "text", node);
		LoadNum(col_s,"color_s",node,false);
		enabled = true;
	}
}

void Caption :: Draw(bool selected, const int &XOffset, const int &YOffset)
{
	if(enabled)
	{
		if(selected)
			TextData::DrawColor(text, col_s, XOffset, YOffset);
		else
			TextData::Draw(text, XOffset, YOffset);
	}
}