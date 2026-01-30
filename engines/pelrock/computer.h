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
	const char *_memorizedMsg;      // "Bueno... Tendre que buscar en la estanteria de la %c"

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

	const char *_menuTitle;
	const char *_menuOption1;       // "CONSULTAR POR TITULO"
	const char *_menuOption2;       // "CONSULTAR POR AUTOR"
	const char *_menuOption3;       // "CANCELAR"
	const char *_promptLetter;      // "Teclea una letra (A-Z):"
	const char *_labelTitle;        // "Titulo    : "
	const char *_labelAuthor;       // "Autor     : "
	const char *_labelGenre;        // "Genero    : "
	const char *_labelSituacion;    // "Situacion : "
	const char *_statusPhysical;    // "Estante %c, fila %d"
	const char *_statusCatalogOnly; // "Solo en catalogo"
	const char *_optMemorizar;      // "(M)emorizar"
	const char *_optSeguir;         // "(S)eguir"
	const char *_optCancelar;       // "(C)ancelar"
	const char *_noResults;         // "No se encontraron libros"

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
