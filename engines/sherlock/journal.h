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

#ifndef SHERLOCK_JOURNAL_H
#define SHERLOCK_JOURNAL_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/str-array.h"
#include "common/stream.h"
#include "sherlock/saveload.h"

namespace Sherlock {

#define JOURNAL_MAX_WIDTH 230
#define JOURNAL_MAX_CHARS 80

enum JournalButton {
	BTN_NONE, BTN_EXIT, BTN_BACK10, BTN_UP, BTN_DOWN, BTN_AHEAD110, BTN_SEARCH,
	BTN_FIRST_PAGE, BTN_LAST_PAGE, BTN_PRINT_TEXT
};


struct JournalEntry {
	int _converseNum;
	bool _replyOnly;
	int _statementNum;

	JournalEntry() : _converseNum(0), _replyOnly(false), _statementNum(0) {}
	JournalEntry(int converseNum, int statementNum, bool replyOnly = false) :
		_converseNum(converseNum), _statementNum(statementNum), _replyOnly(replyOnly) {}
};

class SherlockEngine;

class Journal {
private:
	SherlockEngine *_vm;
	Common::Array<JournalEntry> _journal;
	Common::StringArray _directory;
	Common::StringArray _locations;
	Common::StringArray _lines;
	int _maxPage;
	int _index;
	int _sub;
	bool _up, _down;
	int _page;
	Common::String _find;

	/**
	 * Load the list of location names that the journal will make reference to
	 */
	void loadJournalLocations();

	/**
	 * Loads the description for the current display index in the journal, and then
	 * word wraps the result to prepare it for being displayed
	 * @param alreadyLoaded		Indicates whether the journal file is being loaded for the
	 *		first time, or being reloaded
	 */
	void loadJournalFile(bool alreadyLoaded);

	/**
	 * Display the arrows that can be used to scroll up and down pages
	 */
	void doArrows();

	/**
	 * Displays a page of the journal at the current index
	 */
	bool drawJournal(int direction, int howFar);

	/**
	 * Show the search submenu and allow the player to enter a search string
	 */
	int getSearchString(bool printError);

	/**
	 * Draw the journal background, frame, and interface buttons
	 */
	void drawJournalFrame();

	/**
	 * Returns the button, if any, that is under the specified position
	 */
	JournalButton getHighlightedButton(const Common::Point &pt);
public:
	Journal(SherlockEngine *vm);

	/**
	 * Records statements that are said, in the order which they are said. The player
	 * can then read the journal to review them
	 */
	void record(int converseNum, int statementNum, bool replyOnly = false);

	/**
	 * Display the journal
	 */
	void drawInterface();

	/**
	 * Handle events whilst the journal is being displayed
	 */
	bool handleEvents(int key);

	/**
	 * Reset viewing position to the start of the journal
	 */
	void resetPosition();

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s);
};

} // End of namespace Sherlock

#endif
