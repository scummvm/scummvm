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

#include "notebook_page.h"
#include "tinsel/anim.h"
#include "tinsel/events.h"
#include "tinsel/object.h"

namespace Tinsel {
// links two clue/title objects together
struct HYPERLINK {
	int32 id1 = 0;
	int32 id2 = 0;
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
	OPENED = 3,
	PAGEFLIP = 4
};

class NoteBookPolygons;
class InventoryObjectT3;

class Notebook {
public:
	Notebook();
	~Notebook();

	// Adds a connection between a clue/title
	void addHyperlink(int32 id1, int32 id2);
	void addClue(int id);
	void crossClue(int id);
	// Called within InventoryProcess loop
	void redraw();

	// Called from OPENNOTEBOOK
	void show(bool isOpen);
	bool isOpen() const;
	void close();
	
	bool handlePointer(const Common::Point &point);
	bool handleEvent(PLR_EVENT pEvent, const Common::Point &coOrds);
	void stepAnimScripts();
	void refresh();

	NoteBookPolygons *_polygons = nullptr;
private:
	int addTitle(const InventoryObjectT3 &invObject);
	void addClue(const InventoryObjectT3 &invObject);
	int getPageWithTitle(int id);

	void pageFlip(bool up);

	int32 GetPointedClue(const Common::Point &point) const;

	void clearNotebookPage();

	void setNextPage(int pageIndex);

	const static uint32 MAX_PAGES = 0x15;
	const static uint32 MAX_HYPERS = 0xf;

	HYPERLINK _hyperlinks[MAX_HYPERS];

	uint32 _numPages = 1;
	int32 _prevPage = -1;
	uint32 _currentPage = 1;

	NotebookPage _pages[MAX_PAGES] = {};

	ANIM _anim = {};
	OBJECT *_object = nullptr;

	ANIM _pageAnim = {};
	OBJECT *_pageObject = nullptr;

	BOOKSTATE _state = BOOKSTATE::CLOSED;
};

} // End of namespace Tinsel

#endif
