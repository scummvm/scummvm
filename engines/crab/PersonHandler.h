/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

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
