#include "stdafx.h"
#include "element.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void Element::Init(const int &X, const int &Y, const Align &align_x, const Align &align_y,
	const ImageKey img, const int &W, const int &H)
{
	x = X;
	y = Y;
	align.x = align_x;
	align.y = align_y;

	if (img == 0){ w = W; h = H; }
	else
	{
		Image dat = gImageManager.GetTexture(img);
		w = dat.W();
		h = dat.H();
	}
}

void Element::BasicLoad(rapidxml::xml_node<char> *node, const bool &echo)
{
	raw.Load(node, echo);
	LoadAlign(align.x, node, echo, "align_x");
	LoadAlign(align.y, node, echo, "align_y");
}

void Element::Load(rapidxml::xml_node<char> *node, ImageKey img, const bool &echo)
{
	BasicLoad(node, echo);

	if (node->first_attribute("w") == NULL)
		w = gImageManager.GetTexture(img).W();
	else
		LoadNum(w, "w", node);

	if (node->first_attribute("h") == NULL)
		h = gImageManager.GetTexture(img).H();
	else
		LoadNum(h, "h", node);

	SetUI();
}

void Element::Load(rapidxml::xml_node<char> *node, Rect *parent, const bool &echo)
{
	BasicLoad(node, echo);
	LoadNum(w, "w", node, false);
	LoadNum(h, "h", node, false);
	SetUI(parent);
}

void Element::SetUI(Rect *parent)
{
	if (parent == NULL)
	{
		switch (align.x)
		{
		case ALIGN_CENTER: x = gScreenSettings.cur.w / 2 - w / 2 + raw.x; break;
		case ALIGN_RIGHT: x = gScreenSettings.cur.w - w + raw.x; break;
		default: x = raw.x; break;
		}

		switch (align.y)
		{
		case ALIGN_CENTER: y = gScreenSettings.cur.h / 2 - h / 2 + raw.y; break;
		case ALIGN_RIGHT: y = gScreenSettings.cur.h - h + raw.y; break;
		default: y = raw.y; break;
		}
	}
	else
	{
		switch (align.x)
		{
		case ALIGN_CENTER: x = parent->x + parent->w / 2 - w / 2 + raw.x; break;
		case ALIGN_RIGHT: x = parent->x + parent->w - w + raw.x; break;
		default: x = parent->x + raw.x; break;
		}

		switch (align.y)
		{
		case ALIGN_CENTER: y = parent->y + parent->h / 2 - h / 2 + raw.y; break;
		case ALIGN_RIGHT: y = parent->y + parent->h - h + raw.y; break;
		default: y = parent->y + raw.y; break;
		}
	}
}