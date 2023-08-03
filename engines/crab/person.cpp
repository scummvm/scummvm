#include "stdafx.h"
#include "person.h"

using namespace pyrodactyl::stat;
using namespace pyrodactyl::people;

Person::Person()
{
	type = PE_NEUTRAL;
	state = PST_NORMAL;
	alt_journal_name = false;
	trig.clear();
}

void Person::Load(rapidxml::xml_node<char> *node, const pyrodactyl::stat::StatTemplates &stem)
{
	if (NodeValid(node))
	{
		LoadStr(id, "id", node);
		LoadStr(name, "name", node);
		//LoadImgKey(pic, "img", node);

		if (NodeValid("opinion", node))
			opinion.Load(node->first_node("opinion"));

		if (node->first_attribute("type") != NULL)
		{
			std::string t;
			LoadStr(t, "type", node);
			type = StringToPersonType(t);
		}
		else
			type = PE_NEUTRAL;

		if (node->first_attribute("state") != NULL)
		{
			std::string s;
			LoadStr(s, "state", node);
			state = StringToPersonState(s);
		}
		else
			state = PST_NORMAL;

		if (node->first_attribute("journal_name") != NULL)
		{
			LoadStr(journal_name, "journal_name", node);
			alt_journal_name = true;
		}
		else
			alt_journal_name = false;

		if (NodeValid("stats", node))
		{
			rapidxml::xml_node<char> *statnode = node->first_node("stats");
			if (statnode->first_attribute("template") == NULL)
			{
				stat.Load(statnode);
			}
			else
			{
				unsigned int index = 0;
				LoadNum(index, "template", statnode);
				if (index < stem.collection.size() && index >= 0)
					for (int i = 0; i < STAT_TOTAL; i++)
						stat.val[i] = stem.collection[index].val[i];
			}
		}

		if (NodeValid("traits", node, false))
		{
			rapidxml::xml_node<char> *traitnode = node->first_node("traits");
			for (auto n = traitnode->first_node("trait"); n != NULL; n = n->next_sibling("trait"))
				trait.push_back(n);
		}
	}
}

void Person::Reset()
{
	for (auto i = 0; i < STAT_TOTAL; ++i)
		stat.val[i].Reset();
}

void Person::Validate()
{
	for (int i = 0; i < STAT_TOTAL; ++i)
		stat.val[i].Validate();
}

void Person::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
{
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "object");
	child->append_attribute(doc.allocate_attribute("id", id.c_str()));
	child->append_attribute(doc.allocate_attribute("name", name.c_str()));

	unsigned int val = static_cast<unsigned int>(state);
	child->append_attribute(doc.allocate_attribute("state", gStrPool.Get(val)));

	opinion.SaveState(doc, child);

	rapidxml::xml_node<char> *child_s = doc.allocate_node(rapidxml::node_element, "stats");
	stat.val[STAT_HEALTH].SaveState(doc, child_s, STATNAME_HEALTH);
	stat.val[STAT_ATTACK].SaveState(doc, child_s, STATNAME_ATTACK);
	stat.val[STAT_DEFENSE].SaveState(doc, child_s, STATNAME_DEFENSE);
	stat.val[STAT_SPEED].SaveState(doc, child_s, STATNAME_SPEED);
	/*stat.val[STAT_CHARISMA].SaveState(doc, child_s, STATNAME_CHARISMA);
	stat.val[STAT_INTELLIGENCE].SaveState(doc, child_s, STATNAME_INTELLIGENCE);*/
	child->append_node(child_s);

	rapidxml::xml_node<char> *child_t = doc.allocate_node(rapidxml::node_element, "traits");
	for (auto &i : trait)
		i.SaveState(doc, child_t, "trait");
	child->append_node(child_t);

	root->append_node(child);
}

void Person::LoadState(rapidxml::xml_node<char> *node)
{
	LoadStr(id, "id", node);
	LoadStr(name, "name", node);
	LoadEnum(state, "state", node);

	if (NodeValid("opinion", node))
		opinion.Load(node->first_node("opinion"));

	if (NodeValid("stats", node))
		stat.Load(node->first_node("stats"));

	if (NodeValid("traits", node, false))
	{
		rapidxml::xml_node<char> *traitnode = node->first_node("traits");

		trait.clear();
		for (auto n = traitnode->first_node("trait"); n != NULL; n = n->next_sibling("trait"))
			trait.push_back(n);
	}
}