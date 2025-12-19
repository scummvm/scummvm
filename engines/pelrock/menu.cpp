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

#include "common/debug.h"
#include "common/file.h"
#include "graphics/paletteman.h"

#include "pelrock/menu.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"
#include "menu.h"

namespace Pelrock {

Pelrock::MenuManager::MenuManager(Graphics::Screen *screen, PelrockEventManager *events, ResourceManager *res) : _screen(screen), _events(events), _res(res) {

}

void MenuManager::drawColoredText(Graphics::ManagedSurface *screen, const Common::String &text, int x, int y, int w, Graphics::Font *font) {
	int currentX = x;
	uint32 currentColor = 255;

	Common::String segment;
	for (uint i = 0; i < text.size(); i++) {
		if (text[i] == '@' && i + 1 < text.size()) {
			// Draw accumulated segment
			if (!segment.empty()) {
				font->drawString(screen, segment, currentX, y, w, currentColor);
				currentX += font->getStringWidth(segment);
				segment.clear();
			}
			currentColor = text[i + 1];
			i++; // skip color code
		} else {
			segment += text[i];
		}
	}

	// Draw remaining segment
	if (!segment.empty()) {
		font->drawString(screen, segment, currentX, y, w, currentColor);
	}
}

void MenuManager::checkMouseClick(int x, int y) {

	bool selectedItem = false;
	for (int i = 0; i < 4; i++) {
		if (x >= 140 + (82 * i) && x <= 140 + (82 * i) + 64 &&
			y >= 115 - (8 * i) && y <= 115 - (8 * i) + 64) {
			_selectedInvIndex = g_engine->_inventoryItems[_curInventoryPage * 4 + i];
			_menuText = _inventoryDescriptions[_selectedInvIndex];
			selectedItem = true;
			return;
		}
	}
	if (!selectedItem) {
		_selectedInvIndex = -1;
		_menuText = _menuTexts[0];
	}

	if (x >= 471 && x <= 471 + 23 &&
		y >= 87 && y <= 87 + 33) {
		_curInventoryPage++;
	}
}


void MenuManager::menuLoop() {
	_events->pollEvent();

	if(_events->_leftMouseClicked) {
		_events->_leftMouseClicked = false;
		checkMouseClick(_events->_mouseX, _events->_mouseY);
	}
	else if (_events->_rightMouseClicked) {
		_events->_rightMouseClicked = false;
		g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
		g_engine->stateGame = GAME;
		tearDown();
	}

	memcpy(_compositeBuffer, _mainMenu, 640 * 400);


	for (int i = 0; i < 4; i++) {
		int itemIndex = _curInventoryPage * 4 + i;
		if(g_engine->_inventoryItems.size() <= itemIndex)
			continue;
		InventoryObject item = g_engine->_res->getInventoryObject(g_engine->_inventoryItems[itemIndex]);
		drawSpriteToBuffer(_compositeBuffer, 640, item.iconData, 140 + (82 * i), 115 - (8 * i), 60, 60, 1);
	}

	memcpy(_screen->getPixels(), _compositeBuffer, 640 * 400);
	for(int i = 0; _menuText.size() > i; i++) {
		drawColoredText(_screen, _menuText[i], 230, 200 + (i * 10), 200, g_engine->_smallFont);
	}

	_screen->markAllDirty();
	_screen->update();
}

void MenuManager::loadMenu() {

	bool alternateMenu = false;
	Common::File alfred7;
	if (!alfred7.open(Common::Path("ALFRED.7"))) {
		error("Could not open ALFRED.7");
		return;
	}

	_compositeBuffer = new byte[640 * 400];
	_mainMenu = new byte[640 * 400];
	loadMenuTexts();
	if (!alternateMenu) {
		alfred7.seek(kSettingsPaletteOffset, SEEK_SET);
		alfred7.read(_mainMenuPalette, 768);
		for (int i = 0; i < 256; i++) {
			_mainMenuPalette[i * 3] = _mainMenuPalette[i * 3] << 2;
			_mainMenuPalette[i * 3 + 1] = _mainMenuPalette[i * 3 + 1] << 2;
			_mainMenuPalette[i * 3 + 2] = _mainMenuPalette[i * 3 + 2] << 2;
		}

		uint32 curPos = 0;
		alfred7.seek(2405266, SEEK_SET);
		alfred7.read(_mainMenu, 65536);

		curPos += 65536;

		byte *compressedPart1 = new byte[29418];
		alfred7.read(compressedPart1, 29418);
		byte *decompressedPart1 = nullptr;
		size_t decompressedSize = rleDecompress(compressedPart1, 29418, 0, 0, &decompressedPart1, true);

		memcpy(_mainMenu + curPos, decompressedPart1, decompressedSize);
		curPos += decompressedSize;

		delete[] compressedPart1;
		delete[] decompressedPart1;
		alfred7.seek(2500220, SEEK_SET);
		alfred7.read(_mainMenu + curPos, 32768);
		curPos += 32768;
		byte *compressedPart2 = new byte[30288];
		alfred7.read(compressedPart2, 30288);
		byte *decompressedPart2 = nullptr;
		decompressedSize = rleDecompress(compressedPart2, 30288, 0, 0, &decompressedPart2, true);

		memcpy(_mainMenu + curPos, decompressedPart2, decompressedSize);
		curPos += decompressedSize;
		debug("Settings menu size loaded: %d, with last block %d", curPos, curPos + 92160);
		delete[] compressedPart2;
		delete[] decompressedPart2;
		alfred7.seek(2563266, SEEK_SET);
		alfred7.read(_mainMenu + curPos, 92160);
		alfred7.close();
	} else {
		Common::File alfred7;
		if (!alfred7.open(Common::Path("ALFRED.7"))) {
			error("Could not open ALFRED.7");
			return;
		}

		_mainMenu = new byte[640 * 400];

		alfred7.seek(kAlternateSettingsPaletteOffset, SEEK_SET);
		alfred7.read(_mainMenuPalette, 768);
		for (int i = 0; i < 256; i++) {
			_mainMenuPalette[i * 3] = _mainMenuPalette[i * 3] << 2;
			_mainMenuPalette[i * 3 + 1] = _mainMenuPalette[i * 3 + 1] << 2;
			_mainMenuPalette[i * 3 + 2] = _mainMenuPalette[i * 3 + 2] << 2;
		}

		g_engine->_res->mergeRleBlocks(&alfred7, kAlternateSettingsMenuOffset, 8, _mainMenu);
		alfred7.close();
	}
}

void MenuManager::loadMenuTexts() {

	Common::File exe;
	if (!exe.open("JUEGO.EXE")) {
		error("Couldnt find file JUEGO.EXE");
	}
	byte *descBuffer = new byte[kInventoryDescriptionsSize];
	exe.seek(kInventoryDescriptionsOffset, SEEK_SET);
	exe.read(descBuffer, kInventoryDescriptionsSize);
	_inventoryDescriptions = _res->processTextData(descBuffer, kInventoryDescriptionsSize, true);
	delete[] descBuffer;

	Common::String desc = "";
	byte *textBuffer = new byte[kMenuTextSize];
	exe.seek(kMenuTextOffset, SEEK_SET);
	exe.read(textBuffer, kMenuTextSize);
	_menuTexts = _res->processTextData(textBuffer, kMenuTextSize, true);

	_menuText = _menuTexts[0];
	delete[] textBuffer;

	exe.close();
}

void MenuManager::tearDown() {

}

Pelrock::MenuManager::~MenuManager() {
	delete[] _mainMenu;
}

} // End of namespace Pelrock
