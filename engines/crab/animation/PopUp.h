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

#ifndef CRAB_POPUP_H
#define CRAB_POPUP_H

#include "crab/event/GameEventInfo.h"
#include "crab/ui/ParagraphData.h"
#include "crab/text/TextManager.h"
#include "crab/common_header.h"
#include "crab/event/effect.h"
#include "crab/event/triggerset.h"
#include "crab/timer.h"

namespace Crab {

namespace pyrodactyl {
namespace anim {
struct PopUp {
	// The total time the popup stays on the screen
	Timer _duration;

	// The time we wait before showing the trigger for the first time
	Timer _delay;

	// Should we draw this or not? (Decided by internal events)
	bool _show;

	// Popups with "talk key pressed" condition need to be shown once the key is pressed
	bool _startedShow;

	// Triggers for when you only want to display this in certain conditions
	pyrodactyl::event::TriggerSet _visible;

	// Effects for changing variables and other related stuff
	Common::Array<pyrodactyl::event::Effect> _effect;

	// The text displayed
	Common::String _text;

	// The next popup we should go to, negative values means the end
	int _next;

	PopUp() {
		_next = -1;
		reset();
	}

	PopUp(rapidxml::xml_node<char> *node) : PopUp() {
		load(node);
	}

	void reset() {
		_show = false;
		_startedShow = false;
		_delay.Stop();
		_duration.Stop();
	}

	void load(rapidxml::xml_node<char> *node);
	void draw(const int &x, const int &y, pyrodactyl::ui::ParagraphData &pop, const Rect &camera);

	// return true if we should proceed to next event, false otherwise
	bool internalEvents(pyrodactyl::event::Info &info, const Common::String &playerId,
						Common::Array<pyrodactyl::event::EventResult> &result, Common::Array<pyrodactyl::event::EventSeqInfo> &endSeq);
};

struct PopUpCollection {
	// Collection of environmental dialog
	Common::Array<PopUp> _element;

	// The current dialog being played
	int _cur;

	// true if dialog needs to loop, false otherwise
	bool _loop;

	PopUpCollection() {
		_cur = 0;
		_loop = true;
	}

	// Return true if any of the popup dialog is visible, false otherwise
	bool show() {
		for (auto &i : _element)
			if (i._show)
				return true;

		return false;
	}

	void load(rapidxml::xml_node<char> *node);

	void internalEvents(pyrodactyl::event::Info &info, const Common::String &playerId,
						Common::Array<pyrodactyl::event::EventResult> &result, Common::Array<pyrodactyl::event::EventSeqInfo> &endSeq);

	void draw(const int &x, const int &y, pyrodactyl::ui::ParagraphData &pop, const Rect &camera);
};
} // End of namespace anim
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_POPUP_H
