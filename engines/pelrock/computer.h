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

#include "pelrock/events.h"
#include "pelrock/library_books.h"

namespace Pelrock {

class PelrockEngine;

class Computer {
public:
	Computer(PelrockEventManager *eventMan);

	~Computer();

	/**
	 * @return Book index if a book was memorized, -1 otherwise
	 */
	int run();
	Common::String _titleMsg;
	Common::String _authorMsg;
	Common::String _memorizedMsg;      // "Bueno... Tendre que buscar en la estanteria de la %c"

private:
	enum ComputerState {
		STATE_MAIN_MENU,
		STATE_SEARCH_BY_TITLE,
		STATE_SEARCH_BY_AUTHOR,
		STATE_SHOW_RESULTS,
		STATE_EXIT
	};

	PelrockEventManager *_events;
	byte *_backgroundScreen;
	byte *_palette;

	// State variables
	ComputerState _state;
	char _searchLetter;
	int _searchType;  // 0 = title, 1 = author
	Common::Array<int> _searchResults;
	Common::Array<LibraryBook> _libraryBooks;
	int _currentResult;
	int _memorizedBookIndex;  // Index of book that was memorized (-1 if none)
	int _lineHeight;

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
