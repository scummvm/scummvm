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
#include "graphics/paletteman.h"

#include "extrascreens.h"
#include "pelrock/extrascreens.h"
#include "pelrock/graphics.h"
#include "pelrock/util.h"

namespace Pelrock {

SpellBook::SpellBook(PelrockEventManager *eventMan, ResourceManager *res)
	: _backgroundScreen(nullptr),
	  _palette(nullptr),
	  _events(eventMan),
	  _res(res),
	  _spell(nullptr) {
	init();
}

SpellBook::~SpellBook() {
	cleanup();
}

Spell *SpellBook::run() {
	loadBackground();
	g_engine->changeCursor(DEFAULT);
	while (!g_engine->shouldQuit() && !_selectedSpell) {
		_events->pollEvent();
		drawScreen();
		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			checkMouse(_events->_mouseClickX, _events->_mouseClickY);
		}
		g_engine->_screen->markAllDirty();
		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
	memset(g_engine->_screen->getPixels(), 0, 640 * 400);
	// Restore room palette
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
	return _selectedSpell;
}

void SpellBook::init() {
	_compositeScreen = new byte[640 * 400];

	// selectPage(0);
}

void SpellBook::selectPage(int page) {
	_spell = new Spell();
	_spell->page = page;
	Common::File alfred7;
	if (!alfred7.open("ALFRED.7")) {
		return;
	}

	Common::File juegoFile;
	if (!juegoFile.open("JUEGO.EXE")) {
		return;
	}

	alfred7.seek(1268719, SEEK_SET);
	int w = 119;
	int h = 99;
	int nFrames = 13;
	byte *compressedData = nullptr;
	byte *spriteData = nullptr;
	size_t outSize = 0;
	readUntilBuda(&alfred7, 1268723, compressedData, outSize);
	rleDecompress(compressedData, outSize, 0, w * h * nFrames, &spriteData, false);
	_spell->image = new byte[w * h];
	extractSingleFrame(spriteData, _spell->image, page, w, h);

	juegoFile.seek(0x0004661C, SEEK_SET);
	byte *textData = new byte[2861];
	juegoFile.read(textData, 2861);

	for (int i = 0; i < 2861; ++i) {
		if (textData[i] == 0x0D)
			textData[i] = 23;
	}

	Common::Array<Common::StringArray> spells = _res->processTextData(textData, 2861, true);

	_spell->text = spells[page];
	delete[] compressedData;
	delete[] spriteData;
	alfred7.close();
	juegoFile.close();
}

void SpellBook::drawScreen() {
	memcpy(_compositeScreen, _backgroundScreen, 640 * 400);

	int textY = 83;
	int textX = 317;

	if (_spell != nullptr) {
		drawSpriteToBuffer(_compositeScreen, 640, _spell->image, 168, 143, 119, 99, 207);
		g_engine->_graphics->drawColoredTexts(_compositeScreen, _spell->text, textX, textY, 640, 0, g_engine->_smallFont);
	}

	drawPaletteSquares(_compositeScreen, _palette);
	memcpy(g_engine->_screen->getPixels(), _compositeScreen, 640 * 400);
	if (_spell != nullptr) {
		g_engine->_graphics->drawColoredTexts(g_engine->_screen, _spell->text, textX, textY, 640, 0, g_engine->_smallFont);
	}
}

void SpellBook::loadBackground() {
	_backgroundScreen = new byte[640 * 400];
	_palette = new byte[768];
	_res->getExtraScreen(8, _backgroundScreen, _palette);
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void SpellBook::cleanup() {
	if (_backgroundScreen) {
		delete[] _backgroundScreen;
		_backgroundScreen = nullptr;
	}
	if (_palette) {
		delete[] _palette;
		_palette = nullptr;
	}
	if (_spell) {
		delete _spell;
		_spell = nullptr;
	}
	g_engine->_screen->markAllDirty();
	g_engine->_screen->update();
}

void SpellBook::checkMouse(int x, int y) {
	// Check bookmarks
	for (int i = 0; i < 13; i++) {
		Common::Rect r = Common::Rect(_bookmarks[i].x, _bookmarks[i].y, _bookmarks[i].x + _bookmarks[i].w, _bookmarks[i].y + _bookmarks[i].h);
		if (r.contains(x, y)) {
			selectPage(_bookmarks[i].page);
			return;
		}
	}

	// Check text area
	if (_spell == nullptr) {
		return;
	}
	Common::Rect textArea = Common::Rect(321, 81, 321 + 140, 81 + (_spell->text.size() * 10));
	if (textArea.contains(x, y)) {
		_selectedSpell = _spell;
	}
}

} // End of namespace Pelrock
