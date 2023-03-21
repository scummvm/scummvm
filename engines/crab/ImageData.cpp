#include "stdafx.h"
#include "ImageData.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ImageData::Load(rapidxml::xml_node<char> *node, const bool &echo)
{
	LoadImgKey(key, "img", node, echo);
	LoadBool(crop, "crop", node, false);

	if (NodeValid("clip", node, false))
		clip.Load(node->first_node("clip"));

	Element::Load(node, key, echo);
}

void ImageData::Draw(const int &XOffset, const int &YOffset)
{
	if (crop)
		gImageManager.Draw(x + XOffset, y + YOffset, key, &clip);
	else
		gImageManager.Draw(x + XOffset, y + YOffset, key);
}