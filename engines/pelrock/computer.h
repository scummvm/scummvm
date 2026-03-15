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

#ifndef PELROCK_COMPUTER_H
#define PELROCK_COMPUTER_H

#include "common/array.h"
#include "common/str.h"
#include "graphics/managed_surface.h"

#include "pelrock/events.h"

namespace Pelrock {

// Book data location in ALFRED.7
static const uint32 kBookDataOffset = 0x309E0;
static const uint32 kBookDataEnd = 0x33F05;

// Field sizes
static const int kBookTitleSize = 55;
static const int kBookAuthorSize = 30;
static const int kBookGenreSize = 20;

struct LibraryBook {
	Common::StringArray title;  // 55 bytes for title
	Common::StringArray author; // 30 bytes for author
	Common::String genre;       // 20 bytes for genre
	byte inventoryIndex;
	byte shelf;     // 1-3 for row number
	bool available; // true = can be found on shelf, false = catalog only
};

class Computer {
public:
	Computer(PelrockEventManager *eventMan);
	~Computer();

	/**
	 * @return Book index if a book was memorized, -1 otherwise
	 */
	int run();
	Common::String _memorizedMsg; // "Bueno... Tendre que buscar en la estanteria de la %c"

private:
	enum ComputerState {
		STATE_MAIN_MENU,
		STATE_SEARCH_BY_TITLE,
		STATE_SEARCH_BY_AUTHOR,
		STATE_SHOW_RESULTS,
		STATE_EXIT
	};

	PelrockEventManager *_events = nullptr;
	Graphics::ManagedSurface _backgroundScreen;
	byte *_palette = nullptr;

	// State variables
	ComputerState _state;
	char _searchLetter = ' ';
	int _searchType = 0; // 0 = title, 1 = author
	Common::Array<int> _searchResults;
	Common::Array<LibraryBook> _libraryBooks;
	uint _currentResult;
	int _memorizedBookIndex; // Index of book that was memorized (-1 if none)
	int _lineHeight;
	Common::String _titleMsg;
	Common::String _authorMsg;

	Common::Array<Common::StringArray> _computerText;

	void loadBackground();
	void cleanup();
	void handleMainMenu();
	void handleSearchInput();
	void handleResultsDisplay();
	void performSearch();
	void drawScreen();
	void memorizeBook(int bookIndex);
	void init();
};

} // End of namespace Pelrock

#endif
