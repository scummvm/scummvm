#include "stdafx.h"
#include "StateButton.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void StateButton::Init(const StateButton &ref, const int &XOffset, const int &YOffset)
{
	Button::Init(ref, XOffset, YOffset);
	img_set = ref.img_set;
	col_normal = ref.col_normal;
	col_select = ref.col_select;
}

void StateButton::Load(rapidxml::xml_node<char> * node, const bool &echo)
{
	Button::Load(node, echo);

	img_set.normal = img;
	col_normal.col = caption.col;
	col_normal.col_s = caption.col_s;

	if (NodeValid("select", node, false))
	{
		rapidxml::xml_node<char> *selnode = node->first_node("select");

		img_set.select.Load(selnode, echo);
		LoadNum(col_select.col, "color", selnode);
		LoadNum(col_select.col_s, "color_s", selnode);
	}
	else
	{
		img_set.select = img;
		col_select.col = caption.col;
		col_select.col_s = caption.col_s;
	}
}

void StateButton::State(const bool val)
{
	if (val)
	{
		img = img_set.select;
		caption.col = col_select.col;
		caption.col_s = col_select.col_s;
	}
	else
	{
		img = img_set.normal;
		caption.col = col_normal.col;
		caption.col_s = col_normal.col_s;
	}

	//Images might be different in size
	w = gImageManager.GetTexture(img.normal).W();
	h = gImageManager.GetTexture(img.normal).H();
}

void StateButton::Img(const StateButtonImage &sbi)
{
	//Find which is the current image and set it
	if (img == img_set.normal)
		img = sbi.normal;
	else
		img = sbi.select;

	img_set = sbi;
}