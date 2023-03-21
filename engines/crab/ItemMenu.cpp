#include "stdafx.h"
#include "ItemMenu.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::item;
using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Load the layout and items
//------------------------------------------------------------------------
void ItemMenu::Init(const ItemSlot &ref, const Vector2i &inc, const unsigned int &rows, const unsigned int &cols,
	const bool &keyboard)
{
	unsigned int size = rows * cols;
	for (unsigned int i = 0; i < size; ++i)
	{
		ItemSlot b;
		b.Init(ref, inc.x*(i%cols), inc.y*(i / cols));
		element.push_back(b);
	}

	use_keyboard = keyboard;
	AssignPaths();
}

//------------------------------------------------------------------------
// Purpose: Load items from file
//------------------------------------------------------------------------
void ItemMenu::LoadState(rapidxml::xml_node<char> *node)
{
	unsigned int count = 0;
	for (auto n = node->first_node(); n != NULL && count < element.size(); n = n->next_sibling(), ++count)
		element[count].LoadState(n);
}

//------------------------------------------------------------------------
// Purpose: Save to file
//------------------------------------------------------------------------
void ItemMenu::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
{
	for (auto i = element.begin(); i != element.end(); ++i)
		i->SaveState(doc, root);
}

//------------------------------------------------------------------------
// Purpose: Handles drag n' drop - return type is Boolean because we only need to communicate stat changes
//------------------------------------------------------------------------
void ItemMenu::HandleEvents(const SDL_Event &Event, const int &XOffset, const int &YOffset)
{
	int result = Menu<ItemSlot>::HandleEvents(Event);
	if (result != -1)
	{
		select_index = result;
		for (unsigned int i = 0; i < element.size(); ++i)
			element.at(i).State(i == select_index);
	}
}

//------------------------------------------------------------------------
// Purpose: Draw the slot backgrounds first, then the items
//------------------------------------------------------------------------
void ItemMenu::Draw(ItemDesc &item_info)
{
	if (select_index != -1)
		item_info.Draw(element[select_index].item);

	for (auto i = element.begin(); i != element.end(); ++i)
		i->Draw();
}

//------------------------------------------------------------------------
// Purpose: Equip an item at the first available location (used in events)
//------------------------------------------------------------------------
bool ItemMenu::Equip(Item &item)
{
	for (auto i = element.begin(); i != element.end(); ++i)
		if (i->category == SLOT_STORAGE && i->Equip(item))
			return true;

	return false;
}

//------------------------------------------------------------------------
// Purpose: Remove all instances of an item
//------------------------------------------------------------------------
bool ItemMenu::Del(const std::string &id)
{
	bool result = false;

	for (auto &i : element)
		if (i.item.id == id)
		{
			i.empty = true;
			i.item.Clear();
			i.unread = false;
			result = true;
			break;
		}

	return result;
}

//------------------------------------------------------------------------
// Purpose: Find out if we have an item with a name
//------------------------------------------------------------------------
bool ItemMenu::Has(const std::string &container, const std::string &id)
{
	for (auto i = element.begin(); i != element.end(); ++i)
		if (i->item.id == id)
		{
			if (container == "equip")
			{
				if (i->category == SLOT_EQUIP)
					return true;
			}
			else if (container == "storage")
			{
				if (i->category == SLOT_STORAGE)
					return true;
			}
			else
				return true;
		}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Swap an item with another item menu
//------------------------------------------------------------------------
bool ItemMenu::Swap(ItemMenu &target, int index)
{
	//We need to scan the slots first for an empty slot to store the item.
	//If no empty slot is found, then swap with a filled slot of same type
	//If no slot of type is found, don't swap items at all
	int found_index = -1, cur_index = 0;

	for (auto i = element.begin(); i != element.end(); ++i, ++cur_index)
		if (i->item_type == target.element[index].item_type)
		{
			if (i->empty)
			{
				i->Swap(target.element[index]);
				return true;
			}
			else
				found_index = cur_index;
		}

	if (found_index != -1)
	{
		element[found_index].Swap(target.element[index]);
		return true;
	}

	return false;
}