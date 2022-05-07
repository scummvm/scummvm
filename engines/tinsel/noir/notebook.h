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

#ifndef TINSEL_NOTEBOOK_H // prevent multiple includes
#define TINSEL_NOTEBOOK_H

#include "common/scummsys.h"

#include "tinsel/events.h"

namespace Tinsel {
// links two clue/title objects together
struct HYPERLINK {
	int32 id1;
	int32 id2;
};

// 6 bytes large
struct ENTRY {
	int32 id;
	bool active;
	int32 page1;
	int32 indexOnPage1;
	int32 page2;
	int32 indexOnPage2;
};

enum class BOOKSTATE {
	CLOSED = 0,
	OPEN_UNKNOWN = 1,
	OPEN_ANIMATING = 2,
	OPENED = 3
};

class Notebook {
public:
	Notebook() = default;

	// Can be a title or clue
	void AddEntry(int32 entryIdx, int32 page1, int32 page2);
	// Adds a connection between a clue/title
	void AddHyperlink(int32 id1, int32 id2);
	// Called within InventoryProcess loop
	void Redraw();
	// Called by EventToInventory
	void EventToNotebook(PLR_EVENT event, bool p2, bool p3);
	// Called from OPENNOTEBOOK
	void Show(bool isOpen);
	bool IsOpen() const;
private:
	const static uint32 MAX_ENTRIES = 100;
	const static uint32 MAX_PAGES = 0x15;
	const static uint32 MAX_HYPERS = 0xf;
	const static uint32 MAX_ENTRIES_PER_PAGE = 8;

	HYPERLINK _hyperlinks[MAX_HYPERS];

	const static uint32 _numEntries = 0;

	ENTRY _entries[MAX_ENTRIES];

	BOOKSTATE _state;
};

} // End of namespace Tinsel

#endif
