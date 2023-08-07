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

#ifndef CRAB_QUEST_H
#define CRAB_QUEST_H

#include "common/array.h"
#include "crab/loaders.h"

namespace Crab {

namespace pyrodactyl {
namespace event {
struct Quest {
	// The title of the quest
	Common::String _title;

	// The objective text, the last one being the current objective
	Common::Array<Common::String> _text;

	// Keep track of which quests the player has read
	bool _unread;

	// Does this quest have an associated destination marker
	bool _marker;

	Quest() {
		_unread = true;
		_marker = false;
	}

	Quest(const Common::String &title, const Common::String &text, const bool &unread, const bool &marker);

	void loadState(rapidxml::xml_node<char> *node);
	void saveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root);
};
} // End of namespace event
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_QUEST_H
