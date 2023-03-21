#include "stdafx.h"

#include "StatPreview.h"

using namespace pyrodactyl::item;

void StatPreview :: Load(rapidxml::xml_node<char> *node)
{
	LoadBool(enabled, "enabled", node);

	if(NodeValid("stat",node))
	{
		rapidxml::xml_node<char> *snode = node->first_node("stat");
		stat.Load(snode);
		LoadNum(inc_s.x, "w", snode);
		LoadNum(inc_s.y, "h", snode);
	}

	if(NodeValid("unit",node))
	{
		rapidxml::xml_node<char> *snode = node->first_node("unit");
		unit.Load(snode);
		LoadNum(inc_u.x, "w", snode);
		LoadNum(inc_u.y, "h", snode);
	}
}

void StatPreview :: Draw(Item &item, pyrodactyl::stat::StatDrawHelper &helper)
{
	if(enabled)
	{
		int count = 0;
		for(auto i = item.bonus.begin(); i != item.bonus.end(); ++i, ++count)
		{
			stat.Draw(helper.Name(i->type),inc_s.x*count,inc_s.y*count);
			unit.Draw(gStrPool.Get(i->val),inc_u.x*count, inc_u.y*count);
		}
	}
}