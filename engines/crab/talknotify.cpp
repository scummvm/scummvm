#include "stdafx.h"
#include "talknotify.h"

using namespace pyrodactyl::text;
using namespace pyrodactyl::level;
using namespace pyrodactyl::image;

void TalkNotify::Load(rapidxml::xml_node<char> *node)
{
	offset.Load(node);
	LoadNum(font, "font", node);
	LoadAlign(align, node);
	LoadNum(col, "col", node);
}

void TalkNotify::Draw(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s, const Rect &camera)
{
	Rect rect = s.PosRect();

	//Find position to draw name (over the sprite's top edge)
	Vector2i pos;
	pos.x = rect.x + rect.w / 2 + offset.x;
	pos.y = rect.y + offset.y;

	//Find the sprite name
	std::string text = info.GetName(s.ID());

	gTextManager.Draw(pos.x - camera.x, pos.y - camera.y, text, col, font, align, true);
}