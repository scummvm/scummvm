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

#include "menu.h"
#include "pelrock/menu.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

Pelrock::MenuManager::MenuManager(Graphics::Screen *screen, PelrockEventManager *events, ResourceManager *res, SoundManager *sound) : _screen(screen), _events(events), _res(res), _sound(sound) {
}

MenuButton MenuManager::isButtonClicked(int x, int y) {
	if (_questionMarkRect.contains(x, y)) {
		return QUESTION_MARK_BUTTON;
	}
	if (_invLeft.contains(x, y)) {
		return INVENTORY_PREV_BUTTON;
	}
	if (_invRight.contains(x, y)) {
		return INVENTORY_NEXT_BUTTON;
	}
	if (_saveGameRect.contains(x, y)) {
		return SAVE_GAME_BUTTON;
	}
	if (_loadGameRect.contains(x, y)) {
		return LOAD_GAME_BUTTON;
	}
	if (_soundsRect.contains(x, y)) {
		return SOUNDS_BUTTON;
	}
	if (_exitToDosRect.contains(x, y)) {
		return EXIT_MENU_BUTTON;
	}
	if (_savesUp.contains(x, y)) {
		return SAVEGAME_PREV_BUTTON;
	}
	if (_savesDown.contains(x, y)) {
		return SAVEGAME_NEXT_BUTTON;
	}
	return NO_BUTTON; // Default fallback
}

void MenuManager::checkMouseClick(int x, int y) {

	bool selectedItem = false;
	for (int i = 0; i < 4; i++) {
		if (x >= 140 + (82 * i) && x <= 140 + (82 * i) + 64 &&
			y >= 115 - (8 * i) && y <= 115 - (8 * i) + 64) {
			selectedItem = selectInventoryItem(i);
			return;
		}
	}
	if (!selectedItem) {
		_selectedInvIndex = -1;
		_menuText = _menuTexts[0];
	}

	MenuButton button = isButtonClicked(x, y);
	switch (button) {
	case QUESTION_MARK_BUTTON:
		debug("Show credits");
		break;
	case INVENTORY_PREV_BUTTON:
		if (_curInventoryPage > 0)
			_curInventoryPage--;
		break;
	case INVENTORY_NEXT_BUTTON:
		if ((_curInventoryPage + 1) * 4 < g_engine->_state->inventoryItems.size())
			_curInventoryPage++;
		break;
	case SAVE_GAME_BUTTON:
		g_engine->saveGameDialog();
		break;
	case LOAD_GAME_BUTTON:
		g_engine->loadGameDialog();
		break;
	default:
		break;
	}
}

bool MenuManager::selectInventoryItem(int i) {
	if (_curInventoryPage * 4 + i >= g_engine->_state->inventoryItems.size())
		return false;

	_selectedInvIndex = g_engine->_state->inventoryItems[_curInventoryPage * 4 + i];
	_menuText = _inventoryDescriptions[_selectedInvIndex];
	_sound->playSound(inventorySounds[_selectedInvIndex], 100, 0);
	g_engine->_state->selectedInventoryItem = _selectedInvIndex;
	debug("Selected inventory item %d", _selectedInvIndex);
	return true;
}

void MenuManager::menuLoop() {

	g_system->getPaletteManager()->setPalette(_mainMenuPalette, 0, 256);
	g_engine->changeCursor(DEFAULT);
	while (!g_engine->shouldQuit() && !_events->_rightMouseClicked) {

		_events->pollEvent();

		if (_events->_leftMouseClicked) {
			checkMouseClick(_events->_mouseX, _events->_mouseY);
			_events->_leftMouseClicked = false;
		}

		drawScreen();
		_screen->markAllDirty();
		_screen->update();
		g_system->delayMillis(10);
	}
	g_engine->_graphics->clearScreen();
	_events->_rightMouseClicked = false;
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
	cleanUp();
}

void MenuManager::drawScreen() {
	memcpy(_compositeBuffer, _mainMenu, 640 * 400);
	// memset(_compositeBuffer, 0, 640 * 400);
	if (showButtons)
		drawButtons();

	drawInventoryIcons();

	memcpy(_screen->getPixels(), _compositeBuffer, 640 * 400);
	byte defaultColor = 255;
	for (int i = 0; _menuText.size() > i; i++) {
		g_engine->_graphics->drawColoredText(_screen, _menuText[i], 230, 200 + (i * 10), 200, defaultColor, g_engine->_smallFont);
	}

	drawText(g_engine->_smallFont, Common::String::format("%d,%d", _events->_mouseX, _events->_mouseY), 0, 0, 640, 13);
}

void MenuManager::drawInventoryIcons() {
	bool debugIcons = true;
	for (int i = 0; i < 4; i++) {
		int itemIndex = _curInventoryPage * 4 + i;
		if (g_engine->_state->inventoryItems.size() <= itemIndex)
			continue;
		InventoryObject item = g_engine->_res->getIconForObject(g_engine->_state->inventoryItems[itemIndex]);
		drawSpriteToBuffer(_compositeBuffer, 640, item.iconData, 140 + (82 * i), 115 - (8 * i), 60, 60, 1);
		if (debugIcons) {
			drawRect(_compositeBuffer, 140 + (82 * i), 115 - (8 * i), 60, 60, 13);
			drawText(_compositeBuffer, g_engine->_smallFont, Common::String::format("ID %d", g_engine->_state->inventoryItems[itemIndex]), 140 + (82 * i) + 2, 115 - (8 * i) + 2, 640, 13);
		}
	}
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
	}

	readButton(alfred7, 3193376, _saveButtons, _saveGameRect);
	readButton(alfred7, alfred7.pos(), _loadButtons, _loadGameRect);
	readButton(alfred7, alfred7.pos(), _soundsButtons, _soundsRect);
	readButton(alfred7, alfred7.pos(), _exitToDosButtons, _exitToDosRect);
	readButton(alfred7, kInvLeftArrowOffset, _inventoryLeftArrow, _invLeft);
	readButton(alfred7, alfred7.pos(), _inventoryRightArrow, _invRight);
	readButton(alfred7, alfred7.pos(), _savesUpArrows, _savesUp);
	readButton(alfred7, alfred7.pos(), _savesDownArrows, _savesDown);
	readButton(alfred7, 3214046, _questionMark, _questionMarkRect);

	_menuText = _menuTexts[0];
	alfred7.close();
}

void MenuManager::readButton(Common::File &alfred7, uint32 offset, byte *outBuffer[2], Common::Rect rect) {
	alfred7.seek(offset, SEEK_SET);
	byte *buttonData = new byte[rect.width() * rect.height() * 2];
	alfred7.read(buttonData, rect.width() * rect.height() * 2);
	outBuffer[0] = new byte[rect.width() * rect.height()];
	outBuffer[1] = new byte[rect.width() * rect.height()];
	extractSingleFrame(buttonData, outBuffer[0], 0, rect.width(), rect.height());
	extractSingleFrame(buttonData, outBuffer[1], 1, rect.width(), rect.height());
	delete[] buttonData;
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

void MenuManager::cleanUp() {
}

void MenuManager::drawButtons() {
	MenuButton button = NO_BUTTON;
	if (_events->_leftMouseButton != 0) {
		button = isButtonClicked(_events->_mouseX, _events->_mouseY);
	}
	byte *buf = button == QUESTION_MARK_BUTTON ? _questionMark[1] : _questionMark[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _questionMarkRect.left, _questionMarkRect.top, _questionMarkRect.width(), _questionMarkRect.height(), kTransparentColor);

	buf = button == INVENTORY_PREV_BUTTON ? _inventoryLeftArrow[1] : _inventoryLeftArrow[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _invLeft.left, _invLeft.top, _invLeft.width(), _invLeft.height(), kTransparentColor);

	buf = button == INVENTORY_NEXT_BUTTON ? _inventoryRightArrow[1] : _inventoryRightArrow[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _invRight.left, _invRight.top, _invRight.width(), _invRight.height(), kTransparentColor);

	buf = button == SAVE_GAME_BUTTON ? _saveButtons[1] : _saveButtons[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _saveGameRect.left, _saveGameRect.top, _saveGameRect.width(), _saveGameRect.height(), kTransparentColor);

	buf = button == LOAD_GAME_BUTTON ? _loadButtons[1] : _loadButtons[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _loadGameRect.left, _loadGameRect.top, _loadGameRect.width(), _loadGameRect.height(), kTransparentColor);

	buf = button == LOAD_GAME_BUTTON ? _loadButtons[1] : _loadButtons[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _loadGameRect.left, _loadGameRect.top, _loadGameRect.width(), _loadGameRect.height(), kTransparentColor);

	buf = button == SOUNDS_BUTTON ? _soundsButtons[1] : _soundsButtons[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _soundsRect.left, _soundsRect.top, _soundsRect.width(), _soundsRect.height(), kTransparentColor);

	buf = button == EXIT_MENU_BUTTON ? _exitToDosButtons[1] : _exitToDosButtons[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _exitToDosRect.left, _exitToDosRect.top, _exitToDosRect.width(), _exitToDosRect.height(), kTransparentColor);

	buf = button == SAVEGAME_PREV_BUTTON ? _savesUpArrows[1] : _savesUpArrows[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _savesUp.left, _savesUp.top, _savesUp.width(), _savesUp.height(), kTransparentColor);

	buf = button == SAVEGAME_NEXT_BUTTON ? _savesDownArrows[1] : _savesDownArrows[0];
	drawSpriteToBuffer(_compositeBuffer, 640, buf, _savesDown.left, _savesDown.top, _savesDown.width(), _savesDown.height(), kTransparentColor);
}

Pelrock::MenuManager::~MenuManager() {
	delete[] _mainMenu;
	delete[] _compositeBuffer;
	delete[] _questionMark[0];
	delete[] _questionMark[1];
	delete[] _inventoryLeftArrow[0];
	delete[] _inventoryLeftArrow[1];
	delete[] _inventoryRightArrow[0];
	delete[] _inventoryRightArrow[1];
}

} // End of namespace Pelrock
