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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef SHERLOCK_JOURNAL_H
#define SHERLOCK_JOURNAL_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/str-array.h"
#include "common/stream.h"

namespace Sherlock {

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
	int _count;
	int _maxPage;
	int _index;
	int _sub;
	bool _up, _down;
	int _page;
	int _converseNum;
	Common::String _find;

	void loadJournalLocations();

	bool loadJournalFile(bool alreadyLoaded);

	void doArrows();

	bool doJournal(int direction, int howFar);

	void clearPage();
public:
public:
	Journal(SherlockEngine *vm);

	void record(int converseNum, int statementNum);

	void drawInterface();
};

} // End of namespace Sherlock

#endif
