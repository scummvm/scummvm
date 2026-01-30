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
#include "graphics/paletteman.h"

#include "computer.h"
#include "pelrock/computer.h"
#include "pelrock/library_books.h"
#include "pelrock/pelrock.h"

namespace Pelrock {

Computer::Computer(PelrockEventManager *eventMan)
	: _backgroundScreen(nullptr),
	  _palette(nullptr),
	  _state(STATE_MAIN_MENU),
	  _searchLetter(0),
	  _searchType(0),
	  _currentResult(0),
	  _memorizedBookIndex(-1),
	  _events(eventMan) {

	// Initialize UI strings (Spanish - original game language)
	_menuTitle = "MENU PRINCIPAL";
	_menuOption1 = "1    CONSULTAR POR TITULO";
	_menuOption2 = "2    CONSULTAR POR AUTOR";
	_menuOption3 = "3    CANCELAR";
	_promptLetter = "Teclea una letra (A-Z):";
	_labelTitle = "Titulo    : ";
	_labelAuthor = "Autor     : ";
	_labelGenre = "Genero    : ";
	_labelSituacion = "Situacion : ";
	_statusPhysical = "Estante %c, fila %d";
	_statusCatalogOnly = "Solo en catalogo";
	_optMemorizar = "(M)emorizar";
	_optSeguir = "(S)eguir";
	_optCancelar = "(C)ancelar";
	_noResults = "No se encontraron libros";
	_memorizedMsg = "Bueno... Tendre que buscar en la estanteria de la %c";

	init();
}

void Computer::init() {
	Common::File alfred7File;
	if (!alfred7File.open("ALFRED.7")) {
		error("Could not open ALFRED.7");
		return;
	}

	alfred7File.seek(kBookDataOffset, SEEK_SET);
	int index = 0;
	int index2 = 0;
	while (alfred7File.pos() < kBookDataEnd) {
		LibraryBook book;

		book.title = alfred7File.readString(0, kBookTitleSize);
		book.author = alfred7File.readString(0, kBookAuthorSize);
		book.genre = alfred7File.readString(0, kBookGenreSize);
		book.title.trim();
		book.author.trim();
		book.genre.trim();
		book.inventoryIndex = alfred7File.readByte() - 55;
		book.shelf = alfred7File.readByte();
		book.available = alfred7File.readByte() == 2;
		// book.index = book.available ? index++ : -1;
		// book.index2 = index2++;
		_libraryBooks.push_back(book);
	}

	for(int i = 0; i < _libraryBooks.size(); i++) {
		const LibraryBook &book = _libraryBooks[i];
		debug("Loaded book: title='%s', author='%s', genre='%s', unknown=%d, shelf=%d, available=%d",
			  book.title.c_str(), book.author.c_str(), book.genre.c_str(),
			  book.inventoryIndex, book.shelf, book.available);
	}

	_searchResults.clear();
	_currentResult = 0;
	_searchLetter = 0;
	_memorizedBookIndex = -1;
}


Computer::~Computer() {
	cleanup();
}

void Computer::loadBackground() {
	_palette = new byte[768];
	_backgroundScreen = new byte[640 * 400];

	g_engine->_res->getExtraScreen(1, _backgroundScreen, _palette);
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void Computer::cleanup() {
	if (_backgroundScreen) {
		delete[] _backgroundScreen;
		_backgroundScreen = nullptr;
	}
	if (_palette) {
		// Restore room palette
		g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
		delete[] _palette;
		_palette = nullptr;
	}
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
}

int Computer::run() {
	loadBackground();
	_state = STATE_MAIN_MENU;

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
	memcpy(g_engine->_screen->getPixels(), _backgroundScreen, 640 * 400);

	int textY = 100;
	int textX = 180;

	switch (_state) {
	case STATE_MAIN_MENU:
		g_engine->_smallFont->drawString(g_engine->_screen, _menuTitle, textX, textY, 280, 15, Graphics::kTextAlignCenter);
		g_engine->_smallFont->drawString(g_engine->_screen, _menuOption1, textX, textY + 40, 280, 14);
		g_engine->_smallFont->drawString(g_engine->_screen, _menuOption2, textX, textY + 60, 280, 14);
		g_engine->_smallFont->drawString(g_engine->_screen, _menuOption3, textX, textY + 80, 280, 14);
		break;

	case STATE_SEARCH_BY_TITLE:
	case STATE_SEARCH_BY_AUTHOR:
		g_engine->_smallFont->drawString(g_engine->_screen,
										 _searchType == 0 ? "CONSULTAR POR TITULO" : "CONSULTAR POR AUTOR",
										 textX, textY, 280, 15, Graphics::kTextAlignCenter);
		g_engine->_smallFont->drawString(g_engine->_screen, _promptLetter, textX, textY + 40, 280, 14);
		break;

	case STATE_SHOW_RESULTS: {
		Common::String header = Common::String::format(
			"Consulta de %s, letra %c",
			_searchType == 0 ? "TITULO" : "AUTOR",
			_searchLetter);
		g_engine->_smallFont->drawString(g_engine->_screen, header, textX, textY, 280, 15, Graphics::kTextAlignCenter);

		if (_searchResults.empty()) {
			g_engine->_smallFont->drawString(g_engine->_screen, _noResults, textX, textY + 50, 280, 14);
		} else {
			// Display current book
			int bookIdx = _searchResults[_currentResult];
			const LibraryBook &book = _libraryBooks[bookIdx];

			// Title (may be long, truncate if needed)
			Common::String titleLine = Common::String::format("%s%s", _labelTitle, book.title.c_str());
			g_engine->_smallFont->drawString(g_engine->_screen, titleLine, textX - 50, textY + 40, 340, 14);

			// Author
			Common::String authorLine = Common::String::format("%s%s", _labelAuthor, book.author.c_str());
			g_engine->_smallFont->drawString(g_engine->_screen, authorLine, textX - 50, textY + 60, 340, 14);

			// Genre
			Common::String genreLine = Common::String::format("%s%s", _labelGenre, book.genre.c_str());
			g_engine->_smallFont->drawString(g_engine->_screen, genreLine, textX - 50, textY + 80, 340, 14);

			// Situacion (location/availability)
			Common::String situacionLine;
			if (book.available) {
				situacionLine = Common::String::format("%s Estanteria %d",
													   _labelSituacion, book.shelf);
			} else {
				situacionLine = Common::String::format("%s%s",
													   _labelSituacion, _statusCatalogOnly);
			}
			g_engine->_smallFont->drawString(g_engine->_screen, situacionLine, textX - 50, textY + 100, 340,
											 book.available ? 10 : 8); // Green if physical, gray if catalog-only

			// Show result counter
			Common::String counter = Common::String::format("Libro %d de %d",
															_currentResult + 1, (int)_searchResults.size());
			g_engine->_smallFont->drawString(g_engine->_screen, counter, textX, textY + 130, 280, 14, Graphics::kTextAlignCenter);

			// Show navigation options
			Common::String navOptions;
			if (book.available) {
				navOptions = Common::String::format("%s   %s   %s", _optMemorizar, _optSeguir, _optCancelar);
			} else {
				navOptions = Common::String::format("%s   %s", _optSeguir, _optCancelar);
			}
			g_engine->_smallFont->drawString(g_engine->_screen, navOptions, textX, textY + 160, 280, 8, Graphics::kTextAlignCenter);
		}
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
		_currentResult = 0;
		_state = STATE_SHOW_RESULTS;
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	} else if (_events->_lastKeyEvent == Common::KEYCODE_ESCAPE) {
		_state = STATE_MAIN_MENU;
		_events->_lastKeyEvent = Common::KEYCODE_INVALID;
	}
}

void Computer::handleResultsDisplay() {
	if (_events->_lastKeyEvent == Common::KEYCODE_s) {
		if (!_searchResults.empty()) {
			_currentResult = (_currentResult + 1) % _searchResults.size();
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
	g_engine->_state->bookLetter = book.title[0];

	debug("Memorized book '%s' with index %d, shelf %d, letter %c", book.title.c_str(), g_engine->_state->selectedBookIndex, g_engine->_state->libraryShelf, g_engine->_state->bookLetter);
}

void Computer::performSearch() {
	_searchResults.clear();

	for (int i = 0; i < _libraryBooks.size(); i++) {
		Common::String searchField = _searchType == 0 ? _libraryBooks[i].title : _libraryBooks[i].author;

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
