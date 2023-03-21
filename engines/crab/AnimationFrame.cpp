#include "stdafx.h"
#include "AnimationFrame.h"

using namespace pyrodactyl::image;
using namespace pyrodactyl::anim;

AnimationFrame::AnimationFrame(rapidxml::xml_node<char> *node) : eff(node)
{
	Vector2i::Load(node);
	LoadImgKey(img, "img", node);
	LoadNum(start, "start", node);
	LoadNum(finish, "finish", node);
	LoadColor(col, node);

	if (NodeValid("text", node, false))
		text.Load(node->first_node("text"));

	Reset();
}

void AnimationFrame::Reset()
{
	switch (eff.type)
	{
	case FADE_IN: col.a = 0; break;
	case FADE_OUT: col.a = 255; break;
	default: col.a = 255; break;
	}
}

void AnimationFrame::Draw(const Uint32 &timestamp)
{
	//Only draw the frame in the specified duration
	if (timestamp >= start && timestamp <= finish)
	{
		//Fill the screen with the color indicated
		SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(gRenderer, col.r, col.g, col.b, col.a);
		SDL_RenderFillRect(gRenderer, NULL);

		gImageManager.Draw(x, y, img);
		text.Draw();
	}
}

DrawType AnimationFrame::InternalEvents(const Uint32 &timestamp)
{
	//Vary alpha according to the effect values in the variation time frame
	if (timestamp >= eff.start && timestamp <= eff.finish)
	{
		//These equations courtesy of linear algebra
		switch (eff.type)
		{
		case FADE_IN: col.a = (255 * (timestamp - eff.start)) / (eff.finish - eff.start); break;
		case FADE_OUT: col.a = (255 * (eff.finish - timestamp)) / (eff.finish - eff.start); break;
		default: break;
		}

		return eff.draw_game;
	}

	return DRAW_SAME;
}