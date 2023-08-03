#include "stdafx.h"
#include "Inventory.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::item;
using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Load layout
//------------------------------------------------------------------------
void Inventory::Load(const std::string &filename)
{
	XMLDoc conf(filename);
	if (conf.ready())
	{
		rapidxml::xml_node<char> * node = conf.Doc()->first_node("inventory");
		if (NodeValid(node))
		{
			if (NodeValid("bg", node))
				bg.Load(node->first_node("bg"));

			collection.Load(node->first_node("items"));

			/*if (NodeValid("stats", node))
				helper.Load(node->first_node("stats"));*/

			if (NodeValid("money", node))
				money.Load(node->first_node("money"));
		}
	}
}

void Inventory::LoadItem(const std::string &char_id, const std::string &id)
{
	Item i;
	XMLDoc item_list(itemfile);
	if (item_list.ready())
	{
		rapidxml::xml_node<char> *node = item_list.Doc()->first_node("items");
		for (auto n = node->first_node("item"); n != NULL; n = n->next_sibling("item"))
		{
			std::string str = n->first_attribute("id")->value();
			if (id == str)
			{
				i.Load(n);
				AddItem(char_id, i);
				break;
			}
		}
	}
}

void Inventory::DelItem(const std::string &char_id, const std::string &item_id)
{
	collection.Del(char_id, item_id);
}

void Inventory::AddItem(const std::string &char_id, Item &item)
{
	collection.Add(char_id, item);
}

bool Inventory::HasItem(const std::string &char_id, const std::string &container, const std::string &item_id)
{
	return collection.Has(char_id, container, item_id);
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Inventory::Draw(Person &obj, const int &money_val)
{
	bg.Draw();
	//helper.DrawInfo(obj);
	collection.Draw(obj.id/*, helper*/);

	money.caption.text = NumberToString(money_val);
	money.Draw();
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void Inventory::HandleEvents(const std::string &char_id, const SDL_Event &Event)
{
	collection.HandleEvents(char_id, Event);
	money.HandleEvents(Event);
}

//------------------------------------------------------------------------
// Purpose: Load and save items
//------------------------------------------------------------------------
void Inventory::LoadState(rapidxml::xml_node<char> *node)
{
	if (NodeValid("items", node))
		collection.LoadState(node->first_node("items"));
}

void Inventory::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
{
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, "items");
	collection.SaveState(doc, child);
	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: Set UI positions after screen size change
//------------------------------------------------------------------------
void Inventory::SetUI()
{
	bg.SetUI();
	collection.SetUI();
	money.SetUI();
}