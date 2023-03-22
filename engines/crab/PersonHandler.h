#pragma once

#include "GameEventInfo.h"
#include "ProgressBar.h"
#include "common_header.h"
#include "dialogbox.h"
#include "gameevent.h"
#include "person.h"
#include "sprite.h"

namespace pyrodactyl {
namespace ui {
class PersonHandler {
	// The positions of various elements
	// img = player image position
	Element img;

	// How the individual sprite clips are drawn
	struct ImageAnchor {
		Align x, y;

		ImageAnchor() {
			x = ALIGN_CENTER;
			y = ALIGN_RIGHT;
		}

		void Load(rapidxml::xml_node<char> *node, const bool &echo = true) {
			LoadAlign(x, node, echo, "align_x");
			LoadAlign(y, node, echo, "align_y");
		}
	} sprite_align;

	// For drawing the name
	TextData name;

	// The dialog box used to draw dialog
	pyrodactyl::ui::GameDialogBox dlbox;

	// The three opinion bars
	pyrodactyl::ui::ProgressBar opinion[pyrodactyl::people::OPI_TOTAL];

	// The button for selecting the journal
	Button jb;

	// The person id of the changed opinion, we use this to reset bar
	std::string prev;

public:
	// Used by other objects to see if journal needs to be displayed or not
	bool show_journal;

	PersonHandler() { show_journal = false; }
	~PersonHandler() {}

	void Reset(const std::string &id);

	void Load(rapidxml::xml_node<char> *node);

	// Handle events for the three opinion bars and journal - used in both dialog box and reply events
	bool HandleCommonEvents(const SDL_Event &Event);

	// Handle events for the dialog box
	bool HandleDlboxEvents(const SDL_Event &Event);

	void InternalEvents(const pyrodactyl::people::PersonState &state, pyrodactyl::anim::Sprite *s);

	void Draw(pyrodactyl::event::Info &info, pyrodactyl::event::GameEvent *Event, const std::string &person_id,
			  const bool &player, pyrodactyl::anim::Sprite *s = nullptr);

	void OpinionChange(pyrodactyl::event::Info &info, const std::string &id,
					   const pyrodactyl::people::OpinionType &type, const int &val);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl