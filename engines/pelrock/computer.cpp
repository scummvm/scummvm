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

#include "common/events.h"
#include "common/system.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "pelrock/computer.h"
#include "pelrock/pelrock.h"

namespace Pelrock {

Computer::Computer(PelrockEventManager *eventMan)
	: _palette(nullptr),
	  _state(STATE_MAIN_MENU),
	  _searchLetter(0),
	  _searchType(0),
	  _currentResult(0),
	  _memorizedBookIndex(-1),
	  _events(eventMan) {
	init();
}

Common::StringArray split(const Common::String &str) {
	Common::StringArray result;
	Common::String token;

	for (uint i = 0; i < str.size(); i++) {
		if ((unsigned char)str[i] == 0xC8) {
			result.push_back(token);
			token.clear();
		} else {
			token += str[i];
		}
	}
	result.push_back(token); // last token
	return result;
}

void Computer::init() {
	Common::File alfred7File;
	if (!alfred7File.open("ALFRED.7")) {
		error("Could not open ALFRED.7");
		return;
	}

	alfred7File.seek(kBookDataOffset, SEEK_SET);
	while (alfred7File.pos() < kBookDataEnd) {
		LibraryBook book;
		Common::String title = alfred7File.readString(0, kBookTitleSize);
		title.trim();
		book.title = split(title);
		Common::String author = alfred7File.readString(0, kBookAuthorSize);
		author.trim();
		book.author = split(author);
		book.genre = alfred7File.readString(0, kBookGenreSize);
		book.genre.trim();
		book.inventoryIndex = alfred7File.readByte() - 55;
		book.shelf = alfred7File.readByte();
		book.available = alfred7File.readByte() == 2;
		_libraryBooks.push_back(book);
	}

	_computerText = g_engine->_res->loadComputerText();

	_searchResults.clear();
	_currentResult = 0;
	_searchLetter = 0;
	_memorizedBookIndex = -1;
	_titleMsg = _computerText[10][0].substr(0, 7);
	_authorMsg = _computerText[10][0].substr(7, 6);
	_memorizedMsg = _computerText[10][0].substr(16, _computerText[10][0].size() - 16); // "Bueno... Tendre que buscar en la estanteria de la %c"
	_lineHeight = g_engine->_smallFont->getFontHeight();
}

Computer::~Computer() {
	cleanup();
}

void Computer::loadBackground() {
	_palette = new byte[768];
	_backgroundScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());

	g_engine->_res->getExtraScreen(1, (byte *)_backgroundScreen.getPixels(), _palette);
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void Computer::cleanup() {
	_backgroundScreen.free();
	if (_palette) {
		// Restore room palette
		g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
		delete[] _palette;
		_palette = nullptr;
	}
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
	CursorMan.showMouse(true);
}

int Computer::run() {
	loadBackground();
	_state = STATE_MAIN_MENU;
	CursorMan.showMouse(false);
	g_engine->changeCursor(DEFAULT);

	while (!g_engine->shouldQuit() && _state != STATE_EXIT) {
		_events->pollEvent();
		drawScreen();

		switch (_state) {
		case STATE_MAIN_MENU:
			handleMainMenu();
			break;

		case STATE_SEARCH_BY_TITLE:
		case STATE_SEARCH_BY_AUTHOR:
			handleSearchInput();
			break;

		case STATE_SHOW_RESULTS:
			handleResultsDisplay();
			break;

		case STATE_EXIT:
			break;
		}

		g_engine->_screen->markAllDirty();
		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
	cleanup();
	return _memorizedBookIndex;
}

void Computer::drawScreen() {
	// Clear to background
	g_engine->_screen->blitFrom(_backgroundScreen);

	byte defaultColor = 0; // Light gray

	switch (_state) {
	case STATE_MAIN_MENU: {
		int textY = 97;
		int textX = 225;
		for (int i = 0; _computerText[0].size() > i; i++) {
			Common::String line = _computerText[0][i];
			g_engine->_graphics->drawColoredText(g_engine->_screen, line, textX, textY + i * _lineHeight, 200, defaultColor, g_engine->_smallFont);
		}
		break;
	}

	case STATE_SEARCH_BY_TITLE:
	case STATE_SEARCH_BY_AUTHOR:
		for (int i = 0; _computerText[1].size() > i; i++) {
			Common::String line = _computerText[1][i];
			g_engine->_graphics->drawColoredText(g_engine->_screen, line, 172, 258 + i * _lineHeight, 200, defaultColor, g_engine->_smallFont);
		}
		break;

	case STATE_SHOW_RESULTS: {

		const char *section = _searchType == 0 ? _titleMsg.c_str() : _authorMsg.c_str();
		Common::String title = _computerText[2][0];
		int replacementIndex = title.findFirstOf("XXXXXXX");

		title.replace(replacementIndex, 7, section);
		int replacementIndex2 = title.findFirstOf("X");
		title.replace(replacementIndex2, 1, Common::String(1, _searchLetter));
		g_engine->_graphics->drawColoredText(g_engine->_screen, title, 210, 97, 200, defaultColor, g_engine->_smallFont);

		int textX = 161;
		int textY = 131;
		int increment = 28;

		// Display current book
		int bookIdx = _searchResults[_currentResult];
		const LibraryBook &book = _libraryBooks[bookIdx];

		// Title (may be long, truncate if needed)
		Common::String titleLine = _computerText[3][0];
		int titlePlaceholderIndex = titleLine.findFirstOf("XXXX");

		int titleIndex = 0;
		while (titleIndex < book.title.size()) {
			Common::String thisLine;
			if (titleIndex == 0) {
				thisLine = titleLine;
				thisLine.replace(titlePlaceholderIndex, titleLine.size() - titlePlaceholderIndex, book.title[titleIndex]);
			} else {
				thisLine = book.title[titleIndex];
			}
			g_engine->_graphics->drawColoredText(g_engine->_screen, thisLine, textX, textY + _lineHeight * titleIndex, 340, defaultColor, g_engine->_smallFont);
			titleIndex++;
		}

		// Author
		Common::String authorLine = _computerText[4][0];
		int authorPlaceholderIndex = authorLine.findFirstOf("XXXX");
		int authorIndex = 0;

		while (authorIndex < book.author.size()) {
			Common::String thisLine;
			if (authorIndex == 0) {
				thisLine = authorLine;
				thisLine.replace(authorPlaceholderIndex, authorLine.size() - authorPlaceholderIndex, book.author[authorIndex]);
			} else {
				thisLine = book.author[authorIndex];
			}
			g_engine->_graphics->drawColoredText(g_engine->_screen, thisLine, textX, textY + increment + _lineHeight * authorIndex, 340, defaultColor, g_engine->_smallFont);
			authorIndex++;
		}

		// Genre
		Common::String genreLine = _computerText[5][0];
		int genrePlaceholderIndex = genreLine.findFirstOf("XXXX");
		genreLine.replace(genrePlaceholderIndex, genreLine.size() - genrePlaceholderIndex, book.genre);
		g_engine->_graphics->drawColoredText(g_engine->_screen, genreLine, textX, textY + increment * 2, 340, defaultColor, g_engine->_smallFont);

		// "Situacion" (location/availability)
		Common::String situacionLine = _computerText[6][0];
		int situacionPlaceholderIndex = situacionLine.findFirstOf("XXXX");
		situacionLine.replace(situacionPlaceholderIndex, situacionLine.size() - situacionPlaceholderIndex, book.available ? _computerText[8][0] : _computerText[9][0]);
		g_engine->_graphics->drawColoredText(g_engine->_screen, situacionLine, textX, textY + increment * 3, 340, defaultColor, g_engine->_smallFont);

		// Show navigation options
		Common::String navOptions;
		Common::String actions = _computerText[7][0];
		if (!book.available) {
			actions.setChar(180, 1);
			actions.setChar(180, 4);
		}
		g_engine->_graphics->drawColoredText(g_engine->_screen, actions, 174, 258, 325, defaultColor, g_engine->_smallFont);

	} break;

	case STATE_EXIT:
		break;
	}
}

void Computer::handleMainMenu() {
	if (_events->_lastKeyEvent == Common::KEYCODE_1) {
		_searchType = 0;
		_state = STATE_SEARCH_BY_TITLE;
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	} else if (_events->_lastKeyEvent == Common::KEYCODE_2) {
		_searchType = 1;
		_state = STATE_SEARCH_BY_AUTHOR;
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	} else if (_events->_lastKeyEvent == Common::KEYCODE_3) {
		_state = STATE_EXIT;
	}
}

void Computer::handleSearchInput() {
	if (_events->_lastKeyEvent >= Common::KEYCODE_a &&
		_events->_lastKeyEvent <= Common::KEYCODE_z) {
		_searchLetter = 'A' + (_events->_lastKeyEvent - Common::KEYCODE_a);
		performSearch();
		if (!_searchResults.empty()) {
			_currentResult = 0;
			_state = STATE_SHOW_RESULTS;
		} else {
			// No results, return to main menu
			_state = STATE_MAIN_MENU;
		}
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	} else if (_events->_lastKeyEvent == Common::KEYCODE_ESCAPE) {
		_state = STATE_MAIN_MENU;
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	}
}

void Computer::handleResultsDisplay() {
	if (_events->_lastKeyEvent == Common::KEYCODE_s) {
		if (!_searchResults.empty()) {
			_currentResult = (_currentResult + 1);
			if (_currentResult >= _searchResults.size())
				_state = STATE_MAIN_MENU;
		}
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	} else if (_events->_lastKeyEvent == Common::KEYCODE_m) {
		if (!_searchResults.empty()) {
			int bookIdx = _searchResults[_currentResult];
			const LibraryBook &book = _libraryBooks[bookIdx];
			if (book.available) {
				memorizeBook(bookIdx);
			}
		}
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	}
	// C key or ESC - Cancel (return to main menu)
	else if (_events->_lastKeyEvent == Common::KEYCODE_c ||
			 _events->_lastKeyEvent == Common::KEYCODE_ESCAPE) {
		_state = STATE_MAIN_MENU;
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
		g_engine->_state->libraryShelf = -1; // 0-based shelf index
		g_engine->_state->selectedBookIndex = -1;
		g_engine->_state->bookLetter = '\0';
	}
}

void Computer::memorizeBook(int bookIndex) {
	const LibraryBook &book = _libraryBooks[bookIndex];

	// Store the memorized book for later pickup from the shelf
	_memorizedBookIndex = bookIndex;

	_state = STATE_EXIT;

	g_engine->_state->libraryShelf = book.shelf; // 0-based shelf index
	g_engine->_state->selectedBookIndex = book.inventoryIndex;
	g_engine->_state->bookLetter = book.title[0][0];

}

void Computer::performSearch() {
	_searchResults.clear();

	for (int i = 0; i < _libraryBooks.size(); i++) {
		Common::String searchField = _searchType == 0 ? _libraryBooks[i].title[0] : _libraryBooks[i].author[0];

		// Check if first letter matches (case-insensitive)
		char firstChar = searchField[0];
		if (firstChar >= 'a' && firstChar <= 'z')
			firstChar = firstChar - 'a' + 'A';

		if (firstChar == _searchLetter) {
			_searchResults.push_back(i);
		}
	}
}

} // End of namespace Pelrock
