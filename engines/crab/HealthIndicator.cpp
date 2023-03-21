#include "stdafx.h"

#include "HealthIndicator.h"

using namespace pyrodactyl::ui;

void HealthIndicator :: Load(rapidxml::xml_node<char> *node)
{
	if(NodeValid(node))
	{
		LoadXY(x, y, node);

		for(auto n = node->first_node("img"); n != NULL; n = n->next_sibling("img"))
		{
			HealthImage hi;
			LoadImgKey(hi.normal, "normal", n);
			LoadImgKey(hi.glow, "glow", n);
			LoadNum(hi.val, "val", n);

			img.push_back(hi);
		}
	}
}

void HealthIndicator :: Draw(int num)
{
	for(auto i = img.begin(); i != img.end(); ++i)
		if(num == i->val)
		{
			using namespace pyrodactyl::image;
			gImageManager.Draw(x, y, i->normal);
			gImageManager.GetTexture(i->glow).Alpha(alpha);
			gImageManager.Draw(x, y, i->glow);

			if(inc)
			{
				alpha += 2;
				if(alpha >= 250)
					inc = false;
			}
			else
			{
				alpha -= 2;
				if(alpha < 4)
					inc = true;
			}

			break;
		}
}