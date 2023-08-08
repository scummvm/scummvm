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

#ifndef CRAB_PERSONHANDLER_H
#define CRAB_PERSONHANDLER_H

#include "crab/event/gameevent.h"
#include "crab/event/GameEventInfo.h"
#include "crab/ui/ProgressBar.h"
#include "crab/ui/dialogbox.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
class Sprite;
} // End of namespace Sprite

namespace ui {
class PersonHandler {
	// The positions of various elements
	// img = player image position
	Element _img;

	// How the individual sprite clips are drawn
	struct ImageAnchor {
		Align _x, _y;

		ImageAnchor() {
			_x = ALIGN_CENTER;
			_y = ALIGN_RIGHT;
		}

		void load(rapidxml::xml_node<char> *node, const bool &echo = true) {
			loadAlign(_x, node, echo, "align_x");
			loadAlign(_y, node, echo, "align_y");
		}
	} _spriteAlign;

	// For drawing the name
	TextData _name;

	// The dialog box used to draw dialog
	pyrodactyl::ui::GameDialogBox _dlbox;

	// The three opinion bars
	pyrodactyl::ui::ProgressBar _opinion[pyrodactyl::people::OPI_TOTAL];

	// The button for selecting the journal
	Button _jb;

	// The person id of the changed opinion, we use this to reset bar
	Common::String _prev;

public:
	// Used by other objects to see if journal needs to be displayed or not
	bool _showJournal;

	PersonHandler() {
		_showJournal = false;
	}

	~PersonHandler() {}

	void reset(const Common::String &id);

	void load(rapidxml::xml_node<char> *node);

	// Handle events for the three opinion bars and journal - used in both dialog box and reply events
	bool handleCommonEvents(const Common::Event &event);

	// Handle events for the dialog box
	bool handleDlboxEvents(const Common::Event &event);

#if 0
	bool HandleCommonEvents(const SDL_Event &Event);
	bool HandleDlboxEvents(const SDL_Event &Event);
#endif

	void internalEvents(const pyrodactyl::people::PersonState &state, pyrodactyl::anim::Sprite *s);

	void draw(pyrodactyl::event::Info &info, pyrodactyl::event::GameEvent *event, const Common::String &personId,
			  const bool &player, pyrodactyl::anim::Sprite *s = nullptr);

	void opinionChange(pyrodactyl::event::Info &info, const Common::String &id,
					   const pyrodactyl::people::OpinionType &type, const int &val);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_PERSONHANDLER_H
