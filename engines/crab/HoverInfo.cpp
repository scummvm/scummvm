#include "stdafx.h"

#include "HoverInfo.h"

using namespace pyrodactyl::text;
using namespace pyrodactyl::ui;

void HoverInfo::Init(const HoverInfo &h, const int &XOffset, const int &YOffset) {
	*this = h;
	x += XOffset;
	y += YOffset;
}

void HoverInfo::Load(rapidxml::xml_node<char> *node, Rect *parent) {
	if (TextData::Load(node, parent, false)) {
		LoadStr(text, "text", node);
		enabled = true;
	}
}

void HoverInfo::Draw(const int &XOffset, const int &YOffset) {
	if (enabled)
		TextData::Draw(text, XOffset, YOffset);
}