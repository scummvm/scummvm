#include "stdafx.h"
#include "animation.h"

using namespace pyrodactyl::image;
using namespace pyrodactyl::anim;

Animation::Animation(rapidxml::xml_node<char> *node)
{
	LoadNum(length, "length", node);
	for (auto n = node->first_node("frame"); n != NULL; n = n->next_sibling("frame"))
		frame.push_back(n);
}

void Animation::Draw()
{
	Uint32 timestamp = timer.Ticks();
	for (auto i = frame.begin(); i != frame.end(); ++i)
		i->Draw(timestamp);
}

bool Animation::InternalEvents(DrawType &game_draw)
{
	Uint32 timestamp = timer.Ticks();
	for (auto i = frame.begin(); i != frame.end(); ++i)
	{
		DrawType result = i->InternalEvents(timestamp);
		//if (result != DRAW_SAME)
		game_draw = result;
	}

	return timer.Ticks() >= length;
}

void Animation::Reset()
{
	for (auto i = frame.begin(); i != frame.end(); ++i)
		i->Reset();
}