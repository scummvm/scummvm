/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SHERLOCK_SCALPEL_JOURNAL_H
#define SHERLOCK_SCALPEL_JOURNAL_H

#include "sherlock/journal.h"
#include "sherlock/saveload.h"
#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "common/stream.h"

namespace Sherlock {

namespace Scalpel {

#define JOURNAL_MAX_WIDTH 230
#define JOURNAL_MAX_CHARS 80

enum JournalButton {
	BTN_NONE, BTN_EXIT, BTN_BACK10, BTN_UP, BTN_DOWN, BTN_AHEAD110, BTN_SEARCH,
	BTN_FIRST_PAGE, BTN_LAST_PAGE, BTN_PRINT_TEXT
};

class ScalpelJournal: public Journal {
private:
	/**
	 * Load the list of journal locations
	 */
	void loadLocations();

	/**
	 * Display the arrows that can be used to scroll up and down pages
	 */
	void doArrows();

	/**
	 * Show the search submenu and allow the player to enter a search string
	 */
	int getSearchString(bool printError);

	/**
	 * Returns the button, if any, that is under the specified position
	 */
	JournalButton getHighlightedButton(const Common::Point &pt);
public:
	ScalpelJournal(SherlockEngine *vm);
	virtual ~ScalpelJournal() {}

	/**
	 * Display the journal
	 */
	void drawInterface();

	/**
	 * Handle events whilst the journal is being displayed
	 */
	bool handleEvents(int key);
public:
	/**
	 * Draw the journal background, frame, and interface buttons
	 */
	virtual void drawFrame();

	/**
	 * Records statements that are said, in the order which they are said. The player
	 * can then read the journal to review them
	 */
	virtual void record(int converseNum, int statementNum, bool replyOnly = false);

	/**
	 * Reset viewing position to the start of the journal
	 */
	virtual void resetPosition();

	/**
	 * Synchronize the data for a savegame
	 */
	virtual void synchronize(Serializer &s);
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
