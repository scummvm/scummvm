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

#define LINES_PER_PAGE (IS_SERRATED_SCALPEL ? 11 : 17)
#define JOURNAL_MAX_WIDTH (IS_SERRATED_SCALPEL ? 230 : 422)
#define JOURNAL_MAX_CHARS 80
#define JOURNAL_LEFT_X (IS_SERRATED_SCALPEL ? 53 : 156)

class SherlockEngine;

struct JournalEntry {
	int _converseNum;
	bool _replyOnly;
	int _statementNum;

	JournalEntry() : _converseNum(0), _replyOnly(false), _statementNum(0) {}
	JournalEntry(int converseNum, int statementNum, bool replyOnly = false) :
		_converseNum(converseNum), _statementNum(statementNum), _replyOnly(replyOnly) {}
};

class Journal {
protected:
	SherlockEngine *_vm;
	Common::StringArray _directory;
	Common::StringArray _locations;
	Common::Array<JournalEntry> _journal;
	Common::StringArray _lines;
	bool _up, _down;
	int _index;
	int _page;
	int _maxPage;
	int _sub;
	Common::String _find;

	Journal(SherlockEngine *vm);

	/**
	 * Loads the description for the current display index in the journal, and then
	 * word wraps the result to prepare it for being displayed
	 * @param alreadyLoaded		Indicates whether the journal file is being loaded for the
	 *		first time, or being reloaded
	 */
	void loadJournalFile(bool alreadyLoaded);

	/**
	 * Returns true if a given character is printable
	 */
	bool isPrintable(byte ch) const;
public:
	static Journal *init(SherlockEngine *vm);
	virtual ~Journal() {}

	/**
	* Displays a page of the journal at the current index
	*/
	bool drawJournal(int direction, int howFar);

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s);
public:
	/**
	 * Draw the journal background, frame, and interface buttons
	 */
	virtual void drawFrame() = 0;

	/**
	 * Reset viewing position to the start of the journal
	 */
	virtual void resetPosition() {}

	/**
	 * Records statements that are said, in the order which they are said. The player
	 * can then read the journal to review them
	 */
	virtual void record(int converseNum, int statementNum, bool replyOnly = false);
};

} // End of namespace Sherlock

#endif
