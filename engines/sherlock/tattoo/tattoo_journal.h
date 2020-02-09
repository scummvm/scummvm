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

#ifndef SHERLOCK_TATTOO_JOURNAL_H
#define SHERLOCK_TATTOO_JOURNAL_H

#include "sherlock/journal.h"
#include "sherlock/image_file.h"

namespace Sherlock {

namespace Tattoo {

enum JournalHighlight {
	JH_NONE = -1, JH_CLOSE = 0, JH_SEARCH = 1, JH_SAVE = 2,
	JH_SCROLL_LEFT = 3, JH_PAGE_LEFT = 4, JH_PAGE_RIGHT = 5, JH_SCROLL_RIGHT = 6, JH_THUMBNAIL = 7
};

class TattooJournal : public Journal {
private:
	ImageFile *_journalImages;
	int _selector, _oldSelector;
	bool _wait;
	bool _exitJournal;
	uint32 _scrollingTimer;
	int _savedIndex, _savedSub, _savedPage;

	/**
	 * Load the list of journal locations
	 */
	void loadLocations();

	/**
	 * Displays the controls used by the journal
	 * @param mode	0: Normal journal buttons, 1: Search interface
	 */
	void drawControls(int mode);

	/**
	 * Draw the journal controls used by the journal
	 */
	void highlightJournalControls(bool slamIt);

	/**
	 * Draw the journal controls used in search mode
	 */
	void highlightSearchControls(bool slamIt);

	void drawScrollBar();

	/**
	 * Check for and handle any pending keyboard events
	 */
	void handleKeyboardEvents();

	/**
	 * Handle mouse presses on interface buttons
	 */
	void handleButtons();

	/**
	 * Disable the journal controls
	 */
	void disableControls();

	/**
	 * Get in a name to search through the journal for
	 */
	int getFindName(bool printError);

	/**
	 * Save the journal to file
	 */
	void saveJournal();

	/**
	 * Show a message that the journal has been saved to file
	 */
	void showSavedDialog();
public:
	TattooJournal(SherlockEngine *vm);
	~TattooJournal() override {}

	/**
	 * Show the journal
	 */
	void show();
public:
	/**
	 * Draw the journal background, frame, and interface buttons
	 */
	void drawFrame() override;

	/**
	 * Records statements that are said, in the order which they are said. The player
	 * can then read the journal to review them
	 */
	void record(int converseNum, int statementNum, bool replyOnly = false) override;
};

} // End of namespace Tattoo

} // End of namespace Sherlock

#endif
