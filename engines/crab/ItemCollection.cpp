#include "ItemCollection.h"
#include "stdafx.h"

using namespace pyrodactyl::people;
using namespace pyrodactyl::item;
using namespace pyrodactyl::ui;

//------------------------------------------------------------------------
// Purpose: Load the reference information
//------------------------------------------------------------------------
void ItemCollection::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("info", node))
		item_info.Load(node->first_node("info"));

	if (NodeValid("ref", node))
		ref.Load(node->first_node("ref"));

	if (NodeValid("inc", node))
		inc.Load(node->first_node("inc"));

	if (NodeValid("dim", node)) {
		rapidxml::xml_node<char> *dimnode = node->first_node("dim");
		LoadNum(rows, "rows", dimnode);
		LoadNum(cols, "cols", dimnode);
	}

	LoadBool(usekeyboard, "keyboard", node);
}

//------------------------------------------------------------------------
// Purpose: Add a character's inventory if not added already
//------------------------------------------------------------------------
void ItemCollection::Init(const std::string &char_id) {
	if (item.count(char_id) == 0)
		item[char_id].Init(ref, inc, rows, cols, usekeyboard);
}

//------------------------------------------------------------------------
// Purpose: Handle events
//------------------------------------------------------------------------
void ItemCollection::HandleEvents(const std::string &char_id, const SDL_Event &Event) {
	if (item.count(char_id) > 0)
		item[char_id].HandleEvents(Event);
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void ItemCollection::Draw(const std::string &char_id) {
	if (item.count(char_id) > 0)
		item[char_id].Draw(item_info);
}

//------------------------------------------------------------------------
// Purpose: Delete an item from a character's inventory
//------------------------------------------------------------------------
void ItemCollection::Del(const std::string &char_id, const std::string &item_id) {
	if (item.count(char_id) > 0)
		item[char_id].Del(item_id);
}

//------------------------------------------------------------------------
// Purpose: Add an item to a character's inventory
//------------------------------------------------------------------------
void ItemCollection::Add(const std::string &char_id, Item &item_data) {
	// We might want to give a player character not yet encountered an item before we ever meet them
	// Which is why we add a new inventory in case the character inventory does not exist yet
	Init(char_id);

	item[char_id].Equip(item_data);
}

//------------------------------------------------------------------------
// Purpose: Find if a character has an item
//------------------------------------------------------------------------
bool ItemCollection::Has(const std::string &char_id, const std::string &container, const std::string &item_id) {
	if (item.count(char_id) > 0)
		return item[char_id].Has(container, item_id);

	return false;
}

//------------------------------------------------------------------------
// Purpose: Load items from save file
//------------------------------------------------------------------------
void ItemCollection::LoadState(rapidxml::xml_node<char> *node) {
	for (auto n = node->first_node(); n != NULL; n = n->next_sibling()) {
		// Add all characters in the save file, whether we have them in the inventory or not
		Init(n->name());
		item[n->name()].LoadState(n);
	}
}

//------------------------------------------------------------------------
// Purpose: Write items to save file
//------------------------------------------------------------------------
void ItemCollection::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root) {
	for (auto i = item.begin(); i != item.end(); ++i) {
		rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, i->first.c_str());
		i->second.SaveState(doc, child);
		root->append_node(child);
	}
}

//------------------------------------------------------------------------
// Purpose: Reset UI elements when resolution changes
//------------------------------------------------------------------------
void ItemCollection::SetUI() {
	item_info.SetUI();

	for (auto i = item.begin(); i != item.end(); ++i)
		i->second.SetUI();
}