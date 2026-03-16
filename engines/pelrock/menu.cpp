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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "engines/metaengine.h"
#include "graphics/paletteman.h"
#include "graphics/thumbnail.h"

#include "menu.h"
#include "pelrock/menu.h"
#include "pelrock/offsets.h"
#include "pelrock/pelrock.h"
#include "pelrock/util.h"

namespace Pelrock {

static const char *inventorySounds[113] = {

	"HOJASZZZ.SMP", // 0
	"11ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"GLASS1ZZ.SMP",
	"11ZZZZZZ.SMP",
	"ELEC3ZZZ.SMP",
	"REMATERL.SMP",
	"81ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"SSSHTZZZ.SMP", // 10
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP", // 20
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP", // 30
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP", // 40
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP", // 50
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"BOTEZZZZ.SMP", // 60
	"BOTEZZZZ.SMP",
	"BOTEZZZZ.SMP",
	"BELCHZZZ.SMP",
	"BEAMZZZZ.SMP",
	"ELVIS1ZZ.SMP",
	"CAT_1ZZZ.SMP",
	"BOOOOOIZ.SMP",
	"DISCOSZZ.SMP",
	"MONORLZZ.SMP",
	"11ZZZZZZ.SMP", // 70
	"11ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"CARACOLA.SMP",
	"11ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"WATER_2Z.SMP",
	"11ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"EEEEKZZZ.SMP",
	"REMATERL.SMP", // 80 - Rematerialize
	"11ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"ELVIS1ZZ.SMP",
	"RIMSHOTZ.SMP",
	"11ZZZZZZ.SMP",
	"WATER_2Z.SMP",
	"MOTOSZZZ.SMP",
	"HOJASZZZ.SMP",
	"TWANGZZZ.SMP",
	"11ZZZZZZ.SMP", // 90
	"QUAKE2ZZ.SMP",
	"11ZZZZZZ.SMP",
	"SORBOZZZ.SMP",
	"BOTEZZZZ.SMP",
	"ELVIS1ZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"HOJASZZZ.SMP",
	"11ZZZZZZ.SMP",
	"LLAVESZZ.SMP", // 100
	"HOJASZZZ.SMP",
	"11ZZZZZZ.SMP",
	"11ZZZZZZ.SMP",
	"EVLLAUGH.SMP",
	"11ZZZZZZ.SMP",
	"BURROLZZ.SMP",
	"11ZZZZZZ.SMP",
	"TWANGZZZ.SMP",
	"11ZZZZZZ.SMP",
	"TWANGZZZ.SMP", // 110
	"ELVIS1ZZ.SMP",
	"SEX3ZZZZ.SMP"
};

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

static const uint32 kQuestionMarkOffset = 3214046;
static const uint32 kInvLeftArrowOffset = 3215906;
static const uint32 kSoundControlOffset = 3037008;
static const uint32 kSoundMasterOffset = 2662588;
static const uint32 kSoundMusicOffset = 2664746;
static const uint32 kSoundSfxOffset = 2667140;
static const int kTransparentColor = 65;
static const int kSoundControlsTransparentColor = 195;
static const uint32 kCreditsBackgroundOffset = 3271454;
static const int16 kTextStartX = 227;
static const int16 kTextStartY = 191;
static const byte kNumberColor = 52;
static const byte kCursorChar = 0xDB;
static const byte kWhiteColor = 0x0F;

Pelrock::MenuManager::MenuManager(Graphics::Screen *screen, PelrockEventManager *events, ResourceManager *res, SoundManager *sound) : _screen(screen), _events(events), _res(res), _sound(sound) {
}

static int levelToMixerVolume(int level) {
	return (level * 255) / 14;
}

static int mixerVolumeToLevel(int volume) {
	return (volume * 14 + 127) / 255;
}

static float levelToScale(int level) {
	return 0.4f + (level / 14.0f) * 0.6f;
}

static void rebuildSoundIcon(Graphics::ManagedSurface &target, const byte *source, int level) {
	float s = levelToScale(level);
	int newW = MAX(1, (int)(66 * s));
	int newH = MAX(1, (int)(64 * s));
	Graphics::ManagedSurface temp;
	temp.create(66, 64, Graphics::PixelFormat::createFormatCLUT8());
	memcpy(temp.getPixels(), source, 66 * 64);
	Graphics::ManagedSurface *scaled = temp.scale(newW, newH, false);
	target.copyFrom(*scaled);
	delete scaled;
}

SoundMenuButton MenuManager::isSoundMenuButtonUnder(int x, int y) {
	if (_menuState != SOUND) {
		return NO_SOUND_BUTTON;
	}
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
	return NO_SOUND_BUTTON; // Default fallback
}

MainMenuButton MenuManager::isMainMenuButtonUnder(int x, int y) {
	if (_menuState != MAIN_MENU) {
		return NO_MAIN_BUTTON;
	}
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
	return NO_MAIN_BUTTON; // Default fallback
}

void MenuManager::checkMouseDown(int x, int y) {
	if (!_events->_leftMouseButton) {
		return;
	}
	SoundMenuButton b = isSoundMenuButtonUnder(x, y);
	switch (b) {
	case MASTER_LEFT_BUTTON:
		_masterVolumeLevel = MAX(0, _masterVolumeLevel - 1);
		_sound->setVolumeMaster(levelToMixerVolume(_masterVolumeLevel));
		rebuildSoundIcon(_masterSoundIcon, _soundControlMasterIcon, _masterVolumeLevel);
		break;
	case MASTER_RIGHT_BUTTON:
		_masterVolumeLevel = MIN(14, _masterVolumeLevel + 1);
		_sound->setVolumeMaster(levelToMixerVolume(_masterVolumeLevel));
		rebuildSoundIcon(_masterSoundIcon, _soundControlMasterIcon, _masterVolumeLevel);
		break;
	case SFX_LEFT_BUTTON:
		_sfxVolumeLevel = MAX(0, _sfxVolumeLevel - 1);
		_sound->setVolumeSfx(levelToMixerVolume(_sfxVolumeLevel));
		rebuildSoundIcon(_sfxSoundIcon, _soundControlSfxIcon, _sfxVolumeLevel);
		break;
	case SFX_RIGHT_BUTTON:
		_sfxVolumeLevel = MIN(14, _sfxVolumeLevel + 1);
		_sound->setVolumeSfx(levelToMixerVolume(_sfxVolumeLevel));
		rebuildSoundIcon(_sfxSoundIcon, _soundControlSfxIcon, _sfxVolumeLevel);
		break;
	case MUSIC_LEFT_BUTTON:
		_musicVolumeLevel = MAX(0, _musicVolumeLevel - 1);
		_sound->setVolumeMusic(levelToMixerVolume(_musicVolumeLevel));
		rebuildSoundIcon(_musicSoundIcon, _soundControlMusicIcon, _musicVolumeLevel);
		break;
	case MUSIC_RIGHT_BUTTON:
		_musicVolumeLevel = MIN(14, _musicVolumeLevel + 1);
		_sound->setVolumeMusic(levelToMixerVolume(_musicVolumeLevel));
		rebuildSoundIcon(_musicSoundIcon, _soundControlMusicIcon, _musicVolumeLevel);
		break;
	default:
		break;
	}
}

bool MenuManager::checkMouseClick(int x, int y) {

	switch (_menuState) {
	case MAIN_MENU: {
		return checkMainMenuMouse(x, y);
		break;
	}
	case SOUND: {
		checkSoundMenuClick(x, y);
		break;
	}
	case ORIGINAL_SAVE: {
		// Pagination arrows
		if (_savesUp.contains(x, y)) {
			if (_saveGamePage > 0) {
				_saveGamePage--;
				_editingSaveSlot = -1;
			}
			break;
		}
		if (_savesDown.contains(x, y)) {
			if ((_saveGamePage + 1) * 8 < 256) {
				_saveGamePage++;
				_editingSaveSlot = -1;
			}
			break;
		}
		// CANCEL
		if (_cancelarRect.contains(x, y)) {
			_editingSaveSlot = -1;
			backToMainMenu();
			break;
		}
		// Save slot click: begin (or switch) editing
		for (int i = 0; i < (int)_saveSlotRects.size(); i++) {
			if (_saveSlotRects[i].contains(x, y)) {
				int slot = _saveGamePage * 8 + i;
				_editingSaveSlot = slot;
				_editingName = _saveDescriptions[slot];
				break;
			}
		}
		break;
	}
	case ORIGINAL_LOAD: {
		// Pagination arrows
		if (_savesUp.contains(x, y)) {
			if (_saveGamePage > 0)
				_saveGamePage--;
			break;
		}
		if (_savesDown.contains(x, y)) {
			if ((_saveGamePage + 1) * 8 < 256)
				_saveGamePage++;
			break;
		}
		// CANCELAR
		if (_cancelarRect.contains(x, y)) {
			backToMainMenu();
			break;
		}
		// Save slot click: load if slot has a save
		for (int i = 0; i < (int)_saveSlotRects.size(); i++) {
			if (_saveSlotRects[i].contains(x, y)) {
				int slot = _saveGamePage * 8 + i;
				if (!_saveDescriptions[slot].empty()) {
					g_engine->loadGameState(slot);
					backToMainMenu();
					return true;
				}
				else {
					_menuState = MAIN_MENU;
					_menuText = _menuTexts[6];
				}
				break;
			}
		}
		break;
	}
	case EXIT_GAME: {
		break;
	}
	}
	return false;
}

void MenuManager::checkSoundMenuClick(int x, int y) {
	SoundMenuButton soundMenuButton = isSoundMenuButtonUnder(x, y);

	if (soundMenuButton != MUSIC_LEFT_BUTTON &&
		soundMenuButton != MUSIC_RIGHT_BUTTON &&
		soundMenuButton != SFX_LEFT_BUTTON &&
		soundMenuButton != SFX_RIGHT_BUTTON &&
		soundMenuButton != MASTER_LEFT_BUTTON &&
		soundMenuButton != MASTER_RIGHT_BUTTON) {
		backToMainMenu();
		return;
	}

	if (soundMenuButton == SFX_LEFT_BUTTON || soundMenuButton == SFX_RIGHT_BUTTON) {
		_sound->playSound("CAT_1ZZZ.SMP", -1);
	}
}

bool MenuManager::checkMainMenuMouse(int x, int y) {

	// Inventory items

	for (int i = 0; i < 4; i++) {
		Common::Rect itemRect = Common::Rect(_inventorySlots[i], 60, 60);
		if (itemRect.contains(x, y)) {
			selectInventoryItem(i);
			return false;
		}
	}

	// Buttons
	MainMenuButton mainMenuButton = isMainMenuButtonUnder(x, y);
	switch (mainMenuButton) {
	case QUESTION_MARK_BUTTON:
		_sound->playSound("56ZZZZZZ.SMP", 0);
		_events->_leftMouseClicked = false;
		showCredits();
		break;
	case INVENTORY_PREV_BUTTON:
		_sound->playSound("56ZZZZZZ.SMP", 0);
		if (_curInventoryPage > 0)
			_curInventoryPage--;
		break;
	case INVENTORY_NEXT_BUTTON:
		_sound->playSound("56ZZZZZZ.SMP", 0);
		if ((_curInventoryPage + 1) * 4 < g_engine->_state->inventoryItems.size())
			_curInventoryPage++;
		break;
	case SAVE_GAME_BUTTON:
		_sound->playSound("11ZZZZZZ.SMP", 0);
		if (ConfMan.getBool("original_menus") == true) {
			_saveGamePage = 0;
			_editingSaveSlot = -1;
			refreshSaveDescriptions();
			_menuState = ORIGINAL_SAVE;
			_menuText = Common::StringArray();
		} else {
			return g_engine->saveGameDialog();
		}
		break;
	case LOAD_GAME_BUTTON:
		_sound->playSound("11ZZZZZZ.SMP", 0);
		if (ConfMan.getBool("original_menus") == true) {
			_saveGamePage = 0;
			refreshSaveDescriptions();
			_menuState = ORIGINAL_LOAD;
			_menuText = Common::StringArray();
		} else {
			return g_engine->loadGameDialog();
		}
		break;
	case EXIT_MENU_BUTTON:
		_sound->playSound("11ZZZZZZ.SMP", 0);
		_menuState = EXIT_GAME;
		break;
	case SOUNDS_BUTTON:
		_sound->playSound("11ZZZZZZ.SMP", 0);
		_menuState = SOUND;
		_menuText = Common::StringArray();
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

void MenuManager::handleSaveMenuKey(Common::KeyCode key, uint16 ascii) {
	if (key == Common::KEYCODE_ESCAPE) {
		_editingSaveSlot = -1;
		backToMainMenu();
		return;
	}
	if (_editingSaveSlot < 0)
		return;

	if (key == Common::KEYCODE_RETURN || key == Common::KEYCODE_KP_ENTER) {
		// Commit save
		g_engine->saveGameState(_editingSaveSlot, _editingName);
		_saveDescriptions[_editingSaveSlot] = _editingName;
		_editingSaveSlot = -1;
		backToMainMenu();
	} else if (key == Common::KEYCODE_BACKSPACE) {
		if (!_editingName.empty())
			_editingName.deleteLastChar();
	} else if (ascii >= 32 && ascii < 256 && ascii != 127) {
		_editingName += (char)ascii;
	}
}

void MenuManager::backToMainMenu() {
	_menuState = MAIN_MENU;
	_menuText = _menuTexts[0];
}

void MenuManager::menuLoop() {

	// Save screenshot in case the user saves
	saveScreenshot();
	g_engine->_autoSaveAllowed = false;

	g_system->getPaletteManager()->setPalette(_mainMenuPalette, 0, 256);
	g_engine->changeCursor(DEFAULT);
	backToMainMenu();

	// Initialize volume levels from current settings
	_sfxVolumeLevel = mixerVolumeToLevel(_sound->getVolumeSfx());
	_musicVolumeLevel = mixerVolumeToLevel(_sound->getVolumeMusic());
	_masterVolumeLevel = mixerVolumeToLevel(_sound->getVolumeMaster());

	debug("Initial master volume level: %d", _masterVolumeLevel);
	debug("Initial SFX volume level: %d", _sfxVolumeLevel);
	debug("Initial Music volume level: %d", _musicVolumeLevel);
	_masterSoundIcon.create(66, 64, Graphics::PixelFormat::createFormatCLUT8());
	_sfxSoundIcon.create(66, 64, Graphics::PixelFormat::createFormatCLUT8());
	_musicSoundIcon.create(66, 64, Graphics::PixelFormat::createFormatCLUT8());

	rebuildSoundIcon(_masterSoundIcon, _soundControlMasterIcon, _masterVolumeLevel);
	rebuildSoundIcon(_sfxSoundIcon, _soundControlSfxIcon, _sfxVolumeLevel);
	rebuildSoundIcon(_musicSoundIcon, _soundControlMusicIcon, _musicVolumeLevel);

	while (!g_engine->shouldQuit()) {

		_events->pollEvent();
		checkMouseDown(_events->_mouseX, _events->_mouseY);
		if (_events->_leftMouseClicked) {
			_events->_leftMouseClicked = false;
			if (checkMouseClick(_events->_mouseX, _events->_mouseY)) {
				break;
			}
		}
		if (_events->_rightMouseClicked) {
			_events->_rightMouseClicked = false;
			// In original save/load sub-menus only CANCELAR / ESC can exit
			if (_menuState != ORIGINAL_SAVE && _menuState != ORIGINAL_LOAD) {
				break;
			}
		}

		// Keyboard handling
		if (_events->_lastKeyEvent != Common::KEYCODE_INVALID) {
			Common::KeyCode key = _events->_lastKeyEvent;
			uint16 ascii = _events->_lastKeyAscii;
			_events->_lastKeyEvent = Common::KEYCODE_INVALID;
			_events->_lastKeyAscii = 0;

			if (_menuState == ORIGINAL_SAVE) {
				handleSaveMenuKey(key, ascii);
			} else if (_menuState == ORIGINAL_LOAD) {
				if (key == Common::KEYCODE_ESCAPE) {
					backToMainMenu();
				}
			} else if (_menuState == EXIT_GAME) {
				if (key == Common::KEYCODE_s) {
					g_engine->quitGame();
				} else if (key == Common::KEYCODE_n) {
					backToMainMenu();
				}
			}
		}

		drawScreen();
		_screen->markAllDirty();
		_screen->update();
		g_system->delayMillis(10);
	}
	g_engine->_graphics->clearScreen();
	_events->_rightMouseClicked = false;
	_events->_leftMouseClicked = false;
	g_system->getPaletteManager()->setPalette(g_engine->_room->_roomPalette, 0, 256);
	g_engine->_autoSaveAllowed = true;
	cleanUp();
}

void MenuManager::saveScreenshot() {
	g_engine->_saveThumbnail.free();
	Graphics::createThumbnail(g_engine->_saveThumbnail);
}

void MenuManager::drawScreen() {
	_compositeBuffer.blitFrom(_mainMenu);
	if (showButtons)
		drawButtons();

	drawInventoryIcons();

	_screen->blitFrom(_compositeBuffer);
	byte defaultColor = 255;
	for (uint i = 0; _menuText.size() > i; i++) {
		g_engine->_graphics->drawColoredText(_screen, _menuText[i], kTextStartX, kTextStartY + (i * _textLineH), 200, defaultColor, g_engine->_smallFont);
	}

	drawText(g_engine->_smallFont, Common::String::format("%d,%d", _events->_mouseX, _events->_mouseY), 0, 0, 640, 13);
}

void MenuManager::drawInventoryIcons() {
	bool debugIcons = false;
	for (uint i = 0; i < 4; i++) {
		uint itemIndex = _curInventoryPage * 4 + i;
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

	_menuText = _menuTexts[0];
	alfred7.close();

	_textLineH = g_engine->_smallFont->getFontHeight();
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
	Common::Array<Common::StringArray> unprocessedMenuTexts;
	unprocessedMenuTexts = _res->processTextData(textBuffer, kMenuTextSize, true);
	_menuText = Common::StringArray();
	for (int i = 0; i < (int)unprocessedMenuTexts.size(); i++) {
		if (i == 1) {
			Common::StringArray emptyText;
			emptyText.push_back(unprocessedMenuTexts[i][0]);
			Common::StringArray loadText;
			loadText.push_back(unprocessedMenuTexts[i][1]);
			_menuTexts.push_back(emptyText);
			_menuTexts.push_back(loadText);
		} else {
			_menuTexts.push_back(unprocessedMenuTexts[i]);
		}
	}
	debug("Menu texts size after processing: %d", (int)_menuTexts.size());
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
	// always draw main buttons
	drawMainButtons();

	switch (_menuState) {
	case MAIN_MENU:
		break;
	case ORIGINAL_SAVE:
		drawSaves();
		break;
	case ORIGINAL_LOAD:
		drawSaves();
		break;
	case SOUND:
		drawSoundControls();
		break;
	case EXIT_GAME:
		drawConfirmation();
		break;
	}
}

void MenuManager::drawConfirmation() {
	_menuText = _menuTexts[5];
}

void MenuManager::refreshSaveDescriptions() {
	_saveDescriptions.clear();
	_saveDescriptions.resize(256);

	SaveStateList saves = g_engine->getMetaEngine()->listSaves(ConfMan.getActiveDomainName().c_str());
	for (const SaveStateDescriptor &desc : saves) {
		int slot = desc.getSaveSlot();
		if (slot >= 0 && slot < 256)
			_saveDescriptions[slot] = desc.getDescription();
	}
}
void MenuManager::drawSaves() {

	// Compute the area for the overlay
	const int startX = kTextStartX;
	const int startY = kTextStartY;
	const int overlayW = 216;
	const int numSlots = 8;

	// Headline: _menuTexts[1] = save, _menuTexts[2] = load
	Common::StringArray headline;
	if (_menuState == ORIGINAL_SAVE)
		headline = _menuTexts[3];
	else
		headline = _menuTexts[2];
	_menuText = headline;

	_saveSlotRects.clear();

	int y = startY + _textLineH;
	const Common::Point mousePos(_events->_mouseX, _events->_mouseY);
	for (int i = 0; i < numSlots; i++) {
		int slot = _saveGamePage * numSlots + i;
		Common::String slotText;
		Common::String slotNumber = Common::String::format("%02d ", slot + 1);
		int slotNumberWidth = g_engine->_smallFont->getStringWidth(slotNumber);

		Common::Rect slotRect(startX - 2, y - 1, startX + overlayW - 2, y + _textLineH);
		bool hovered = (_editingSaveSlot != slot) && slotRect.contains(mousePos);
		byte textColor = hovered ? 18 : kWhiteColor;

		if (_editingSaveSlot == slot) {
			// Show editing cursor
			_editingName.toUppercase();
			slotText = _editingName;
			textColor = 18; // highlight colour for active editing
		} else {
			const Common::String &desc = _saveDescriptions[slot];
			if (desc.empty())
				slotText = Common::String::format("%s", _menuTexts[1][0].c_str());
			else {
				slotText = desc;
				slotText.toUppercase();
			}
		}
		drawText(_compositeBuffer, g_engine->_smallFont, slotNumber, startX, y, overlayW, kNumberColor);
		drawText(_compositeBuffer, g_engine->_smallFont, slotText, startX + slotNumberWidth, y, overlayW - slotNumberWidth, textColor);

		if(_editingSaveSlot == slot) {
			// Draw cursor
			int cursorX = startX + slotNumberWidth + g_engine->_smallFont->getStringWidth(slotText);
			drawText(_compositeBuffer, g_engine->_smallFont, Common::String(kCursorChar), cursorX, y, overlayW - (cursorX - startX), kWhiteColor);
		}

		_saveSlotRects.push_back(slotRect);
		y += _textLineH;
	}

	// CANCEL row
	_cancelarRect = Common::Rect(startX - 2, y - 1, startX + overlayW - 2, y + _textLineH);
	byte cancelColor = _cancelarRect.contains(mousePos) ? 18 : kWhiteColor;
	Common::String cancelText = _menuTexts[4][0].substr(2, _menuTexts[4][0].size() - 2);
	drawText(_compositeBuffer, g_engine->_smallFont, cancelText, startX, y, overlayW, cancelColor);
}

void MenuManager::drawMainButtons() {
	MainMenuButton button = NO_MAIN_BUTTON;
	if (_events->_leftMouseButton != 0) {
		button = isMainMenuButtonUnder(_events->_mouseX, _events->_mouseY);
	}

	byte *buf;
	buf = button == QUESTION_MARK_BUTTON ? _questionMark[1] : _questionMark[0];
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
}

void MenuManager::drawSoundControls() {
	SoundMenuButton button = NO_SOUND_BUTTON;
	if (_events->_leftMouseButton != 0) {
		button = isSoundMenuButtonUnder(_events->_mouseX, _events->_mouseY);
	}
	byte *buf;
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
