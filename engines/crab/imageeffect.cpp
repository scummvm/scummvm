#include "stdafx.h"

#include "imageeffect.h"

using namespace pyrodactyl::anim;

void ImageEffect :: Load(rapidxml::xml_node<char> * node)
{
	if(NodeValid(node,false))
	{
		if(LoadImgKey(img,"img",node) && LoadXY(x,y,node))
			visible = true;
	}
}

void ImageEffect :: Draw(const int &XOffset, const int &YOffset)
{
	if(visible)
		pyrodactyl::image::gImageManager.Draw(x+XOffset,y+YOffset,img);
}