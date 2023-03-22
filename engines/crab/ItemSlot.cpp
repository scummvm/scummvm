#include "ItemSlot.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::item;
using namespace pyrodactyl::input;
using namespace pyrodactyl::music;
using namespace pyrodactyl::people;

//------------------------------------------------------------------------
// Purpose: Load
//------------------------------------------------------------------------
void ItemSlot::Load(rapidxml::xml_node<char> *node) {
	StateButton::Load(node);

	if (node->first_attribute("slot") == NULL)
		no_type = true;
	else {
		LoadStr(item_type, "slot", node);
		no_type = false;
	}

	std::string name = node->name();
	if (name == "equip")
		category = SLOT_EQUIP;
	else
		category = SLOT_STORAGE;
}

//------------------------------------------------------------------------
// Purpose: Initialize from reference item slot
//------------------------------------------------------------------------
void ItemSlot::Init(const ItemSlot &ref, const int &XOffset, const int &YOffset) {
	StateButton::Init(ref, XOffset, YOffset);
	canmove = ref.canmove;
	no_type = ref.no_type;
	category = ref.category;
	unread = ref.unread;
}

//------------------------------------------------------------------------
// Purpose: Save state to file
//------------------------------------------------------------------------
void ItemSlot::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	rapidxml::xml_node<char> *child;

	if (category == SLOT_EQUIP)
		child = doc.allocate_node(rapidxml::node_element, "equip");
	else
		child = doc.allocate_node(rapidxml::node_element, "storage");

	item.SaveState(doc, child);
	SaveBool(unread, "unread", doc, child);

	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: Load state from file
//------------------------------------------------------------------------
void ItemSlot::LoadState(rapidxml::xml_node<char> *node) {
	item.Load(node);
	LoadBool(unread, "unread", node);

	if (item.id == "")
		empty = true;
	else
		empty = false;
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ItemSlot::Draw() {
	StateButton::Draw();

	if (!empty)
		item.Draw(x, y);

	if (unread)
		pyrodactyl::image::gImageManager.NotifyDraw(x + w, y);
}

//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
ButtonAction ItemSlot::HandleEvents(const SDL_Event &Event, const int &XOffset, const int &YOffset) {
	ButtonAction ac = StateButton::HandleEvents(Event, XOffset, YOffset);
	if (ac == BUAC_LCLICK || ac == BUAC_RCLICK)
		unread = false;

	return ac;
}

//------------------------------------------------------------------------
// Purpose: Exchange items with another slot
// this object is the current slot, parameter object is target slot
//------------------------------------------------------------------------
bool ItemSlot::Swap(ItemSlot &target) {
	if (CanSwap(target)) {
		Item temp = item;
		item = target.item;
		target.item = temp;

		bool val = empty;
		empty = target.empty;
		target.empty = val;

		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Equip an item
//------------------------------------------------------------------------
bool ItemSlot::Equip(Item &i) {
	if ((item_type == i.type || no_type) && empty) {
		item = i;
		empty = false;
		unread = true;
		return true;
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Change stats based on item
//------------------------------------------------------------------------
void ItemSlot::StatChange(Person &obj, bool increase) {
	if (enabled)
		item.StatChange(obj, increase);
}