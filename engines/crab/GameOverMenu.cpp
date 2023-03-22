#include "stdafx.h"

#include "GameOverMenu.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void GameOverMenu::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		if (NodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (NodeValid("title", node)) {
			rapidxml::xml_node<char> *tinode = node->first_node("title");
			title.Load(tinode);

			for (auto n = tinode->first_node("quote"); n != NULL; n = n->next_sibling("quote")) {
				std::string str;
				LoadStr(str, "text", n);
				quote.push_back(str);
			}
		}

		menu.Load(node->first_node("menu"));
	}
}

int GameOverMenu::HandleEvents(const SDL_Event &Event) {
	return menu.HandleEvents(Event);
}

void GameOverMenu::Draw() {
	bg.Draw();
	if (cur < quote.size())
		title.Draw(quote.at(cur));

	menu.Draw();
}

void GameOverMenu::SetUI() {
	bg.SetUI();
	title.SetUI();
	menu.SetUI();
}