#include "questmenu.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::event;

QuestMenu::QuestMenu() {
	sel_quest = -1;
	sel_page = -1;
	sel_bu = -1;
	align = ALIGN_LEFT;
	col_n = 0;
	col_s = 0;
	unread = false;
	font = 0;
}

//------------------------------------------------------------------------
// Purpose: Load layout from file
//------------------------------------------------------------------------
void QuestMenu::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		if (NodeValid("menu", node))
			menu.Load(node->first_node("menu"));

		if (NodeValid("tab", node)) {
			rapidxml::xml_node<char> *tabnode = node->first_node("tab");
			LoadNum(font, "font", tabnode);
			LoadAlign(align, tabnode);
			off_title.Load(tabnode);
			off_unread.Load(tabnode->first_node("unread"));

			if (NodeValid("normal", tabnode)) {
				rapidxml::xml_node<char> *nornode = tabnode->first_node("normal");
				img_n.Load(nornode);
				LoadColor(col_n, nornode);
			}

			if (NodeValid("select", tabnode)) {
				rapidxml::xml_node<char> *selnode = tabnode->first_node("select");
				img_s.Load(selnode);
				LoadColor(col_s, selnode);
			}
		}

		if (NodeValid("text", node))
			text.Load(node->first_node("text"));
	}
}

//------------------------------------------------------------------------
// Purpose: Add an entry to the menu
//------------------------------------------------------------------------
void QuestMenu::Add(const std::string &title, const std::string &text) {
	for (auto i = quest.begin(); i != quest.end(); ++i)
		if (i->title == title) // We already have the quest entry
		{
			i->text.push_front(text); // Just add the new string to the start of the quest messages and return
			i->unread = true;
			return;
		}

	Quest q(title, text, true, false);
	quest.push_front(q);
	menu.Add();
	unread = true;
}

void QuestMenu::Add(const pyrodactyl::event::Quest &q) {
	quest.push_front(q);
	menu.Add();
}

//------------------------------------------------------------------------
// Purpose: Remove an entry from the menu
//------------------------------------------------------------------------
void QuestMenu::Erase(const int &index) {
	quest.erase(quest.begin() + index);
	menu.Erase();
}

//------------------------------------------------------------------------
// Purpose: Indicate that this quest has an associated map marker in world map
//------------------------------------------------------------------------
void QuestMenu::Marker(const std::string &title, const bool &val) {
	for (auto i = quest.begin(); i != quest.end(); ++i)
		if (i->title == title)
			i->marker = val;
}
//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void QuestMenu::Draw(Button &bu_map) {
	menu.Draw();

	using namespace pyrodactyl::text;
	for (auto i = menu.Index(), count = 0u; i < menu.IndexPlusOne() && i < quest.size(); i++, count++) {
		auto base_x = menu.BaseX(count), base_y = menu.BaseY(count);

		// Only draw in _s color if we are on the same button and page
		if (sel_bu == count && sel_page == menu.CurrentPage())
			gTextManager.Draw(base_x + off_title.x, base_y + off_title.y, quest[i].title, col_s, font, align);
		else
			gTextManager.Draw(base_x + off_title.x, base_y + off_title.y, quest[i].title, col_n, font, align);

		if (quest.at(i).unread) {
			using namespace pyrodactyl::image;
			gImageManager.Draw(base_x + off_unread.x, base_y + off_unread.y, gImageManager.notify);
		}
	}

	if (sel_quest >= 0 && sel_quest < quest.size()) {
		text.Draw(quest.at(sel_quest));

		if (quest.at(sel_quest).marker)
			bu_map.Draw();
	}
}

//------------------------------------------------------------------------
// Purpose: Handle user input
//------------------------------------------------------------------------
bool QuestMenu::HandleEvents(Button &bu_map, std::string &map_title, const SDL_Event &Event) {
	int res = menu.HandleEvents(Event);
	if (res != -1) {
		if (sel_bu >= 0 && sel_page >= 0)
			menu.Image(sel_bu, sel_page, img_n);

		sel_bu = res;
		sel_page = menu.CurrentPage();
		sel_quest = menu.Index() + sel_bu;

		quest.at(sel_quest).unread = false;
		text.Reset();

		menu.Image(sel_bu, sel_page, img_s);
	}

	if (sel_quest >= 0 && sel_quest < quest.size()) {
		if (quest.at(sel_quest).marker)
			if (bu_map.HandleEvents(Event) == BUAC_LCLICK) {
				// The title of the quest selected by the "show in map" button
				map_title = quest.at(sel_quest).title;
				return true;
			}

		text.HandleEvents(quest.at(sel_quest), Event);
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Select an entry
//------------------------------------------------------------------------
void QuestMenu::Select(const int &quest_index) {
	if (quest_index >= 0 && quest_index < quest.size()) {
		if (sel_bu >= 0 && sel_page >= 0)
			menu.Image(sel_bu, sel_page, img_n);

		sel_quest = quest_index;

		sel_page = quest_index / menu.ElementsPerPage();
		menu.CurrentPage(sel_page);
		menu.UpdateInfo();

		sel_bu = quest_index % menu.ElementsPerPage();

		quest.at(quest_index).unread = false;
		text.Reset();

		menu.Image(sel_bu, sel_page, img_s);
	}
}

//------------------------------------------------------------------------
// Purpose: Save state to file
//------------------------------------------------------------------------
void QuestMenu::SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char *name) {
	rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);

	SaveBool(unread, "unread", doc, child);

	for (auto q = quest.begin(); q != quest.end(); ++q)
		q->SaveState(doc, child);

	root->append_node(child);
}

//------------------------------------------------------------------------
// Purpose: Load state from file
//------------------------------------------------------------------------
void QuestMenu::LoadState(rapidxml::xml_node<char> *node) {
	LoadBool(unread, "unread", node);

	quest.clear();
	for (auto n = node->first_node("quest"); n != NULL; n = n->next_sibling("quest")) {
		Quest q;
		q.LoadState(n);
		quest.push_back(q);
		menu.Add();
	}
}

//------------------------------------------------------------------------
// Purpose: Reposition UI elements
//------------------------------------------------------------------------
void QuestMenu::SetUI() {
	menu.SetUI();
	text.SetUI();
}