#include "stdafx.h"
#include "SectionHeader.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;

void SectionHeader::Load(rapidxml::xml_node<char> *node)
{
	if (TextData::Load(node, NULL, false))
	{
		LoadStr(text, "text", node);
		text.insert(0, " ");
		text += " ";

		LoadImgKey(img, "img", node);

		LoadBool(draw_l, "left", node);
		LoadBool(draw_r, "right", node);

		SDL_Surface *surf = gTextManager.RenderTextBlended(font, text, col);

		if (align == ALIGN_CENTER)
		{
			left.x = x - surf->w / 2 - gImageManager.GetTexture(img).W();
			left.y = y - surf->h / 2 + gImageManager.GetTexture(img).H() / 2;

			right.x = x + surf->w / 2;
			right.y = y - surf->h / 2 + gImageManager.GetTexture(img).H() / 2;
		}
		else if (align == ALIGN_LEFT)
		{
			left.x = x - gImageManager.GetTexture(img).W();
			left.y = y + surf->h / 2 - gImageManager.GetTexture(img).H() / 2;

			right.x = x + surf->w;
			right.y = y + surf->h / 2 - gImageManager.GetTexture(img).H() / 2;
		}
		else
		{
			left.x = x - surf->w - gImageManager.GetTexture(img).W();
			left.y = y + surf->h / 2 - gImageManager.GetTexture(img).H() / 2;

			right.x = x;
			right.y = y + surf->h / 2 - gImageManager.GetTexture(img).H() / 2;
		}

		SDL_FreeSurface(surf);
	}
}

void SectionHeader::Draw(const int &XOffset, const int &YOffset)
{
	Draw(text, XOffset, YOffset);
}

void SectionHeader::Draw(const std::string &str, const int &XOffset, const int &YOffset)
{
	if (draw_l)
		gImageManager.Draw(left.x + XOffset, left.y + YOffset, img);

	if (draw_r)
		gImageManager.Draw(right.x + XOffset, right.y + YOffset, img, NULL, FLIP_X);

	TextData::Draw(str, XOffset, YOffset);
}