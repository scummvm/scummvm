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

#ifndef CRAB_JOURNAL_H
#define CRAB_JOURNAL_H

#include "crab/ui/ImageData.h"
#include "crab/image/ImageManager.h"
#include "crab/ui/StateButton.h"
#include "crab/common_header.h"
#include "crab/ui/questmenu.h"

namespace Crab {

#define JE_CUR_NAME "cur"
#define JE_DONE_NAME "done"
#define JE_PEOPLE_NAME "people"
#define JE_LOCATION_NAME "location"
#define JE_HISTORY_NAME "history"

namespace pyrodactyl {
namespace ui {
// The categories of journal entries
enum JournalCategory {
	JE_CUR,      // Quests in progress
	JE_DONE,     // Completed quests
	JE_PEOPLE,   // Info about characters
	JE_LOCATION, // Info about locations
	JE_HISTORY,  // All the other info
	JE_TOTAL     // The total number of categories
};

class Journal {
	// The background image data
	ImageData bg;

	// The menu to select the category to display
	Menu<StateButton> category;

	// The selected category
	int select;

	// A group contains the entire journal for a single character
	struct Group {
		// Id of the character who this journal belongs to
		Common::String id;

		// The set of menus containing all categories of journals
		QuestMenu menu[JE_TOTAL];
	};

	// This contains journal entries for all characters
	Common::Array<Group> journal;

	// The reference quest menu, used to copy layouts
	QuestMenu ref;

	// This button is the "go to map" button, shown if the quest has a corresponding map marker
	Button bu_map;

	void Select(const Common::String &id, const int &choice);

public:
	// The title of the quest selected by the "show in map" button
	Common::String marker_title;

	Journal() { select = 0; }
	~Journal() {}

	void load(const Common::String &filename);
	void draw(const Common::String &id);

	// Return true if "go to map" is selected
	bool HandleEvents(const Common::String &id, const Common::Event &Event);
#if 0
	bool HandleEvents(const Common::String &id, const SDL_Event &Event);
#endif

	void Add(const Common::String &id, const Common::String &Category, const Common::String &Title, const Common::String &Text);
	void Move(const Common::String &id, const Common::String &Title, const bool &completed);
	void Marker(const Common::String &id, const Common::String &Title, const bool &val);

	// Open a specific entry in the journal
	void Open(const Common::String &id, const JournalCategory &Category, const Common::String &Title);

	// Prepare a new character's journal
	void Init(const Common::String &id);

	void saveState(rapidxml::xml_document<char> &doc, rapidxml::xml_node<char> *root);
	void loadState(rapidxml::xml_node<char> *node);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_JOURNAL_H
