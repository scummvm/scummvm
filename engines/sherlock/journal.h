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

class SherlockEngine;

class Journal {
protected:
	SherlockEngine *_vm;

	Journal(SherlockEngine *vm);
public:
	static Journal *init(SherlockEngine *vm);
	virtual ~Journal() {}

	/**
	 * Records statements that are said, in the order which they are said. The player
	 * can then read the journal to review them
	 */
	virtual void record(int converseNum, int statementNum, bool replyOnly = false) {}

	/**
	 * Reset viewing position to the start of the journal
	 */
	virtual void resetPosition() {}

	/**
	 * Synchronize the data for a savegame
	 */
	virtual void synchronize(Serializer &s) = 0;
};

} // End of namespace Sherlock

#endif
