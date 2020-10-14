/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "twine/menu.h"
#include "audio/mixer.h"
#include "backends/audiocd/audiocd.h"
#include "common/config-manager.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/hqrdepack.h"
#include "twine/interface.h"
#include "twine/keyboard.h"
#include "twine/menuoptions.h"
#include "twine/movements.h"
#include "twine/music.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

/** Main menu background image number
	Used when returning from credit sequence to redraw the main menu background image */
static const uint32 kPlasmaEffectFilesize = 262176;

/** Menu buttons width */
static const uint16 kMainMenuButtonWidth = 320;
/** Used to calculate the spanning between button and screen */
static const uint16 kMainMenuButtonSpan = 550;

/** Main menu types */
enum MainMenuType {
	kNewGame = 20,
	kContinueGame = 21,
	kOptions = 23,
	kQuit = 22,
	kBackground = 9999
};

/** Give up menu types */
enum GiveUpMenuType {
	kContinue = 28,
	kGiveUp = 27 // quit
};

/** Options menu types */
enum OptionsMenuType {
	kReturnGame = 15,
	kReturnMenu = 26,
	kVolume = 30,
	kSaveManage = 46,
	kAdvanced = 47
};

/** Volume menu types */
enum VolumeMenuType {
	kMusicVolume = 1,
	kSoundVolume = 2,
	kCDVolume = 3,
	kLineVolume = 4,
	kMasterVolume = 5
};

/** Main Menu Settings

	Used to create the game main menu. */
int16 Menu::MainMenuSettings[] = {
    0,   // Current loaded button (button number)
    4,   // Num of buttons
    200, // Buttons box height ( is used to calc the height where the first button will appear )
    0,   // unused
    0,
    20, // new game
    0,
    21, // continue game
    0,
    23, // options
    0,
    22, // quit
};

/** Give Up Menu Settings

	Used to create the in-game menu. */
int16 Menu::GiveUpMenuSettings[] = {
    0,   // Current loaded button (button number)
    2,   // Num of buttons
    240, // Buttons box height ( is used to calc the height where the first button will appear )
    0,   // unused
    0,
    28, // continue game
    0,
    27, // quit game
};

/** Give Up Menu Settings

	Used to create the in-game menu. This menu have one extra item to save the game */
int16 Menu::GiveUpMenuSettingsWithSave[] = {
    0,   // Current loaded button (button number)
    3,   // Num of buttons
    240, // Buttons box height ( is used to calc the height where the first button will appear )
    0,   // unused
    0,
    28, // continue game
    0,
    16, // save game
    0,
    27, // quit game
};

/** Options Menu Settings

	Used to create the options menu. */
int16 Menu::OptionsMenuSettings[] = {
    0, // Current loaded button (button number)
    4, // Num of buttons
    0, // Buttons box height ( is used to calc the height where the first button will appear )
    0, // unused
    0,
    24, // return to previous menu
    0,
    30, // volume settings
    0,
    46, // save game management
    0,
    47, // advanced options
};

/** Advanced Options Menu Settings

	Used to create the advanced options menu. */
int16 Menu::AdvOptionsMenuSettings[] = {
    0, // Current loaded button (button number)
    5, // Num of buttons
    0, // Buttons box height ( is used to calc the height where the first button will appear )
    0, // unused
    0,
    26, // return to main menu
    0,
    4, // aggressive mode (manual|auto)
    6,
    31, // Polygon detail (full|medium|low)
    7,
    32, // Shadows (all|character|no)
    8,
    33, // scenary zoon (on|off)
};

/** Save Game Management Menu Settings

	Used to create the save game management menu. */
int16 Menu::SaveManageMenuSettings[] = {
    0, // Current loaded button (button number)
    3, // Num of buttons
    0, // Buttons box height ( is used to calc the height where the first button will appear )
    0, // unused
    0,
    26, // return to main menu
    0,
    41, // copy saved game
    0,
    45, // delete saved game
};

/** Volume Menu Settings

	Used to create the volume menu. */
int16 Menu::VolumeMenuSettings[] = {
    0, // Current loaded button (button number)
    7, // Num of buttons
    0, // Buttons box height ( is used to calc the height where the first button will appear )
    0, // unused
    0,
    26, // return to main menu
    1,
    10, // music volume
    2,
    11, // sfx volume
    3,
    12, // cd volume
    4,
    13, // line-in volume
    5,
    14, // master volume
    0,
    16, // save parameters
};

#define PLASMA_WIDTH 320
#define PLASMA_HEIGHT 50
#define SCREEN_W 640

Menu::Menu(TwinEEngine *engine) : _engine(engine) {}

void Menu::plasmaEffectRenderFrame() {
	int16 c;
	int32 i, j;
	uint8 *dest;
	uint8 *src;

	for (j = 1; j < PLASMA_HEIGHT - 1; j++) {
		for (i = 1; i < PLASMA_WIDTH - 1; i++) {
			/*Here we calculate the average of all 8 neighbour pixel values*/

			c = plasmaEffectPtr[(i - 1) + (j - 1) * PLASMA_WIDTH];  //top-left
			c += plasmaEffectPtr[(i + 0) + (j - 1) * PLASMA_WIDTH]; //top
			c += plasmaEffectPtr[(i + 1) + (j - 1) * PLASMA_WIDTH]; //top-right

			c += plasmaEffectPtr[(i - 1) + (j + 0) * PLASMA_WIDTH]; //left
			c += plasmaEffectPtr[(i + 1) + (j + 0) * PLASMA_WIDTH]; //right

			c += plasmaEffectPtr[(i - 1) + (j + 1) * PLASMA_WIDTH]; // bottom-left
			c += plasmaEffectPtr[(i + 0) + (j + 1) * PLASMA_WIDTH]; // bottom
			c += plasmaEffectPtr[(i + 1) + (j + 1) * PLASMA_WIDTH]; // bottom-right

			c = (c >> 3) | ((c & 0x0003) << 13); /* And the 2 least significant bits are used as a
              randomizing parameter for statistically fading the flames */

			if (!(c & 0x6500) &&
			    (j >= (PLASMA_HEIGHT - 4) || c > 0)) {
				c--; /*fade this pixel*/
			}

			/* plot the pixel using the calculated color */
			plasmaEffectPtr[i + (PLASMA_HEIGHT + j) * PLASMA_WIDTH] = (uint8)c;
		}
	}

	// flip the double-buffer while scrolling the effect vertically:
	dest = plasmaEffectPtr;
	src = plasmaEffectPtr + (PLASMA_HEIGHT + 1) * PLASMA_WIDTH;
	for (i = 0; i < PLASMA_HEIGHT * PLASMA_WIDTH; i++)
		*(dest++) = *(src++);
}

void Menu::processPlasmaEffect(int32 top, int32 color) {
	uint8 *in;
	uint8 *out;
	int32 i, j, target;
	uint8 c;
	uint8 max_value = color + 15;

	plasmaEffectRenderFrame();

	in = plasmaEffectPtr + 5 * PLASMA_WIDTH;
	out = _engine->frontVideoBuffer + _engine->screenLookupTable[top];

	for (i = 0; i < 25; i++) {
		for (j = 0; j < kMainMenuButtonWidth; j++) {
			c = in[i * kMainMenuButtonWidth + j] / 2 + color;
			if (c > max_value)
				c = max_value;

			/* 2x2 squares sharing the same pixel color: */
			target = 2 * (i * SCREEN_W + j);
			out[target] = c;
			out[target + 1] = c;
			out[target + SCREEN_W] = c;
			out[target + SCREEN_W + 1] = c;
		}
	}
}

void Menu::drawBox(int32 left, int32 top, int32 right, int32 bottom) {
	_engine->_interface->drawLine(left, top, right, top, 79);         // top line
	_engine->_interface->drawLine(left, top, left, bottom, 79);       // left line
	_engine->_interface->drawLine(right, ++top, right, bottom, 73);   // right line
	_engine->_interface->drawLine(++left, bottom, right, bottom, 73); // bottom line
}

void Menu::drawButtonGfx(int32 width, int32 topheight, int32 id, int32 value, int32 mode) {
	/*
	 * int CDvolumeRemaped; int musicVolumeRemaped; int masterVolumeRemaped; int lineVolumeRemaped;
	 * int waveVolumeRemaped;
	 */

	int32 left = width - kMainMenuButtonSpan / 2;
	int32 right = width + kMainMenuButtonSpan / 2;

	// topheigh is the center Y pos of the button
	int32 top = topheight - 25;    // this makes the button be 50 height
	int32 bottom = topheight + 25; // ||
	int32 bottom2 = bottom;

	if (mode != 0) {
		if (id <= kMasterVolume && id >= kMusicVolume) {
			int32 newWidth = 0;
			switch (id) {
			case kMusicVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType);
				newWidth = _engine->_screens->crossDot(left, right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			case kSoundVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kSFXSoundType);
				newWidth = _engine->_screens->crossDot(left, right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			case kCDVolume: {
				const AudioCDManager::Status status = _engine->_system->getAudioCDManager()->getStatus();
				newWidth = _engine->_screens->crossDot(left, right, Audio::Mixer::kMaxMixerVolume, status.volume);
				break;
			}
			case kLineVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kSpeechSoundType);
				newWidth = _engine->_screens->crossDot(left, right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			case kMasterVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kPlainSoundType);
				newWidth = _engine->_screens->crossDot(left, right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			};

			processPlasmaEffect(top, 80);
			if (!(_engine->getRandomNumber() % 5)) {
				plasmaEffectPtr[_engine->getRandomNumber() % 140 * 10 + 1900] = 255;
			}
			_engine->_interface->drawSplittedBox(newWidth, top, right, bottom, 68);
		} else {
			processPlasmaEffect(top, 64);
			if (!(_engine->getRandomNumber() % 5)) {
				plasmaEffectPtr[_engine->getRandomNumber() % 320 * 10 + 6400] = 255;
			}
		}
	} else {
		_engine->_interface->blitBox(left, top, right, bottom, (int8 *)_engine->workVideoBuffer, left, top, (int8 *)_engine->frontVideoBuffer);
		_engine->_interface->drawTransparentBox(left, top, right, bottom2, 4);
	}

	drawBox(left, top, right, bottom);

	_engine->_text->setFontColor(15);
	_engine->_text->setFontParameters(2, 8);
	char dialText[256];
	_engine->_text->getMenuText(value, dialText, sizeof(dialText));
	const int32 textSize = _engine->_text->getTextSize(dialText);
	_engine->_text->drawText(width - (textSize / 2), topheight - 18, dialText);

	// TODO: make volume buttons

	_engine->copyBlockPhys(left, top, right, bottom);
}

void Menu::drawButton(const int16 *menuSettings, int32 mode) {
	const int16 *localData = menuSettings;

	int32 buttonNumber = *localData;
	localData += 1;
	int32 maxButton = *localData;
	localData += 1;
	int32 topHeight = *localData;
	localData += 2;

	if (topHeight == 0) {
		topHeight = 35;
	} else {
		topHeight = topHeight - (((maxButton - 1) * 6) + ((maxButton)*50)) / 2;
	}

	if (maxButton <= 0) {
		return;
	}

	uint8 currentButton = 0;

	do {
		// get menu item settings
		uint8 menuItemId = (uint8)*localData;
		localData += 1;
		// applicable for sound menus, to save the volume/sound bar
		uint16 menuItemValue = *localData;
		localData += 1;
		if (mode != 0) {
			if (currentButton == buttonNumber) {
				drawButtonGfx(kMainMenuButtonWidth, topHeight, menuItemId, menuItemValue, 1);
			}
		} else {
			if (currentButton == buttonNumber) {
				drawButtonGfx(kMainMenuButtonWidth, topHeight, menuItemId, menuItemValue, 1);
			} else {
				drawButtonGfx(kMainMenuButtonWidth, topHeight, menuItemId, menuItemValue, 0);
			}
		}

		currentButton++;
		topHeight += 56; // increase button top height

		// slow down the CPU
		_engine->_system->delayMillis(1);
	} while (currentButton < maxButton);
}

int32 Menu::processMenu(int16 *menuSettings) {
	int16 *localData = menuSettings;
	int16 currentButton = 0; // localData[0];
	int32 buttonReleased = 1;
	int32 musicChanged = 0;
	int32 buttonNeedRedraw = 1;
	int32 numEntry = localData[1];
	int32 localTime = _engine->lbaTime;
	int32 maxButton = numEntry - 1;

	_engine->readKeys();

	do {
		// if its on main menu
		if (localData == MainMenuSettings) {
			if (_engine->lbaTime - localTime > 11650) {
				return kBackground;
			}
			if (_engine->_keyboard.skipIntro == 46) {
				if (_engine->_keyboard.skippedKey != 32) {
					return kBackground;
				}
			}
		}

		if (_engine->_keyboard.pressedKey == 0) {
			buttonReleased = 1;
		}

		if (buttonReleased) {
			_engine->_keyboard.key = _engine->_keyboard.pressedKey;

			if (((uint8)_engine->_keyboard.key & 2)) { // on arrow key down
				currentButton++;
				if (currentButton == numEntry) { // if current button is the last, than next button is the first
					currentButton = 0;
				}
				buttonNeedRedraw = 1;
				buttonReleased = 0;
			}

			if (((uint8)_engine->_keyboard.key & 1)) { // on arrow key up
				currentButton--;
				if (currentButton < 0) { // if current button is the first, than previous button is the last
					currentButton = maxButton;
				}
				buttonNeedRedraw = 1;
				buttonReleased = 0;
			}

			if (*(localData + 8) <= 5) {                         // if its a volume button
				int16 id = *(localData + currentButton * 2 + 4); // get button parameters from settings array

				switch (id) {
				case kMusicVolume: {
					int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType);
					if (((uint8)_engine->_keyboard.key & 4)) { // on arrow key left
						volume -= 4;
					}
					if (((uint8)_engine->_keyboard.key & 8)) { // on arrow key right
						volume += 4;
					}
					_engine->_music->musicVolume(volume);
					break;
				}
				case kSoundVolume: {
					int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
					if (((uint8)_engine->_keyboard.key & 4)) { // on arrow key left
						volume -= 4;
					}
					if (((uint8)_engine->_keyboard.key & 8)) { // on arrow key right
						volume += 4;
					}
					_engine->_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
					break;
				}
				case kCDVolume: {
					AudioCDManager::Status status = _engine->_system->getAudioCDManager()->getStatus();
					if (((uint8)_engine->_keyboard.key & 4)) { // on arrow key left
						status.volume -= 4;
					}
					if (((uint8)_engine->_keyboard.key & 8)) { // on arrow key right
						status.volume += 4;
					}
					_engine->_system->getAudioCDManager()->setVolume(status.volume);
					break;
				}
				case kLineVolume: {
					int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
					if (((uint8)_engine->_keyboard.key & 4)) { // on arrow key left
						volume -= 4;
					}
					if (((uint8)_engine->_keyboard.key & 8)) { // on arrow key right
						volume += 4;
					}
					_engine->_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume);
					break;
				}
				case kMasterVolume: {
					int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kPlainSoundType);
					if (((uint8)_engine->_keyboard.key & 4)) { // on arrow key left
						volume -= 4;
					}
					if (((uint8)_engine->_keyboard.key & 8)) { // on arrow key right
						volume += 4;
					}
					_engine->_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, volume);
					break;
				}
				default:
					break;
				}
			}
		}

		if (buttonNeedRedraw == 1) {
			*localData = currentButton;

			drawButton(localData, 0); // current button
			do {
				_engine->readKeys();
				drawButton(localData, 1);
			} while (_engine->_keyboard.pressedKey == 0 && _engine->_keyboard.skippedKey == 0 && _engine->_keyboard.skipIntro == 0);
			buttonNeedRedraw = 0;
		} else {
			if (musicChanged) {
				// TODO: update volume settings
			}

			buttonNeedRedraw = 0;
			drawButton(localData, 1);
			_engine->readKeys();
			// WARNING: this is here to prevent a fade bug while quit the menu
			_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
		}
	} while (!(_engine->_keyboard.skippedKey & 2) && !(_engine->_keyboard.skippedKey & 1));

	currentButton = *(localData + 5 + currentButton * 2); // get current browsed button

	_engine->readKeys();

	return currentButton;
}

int32 Menu::advoptionsMenu() {
	int32 ret = 0;

	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

	do {
		switch (processMenu(AdvOptionsMenuSettings)) {
		case kReturnMenu: {
			ret = 1; // quit option menu
			break;
		}
		//TODO: add other options
		default:
			break;
		}
	} while (ret != 1);

	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();

	return 0;
}

int32 Menu::savemanageMenu() {
	int32 ret = 0;

	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

	do {
		switch (processMenu(SaveManageMenuSettings)) {
		case kReturnMenu: {
			ret = 1; // quit option menu
			break;
		}
		//TODO: add other options
		default:
			break;
		}
	} while (ret != 1);

	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();

	return 0;
}

int32 Menu::volumeMenu() {
	int32 ret = 0;

	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

	do {
		switch (processMenu(VolumeMenuSettings)) {
		case kReturnMenu: {
			ret = 1; // quit option menu
			break;
		}
		//TODO: add other options
		default:
			break;
		}
	} while (ret != 1);

	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();

	return 0;
}

int32 Menu::optionsMenu() {
	int32 ret = 0;

	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

	_engine->_sound->stopSamples();
	//_engine->_music->playCDtrack(9);

	do {
		switch (processMenu(OptionsMenuSettings)) {
		case kReturnGame:
		case kReturnMenu: {
			ret = 1; // quit option menu
			break;
		}
		case kVolume: {
			_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
			_engine->flip();
			volumeMenu();
			break;
		}
		case kSaveManage: {
			_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
			_engine->flip();
			savemanageMenu();
			break;
		}
		case kAdvanced: {
			_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
			_engine->flip();
			advoptionsMenu();
			break;
		}
		default:
			break;
		}
	} while (ret != 1);

	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();

	return 0;
}

void Menu::mainMenu() {
	_engine->_sound->stopSamples();

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	// load menu effect file only once
	plasmaEffectPtr = (uint8 *)malloc(kPlasmaEffectFilesize);
	memset(plasmaEffectPtr, 0, kPlasmaEffectFilesize);
	_engine->_hqrdepack->hqrGetEntry(plasmaEffectPtr, Resources::HQR_RESS_FILE, RESSHQR_PLASMAEFFECT);

	while (!_engine->cfgfile.Quit) {
		_engine->_text->initTextBank(0);

		_engine->_music->playTrackMusic(9); // LBA's Theme
		_engine->_sound->stopSamples();

		switch (processMenu(MainMenuSettings)) {
		case kNewGame: {
			_engine->_menuOptions->newGameMenu();
			break;
		}
		case kContinueGame: {
			_engine->_menuOptions->continueGameMenu();
			break;
		}
		case kOptions: {
			_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
			_engine->flip();
			OptionsMenuSettings[5] = kReturnMenu;
			optionsMenu();
			break;
		}
		case kQuit: {
			_engine->cfgfile.Quit = 1;
			break;
		}
		case kBackground: {
			_engine->_screens->loadMenuImage();
		}
		}
		_engine->_system->delayMillis(1000 / _engine->cfgfile.Fps);
	}
}

int32 Menu::giveupMenu() {
	//int32 saveLangue=0;
	int32 menuId;
	int16 *localMenu;

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
	_engine->_sound->pauseSamples();

	if (_engine->cfgfile.UseAutoSaving == 1)
		localMenu = GiveUpMenuSettings;
	else
		localMenu = GiveUpMenuSettingsWithSave;

	do {
		//saveLangue = languageCD1;
		//languageCD1 = 0;
		_engine->_text->initTextBank(0);

		menuId = processMenu(localMenu);

		//languageCD1 = saveLangue;

		_engine->_text->initTextBank(_engine->_text->currentTextBank + 3);

		_engine->_system->delayMillis(1000 / _engine->cfgfile.Fps);
	} while (menuId != kGiveUp && menuId != kContinue);

	if (menuId == kGiveUp) {
		_engine->_sound->stopSamples();
		return 1;
	}

	_engine->_sound->resumeSamples();
	return 0;
}

void Menu::drawInfoMenu(int16 left, int16 top) {
	int32 boxLeft, boxTop, boxRight, boxBottom;
	int32 newBoxLeft, newBoxLeft2, i;

	_engine->_interface->resetClip();
	drawBox(left, top, left + 450, top + 80);
	_engine->_interface->drawSplittedBox(left + 1, top + 1, left + 449, top + 79, 0);

	newBoxLeft2 = left + 9;

	_engine->_grid->drawSprite(0, newBoxLeft2, top + 13, _engine->_actor->spriteTable[SPRITEHQR_LIFEPOINTS]);

	boxRight = left + 325;
	newBoxLeft = left + 25;
	boxLeft = _engine->_screens->crossDot(newBoxLeft, boxRight, 50, _engine->_scene->sceneHero->life);

	boxTop = top + 10;
	boxBottom = top + 25;
	_engine->_interface->drawSplittedBox(newBoxLeft, boxTop, boxLeft, boxBottom, 91);
	drawBox(left + 25, top + 10, left + 324, top + 10 + 14);

	if (!_engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] && _engine->_gameState->gameFlags[InventoryItems::kiTunic]) {
		_engine->_grid->drawSprite(0, newBoxLeft2, top + 36, _engine->_actor->spriteTable[SPRITEHQR_MAGICPOINTS]);
		if (_engine->_gameState->magicLevelIdx > 0) {
			_engine->_interface->drawSplittedBox(newBoxLeft, top + 35, _engine->_screens->crossDot(newBoxLeft, boxRight, 80, _engine->_gameState->inventoryMagicPoints), top + 50, 75);
		}
		drawBox(left + 25, top + 35, left + _engine->_gameState->magicLevelIdx * 80 + 20, top + 35 + 15);
	}

	boxLeft = left + 340;

	/** draw coin sprite */
	_engine->_grid->drawSprite(0, boxLeft, top + 15, _engine->_actor->spriteTable[SPRITEHQR_KASHES]);
	_engine->_text->setFontColor(155);
	Common::String inventoryNumKashes = Common::String::format("%d", _engine->_gameState->inventoryNumKashes);
	_engine->_text->drawText(left + 370, top + 5, inventoryNumKashes.c_str());

	/** draw key sprite */
	_engine->_grid->drawSprite(0, boxLeft, top + 55, _engine->_actor->spriteTable[SPRITEHQR_KEY]);
	_engine->_text->setFontColor(155);
	Common::String inventoryNumKeys = Common::String::format("%d", _engine->_gameState->inventoryNumKeys);
	_engine->_text->drawText(left + 370, top + 40, inventoryNumKeys.c_str());

	// prevent
	if (_engine->_gameState->inventoryNumLeafs > _engine->_gameState->inventoryNumLeafsBox) {
		_engine->_gameState->inventoryNumLeafs = _engine->_gameState->inventoryNumLeafsBox;
	}

	// Clover leaf boxes
	for (i = 0; i < _engine->_gameState->inventoryNumLeafsBox; i++) {
		_engine->_grid->drawSprite(0, _engine->_screens->crossDot(left + 25, left + 325, 10, i), top + 58, _engine->_actor->spriteTable[SPRITEHQR_CLOVERLEAFBOX]);
	}

	// Clover leafs
	for (i = 0; i < _engine->_gameState->inventoryNumLeafs; i++) {
		_engine->_grid->drawSprite(0, _engine->_screens->crossDot(left + 25, left + 325, 10, i) + 2, top + 60, _engine->_actor->spriteTable[SPRITEHQR_CLOVERLEAF]);
	}

	_engine->copyBlockPhys(left, top, left + 450, top + 135);
}

void Menu::drawBehaviour(HeroBehaviourType behaviour, int32 angle, int16 cantDrawBox) {
	int32 boxLeft = behaviour * 110 + 110;
	int32 boxRight = boxLeft + 99;
	int32 boxTop = 110;
	int32 boxBottom = 229;

	uint8 *currentAnim = _engine->_animations->animTable[_engine->_actor->heroAnimIdx[behaviour]];
	int32 currentAnimState = behaviourAnimState[behaviour];

	if (_engine->_animations->setModelAnimation(currentAnimState, currentAnim, behaviourEntity, &behaviourAnimData[behaviour])) {
		currentAnimState++; // keyframe
		if (currentAnimState >= _engine->_animations->getNumKeyframes(currentAnim)) {
			currentAnimState = _engine->_animations->getStartKeyframe(currentAnim);
		}
		behaviourAnimState[behaviour] = currentAnimState;
	}

	if (cantDrawBox == 0) {
		drawBox(boxLeft - 1, boxTop - 1, boxRight + 1, boxBottom + 1);
	}

	_engine->_interface->saveClip();
	_engine->_interface->resetClip();

	if (behaviour != _engine->_actor->heroBehaviour) { // unselected
		_engine->_interface->drawSplittedBox(boxLeft, boxTop, boxRight, boxBottom, 0);
	} else { // selected
		_engine->_interface->drawSplittedBox(boxLeft, boxTop, boxRight, boxBottom, 69);

		// behaviour menu title
		_engine->_interface->drawSplittedBox(110, 239, 540, 279, 0);
		drawBox(110, 239, 540, 279);

		_engine->_text->setFontColor(15);

		char dialText[256];
		if (_engine->_actor->heroBehaviour == 2 && _engine->_actor->autoAgressive == 1) {
			_engine->_text->getMenuText(4, dialText, sizeof(dialText));
		} else {
			_engine->_text->getMenuText(_engine->_actor->heroBehaviour, dialText, sizeof(dialText));
		}

		_engine->_text->drawText((650 - _engine->_text->getTextSize(dialText)) / 2, 240, dialText);
	}

	_engine->_renderer->renderBehaviourModel(boxLeft, boxTop, boxRight, boxBottom, -600, angle, behaviourEntity);

	_engine->copyBlockPhys(boxLeft, boxTop, boxRight, boxBottom);
	_engine->copyBlockPhys(110, 239, 540, 279);

	_engine->_interface->loadClip();
}

void Menu::drawBehaviourMenu(int32 angle) {
	drawBox(100, 100, 550, 290);
	_engine->_interface->drawTransparentBox(101, 101, 549, 289, 2);

	_engine->_animations->setAnimAtKeyframe(behaviourAnimState[kNormal], _engine->_animations->animTable[_engine->_actor->heroAnimIdx[kNormal]], behaviourEntity, &behaviourAnimData[kNormal]);
	drawBehaviour(kNormal, angle, 0);

	_engine->_animations->setAnimAtKeyframe(behaviourAnimState[kAthletic], _engine->_animations->animTable[_engine->_actor->heroAnimIdx[kAthletic]], behaviourEntity, &behaviourAnimData[kAthletic]);
	drawBehaviour(kAthletic, angle, 0);

	_engine->_animations->setAnimAtKeyframe(behaviourAnimState[kAggressive], _engine->_animations->animTable[_engine->_actor->heroAnimIdx[kAggressive]], behaviourEntity, &behaviourAnimData[kAggressive]);
	drawBehaviour(kAggressive, angle, 0);

	_engine->_animations->setAnimAtKeyframe(behaviourAnimState[kDiscrete], _engine->_animations->animTable[_engine->_actor->heroAnimIdx[kDiscrete]], behaviourEntity, &behaviourAnimData[kDiscrete]);
	drawBehaviour(kDiscrete, angle, 0);

	drawInfoMenu(100, 300);

	_engine->copyBlockPhys(100, 100, 550, 290);
}

void Menu::processBehaviourMenu() {
	int32 tmpLanguageCD;
	int32 tmpTextBank;
	int32 tmpTime;

	if (_engine->_actor->heroBehaviour == kProtoPack) {
		_engine->_sound->stopSamples();
		_engine->_actor->setBehaviour(kNormal);
	}

	behaviourEntity = _engine->_actor->bodyTable[_engine->_scene->sceneHero->entity];

	_engine->_actor->heroAnimIdx[kNormal] = _engine->_actor->heroAnimIdxNORMAL;
	_engine->_actor->heroAnimIdx[kAthletic] = _engine->_actor->heroAnimIdxATHLETIC;
	_engine->_actor->heroAnimIdx[kAggressive] = _engine->_actor->heroAnimIdxAGGRESSIVE;
	_engine->_actor->heroAnimIdx[kDiscrete] = _engine->_actor->heroAnimIdxDISCRETE;

	_engine->_movements->setActorAngleSafe(_engine->_scene->sceneHero->angle, _engine->_scene->sceneHero->angle - 256, 50, &moveMenu);

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	tmpLanguageCD = _engine->cfgfile.LanguageCDId;
	_engine->cfgfile.LanguageCDId = 0;

	tmpTextBank = _engine->_text->currentTextBank;
	_engine->_text->currentTextBank = -1;

	_engine->_text->initTextBank(0);

	drawBehaviourMenu(_engine->_scene->sceneHero->angle);

	HeroBehaviourType tmpHeroBehaviour = _engine->_actor->heroBehaviour;

	_engine->_animations->setAnimAtKeyframe(behaviourAnimState[_engine->_actor->heroBehaviour], _engine->_animations->animTable[_engine->_actor->heroAnimIdx[_engine->_actor->heroBehaviour]], behaviourEntity, &behaviourAnimData[_engine->_actor->heroBehaviour]);

	_engine->readKeys();

	tmpTime = _engine->lbaTime;

	while (_engine->_keyboard.skippedKey & 4 || (_engine->_keyboard.skipIntro >= 59 && _engine->_keyboard.skipIntro <= 62)) {
		_engine->readKeys();
		_engine->_keyboard.key = _engine->_keyboard.pressedKey;

		int heroBehaviour = (int)_engine->_actor->heroBehaviour;
		if (_engine->_keyboard.key & 8) {
			heroBehaviour++;
		}

		if (_engine->_keyboard.key & 4) {
			heroBehaviour--;
		}

		if (heroBehaviour < kNormal) {
			heroBehaviour = kDiscrete;
		}

		if (heroBehaviour >= kProtoPack) {
			heroBehaviour = kNormal;
		}

		_engine->_actor->heroBehaviour = (HeroBehaviourType)heroBehaviour;

		if (tmpHeroBehaviour != _engine->_actor->heroBehaviour) {
			drawBehaviour(tmpHeroBehaviour, _engine->_scene->sceneHero->angle, 1);
			tmpHeroBehaviour = _engine->_actor->heroBehaviour;
			_engine->_movements->setActorAngleSafe(_engine->_scene->sceneHero->angle, _engine->_scene->sceneHero->angle - 256, 50, &moveMenu);
			_engine->_animations->setAnimAtKeyframe(behaviourAnimState[_engine->_actor->heroBehaviour], _engine->_animations->animTable[_engine->_actor->heroAnimIdx[_engine->_actor->heroBehaviour]], behaviourEntity, &behaviourAnimData[_engine->_actor->heroBehaviour]);

			while (_engine->_keyboard.pressedKey) {
				_engine->readKeys();
				if (_engine->shouldQuit()) {
					break;
				}
				drawBehaviour(_engine->_actor->heroBehaviour, -1, 1);
			}
		}

		drawBehaviour(_engine->_actor->heroBehaviour, -1, 1);

		_engine->_system->delayMillis(1000 / 50);
		_engine->lbaTime++;
	}

	_engine->lbaTime = tmpTime;

	_engine->_actor->setBehaviour(_engine->_actor->heroBehaviour);
	_engine->_gameState->initEngineProjections();

	_engine->_text->currentTextBank = tmpTextBank;
	_engine->_text->initTextBank(_engine->_text->currentTextBank + 3);

	_engine->cfgfile.LanguageCDId = tmpLanguageCD;
}

void Menu::drawMagicItemsBox(int32 left, int32 top, int32 right, int32 bottom, int32 color) { // Rect
	_engine->_interface->drawLine(left, top, right, top, color);                              // top line
	_engine->_interface->drawLine(left, top, left, bottom, color);                            // left line
	_engine->_interface->drawLine(right, ++top, right, bottom, color);                        // right line
	_engine->_interface->drawLine(++left, bottom, right, bottom, color);                      // bottom line
}

void Menu::drawItem(int32 item) {
	int32 itemX = (item / 4) * 85 + 64;
	int32 itemY = (item & 3) * 75 + 52;

	int32 left = itemX - 37;
	int32 right = itemX + 37;
	int32 top = itemY - 32;
	int32 bottom = itemY + 32;

	_engine->_interface->drawSplittedBox(left, top, right, bottom,
	                                     inventorySelectedItem == item ? inventorySelectedColor : 0);

	if (_engine->_gameState->gameFlags[item] && !_engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] && item < NUM_INVENTORY_ITEMS) {
		_engine->_renderer->prepareIsoModel(_engine->_resources->inventoryTable[item]);
		itemAngle[item] += 8;
		_engine->_renderer->renderInventoryItem(itemX, itemY, _engine->_resources->inventoryTable[item], itemAngle[item], 15000);

		if (item == 15) { // has GAS
			_engine->_text->setFontColor(15);
			Common::String inventoryNumGas = Common::String::format("%d", _engine->_gameState->inventoryNumGas);
			_engine->_text->drawText(left + 3, top + 32, inventoryNumGas.c_str());
		}
	}

	drawBox(left, top, right, bottom);
	_engine->copyBlockPhys(left, top, right, bottom);
}

void Menu::drawInventoryItems() {
	int32 item;

	_engine->_interface->drawTransparentBox(17, 10, 622, 320, 4);
	drawBox(17, 10, 622, 320);
	drawMagicItemsBox(110, 18, 188, 311, 75);
	_engine->copyBlockPhys(17, 10, 622, 320);

	for (item = 0; item < NUM_INVENTORY_ITEMS; item++) {
		drawItem(item);
	}
}

void Menu::processInventoryMenu() {
	int32 di = 1;
	int32 prevSelectedItem, tmpLanguageCD, bx, tmpAlphaLight, tmpBetaLight;

	tmpAlphaLight = _engine->_scene->alphaLight;
	tmpBetaLight = _engine->_scene->betaLight;

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	_engine->_renderer->setLightVector(896, 950, 0);

	inventorySelectedColor = 68;

	if (_engine->_gameState->inventoryNumLeafs > 0) {
		_engine->_gameState->gameFlags[InventoryItems::kiCloverLeaf] = 1;
	}

	drawInventoryItems();

	tmpLanguageCD = _engine->cfgfile.LanguageCDId;
	_engine->cfgfile.LanguageCDId = 0;

	_engine->_text->initTextBank(2);

	bx = 3;

	_engine->_text->setFontCrossColor(4);
	_engine->_text->initDialogueBox();

	while (_engine->_keyboard.skipIntro != 1) {
		_engine->readKeys();
		prevSelectedItem = inventorySelectedItem;

		if (!di) {
			_engine->_keyboard.key = _engine->_keyboard.pressedKey;
			_engine->loopPressedKey = _engine->_keyboard.skippedKey;
			_engine->loopCurrentKey = _engine->_keyboard.skipIntro;

			if (_engine->_keyboard.key != 0 || _engine->_keyboard.skippedKey != 0) {
				di = 1;
			}
		} else {
			_engine->loopCurrentKey = 0;
			_engine->_keyboard.key = 0;
			_engine->loopPressedKey = 0;
			if (!_engine->_keyboard.pressedKey && !_engine->_keyboard.skippedKey) {
				di = 0;
			}
		}

		if (_engine->loopCurrentKey == 1 || _engine->loopPressedKey & 0x20)
			break;

		if (_engine->_keyboard.key & 2) { // down
			inventorySelectedItem++;
			if (inventorySelectedItem >= NUM_INVENTORY_ITEMS) {
				inventorySelectedItem = 0;
			}
			drawItem(prevSelectedItem);
			bx = 3;
		}

		if (_engine->_keyboard.key & 1) { // up
			inventorySelectedItem--;
			if (inventorySelectedItem < 0) {
				inventorySelectedItem = NUM_INVENTORY_ITEMS - 1;
			}
			drawItem(prevSelectedItem);
			bx = 3;
		}

		if (_engine->_keyboard.key & 4) { // left
			inventorySelectedItem -= 4;
			if (inventorySelectedItem < 0) {
				inventorySelectedItem += NUM_INVENTORY_ITEMS;
			}
			drawItem(prevSelectedItem);
			bx = 3;
		}

		if (_engine->_keyboard.key & 8) { // right
			inventorySelectedItem += 4;
			if (inventorySelectedItem >= NUM_INVENTORY_ITEMS) {
				inventorySelectedItem -= NUM_INVENTORY_ITEMS;
			}
			drawItem(prevSelectedItem);
			bx = 3;
		}

		if (bx == 3) {
			_engine->_text->initInventoryDialogueBox();

			if (_engine->_gameState->gameFlags[inventorySelectedItem] == 1 && !_engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] && inventorySelectedItem < NUM_INVENTORY_ITEMS) {
				_engine->_text->initText(inventorySelectedItem + 100);
			} else {
				_engine->_text->initText(128);
			}
			bx = 0;
		}

		if (bx != 2) {
			bx = _engine->_text->printText10();
		}

		// TRICKY: 3D model rotation delay - only apply when no text is drawing
		if (bx == 0 || bx == 2) {
			_engine->_system->delayMillis(15);
		}

		if (_engine->loopPressedKey & 1) {
			if (bx == 2) {
				_engine->_text->initInventoryDialogueBox();
				bx = 0;
			} else {
				if (_engine->_gameState->gameFlags[inventorySelectedItem] == 1 && !_engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] && inventorySelectedItem < NUM_INVENTORY_ITEMS) {
					_engine->_text->initInventoryDialogueBox();
					_engine->_text->initText(inventorySelectedItem + 100);
				}
			}
		}

		drawItem(inventorySelectedItem);

		if ((_engine->loopPressedKey & 2) && _engine->_gameState->gameFlags[inventorySelectedItem] == 1 && !_engine->_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED] && inventorySelectedItem < NUM_INVENTORY_ITEMS) {
			_engine->loopInventoryItem = inventorySelectedItem;
			inventorySelectedColor = 91;
			drawItem(inventorySelectedItem);
			break;
		}
	}

	_engine->_text->printTextVar13 = 0;

	_engine->_scene->alphaLight = tmpAlphaLight;
	_engine->_scene->betaLight = tmpBetaLight;

	_engine->_gameState->initEngineProjections();

	_engine->cfgfile.LanguageCDId = tmpLanguageCD;

	_engine->_text->initTextBank(_engine->_text->currentTextBank + 3);

	while (_engine->_keyboard.skipIntro != 0 && _engine->_keyboard.skippedKey != 0) {
		_engine->readKeys();
		_engine->_system->delayMillis(1);
		_engine->flip(); // TODO: needed?
	}
}

} // namespace TwinE
