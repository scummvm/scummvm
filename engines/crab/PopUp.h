#pragma once

#include "GameEventInfo.h"
#include "ParagraphData.h"
#include "TextManager.h"
#include "common_header.h"
#include "effect.h"
#include "triggerset.h"

namespace pyrodactyl {
namespace anim {
struct PopUp {
	// The total time the popup stays on the screen
	Timer duration;

	// The time we wait before showing the trigger for the first time
	Timer delay;

	// Should we draw this or not? (Decided by internal events)
	bool show;

	// Popups with "talk key pressed" condition need to be shown once the key is pressed
	bool started_show;

	// Triggers for when you only want to display this in certain conditions
	pyrodactyl::event::TriggerSet visible;

	// Effects for changing variables and other related stuff
	std::vector<pyrodactyl::event::Effect> effect;

	// The text displayed
	std::string text;

	// The next popup we should go to, negative values means the end
	int next;

	PopUp() {
		next = -1;
		Reset();
	}
	PopUp(rapidxml::xml_node<char> *node) : PopUp() { Load(node); }

	void Reset() {
		show = false;
		started_show = false;
		delay.Stop();
		duration.Stop();
	}

	void Load(rapidxml::xml_node<char> *node);
	void Draw(const int &x, const int &y, pyrodactyl::ui::ParagraphData &pop, const Rect &camera);

	// return true if we should proceed to next event, false otherwise
	bool InternalEvents(pyrodactyl::event::Info &info, const std::string &player_id,
						std::vector<pyrodactyl::event::EventResult> &result, std::vector<pyrodactyl::event::EventSeqInfo> &end_seq);
};

struct PopUpCollection {
	// Collection of environmental dialog
	std::vector<PopUp> element;

	// The current dialog being played
	int cur;

	// true if dialog needs to loop, false otherwise
	bool loop;

	PopUpCollection() {
		cur = 0;
		loop = true;
	}

	// Return true if any of the popup dialog is visible, false otherwise
	bool Show() {
		for (auto &i : element)
			if (i.show)
				return true;

		return false;
	}

	void Load(rapidxml::xml_node<char> *node);

	void InternalEvents(pyrodactyl::event::Info &info, const std::string &player_id,
						std::vector<pyrodactyl::event::EventResult> &result, std::vector<pyrodactyl::event::EventSeqInfo> &end_seq);

	void Draw(const int &x, const int &y, pyrodactyl::ui::ParagraphData &pop, const Rect &camera);
};
} // End of namespace anim
} // End of namespace pyrodactyl
