#include "stdafx.h"
#include "AlphaImage.h"
#include "TextManager.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void AlphaImage::Load(rapidxml::xml_node<char> *node, const bool &echo)
{
	pos.Load(node, echo);
	LoadImgKey(img, "img", node, echo);

	LoadNum(alpha.min, "min", node);
	LoadNum(alpha.max, "max", node);
	LoadNum(alpha.change, "inc", node);

	alpha.cur = alpha.min + gRandom.Num() % (alpha.max - alpha.min - 1);
}

void AlphaImage::InternalEvents()
{
	if (alpha.inc)
	{
		alpha.cur += alpha.change;
		if (alpha.cur >= alpha.max)
		{
			alpha.cur = alpha.max;
			alpha.inc = false;
		}
	}
	else
	{
		alpha.cur -= alpha.change;
		if (alpha.cur <= alpha.min)
		{
			alpha.cur = alpha.min;
			alpha.inc = true;
		}
	}

	if (gImageManager.ValidTexture(img))
		gImageManager.GetTexture(img).Alpha(alpha.cur);
}

void AlphaImage::Draw(const int &XOffset, const int &YOffset)
{
	if (gImageManager.ValidTexture(img))
		gImageManager.GetTexture(img).Draw(pos.x + XOffset, pos.y + YOffset);
}