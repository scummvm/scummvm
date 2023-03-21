#include "stdafx.h"

#include "opinion.h"

namespace pyrodactyl
{
	namespace people
	{
		int OPINION_MIN = 0, OPINION_MAX = 100;
	}
}

using namespace pyrodactyl::people;

Opinion :: Opinion()
{
	val[OPI_LIKE] = 0;
	val[OPI_FEAR] = 0;
	val[OPI_RESPECT] = 0;
}

void Opinion :: Load(rapidxml::xml_node<char> *node)
{
	LoadNum(val[OPI_LIKE], "like", node);
	LoadNum(val[OPI_FEAR], "fear", node);
	LoadNum(val[OPI_RESPECT], "respect", node);
}

void Opinion :: Change(const OpinionType &type, const int &change)
{
	val[type] += change;
	Validate(type);
}

void Opinion :: Set(const OpinionType &type, const int &num)
{
	val[type] = num;
	Validate(type);
}

void Opinion :: Validate(const OpinionType &type)
{
	if(val[type] < OPINION_MIN) val[type] = OPINION_MIN;
	else if(val[type] > OPINION_MAX) val[type] = OPINION_MAX;
}

void Opinion :: SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
{
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "opinion");
	child->append_attribute(doc.allocate_attribute("like",gStrPool.Get(val[OPI_LIKE])));
	child->append_attribute(doc.allocate_attribute("fear",gStrPool.Get(val[OPI_FEAR])));
	child->append_attribute(doc.allocate_attribute("respect",gStrPool.Get(val[OPI_RESPECT])));
	root->append_node(child);
}