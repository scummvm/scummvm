#include "PopUp.h"
#include "stdafx.h"

using namespace pyrodactyl::anim;
using namespace pyrodactyl::event;

//------------------------------------------------------------------------
// Purpose: Load from xml
//------------------------------------------------------------------------
void PopUp::Load(rapidxml::xml_node<char> *node) {
	duration.Load(node, "duration", false);
	delay.Load(node, "delay");
	LoadStr(text, "text", node);
	LoadNum(next, "next", node);

	bool end = false;
	LoadBool(end, "end", node, false);
	if (end)
		next = -1;

	visible.Load(node);

	effect.clear();
	for (rapidxml::xml_node<char> *n = node->first_node("effect"); n != NULL; n = n->next_sibling("effect")) {
		Effect e;
		e.Load(n);
		effect.push_back(e);
	}
}

void PopUpCollection::Load(rapidxml::xml_node<char> *node) {
	LoadBool(loop, "loop", node);
	for (auto n = node->first_node("dialog"); n != NULL; n = n->next_sibling("dialog"))
		element.push_back(n);
}

//------------------------------------------------------------------------
// Purpose: Internal events
//------------------------------------------------------------------------
bool PopUp::InternalEvents(pyrodactyl::event::Info &info, const std::string &player_id,
						   std::vector<EventResult> &result, std::vector<EventSeqInfo> &end_seq) {
	if (visible.Evaluate(info) || started_show) {
		if (delay.TargetReached()) {
			if (duration.TargetReached(gScreenSettings.text_speed)) {
				show = false;

				for (auto &i : effect)
					i.Execute(info, player_id, result, end_seq);

				return true;
			} else {
				started_show = true;
				show = true;
			}
		} else
			show = false;
	} else
		show = false;

	return false;
}

void PopUpCollection::InternalEvents(pyrodactyl::event::Info &info, const std::string &player_id,
									 std::vector<EventResult> &result, std::vector<EventSeqInfo> &end_seq) {
	if (cur >= 0 && cur < element.size()) {
		if (element.at(cur).InternalEvents(info, player_id, result, end_seq)) {
			if (element.at(cur).next <= 0 || element.at(cur).next >= element.size()) {
				// This means that this popup is the "end" node, we must loop back to start or end this
				if (loop) {
					cur = 0;
					element.at(cur).Reset();
				} else
					cur = -1;
			} else {
				cur = element.at(cur).next;
				element.at(cur).Reset();
			}
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Draw functions
//------------------------------------------------------------------------
void PopUp::Draw(const int &x, const int &y, pyrodactyl::ui::ParagraphData &pop, const Rect &camera) {
	if (show) {
		if (x + pop.x < camera.w / 3)
			pyrodactyl::text::gTextManager.Draw(x + pop.x, y + pop.y, text, pop.col, pop.font, ALIGN_LEFT, pop.line.x, pop.line.y, true);
		else if (x + pop.x > (2 * camera.w) / 3)
			pyrodactyl::text::gTextManager.Draw(x + pop.x, y + pop.y, text, pop.col, pop.font, ALIGN_RIGHT, pop.line.x, pop.line.y, true);
		else
			pyrodactyl::text::gTextManager.Draw(x + pop.x, y + pop.y, text, pop.col, pop.font, ALIGN_CENTER, pop.line.x, pop.line.y, true);
	}
}

void PopUpCollection::Draw(const int &x, const int &y, pyrodactyl::ui::ParagraphData &pop, const Rect &camera) {
	if (cur >= 0 && cur < element.size())
		element.at(cur).Draw(x, y, pop, camera);
}