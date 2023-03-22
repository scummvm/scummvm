#include "SlideShow.h"
#include "stdafx.h"

namespace pyrodactyl {
namespace ui {
SlideShow gHelpScreen;
}
} // End of namespace pyrodactyl

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

void SlideShow::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid(node)) {
		if (NodeValid("pos", node))
			pos.Load(node->first_node("pos"));

		if (NodeValid("bg", node))
			bg.Load(node->first_node("bg"));

		if (NodeValid("prev", node)) {
			prev.Load(node->first_node("prev"));
			prev.hotkey.Set(IU_PREV);
		}

		if (NodeValid("next", node)) {
			next.Load(node->first_node("next"));
			next.hotkey.Set(IU_NEXT);
		}

		path.clear();
		for (auto n = node->first_node("slide"); n != NULL; n = n->next_sibling("slide")) {
			std::string p;
			LoadStr(p, "path", n);
			path.push_back(p);
		}

		index = 0;

		LoadBool(usekeyboard, "keyboard", node, false);
	}
}

void SlideShow::Draw() {
	bg.Draw();
	img.Draw(pos.x, pos.y);

	if (index > 0)
		prev.Draw();

	if (index < path.size() - 1)
		next.Draw();
}

void SlideShow::HandleEvents(const SDL_Event &Event) {
	using namespace pyrodactyl::input;

	if (index > 0)
		if (prev.HandleEvents(Event) == BUAC_LCLICK || (usekeyboard && gInput.Equals(IU_LEFT, Event) == SDL_PRESSED)) {
			index--;
			Refresh();
		}

	if (index < path.size() - 1)
		if (next.HandleEvents(Event) == BUAC_LCLICK || (usekeyboard && gInput.Equals(IU_RIGHT, Event) == SDL_PRESSED)) {
			index++;
			Refresh();
		}
}

void SlideShow::Refresh() {
	img.Delete();

	if (index >= 0 && index < path.size())
		img.Load(path.at(index));
}

void SlideShow::SetUI() {
	pos.SetUI();
	bg.SetUI();
	prev.SetUI();
	next.SetUI();
}