#include "stdafx.h"

#include "OptionSelect.h"

using namespace pyrodactyl::text;
using namespace pyrodactyl::ui;

void OptionSelect::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		option.data.Load(node);
		prev.Load(node->first_node("prev"));
		next.Load(node->first_node("next"));
		cur = 0;

		option.text.clear();
		for (auto n = node->first_node("option"); n != NULL; n = n->next_sibling("option")) {
			std::string s;
			LoadStr(s, "name", n);
			option.text.push_back(s);
		}

		LoadBool(usekeyboard, "keyboard", node, false);
	}
}

void OptionSelect::Draw() {
	option.Draw(cur);

	if (cur > 0)
		prev.Draw();

	if (cur < option.text.size() - 1)
		next.Draw();
}

bool OptionSelect::HandleEvents(const SDL_Event &Event) {
	using namespace pyrodactyl::input;

	if (cur > 0) {
		if (prev.HandleEvents(Event) == BUAC_LCLICK || (usekeyboard && gInput.Equals(IU_LEFT, Event) == SDL_PRESSED)) {
			cur--;
			return true;
		}
	}

	if (cur < option.text.size() - 1) {
		if (next.HandleEvents(Event) == BUAC_LCLICK || (usekeyboard && gInput.Equals(IU_RIGHT, Event) == SDL_PRESSED)) {
			cur++;
			return true;
		}
	}

	return false;
}

void OptionSelect::SetUI() {
	option.data.SetUI();
	prev.SetUI();
	next.SetUI();
}