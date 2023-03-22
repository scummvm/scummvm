#pragma once

#include "PageMenu.h"
#include "QuestText.h"
#include "StateButton.h"
#include "common_header.h"
#include "quest.h"

namespace pyrodactyl {
namespace ui {
class QuestMenu {
	// The collection of quest pages
	PageButtonMenu menu;

	// The currently selected quest for reading
	int sel_quest;

	// The currently selected page
	int sel_page;

	// The currently selected button in the page menu
	int sel_bu;

	// For drawing quest text
	QuestText text;

	// For drawing quest tabs
	FontKey font;
	Align align;
	int col_n, col_s;
	ButtonImage img_n, img_s;
	Vector2i off_title, off_unread;

public:
	// All the quests currently in this menu
	std::deque<pyrodactyl::event::Quest> quest;

	// Keep track of unread notifications for each category button
	bool unread;

	QuestMenu();
	~QuestMenu() {}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(Button &bu_map);
	bool HandleEvents(Button &bu_map, std::string &map_title, const SDL_Event &Event);

	void UseKeyboard(const bool &val) { menu.UseKeyboard(val); }
	void AssignPaths() { menu.AssignPaths(); }

	void Marker(const std::string &title, const bool &val);

	void Add(const std::string &title, const std::string &text);
	void Add(const pyrodactyl::event::Quest &q);
	void Erase(const int &index);

	void Select(const int &quest_index);

	void SaveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root, const char *name);
	void LoadState(rapidxml::xml_node<char> *node);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl