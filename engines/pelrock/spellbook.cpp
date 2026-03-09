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

#include "pelrock/graphics.h"
#include "pelrock/room.h"
#include "pelrock/spellbook.h"
#include "pelrock/util.h"

namespace Pelrock {

static const uint32 kSpellbookImgOffset     = 1268719; // ALFRED.7 — spellbook sprite sheet start
static const uint32 kSpellbookImgDataOffset = 1268723; // ALFRED.7 — compressed sprite data
static const uint32 kSpellbookTextOffset    = 288285;  // JUEGO.EXE — spellbook page text
static const uint32 kSpellbookTextSize      = 2861;

SpellBook::SpellBook(PelrockEventManager *eventMan, ResourceManager *res)
	: _palette(nullptr),
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
	bool exit = false;
	while (!g_engine->shouldQuit() && !exit) {
		_events->pollEvent();
		drawScreen();
		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			exit = checkMouse(_events->_mouseClickX, _events->_mouseClickY);
		}
		g_engine->_screen->markAllDirty();
		g_engine->_screen->update();
		g_system->delayMillis(10);
	}
	g_engine->_screen->clear(0);
	// Restore room palette
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
	return _selectedSpell;
}

void SpellBook::init() {
	_compositeScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
}

void SpellBook::selectPage(int page) {
	debug("Selected spell page: %d", page);
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

	alfred7.seek(kSpellbookImgOffset, SEEK_SET);
	int w = 119;
	int h = 99;
	int nFrames = 13;
	byte *compressedData = nullptr;
	byte *spriteData = nullptr;
	size_t outSize = 0;
	readUntilBuda(&alfred7, kSpellbookImgDataOffset, compressedData, outSize);
	rleDecompress(compressedData, outSize, 0, w * h * nFrames, &spriteData, false);
	_spell->image = new byte[w * h];
	extractSingleFrame(spriteData, _spell->image, page, w, h);

	juegoFile.seek(kSpellbookTextOffset, SEEK_SET);
	byte *textData = new byte[kSpellbookTextSize];
	juegoFile.read(textData, kSpellbookTextSize);

	for (int i = 0; i < (int)kSpellbookTextSize; ++i) {
		if (textData[i] == 0x0D)
			textData[i] = 23;
	}

	Common::Array<Common::StringArray> spells = _res->processTextData(textData, kSpellbookTextSize, true);
	_spell->text = spells[page];
	delete[] compressedData;
	delete[] spriteData;
	alfred7.close();
	juegoFile.close();
}

void SpellBook::drawScreen() {
	_compositeScreen.blitFrom(_backgroundScreen);

	int textY = 83;
	int textX = 317;

	if (_spell != nullptr) {
		drawSpriteToBuffer(_compositeScreen, _spell->image, 168, 143, 119, 99, 207);
		g_engine->_graphics->drawColoredTexts(_compositeScreen, _spell->text, textX, textY, 640, 0, g_engine->_smallFont);
	}

	g_engine->_screen->blitFrom(_compositeScreen);
	if (_spell != nullptr) {
		g_engine->_graphics->drawColoredTexts(g_engine->_screen, _spell->text, textX, textY, 640, 0, g_engine->_smallFont);
	}
}

void SpellBook::loadBackground() {
	_backgroundScreen.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	_palette = new byte[768];
	_res->getExtraScreen(8, (byte *)_backgroundScreen.getPixels(), _palette);
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

void SpellBook::cleanup() {
	_backgroundScreen.free();
	_compositeScreen.free();
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

bool SpellBook::checkMouse(int x, int y) {
	// Check bookmarks
	for (int i = 0; i < 13; i++) {
		Common::Rect r = Common::Rect(_bookmarks[i].x, _bookmarks[i].y, _bookmarks[i].x + _bookmarks[i].w, _bookmarks[i].y + _bookmarks[i].h);
		if (r.contains(x, y)) {
			selectPage(_bookmarks[i].page);
			return false;
		}
	}

	// Check text area
	if (_spell == nullptr) {
		return true;
	}

	Common::Rect textArea = Common::Rect(321, 81, 321 + 140, 81 + (_spell->text.size() * 10));
	if (textArea.contains(x, y)) {
		_selectedSpell = _spell;
		return true;
	}
	return false;
}

} // End of namespace Pelrock
