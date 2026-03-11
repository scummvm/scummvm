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

// ALFRED.7 — alternate settings palette
static const uint32 kSettingsPaletteOffset = 0x2884C2;

// JUEGO.EXE — inventory object descriptions
static const uint32 kInventoryDescriptionsOffset = 0x4715E;
static const uint32 kInventoryDescriptionsSize = 7868;

// JUEGO.EXE — in-menu text strings
static const uint32 kMenuTextOffset = 0x49203;
static const uint32 kMenuTextSize = 230;

// ALFRED.7 — main menu background
static const uint32 kMainMenuPart1Offset = 2405266;
static const uint32 kMainMenuPart1RawSize = 65536;        // first uncompressed chunk
static const uint32 kMainMenuPart1CompressedSize = 29418; // following compressed tail
static const uint32 kMainMenuPart2Offset = 2500220;
static const uint32 kMainMenuPart2RawSize = 32768;
static const uint32 kMainMenuPart2CompressedSize = 30288;
static const uint32 kMainMenuPart3Offset = 2563266;
static const uint32 kMainMenuPart3Size = 92160;

// ALFRED.7 — menu buttons (save/load/sound/exit, one contiguous block)
static const uint32 kMenuButtonsOffset = 3193376;

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
	if (_showSoundOptions) {
		if (_masterVolumeLeftRect.contains(x, y)) {
			return MASTER_LEFT_BUTTON;
		}
		if (_masterVolumeRightRect.contains(x, y)) {
			return MASTER_RIGHT_BUTTON;
		}
		if (_sfxVolumeLeftRect.contains(x, y)) {
			return SFX_LEFT_BUTTON;
		}
		if (_sfxVolumeRightRect.contains(x, y)) {
			return SFX_RIGHT_BUTTON;
		}
		if (_musicVolumeLeftRect.contains(x, y)) {
			return MUSIC_LEFT_BUTTON;
		}
		if (_musicVolumeRightRect.contains(x, y)) {
			return MUSIC_RIGHT_BUTTON;
		}
	}
	return NO_BUTTON; // Default fallback
}

Graphics::ManagedSurface scale(Graphics::ManagedSurface, const byte *original, float scale) {
	Graphics::ManagedSurface newSurface = Graphics::ManagedSurface(66, 64, Graphics::PixelFormat::createFormatCLUT8());
	memcpy(newSurface.getPixels(), original, 66 * 64);
	return *newSurface.scale(66 * scale, 64 * scale);
}

void MenuManager::checkMouseDown(int x, int y) {
	if(!_events->_leftMouseButton) {
		return;
	}
	MenuButton b = isButtonClicked(x, y);
	switch (b) {
	case MASTER_LEFT_BUTTON:
		currentMasterVolumeScale = MAX(0.4f, currentMasterVolumeScale - 0.1f);
		_masterSoundIcon = scale(_masterSoundIcon, _soundControlMasterIcon, currentMasterVolumeScale);
		break;
	case MASTER_RIGHT_BUTTON:
		currentMasterVolumeScale = MIN(1.0f, currentMasterVolumeScale + 0.1f);
		_masterSoundIcon = scale(_masterSoundIcon, _soundControlMasterIcon, currentMasterVolumeScale);
		break;
	case SFX_LEFT_BUTTON:
		currentSfxVolumeScale = MAX(0.4f, currentSfxVolumeScale - 0.1f);
		_sfxSoundIcon = scale(_sfxSoundIcon, _soundControlSfxIcon, currentSfxVolumeScale);
		break;
	case SFX_RIGHT_BUTTON:
		currentSfxVolumeScale = MIN(1.0f, currentSfxVolumeScale + 0.1f);
		_sfxSoundIcon = scale(_sfxSoundIcon, _soundControlSfxIcon, currentSfxVolumeScale);
		break;
	case MUSIC_LEFT_BUTTON:
		currentMusicVolumeScale = MAX(0.4f, currentMusicVolumeScale - 0.1f);
		_musicSoundIcon = scale(_musicSoundIcon, _soundControlMusicIcon, currentMusicVolumeScale);
		break;
	case MUSIC_RIGHT_BUTTON:
		currentMusicVolumeScale = MIN(1.0f, currentMusicVolumeScale + 0.1f);
		_musicSoundIcon = scale(_musicSoundIcon, _soundControlMusicIcon, currentMusicVolumeScale);
		break;
	}
}

bool MenuManager::checkMouseClick(int x, int y) {

	bool selectedItem = false;
	for (int i = 0; i < 4; i++) {

		Common::Rect itemRect = Common::Rect(_inventorySlots[i], 60, 60);

		if (itemRect.contains(x, y)) {
			selectedItem = selectInventoryItem(i);
			return false;
		}
	}
	if (!selectedItem) {
		_selectedInvIndex = -1;
		_menuText = _menuTexts[0];
	}

	MenuButton button = isButtonClicked(x, y);

	if (button != MUSIC_LEFT_BUTTON &&
		button != MUSIC_RIGHT_BUTTON &&
		button != SFX_LEFT_BUTTON &&
		button != SFX_RIGHT_BUTTON &&
		button != MASTER_LEFT_BUTTON &&
		button != MASTER_RIGHT_BUTTON) {
		_showSoundOptions = false;
	}

	switch (button) {
	case QUESTION_MARK_BUTTON:
		debug("Show credits");
		_events->_leftMouseClicked = false;
		showCredits();
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
		return g_engine->saveGameDialog();
		break;
	case LOAD_GAME_BUTTON:
		return g_engine->loadGameDialog();
		break;
	case EXIT_MENU_BUTTON:
		g_engine->quitGame();
		break;
	case SOUNDS_BUTTON:
		_showSoundOptions = true;
		_menuText = Common::StringArray();
		break;
	case SFX_LEFT_BUTTON:
		_sound->playSound("CAT_1ZZZ.SMP", -1);
		break;
	case SFX_RIGHT_BUTTON:
		_sound->playSound("CAT_1ZZZ.SMP", -1);
		break;
	default:
		break;
	}
	return false;
}

void MenuManager::showCredits() {
	_compositeBuffer.clear(0);
	Common::File alfred7;
	if (!alfred7.open(Common::Path("ALFRED.7"))) {
		error("Could not open ALFRED.7");
		return;
	}

	alfred7.seek(kCreditsBackgroundOffset, SEEK_SET);
	alfred7.read(_compositeBuffer.getPixels(), 640 * 400);
	byte *creditsBuf = nullptr;
	size_t creditsSize = 0;
	int creditWidth = 240;
	int creditHeight = 22;
	readUntilBuda(&alfred7, kCreditsBackgroundOffset + 256000, creditsBuf, creditsSize);
	byte *decompressedCredits = nullptr;
	rleDecompress(creditsBuf, creditsSize, 0, 0, &decompressedCredits, true);
	// draw credits in two columns taking the entire height of the screen and stating in y = 0
	for (int i = 0; i < 34; i++) {
		byte *singleCredit = new byte[creditWidth * creditHeight];
		int x = (i < 34 / 2) ? 39 : 359;
		int y = 3 + (i % (34 / 2)) * (400 / (34 / 2));
		extractSingleFrame(decompressedCredits, singleCredit, kCreditsOrder[i], creditWidth, creditHeight);
		drawSpriteToBuffer(_compositeBuffer, singleCredit, x, y, creditWidth, creditHeight, 255);
		delete[] singleCredit;
	}

	_screen->blitFrom(_compositeBuffer);
	delete[] decompressedCredits;
	delete[] creditsBuf;

	while (!g_engine->shouldQuit() && !_events->_leftMouseClicked && !_events->_rightMouseClicked) {
		_events->pollEvent();
		_screen->markAllDirty();
		_screen->update();
		g_system->delayMillis(10);
	}
}

bool MenuManager::selectInventoryItem(int i) {
	if (_curInventoryPage * 4 + i >= g_engine->_state->inventoryItems.size())
		return false;

	_selectedInvIndex = g_engine->_state->inventoryItems[_curInventoryPage * 4 + i];
	_menuText = _inventoryDescriptions[_selectedInvIndex];
	_sound->playSound(inventorySounds[_selectedInvIndex], 0);
	g_engine->_state->selectedInventoryItem = _selectedInvIndex;
	debug("Selected inventory item %d", _selectedInvIndex);
	return true;
}

void MenuManager::menuLoop() {

	g_system->getPaletteManager()->setPalette(_mainMenuPalette, 0, 256);
	g_engine->changeCursor(DEFAULT);
	_menuText = _menuTexts[0];
	while (!g_engine->shouldQuit()) {

		_events->pollEvent();
		checkMouseDown(_events->_mouseX, _events->_mouseY);
		if (_events->_leftMouseClicked) {
			if (checkMouseClick(_events->_mouseX, _events->_mouseY)) {
				break;
			}
			_events->_leftMouseClicked = false;
		}
		if (_events->_rightMouseClicked) {
			break;
		}
		drawScreen();
		drawPaletteSquares((byte *)_screen->getPixels(), _mainMenuPalette);
		_screen->markAllDirty();
		_screen->update();
		g_system->delayMillis(10);
	}
	g_engine->_graphics->clearScreen();
	_events->_rightMouseClicked = false;
	_events->_leftMouseClicked = false;
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
	cleanUp();
}

void MenuManager::drawScreen() {
	_compositeBuffer.blitFrom(_mainMenu);
	if (showButtons)
		drawButtons();

	drawInventoryIcons();

	_screen->blitFrom(_compositeBuffer);
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
		Common::Point slot = _inventorySlots[i];
		drawSpriteToBuffer(_compositeBuffer, item.iconData, slot.x, slot.y, 60, 60, 1);
		if (debugIcons) {
			drawRect(&_compositeBuffer, slot.x, slot.y, 60, 60, 13);
			drawText(_compositeBuffer, g_engine->_smallFont, Common::String::format("ID %d", g_engine->_state->inventoryItems[itemIndex]), slot.x + 2, slot.y + 2, 640, 13);
		}
	}
}

void MenuManager::loadMenu() {

	Common::File alfred7;
	if (!alfred7.open(Common::Path("ALFRED.7"))) {
		error("Could not open ALFRED.7");
		return;
	}

	_compositeBuffer.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	_mainMenu.create(640, 400, Graphics::PixelFormat::createFormatCLUT8());
	loadMenuTexts();
	alfred7.seek(kSettingsPaletteOffset, SEEK_SET);
	alfred7.read(_mainMenuPalette, 768);
	for (int i = 0; i < 256; i++) {
		_mainMenuPalette[i * 3] = _mainMenuPalette[i * 3] << 2;
		_mainMenuPalette[i * 3 + 1] = _mainMenuPalette[i * 3 + 1] << 2;
		_mainMenuPalette[i * 3 + 2] = _mainMenuPalette[i * 3 + 2] << 2;
	}

	uint32 curPos = 0;
	alfred7.seek(kMainMenuPart1Offset, SEEK_SET);
	alfred7.read(_mainMenu.getPixels(), kMainMenuPart1RawSize);

	curPos += kMainMenuPart1RawSize;

	byte *compressedPart1 = new byte[kMainMenuPart1CompressedSize];
	alfred7.read(compressedPart1, kMainMenuPart1CompressedSize);
	byte *decompressedPart1 = nullptr;
	size_t decompressedSize = rleDecompress(compressedPart1, kMainMenuPart1CompressedSize, 0, 0, &decompressedPart1, true);

	memcpy((byte *)_mainMenu.getPixels() + curPos, decompressedPart1, decompressedSize);
	curPos += decompressedSize;

	delete[] compressedPart1;
	delete[] decompressedPart1;
	alfred7.seek(kMainMenuPart2Offset, SEEK_SET);
	alfred7.read((byte *)_mainMenu.getPixels() + curPos, kMainMenuPart2RawSize);
	curPos += kMainMenuPart2RawSize;
	byte *compressedPart2 = new byte[kMainMenuPart2CompressedSize];
	alfred7.read(compressedPart2, kMainMenuPart2CompressedSize);
	byte *decompressedPart2 = nullptr;
	decompressedSize = rleDecompress(compressedPart2, kMainMenuPart2CompressedSize, 0, 0, &decompressedPart2, true);

	memcpy((byte *)_mainMenu.getPixels() + curPos, decompressedPart2, decompressedSize);
	curPos += decompressedSize;
	delete[] compressedPart2;
	delete[] decompressedPart2;
	alfred7.seek(kMainMenuPart3Offset, SEEK_SET);
	alfred7.read((byte *)_mainMenu.getPixels() + curPos, kMainMenuPart3Size);

	readButton(alfred7, kMenuButtonsOffset, _saveButtons, _saveGameRect);
	readButton(alfred7, alfred7.pos(), _loadButtons, _loadGameRect);
	readButton(alfred7, alfred7.pos(), _soundsButtons, _soundsRect);
	readButton(alfred7, alfred7.pos(), _exitToDosButtons, _exitToDosRect);
	readButton(alfred7, kInvLeftArrowOffset, _inventoryLeftArrow, _invLeft);
	readButton(alfred7, alfred7.pos(), _inventoryRightArrow, _invRight);
	readButton(alfred7, alfred7.pos(), _savesUpArrows, _savesUp);
	readButton(alfred7, alfred7.pos(), _savesDownArrows, _savesDown);
	readButton(alfred7, kQuestionMarkOffset, _questionMark, _questionMarkRect);

	byte *soundArrowsData = nullptr;
	size_t soundArrowsSize = 0;
	rleDecompressSingleBuda(&alfred7, kSoundControlOffset, soundArrowsData, soundArrowsSize);

	readButton(soundArrowsData, 0, _soundControlArrowLeft, 36, 28);
	readButton(soundArrowsData, 36 * 28 * 2, _soundControlArrowRight, 31, 28);
	delete[] soundArrowsData;

	byte *soundIconMasterData = nullptr;
	size_t soundIconMasterSize = 0;
	rleDecompressSingleBuda(&alfred7, kSoundMasterOffset, soundIconMasterData, soundIconMasterSize);
	_soundControlMasterIcon = new byte[66 * 64];
	extractSingleFrame(soundIconMasterData, _soundControlMasterIcon, 0, 66, 64);
	delete[] soundIconMasterData;

	byte *soundIconSfxData = nullptr;
	size_t soundIconSfxSize = 0;
	rleDecompressSingleBuda(&alfred7, kSoundSfxOffset, soundIconSfxData, soundIconSfxSize);
	_soundControlSfxIcon = new byte[66 * 64];
	extractSingleFrame(soundIconSfxData, _soundControlSfxIcon, 0, 66, 64);
	delete[] soundIconSfxData;

	byte *soundIconMusicData = nullptr;
	size_t soundIconMusicSize = 0;
	rleDecompressSingleBuda(&alfred7, kSoundMusicOffset, soundIconMusicData, soundIconMusicSize);
	_soundControlMusicIcon = new byte[66 * 64];
	extractSingleFrame(soundIconMusicData, _soundControlMusicIcon, 0, 66, 64);
	delete[] soundIconMusicData;

	_masterSoundIcon = Graphics::ManagedSurface(66, 64, Graphics::PixelFormat::createFormatCLUT8());
	_sfxSoundIcon = Graphics::ManagedSurface(66, 64, Graphics::PixelFormat::createFormatCLUT8());
	_musicSoundIcon = Graphics::ManagedSurface(66, 64, Graphics::PixelFormat::createFormatCLUT8());
	memcpy(_masterSoundIcon.getPixels(), _soundControlMasterIcon, 66 * 64);
	memcpy(_sfxSoundIcon.getPixels(), _soundControlSfxIcon, 66 * 64);
	memcpy(_musicSoundIcon.getPixels(), _soundControlMusicIcon, 66 * 64);

	_menuText = _menuTexts[0];
	alfred7.close();

	for (int i = 0; i < 4; i++) {
		_inventorySlots.push_back(Common::Point(140 + (82 * i), 115 - (8 * i)));
	}
}

void MenuManager::readButton(byte *rawData, uint32 offset, byte *outBuffer[2], int w, int h) {
	byte *buttonData = new byte[w * h * 2];
	outBuffer[0] = new byte[w * h];
	outBuffer[1] = new byte[w * h];
	Common::copy(rawData + offset, rawData + offset + w * h * 2, buttonData);
	extractSingleFrame(buttonData, outBuffer[0], 0, w, h);
	extractSingleFrame(buttonData, outBuffer[1], 1, w, h);
	delete[] buttonData;
}

void MenuManager::readButton(Common::File &alfred7, uint32 offset, byte *outBuffer[2], Common::Rect rect) {
	alfred7.seek(offset, SEEK_SET);
	byte *buttonData = new byte[rect.width() * rect.height() * 2];
	outBuffer[0] = new byte[rect.width() * rect.height()];
	outBuffer[1] = new byte[rect.width() * rect.height()];
	alfred7.read(buttonData, rect.width() * rect.height() * 2);

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
	_masterSoundIcon.free();
	_sfxSoundIcon.free();
	_musicSoundIcon.free();
}

void MenuManager::drawButtons() {
	MenuButton button = NO_BUTTON;
	if (_events->_leftMouseButton != 0) {
		button = isButtonClicked(_events->_mouseX, _events->_mouseY);
	}
	byte *buf = button == QUESTION_MARK_BUTTON ? _questionMark[1] : _questionMark[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _questionMarkRect.left, _questionMarkRect.top, _questionMarkRect.width(), _questionMarkRect.height(), kTransparentColor);

	buf = button == INVENTORY_PREV_BUTTON ? _inventoryLeftArrow[1] : _inventoryLeftArrow[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _invLeft.left, _invLeft.top, _invLeft.width(), _invLeft.height(), kTransparentColor);

	buf = button == INVENTORY_NEXT_BUTTON ? _inventoryRightArrow[1] : _inventoryRightArrow[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _invRight.left, _invRight.top, _invRight.width(), _invRight.height(), kTransparentColor);

	buf = button == SAVE_GAME_BUTTON ? _saveButtons[1] : _saveButtons[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _saveGameRect.left, _saveGameRect.top, _saveGameRect.width(), _saveGameRect.height(), kTransparentColor);

	buf = button == LOAD_GAME_BUTTON ? _loadButtons[1] : _loadButtons[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _loadGameRect.left, _loadGameRect.top, _loadGameRect.width(), _loadGameRect.height(), kTransparentColor);

	buf = button == LOAD_GAME_BUTTON ? _loadButtons[1] : _loadButtons[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _loadGameRect.left, _loadGameRect.top, _loadGameRect.width(), _loadGameRect.height(), kTransparentColor);

	buf = button == SOUNDS_BUTTON ? _soundsButtons[1] : _soundsButtons[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _soundsRect.left, _soundsRect.top, _soundsRect.width(), _soundsRect.height(), kTransparentColor);

	buf = button == EXIT_MENU_BUTTON ? _exitToDosButtons[1] : _exitToDosButtons[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _exitToDosRect.left, _exitToDosRect.top, _exitToDosRect.width(), _exitToDosRect.height(), kTransparentColor);

	buf = button == SAVEGAME_PREV_BUTTON ? _savesUpArrows[1] : _savesUpArrows[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _savesUp.left, _savesUp.top, _savesUp.width(), _savesUp.height(), kTransparentColor);

	buf = button == SAVEGAME_NEXT_BUTTON ? _savesDownArrows[1] : _savesDownArrows[0];
	drawSpriteToBuffer(_compositeBuffer, buf, _savesDown.left, _savesDown.top, _savesDown.width(), _savesDown.height(), kTransparentColor);

	if (_showSoundOptions) {
		// _compositeBuffer.transBlitFrom(_masterSoundIcon, Common::Point(233, 188), kSoundControlsTransparentColor);
		// _compositeBuffer.transBlitFrom(_musicSoundIcon, Common::Point(299, 188), kSoundControlsTransparentColor);
		// _compositeBuffer.transBlitFrom(_sfxSoundIcon, Common::Point(365, 188), kSoundControlsTransparentColor);
		_compositeBuffer.transBlitFrom(_masterSoundIcon, Common::Point(266 - _masterSoundIcon.w / 2, 212 - _masterSoundIcon.h / 2), kSoundControlsTransparentColor);
		_compositeBuffer.transBlitFrom(_musicSoundIcon, Common::Point(333 - _musicSoundIcon.w / 2, 212 - _musicSoundIcon.h / 2), kSoundControlsTransparentColor);
		_compositeBuffer.transBlitFrom(_sfxSoundIcon, Common::Point(399 - _sfxSoundIcon.w / 2, 212 - _sfxSoundIcon.h / 2), kSoundControlsTransparentColor);

		buf = button == MASTER_LEFT_BUTTON ? _soundControlArrowLeft[1] : _soundControlArrowLeft[0];
		drawSpriteToBuffer(_compositeBuffer, buf, _masterVolumeLeftRect.left, _masterVolumeLeftRect.top, _masterVolumeLeftRect.width(), _masterVolumeLeftRect.height(), kSoundControlsTransparentColor);
		buf = button == MASTER_RIGHT_BUTTON ? _soundControlArrowRight[1] : _soundControlArrowRight[0];
		drawSpriteToBuffer(_compositeBuffer, buf, _masterVolumeRightRect.left, _masterVolumeRightRect.top, _masterVolumeRightRect.width(), _masterVolumeRightRect.height(), kSoundControlsTransparentColor);
		buf = button == SFX_LEFT_BUTTON ? _soundControlArrowLeft[1] : _soundControlArrowLeft[0];
		drawSpriteToBuffer(_compositeBuffer, buf, _sfxVolumeLeftRect.left, _sfxVolumeLeftRect.top, _sfxVolumeLeftRect.width(), _sfxVolumeLeftRect.height(), kSoundControlsTransparentColor);
		buf = button == SFX_RIGHT_BUTTON ? _soundControlArrowRight[1] : _soundControlArrowRight[0];
		drawSpriteToBuffer(_compositeBuffer, buf, _sfxVolumeRightRect.left, _sfxVolumeRightRect.top, _sfxVolumeRightRect.width(), _sfxVolumeRightRect.height(), kSoundControlsTransparentColor);
		buf = button == MUSIC_LEFT_BUTTON ? _soundControlArrowLeft[1] : _soundControlArrowLeft[0];
		drawSpriteToBuffer(_compositeBuffer, buf, _musicVolumeLeftRect.left, _musicVolumeLeftRect.top, _musicVolumeLeftRect.width(), _musicVolumeLeftRect.height(), kSoundControlsTransparentColor);
		buf = button == MUSIC_RIGHT_BUTTON ? _soundControlArrowRight[1] : _soundControlArrowRight[0];
		drawSpriteToBuffer(_compositeBuffer, buf, _musicVolumeRightRect.left, _musicVolumeRightRect.top, _musicVolumeRightRect.width(), _musicVolumeRightRect.height(), kSoundControlsTransparentColor);
	}
}

Pelrock::MenuManager::~MenuManager() {
	_mainMenu.free();
	_compositeBuffer.free();
	delete[] _questionMark[0];
	delete[] _questionMark[1];
	delete[] _inventoryLeftArrow[0];
	delete[] _inventoryLeftArrow[1];
	delete[] _inventoryRightArrow[0];
	delete[] _inventoryRightArrow[1];
}

} // End of namespace Pelrock
