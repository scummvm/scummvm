#include "stdafx.h"
#include "level_objects.h"

using namespace pyrodactyl::image;
using namespace pyrodactyl::level;

void PlayerDestMarker::Load(rapidxml::xml_node<char> *node)
{
	LoadImgKey(img, "dest", node);
	size.x = gImageManager.GetTexture(img).W();
	size.y = gImageManager.GetTexture(img).H();
}

void PlayerDestMarker::Draw(const Vector2i &pos, const Rect &camera)
{
	gImageManager.Draw(pos.x - (size.x / 2) - camera.x, pos.y - (size.y / 2) - camera.y, img);
}