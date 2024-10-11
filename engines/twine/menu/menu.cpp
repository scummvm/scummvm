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

#include "twine/menu/menu.h"
#include "audio/mixer.h"
#include "backends/audiocd/audiocd.h"
#include "backends/keymapper/keymapper.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/cursorman.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/menu/menuoptions.h"
#include "twine/movies.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/hqr.h"
#include "twine/resources/resources.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/scene/movements.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

static const uint32 kPlasmaEffectFilesize = 262176;

#define HEIGHT_STANDARD 50
#define MENU_SPACE 6

#define	SIZE_INV_OBJ_X	75
#define	SIZE_INV_OBJ_Y	65

namespace MenuButtonTypes {
enum MenuButtonTypesEnum {
	kMusicVolume = 1,
	kSoundVolume = 2,
	kCDVolume = 3,
	kSpeechVolume = 4,
	kAggressiveMode = 5,
	kPolygonDetails = 6,
	kShadowSettings = 7,
	kSceneryZoom = 8,
	kHighResolution = 9,
	kWallCollision = 10,
	kVoice = 11
};
}

#define checkMenuQuit(callMenu)      \
	if ((callMenu) == kQuitEngine) { \
		return kQuitEngine;          \
	}

namespace _priv {

static MenuSettings createMainMenu(bool lba1) {
	MenuSettings settings;
	if (lba1) {
		settings.setButtonsBoxHeight(200);
		settings.addButton(TextId::kNewGame);
		settings.addButton(TextId::kContinueGame);
		settings.addButton(TextId::kOptions);
		settings.addButton(TextId::kQuit);
	} else {
		settings.setButtonsBoxHeight(335);
		settings.addButton(TextId::toContinueGame);
		settings.addButton(TextId::toNewGame);
		settings.addButton(TextId::toOptions);
		settings.addButton(TextId::toQuit);
	}
	return settings;
}

static MenuSettings createLba1ClassicNewGame() {
	MenuSettings settings;
	settings.addButton(TextId::kReturnMenu);
	settings.addButton(TextId::kNewGame);
	settings.addButton(TextId::kNewGamePlus);
	return settings;
}

static MenuSettings createGiveUpMenu() {
	MenuSettings settings;
	settings.setButtonsBoxHeight(240);
	settings.addButton(TextId::kContinue);
	settings.addButton(TextId::kGiveUp);
	return settings;
}

static MenuSettings createGiveUpSaveMenu() {
	MenuSettings settings;
	settings.setButtonsBoxHeight(240);
	settings.addButton(TextId::kContinue);
	settings.addButton(TextId::kCreateSaveGame);
	settings.addButton(TextId::kGiveUp);
	return settings;
}

static MenuSettings createLanguageMenu(bool lba1) {
	MenuSettings settings;
	settings.addButton(TextId::kReturnMenu);
	settings.addButton(TextId::kCustomVoicesEnglish, MenuButtonTypes::kVoice);
	return settings;
}

static MenuSettings createOptionsMenu(bool lba1) {
	MenuSettings settings;
	settings.addButton(TextId::kReturnMenu);
	settings.addButton(TextId::kVolumeSettings);
	settings.addButton(TextId::kSaveManage);
	settings.addButton(TextId::kCustomLanguageOption);
	settings.addButton(TextId::kAdvanced);
	return settings;
}

static MenuSettings createAdvancedOptionsMenu() {
	MenuSettings settings;
	settings.addButton(TextId::kReturnMenu);
	settings.addButton(TextId::kBehaviourAggressiveManual, MenuButtonTypes::kAggressiveMode);
	settings.addButton(TextId::kDetailsPolygonsHigh, MenuButtonTypes::kPolygonDetails);
	settings.addButton(TextId::kDetailsShadowHigh, MenuButtonTypes::kShadowSettings);
	settings.addButton(TextId::kSceneryZoomOn, MenuButtonTypes::kSceneryZoom);
	settings.addButton(TextId::kCustomHighResOptionOn, MenuButtonTypes::kHighResolution);
	settings.addButton(TextId::kCustomWallCollisionOff, MenuButtonTypes::kWallCollision);
	return settings;
}

static MenuSettings createSaveManageMenu() {
	MenuSettings settings;
	settings.addButton(TextId::kReturnMenu);
	settings.addButton(TextId::kCreateSaveGame);
	settings.addButton(TextId::kDeleteSaveGame);
	return settings;
}

static MenuSettings createVolumeMenu() {
	MenuSettings settings;
	settings.addButton(TextId::kReturnMenu);
	settings.addButton(TextId::kMusicVolume, MenuButtonTypes::kMusicVolume);
	settings.addButton(TextId::kSoundVolume, MenuButtonTypes::kSoundVolume);
	settings.addButton(TextId::kCDVolume, MenuButtonTypes::kCDVolume);
	settings.addButton(TextId::kSpeechVolume, MenuButtonTypes::kSpeechVolume);
	return settings;
}

} // namespace _priv

const char *MenuSettings::getButtonText(Text *text, int buttonIndex) {
	if (_buttonTexts[buttonIndex].empty()) {
		TextId textId = getButtonTextId(buttonIndex);
		char dialText[256] = "";
		if (textId == TextId::kNewGamePlus) {
			text->getMenuText(TextId::kNewGame, dialText, sizeof(dialText));
			Common::strlcat(dialText, "+", sizeof(dialText));
		} else {
			text->getMenuText(textId, dialText, sizeof(dialText));
		}
		_buttonTexts[buttonIndex] = dialText;
	}
	return _buttonTexts[buttonIndex].c_str();
}

Menu::Menu(TwinEEngine *engine) {
	_engine = engine;

	_optionsMenuState = _priv::createOptionsMenu(engine->isLBA1());
	_languageMenuState = _priv::createLanguageMenu(engine->isLBA1());
	_giveUpMenuWithSaveState = _priv::createGiveUpSaveMenu();
	_volumeMenuState = _priv::createVolumeMenu();
	_saveManageMenuState = _priv::createSaveManageMenu();
	_giveUpMenuState = _priv::createGiveUpMenu();
	_mainMenuState = _priv::createMainMenu(engine->isLBA1());
	_newGameMenuState = _priv::createLba1ClassicNewGame();
	_advOptionsMenuState = _priv::createAdvancedOptionsMenu();

	Common::fill(&_behaviourAnimState[0], &_behaviourAnimState[4], 0);
	Common::fill(&_itemAngle[0], &_itemAngle[NUM_INVENTORY_ITEMS], 0);
}

Menu::~Menu() {
	free(_plasmaEffectPtr);
}

void Menu::plasmaEffectRenderFrame() {
	for (int32 j = 1; j < PLASMA_HEIGHT - 1; j++) {
		for (int32 i = 1; i < PLASMA_WIDTH - 1; i++) {
			/* Here we calculate the average of all 8 neighbour pixel values */

			int16 c;
			c = _plasmaEffectPtr[(i - 1) + (j - 1) * PLASMA_WIDTH];  // top-left
			c += _plasmaEffectPtr[(i + 0) + (j - 1) * PLASMA_WIDTH]; // top
			c += _plasmaEffectPtr[(i + 1) + (j - 1) * PLASMA_WIDTH]; // top-right

			c += _plasmaEffectPtr[(i - 1) + (j + 0) * PLASMA_WIDTH]; // left
			c += _plasmaEffectPtr[(i + 1) + (j + 0) * PLASMA_WIDTH]; // right

			c += _plasmaEffectPtr[(i - 1) + (j + 1) * PLASMA_WIDTH]; // bottom-left
			c += _plasmaEffectPtr[(i + 0) + (j + 1) * PLASMA_WIDTH]; // bottom
			c += _plasmaEffectPtr[(i + 1) + (j + 1) * PLASMA_WIDTH]; // bottom-right

			/* And the 2 least significant bits are used as a
			 * randomizing parameter for statistically fading the flames */
			c = (c >> 3) | ((c & 0x0003) << 13);

			if (!(c & 0x6500) &&
				(j >= (PLASMA_HEIGHT - 4) || c > 0)) {
				c--; /*fade this pixel*/
			}

			/* plot the pixel using the calculated color */
			_plasmaEffectPtr[i + (PLASMA_HEIGHT + j) * PLASMA_WIDTH] = (uint8)c;
		}
	}

	// flip the double-buffer while scrolling the effect vertically:
	const uint8 *src = _plasmaEffectPtr + (PLASMA_HEIGHT + 1) * PLASMA_WIDTH;
	memcpy(_plasmaEffectPtr, src, PLASMA_HEIGHT * PLASMA_WIDTH);
}

void Menu::processPlasmaEffect(const Common::Rect &rect, int32 color) {
	if (_engine->isLBA2()) {
		// TODO: effects are handled differently here.
		return;
	}
	const int32 max_value = color + 15;

	plasmaEffectRenderFrame();

	const uint8 *in = _plasmaEffectPtr + 5 * PLASMA_WIDTH;
	uint8 *out = (uint8 *)_engine->_imageBuffer.getBasePtr(0, 0);

	for (int32 y = 0; y < PLASMA_HEIGHT / 2; y++) {
		int32 yOffset = y * _engine->_imageBuffer.w;
		const uint8 *colPtr = &in[y * PLASMA_WIDTH];
		for (int32 x = 0; x < PLASMA_WIDTH; x++) {
			const uint8 c = MIN(*colPtr / 2 + color, max_value);
			/* 2x2 squares sharing the same pixel color: */
			const int32 target = 2 * yOffset;
			out[target + 0] = c;
			out[target + 1] = c;
			out[target + _engine->_imageBuffer.w + 0] = c;
			out[target + _engine->_imageBuffer.w + 1] = c;
			++colPtr;
			++yOffset;
		}
	}
	const Common::Rect prect(0, 0, PLASMA_WIDTH, PLASMA_HEIGHT);
	_engine->_frontVideoBuffer.blitFrom(_engine->_imageBuffer, prect, rect);
}

void Menu::drawRectBorders(const Common::Rect &rect, int32 colorLeftTop, int32 colorRightBottom) { // DrawCadre
	_engine->_interface->drawLine(rect.left, rect.top, rect.right, rect.top, colorLeftTop);                   // top line
	_engine->_interface->drawLine(rect.left, rect.top + 1, rect.left, rect.bottom, colorLeftTop);             // left line
	_engine->_interface->drawLine(rect.right, rect.top + 1, rect.right, rect.bottom, colorRightBottom);       // right line
	_engine->_interface->drawLine(rect.left + 1, rect.bottom, rect.right - 1, rect.bottom, colorRightBottom); // bottom line
}

void Menu::drawRectBorders(int32 left, int32 top, int32 right, int32 bottom, int32 colorLeftTop, int32 colorRightBottom) { // DrawCadre
	drawRectBorders(Common::Rect(left, top, right, bottom), colorLeftTop, colorLeftTop);
}

void Menu::drawButtonGfx(const MenuSettings *menuSettings, const Common::Rect &rect, int32 buttonId, const char *dialText, bool hover) {
	if (hover) {
		if (menuSettings == &_volumeMenuState && buttonId <= MenuButtonTypes::kSpeechVolume && buttonId >= MenuButtonTypes::kMusicVolume) {
			int32 newWidth = 0;
			switch (buttonId) {
			case MenuButtonTypes::kMusicVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
				newWidth = ruleThree32(rect.left, rect.right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			case MenuButtonTypes::kSoundVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
				newWidth = ruleThree32(rect.left, rect.right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			case MenuButtonTypes::kCDVolume: {
				const AudioCDManager::Status status = _engine->_system->getAudioCDManager()->getStatus();
				newWidth = ruleThree32(rect.left, rect.right, Audio::Mixer::kMaxMixerVolume, status.volume);
				break;
			}
			case MenuButtonTypes::kSpeechVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
				newWidth = ruleThree32(rect.left, rect.right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			}

			processPlasmaEffect(rect, COLOR_80);
			if (!(_engine->getRandomNumber() % 5)) {
				_plasmaEffectPtr[(_engine->getRandomNumber() % PLASMA_WIDTH * 10) + 20 * PLASMA_WIDTH] = 255;
			}
			_engine->_interface->box(Common::Rect(newWidth, rect.top, rect.right, rect.bottom), COLOR_SELECT_MENU);
		} else {
			processPlasmaEffect(rect, COLOR_64);
			if (!(_engine->getRandomNumber() % 5)) {
				_plasmaEffectPtr[_engine->getRandomNumber() % PLASMA_WIDTH * 10 + 6400] = 255;
			}
		}
	} else {
		_engine->blitWorkToFront(rect);
		_engine->_interface->shadeBox(rect, 4);
	}

	drawRectBorders(rect);

	_engine->_text->setFontColor(COLOR_WHITE);
	_engine->_text->setFont(INTER_LEAVE, INTER_SPACE);
	const int32 textSize = _engine->_text->sizeFont(dialText);
	_engine->_text->drawText((_engine->width() / 2) - (textSize / 2), rect.top + 7, dialText);
}

int16 Menu::drawButtons(MenuSettings *menuSettings, bool hover) {
	int16 buttonNumber = menuSettings->getActiveButton();
	const int32 maxButton = menuSettings->getButtonCount();
	int32 topHeight = menuSettings->getButtonBoxHeight();

	if (topHeight == 0) {
		topHeight = 35;
	} else {
		topHeight = topHeight - (((maxButton - 1) * 6) + (maxButton * HEIGHT_STANDARD)) / 2;
	}

	if (maxButton <= 0) {
		return -1;
	}

	int16 mouseActiveButton = -1;

	for (int16 i = 0; i < maxButton; ++i) {
		if (menuSettings == &_advOptionsMenuState) {
			int16 id = menuSettings->getButtonState(i);
			switch (id) {
			case MenuButtonTypes::kAggressiveMode:
				if (_engine->_actor->_combatAuto) {
					menuSettings->setButtonTextId(i, TextId::kBehaviourAggressiveAuto);
				} else {
					menuSettings->setButtonTextId(i, TextId::kBehaviourAggressiveManual);
				}
				break;
			case MenuButtonTypes::kPolygonDetails:
				if (_engine->_cfgfile.PolygonDetails == 0) {
					menuSettings->setButtonTextId(i, TextId::kDetailsPolygonsLow);
				} else if (_engine->_cfgfile.PolygonDetails == 1) {
					menuSettings->setButtonTextId(i, TextId::kDetailsPolygonsMiddle);
				} else {
					menuSettings->setButtonTextId(i, TextId::kDetailsPolygonsHigh);
				}
				break;
			case MenuButtonTypes::kShadowSettings:
				if (_engine->_cfgfile.ShadowMode == 0) {
					menuSettings->setButtonTextId(i, TextId::kShadowsDisabled);
				} else if (_engine->_cfgfile.ShadowMode == 1) {
					menuSettings->setButtonTextId(i, TextId::kShadowsFigures);
				} else {
					menuSettings->setButtonTextId(i, TextId::kDetailsShadowHigh);
				}
				break;
			case MenuButtonTypes::kSceneryZoom:
				if (_engine->_cfgfile.SceZoom) {
					menuSettings->setButtonTextId(i, TextId::kSceneryZoomOn);
				} else {
					menuSettings->setButtonTextId(i, TextId::kNoSceneryZoom);
				}
				break;
			case MenuButtonTypes::kHighResolution: {
				if (ConfMan.getBool("usehighres")) {
					menuSettings->setButtonTextId(i, TextId::kCustomHighResOptionOn);
				} else {
					menuSettings->setButtonTextId(i, TextId::kCustomHighResOptionOff);
				}
				break;
			}
			case MenuButtonTypes::kWallCollision: {
				if (ConfMan.getBool("wallcollision")) {
					menuSettings->setButtonTextId(i, TextId::kCustomWallCollisionOn);
				} else {
					menuSettings->setButtonTextId(i, TextId::kCustomWallCollisionOff);
				}
				break;
			}
			default:
				break;
			}
		} else if (menuSettings == &_languageMenuState) {
			int16 id = menuSettings->getButtonState(i);
			switch (id) {
			case MenuButtonTypes::kVoice: {
				const int voiceLanguage = ConfMan.getInt("audio_language");
				menuSettings->setButtonTextId(i, (TextId)((int)TextId::kCustomVoicesNone - voiceLanguage));
				break;
			}
			default:
				break;
			}
		}
		const int32 menuItemId = menuSettings->getButtonState(i);
		const char *text = menuSettings->getButtonText(_engine->_text, i);
		const int32 border = 45;
		const int32 mainMenuButtonHeightHalf = HEIGHT_STANDARD / 2;
		const Common::Rect rect(border, topHeight - mainMenuButtonHeightHalf, _engine->width() - border, topHeight + mainMenuButtonHeightHalf);
		if (hover) {
			if (i == buttonNumber) {
				drawButtonGfx(menuSettings, rect, menuItemId, text, hover);
			}
		} else {
			if (i == buttonNumber) {
				drawButtonGfx(menuSettings, rect, menuItemId, text, true);
			} else {
				drawButtonGfx(menuSettings, rect, menuItemId, text, false);
			}
		}
		if (_engine->_input->isMouseHovering(rect)) {
			mouseActiveButton = i;
		}

		topHeight += HEIGHT_STANDARD + MENU_SPACE; // increase button top height
	}
	return mouseActiveButton;
}

void Menu::menuDemo() {
	// TODO: lba2 only show the credits only in the main menu and you could force it by pressing shift+c
	// TODO: lba2 has a cd audio track (2) for the credits
	_engine->_menuOptions->showCredits();
	if (_engine->_movie->playMovie(FLA_DRAGON3)) {
		if (!_engine->_screens->loadImageDelay(TwineImage(Resources::HQR_RESS_FILE, 15, 16), 3)) {
			if (!_engine->_screens->loadImageDelay(TwineImage(Resources::HQR_RESS_FILE, 17, 18), 3)) {
				if (!_engine->_screens->loadImageDelay(TwineImage(Resources::HQR_RESS_FILE, 19, 20), 3)) {
					if (_engine->_movie->playMovie(FLA_BATEAU)) {
						if (_engine->_cfgfile.Version == USA_VERSION) {
							_engine->_screens->loadImageDelay(_engine->_resources->relentLogo(), 3);
						} else {
							_engine->_screens->loadImageDelay(_engine->_resources->lbaLogo(), 3);
						}
						_engine->_screens->adelineLogo();
					}
				}
			}
		}
	}
}

int32 Menu::doGameMenu(MenuSettings *menuSettings) {
	int16 currentButton = menuSettings->getActiveButton();
	bool buttonsNeedRedraw = true;
	const int32 numEntry = menuSettings->getButtonCount();
	int32 maxButton = numEntry - 1;

	_engine->_input->enableKeyMap(uiKeyMapId);

	// if we are running the game already, the buttons are just rendered on top of the scene
	if (_engine->_scene->isGameRunning()) {
		_engine->restoreFrontBuffer();
	} else {
		_engine->_screens->loadMenuImage(false);
	}
	uint32 startMillis = _engine->_system->getMillis();
	do {
		FrameMarker frame(_engine);
		const uint32 loopMillis = _engine->_system->getMillis();
		_engine->readKeys();

		if (_engine->_input->toggleActionIfActive(TwinEActionType::UIDown)) {
			currentButton++;
			if (currentButton == numEntry) { // if current button is the last, than next button is the first
				currentButton = 0;
			}
			buttonsNeedRedraw = true;
			startMillis = loopMillis;
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIUp)) {
			currentButton--;
			if (currentButton < 0) { // if current button is the first, than previous button is the last
				currentButton = maxButton;
			}
			buttonsNeedRedraw = true;
			startMillis = loopMillis;
		}

		const int16 id = menuSettings->getActiveButtonState();
		if (menuSettings == &_advOptionsMenuState) {
			switch (id) {
			case MenuButtonTypes::kAggressiveMode:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft) || _engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					_engine->_actor->_combatAuto = !_engine->_actor->_combatAuto;
					startMillis = loopMillis;
				}
				break;
			case MenuButtonTypes::kPolygonDetails:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft)) {
					_engine->_cfgfile.PolygonDetails--;
					_engine->_cfgfile.PolygonDetails %= 3;
					startMillis = loopMillis;
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					_engine->_cfgfile.PolygonDetails++;
					_engine->_cfgfile.PolygonDetails %= 3;
					startMillis = loopMillis;
				}
				break;
			case MenuButtonTypes::kShadowSettings:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft)) {
					_engine->_cfgfile.ShadowMode--;
					_engine->_cfgfile.ShadowMode %= 3;
					startMillis = loopMillis;
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					_engine->_cfgfile.ShadowMode++;
					_engine->_cfgfile.ShadowMode %= 3;
					startMillis = loopMillis;
				}
				break;
			case MenuButtonTypes::kSceneryZoom:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft) || _engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					_engine->_cfgfile.SceZoom = !_engine->_cfgfile.SceZoom;
					startMillis = loopMillis;
				}
				break;
			case MenuButtonTypes::kHighResolution:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft) || _engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					const bool highRes = ConfMan.getBool("usehighres");
					ConfMan.setBool("usehighres", !highRes);
					startMillis = loopMillis;
				}
				break;
			case MenuButtonTypes::kWallCollision:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft) || _engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					const bool highRes = ConfMan.getBool("wallcollision");
					ConfMan.setBool("wallcollision", !highRes);
					startMillis = loopMillis;
				}
				break;
			default:
				break;
			}
		} else if (menuSettings == &_languageMenuState) {
			switch (id) {
			case MenuButtonTypes::kVoice:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft)) {
					int voiceLanguage = ConfMan.getInt("audio_language");
					--voiceLanguage;
					if (voiceLanguage < 0) {
						voiceLanguage = 3;
					}
					ConfMan.setInt("audio_language", voiceLanguage);
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					int voiceLanguage = ConfMan.getInt("audio_language");
					++voiceLanguage;
					if (voiceLanguage > 3) {
						voiceLanguage = 0;
					}
					ConfMan.setInt("audio_language", voiceLanguage);
				}
				break;
			default:
				break;
			}
		} else if (menuSettings == &_volumeMenuState) {
			Audio::Mixer *mixer = _engine->_system->getMixer();
			switch (id) {
			case MenuButtonTypes::kMusicVolume: {
				int volume = mixer->getVolumeForSoundType(Audio::Mixer::SoundType::kMusicSoundType);
				if (_engine->_input->isActionActive(TwinEActionType::UILeft)) {
					volume -= 4;
					startMillis = loopMillis;
				} else if (_engine->_input->isActionActive(TwinEActionType::UIRight)) {
					volume += 4;
					startMillis = loopMillis;
				}
				_engine->_music->musicVolume(volume);
				ConfMan.setInt("music_volume", mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
				break;
			}
			case MenuButtonTypes::kSoundVolume: {
				int volume = mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
				if (_engine->_input->isActionActive(TwinEActionType::UILeft)) {
					volume -= 4;
					startMillis = loopMillis;
				} else if (_engine->_input->isActionActive(TwinEActionType::UIRight)) {
					volume += 4;
					startMillis = loopMillis;
				}

				mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
				ConfMan.setInt("sfx_volume", mixer->getVolumeForSoundType(Audio::Mixer::kSFXSoundType));
				break;
			}
			case MenuButtonTypes::kCDVolume: {
				AudioCDManager::Status status = _engine->_system->getAudioCDManager()->getStatus();
				if (_engine->_input->isActionActive(TwinEActionType::UILeft)) {
					status.volume -= 4;
					startMillis = loopMillis;
				} else if (_engine->_input->isActionActive(TwinEActionType::UIRight)) {
					status.volume += 4;
					startMillis = loopMillis;
				}
				status.volume = CLIP(status.volume, 0, 255);
				_engine->_system->getAudioCDManager()->setVolume(status.volume);
				break;
			}
			case MenuButtonTypes::kSpeechVolume: {
				int volume = mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
				if (_engine->_input->isActionActive(TwinEActionType::UILeft)) {
					volume -= 4;
					startMillis = loopMillis;
				} else if (_engine->_input->isActionActive(TwinEActionType::UIRight)) {
					volume += 4;
					startMillis = loopMillis;
				}
				mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume);
				ConfMan.setInt("speech_volume", mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType));
				break;
			}
			default:
				break;
			}
		}

		if (buttonsNeedRedraw) {
			// draw all buttons
			const int16 mouseButtonHovered = drawButtons(menuSettings, false);
			if (mouseButtonHovered != -1) {
				currentButton = mouseButtonHovered;
			}
			menuSettings->setActiveButton(currentButton);
		}

		// draw plasma effect for the current selected button
		const int16 mouseButtonHovered = drawButtons(menuSettings, true);
		if (mouseButtonHovered != -1) {
			if (mouseButtonHovered != currentButton) {
				buttonsNeedRedraw = true;
			}
			currentButton = mouseButtonHovered;
		}

		if (_engine->shouldQuit()) {
			return kQuitEngine;
		}
		if (_engine->_input->toggleActionIfActive(TwinEActionType::UIAbort)) {
			for (int i = 0; i < menuSettings->getButtonCount(); ++i) {
				const TextId textId = menuSettings->getButtonTextId(i);
				if (textId == TextId::kReturnMenu || textId == TextId::kReturnGame || textId == TextId::kContinue) {
					return (int32)textId;
				}
			}
			startMillis = loopMillis;
		}
		if (menuSettings == &_mainMenuState) {
			uint32 idleTime = 60 * 3 + 53 * 1000;
			if (_engine->isDemo()) {
				idleTime = 60 * 1000;
			}
			if (loopMillis - startMillis > idleTime) {
				return kDemoMenu;
			}
		}
	} while (!_engine->_input->toggleActionIfActive(TwinEActionType::UIEnter));

	return (int32)menuSettings->getActiveButtonTextId();
}

int32 Menu::advoptionsMenu() {
	_engine->restoreFrontBuffer();

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (doGameMenu(&_advOptionsMenuState)) {
		case (int32)TextId::kReturnMenu: {
			return 0;
		}
		case kQuitEngine:
			return kQuitEngine;
		case (int32)TextId::kBehaviourAggressiveManual:
		case (int32)TextId::kDetailsPolygonsHigh:
		case (int32)TextId::kDetailsShadowHigh:
		case (int32)TextId::kSceneryZoomOn:
		default:
			warning("Unknown menu button handled");
			break;
		}
	}

	return 0;
}

int32 Menu::savemanageMenu() {
	_engine->restoreFrontBuffer();

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (doGameMenu(&_saveManageMenuState)) {
		case (int32)TextId::kReturnMenu:
			return 0;
		case (int32)TextId::kCreateSaveGame:
			_engine->_menuOptions->saveGameMenu();
			break;
		case (int32)TextId::kDeleteSaveGame:
			_engine->_menuOptions->deleteSaveMenu();
			break;
		case kQuitEngine:
			return kQuitEngine;
		default:
			warning("Unknown menu button handled");
			break;
		}
	}

	return 0;
}

int32 Menu::volumeOptions() {
	_engine->restoreFrontBuffer();

	if (_engine->isLBA1()) {
		if (_engine->isCDROM()) {
			_engine->_music->playAllMusic(9);
		} else {
			_engine->_music->playMidiFile(9);
		}
	}

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (doGameMenu(&_volumeMenuState)) {
		case (int32)TextId::kReturnMenu:
			return 0;
		case kQuitEngine:
			return kQuitEngine;
		case (int32)TextId::kMusicVolume:
		case (int32)TextId::kSoundVolume:
		case (int32)TextId::kCDVolume:
		case (int32)TextId::kSpeechVolume:
		default:
			warning("Unknown menu button handled");
			break;
		}
	}

	return 0;
}

int32 Menu::languageMenu() {
	_engine->restoreFrontBuffer();

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (doGameMenu(&_languageMenuState)) {
		case (int32)TextId::kReturnMenu:
			return 0;
		case kQuitEngine:
			return kQuitEngine;
		case (int32)MenuButtonTypes::kVoice:
		default:
			warning("Unknown menu button handled");
			break;
		}
	}

	return 0;
}

void Menu::inGameOptionsMenu() {
	_engine->_text->initDial(TextBankId::Options_and_menus);
	_optionsMenuState.setButtonTextId(0, TextId::kReturnGame);
	_engine->saveFrontBuffer();
	optionsMenu();
	_engine->_text->initSceneTextBank();
	_optionsMenuState.setButtonTextId(0, TextId::kReturnMenu);
}

int32 Menu::optionsMenu() {
	_engine->restoreFrontBuffer();

	_engine->_sound->stopSamples();
	if (_engine->isLBA1()) {
		// LBA's Theme
		if (_engine->isCDROM()) {
			_engine->_music->playCdTrack(9);
		} else {
			_engine->_music->playMidiFile(9);
		}
	}

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (doGameMenu(&_optionsMenuState)) {
		case (int32)TextId::kReturnGame:
		case (int32)TextId::kReturnMenu: {
			return 0;
		}
		case (int32)TextId::kVolumeSettings: {
			checkMenuQuit(volumeOptions()) break;
		}
		case (int32)TextId::kCustomLanguageOption: {
			checkMenuQuit(languageMenu()) break;
		}
		case (int32)TextId::kSaveManage: {
			checkMenuQuit(savemanageMenu()) break;
		}
		case (int32)TextId::kAdvanced: {
			checkMenuQuit(advoptionsMenu()) break;
		}
		case kQuitEngine:
			return kQuitEngine;
		default:
			break;
		}
	}

	return 0;
}

int32 Menu::newGameClassicMenu() {
	_engine->restoreFrontBuffer();

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (doGameMenu(&_newGameMenuState)) {
		case (int32)TextId::kReturnGame:
		case (int32)TextId::kReturnMenu: {
			return 0;
		}
		case (int32)TextId::kNewGamePlus:
			_engine->_gameState->_endGameItems = true;
			if (_engine->_menuOptions->newGameMenu()) {
				return 1;
			}
			break;
		case (int32)TextId::kNewGame: {
			_engine->_gameState->_endGameItems = false;
			if (_engine->_menuOptions->newGameMenu()) {
				return 1;
			}
			break;
		}
		case kQuitEngine:
			return kQuitEngine;
		default:
			break;
		}
	}

	return 0;
}

static const byte cursorArrow[] = {
	1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3,
	1, 0, 1, 3, 3, 3, 3, 3, 3, 3, 3,
	1, 0, 0, 1, 3, 3, 3, 3, 3, 3, 3,
	1, 0, 0, 0, 1, 3, 3, 3, 3, 3, 3,
	1, 0, 0, 0, 0, 1, 3, 3, 3, 3, 3,
	1, 0, 0, 0, 0, 0, 1, 3, 3, 3, 3,
	1, 0, 0, 0, 0, 0, 0, 1, 3, 3, 3,
	1, 0, 0, 0, 0, 0, 0, 0, 1, 3, 3,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 3,
	1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
	1, 0, 0, 1, 0, 0, 1, 3, 3, 3, 3,
	1, 0, 1, 3, 1, 0, 0, 1, 3, 3, 3,
	1, 1, 3, 3, 1, 0, 0, 1, 3, 3, 3,
	1, 3, 3, 3, 3, 1, 0, 0, 1, 3, 3,
	3, 3, 3, 3, 3, 1, 0, 0, 1, 3, 3,
	3, 3, 3, 3, 3, 3, 1, 1, 1, 3, 3};

static const byte cursorPalette[] = {
	0, 0, 0,
	0xff, 0xff, 0xff};

bool Menu::init() {
	_engine->_input->enableKeyMap(uiKeyMapId);
	_engine->_screens->loadMenuImage();
	// load menu effect file only once
	_plasmaEffectPtr = (uint8 *)malloc(kPlasmaEffectFilesize);
	memset(_plasmaEffectPtr, 0, kPlasmaEffectFilesize);

	CursorMan.pushCursor(cursorArrow, 11, 16, 1, 1, 3);
	CursorMan.pushCursorPalette(cursorPalette, 0, 2);
	return HQR::getEntry(_plasmaEffectPtr, Resources::HQR_RESS_FILE, RESSHQR_PLASMAEFFECT) > 0;
}

EngineState Menu::run() {
	FrameMarker frame(_engine);
	_engine->_text->initDial(TextBankId::Options_and_menus);

	_engine->_sound->stopSamples();
	if (_engine->isLBA1()) {
		if (_engine->isCDROM()) {
			_engine->_music->playCdTrack(9); // LBA's Theme
		} else {
			_engine->_music->playMidiFile(9); // LBA's Theme
		}
	} else {
		_engine->_music->playMusic(6); // LBA2's Theme
	}

	ScopedCursor scoped(_engine);
	switch (doGameMenu(&_mainMenuState)) {
	case (int32)TextId::toNewGame:
	case (int32)TextId::kNewGame: {
		if (_engine->isLba1Classic()) {
			if (newGameClassicMenu()) {
				return EngineState::GameLoop;
			}
			break;
		}
		if (_engine->_menuOptions->newGameMenu()) {
			return EngineState::GameLoop;
		}
		break;
	}
	case (int32)TextId::toContinueGame:
	case (int32)TextId::kContinueGame: {
		if (_engine->_menuOptions->continueGameMenu()) {
			return EngineState::LoadedGame;
		}
		break;
	}
	case (int32)TextId::toOptions:
	case (int32)TextId::kOptions: {
		optionsMenu();
		break;
	}
	case kDemoMenu: {
		menuDemo();
		_engine->_screens->loadMenuImage(false);
		break;
	}
	case (int32)TextId::kQuit:
	case (int32)TextId::toQuit:
	case kQuitEngine:
		debug("quit the game");
		return EngineState::QuitGame;
	}
	return EngineState::Menu;
}

int32 Menu::quitMenu() {
	_engine->saveFrontBuffer();
	_engine->_sound->pauseSamples();

	MenuSettings *localMenu;
	if (_engine->_cfgfile.UseAutoSaving) {
		localMenu = &_giveUpMenuState;
	} else {
		localMenu = &_giveUpMenuWithSaveState;
	}

	ScopedCursor scoped(_engine);

	int32 menuId;
	do {
		FrameMarker frame(_engine);
		_engine->_text->initDial(TextBankId::Options_and_menus);
		menuId = doGameMenu(localMenu);
		switch (menuId) {
		case (int32)TextId::kContinue:
			_engine->_sound->resumeSamples();
			break;
		case (int32)TextId::kGiveUp:
			_engine->_gameState->giveUp();
			return 1;
		case (int32)TextId::kCreateSaveGame:
			_engine->_menuOptions->saveGameMenu();
			break;
		case kQuitEngine:
			return kQuitEngine;
		default:
			warning("Unknown menu button handled: %i", menuId);
		}
		_engine->_text->initSceneTextBank();
	} while (menuId != (int32)TextId::kGiveUp && menuId != (int32)TextId::kContinue && menuId != (int32)TextId::kCreateSaveGame);

	return 0;
}

void Menu::drawHealthBar(int32 left, int32 right, int32 top, int32 barLeftPadding, int32 barHeight) {
	_engine->_grid->drawSprite(left, top + 3, _engine->_resources->_spriteData[SPRITEHQR_LIFEPOINTS]);
	const int32 barLeft = left + barLeftPadding;
	const int32 healthBarRight = ruleThree32(barLeft, right, 50, _engine->_scene->_sceneHero->_lifePoint);
	const int32 barBottom = top + barHeight;
	_engine->_interface->box(Common::Rect(barLeft, top, healthBarRight, barBottom), COLOR_91);
	_engine->_interface->box(Common::Rect(healthBarRight, top, left + 325, barBottom), COLOR_BLACK);
	drawRectBorders(Common::Rect(barLeft, top, right, barBottom));
}

void Menu::drawCloverLeafs(int32 newBoxLeft, int32 boxRight, int32 top) {
	// Clover leaf boxes
	for (int32 i = 0; i < _engine->_gameState->_inventoryNumLeafsBox; i++) {
		const int32 leftSpritePos = ruleThree32(newBoxLeft, boxRight, 10, i);
		_engine->_grid->drawSprite(leftSpritePos, top + 58, _engine->_resources->_spriteData[SPRITEHQR_CLOVERLEAFBOX]);
	}

	// Clover leafs
	for (int32 i = 0; i < _engine->_gameState->_inventoryNumLeafs; i++) {
		const int32 leftSpritePos = ruleThree32(newBoxLeft, boxRight, 10, i);
		_engine->_grid->drawSprite(leftSpritePos + 2, top + 60, _engine->_resources->_spriteData[SPRITEHQR_CLOVERLEAF]);
	}
}

void Menu::drawMagicPointsBar(int32 left, int32 right, int32 top, int32 barLeftPadding, int32 barHeight) {
	if (_engine->_gameState->inventoryDisabled()) {
		return;
	}
	if (!_engine->_gameState->hasItem(InventoryItems::kiTunic)) {
		return;
	}
	_engine->_grid->drawSprite(left, top + 1, _engine->_resources->_spriteData[SPRITEHQR_MAGICPOINTS]);
	if (_engine->_gameState->_magicLevelIdx <= 0) {
		return;
	}
	const int32 barLeft = left + barLeftPadding;
	const int32 barBottom = top + barHeight;
	// max magic level is 4
	const int32 maxMagicPoints = 4 * 20;
	const int32 barRight = ruleThree32(barLeft, right, maxMagicPoints, _engine->_gameState->_magicPoint);
	const Common::Rect pointsRect(barLeft, top, barRight, barBottom);
	_engine->_interface->box(pointsRect, COLOR_75);

	for (int32 l = 0; l < _engine->_gameState->_magicLevelIdx; l++) {
		const int32 x1 = ruleThree32(barLeft, right, 40, _engine->_gameState->_magicLevelIdx * 10);
		_engine->_interface->drawLine(x1, top + barHeight, x1, top + 35 + 15 - 1, 0);
	}

	const int32 rectRight = ruleThree32(barLeft, right, 40, _engine->_gameState->_magicLevelIdx * 10);
	drawRectBorders(barLeft, top, rectRight, barBottom);
}

void Menu::drawSpriteAndString(int32 left, int32 top, const SpriteData &spriteData, const Common::String &str, int32 color) {
	_engine->_grid->drawSprite(left, top + 15, spriteData);
	_engine->_text->setFontColor(color);
	_engine->_text->drawText(left + 30, top + 5, str.c_str());
}

void Menu::drawCoins(int32 left, int32 top) {
	const Common::String &inventoryNumKashes = Common::String::format("%d", _engine->_gameState->_goldPieces);
	drawSpriteAndString(left, top, _engine->_resources->_spriteData[SPRITEHQR_KASHES], inventoryNumKashes);
}

void Menu::drawKeys(int32 left, int32 top) {
	const Common::String &inventoryNumKeys = Common::String::format("%d", _engine->_gameState->_inventoryNumKeys);
	drawSpriteAndString(left, top, _engine->_resources->_spriteData[SPRITEHQR_KEY], inventoryNumKeys);
}

void Menu::drawInfoMenu(int16 left, int16 top, int16 width) {
	_engine->_interface->unsetClip();
	const int16 height = 80;
	const Common::Rect rect(left, top, left + width, top + height);
	drawRectBorders(rect);
	Common::Rect filledRect(rect);
	filledRect.grow(-1);
	_engine->_interface->box(filledRect, COLOR_BLACK);

	const int32 boxLeft = left + 9;
	const int32 boxRight = left + 325;
	const int32 barPadding = 25;
	const int32 boxTop = top + 10;
	const int32 barHeight = 14;
	drawHealthBar(boxLeft, boxRight, boxTop, barPadding, barHeight);
	drawMagicPointsBar(boxLeft, boxRight, boxTop + 25, barPadding, barHeight);

	const int32 posLeft = left + 340;
	drawCoins(posLeft, top);
	drawKeys(posLeft, top + 35);
	drawCloverLeafs(left + barPadding, boxRight, top);

	_engine->copyBlockPhys(left, top, left + width, top + 135);
}

Common::Rect Menu::calcBehaviourRect(int32 left, int32 top, HeroBehaviourType behaviour) const {
	const int border = 10;
	const int32 padding = 11;
	const int32 width = 99;
	const int height = 119;

	const int32 boxLeft = (int32)behaviour * (width + padding) + left + border;
	const int32 boxRight = boxLeft + width;
	const int32 boxTop = top + border;
	const int32 boxBottom = boxTop + height;
	return Common::Rect(boxLeft, boxTop, boxRight, boxBottom);
}

bool Menu::isBehaviourHovered(int32 left, int32 top, HeroBehaviourType behaviour) const {
	if (!_engine->_cfgfile.Mouse) {
		return false;
	}
	const Common::Rect &boxRect = calcBehaviourRect(left, top, behaviour);
	return _engine->_input->isMouseHovering(boxRect);
}

void Menu::drawBehaviour(int32 left, int32 top, HeroBehaviourType behaviour, int32 angle, bool cantDrawBox) {
	const Common::Rect &boxRect = calcBehaviourRect(left, top, behaviour);

	const int animIdx = _engine->_actor->_heroAnimIdx[(byte)behaviour];
	const AnimData &currentAnimData = _engine->_resources->_animData[animIdx];

	uint currentAnimState = _behaviourAnimState[(byte)behaviour];

	if (_engine->_animations->setInterAnimObjet(currentAnimState, currentAnimData, *_behaviourEntity, &_behaviourAnimData[(byte)behaviour])) {
		currentAnimState++; // keyframe
		if (currentAnimState >= currentAnimData.getNbFramesAnim()) {
			currentAnimState = currentAnimData.getLoopFrame();
		}
		_behaviourAnimState[(byte)behaviour] = currentAnimState;
	}

	if (!cantDrawBox) {
		Common::Rect boxRectCopy(boxRect);
		boxRectCopy.grow(1);
		drawRectBorders(boxRectCopy);
	}

	_engine->_interface->memoClip();
	_engine->_interface->unsetClip();

	if (behaviour == _engine->_actor->_heroBehaviour) {
		const int titleOffset = 10;
		const int titleHeight = 40;
		const int32 titleBoxLeft = left + 10;
		const int32 titleBoxWidth = 430;
		const int32 titleBoxCenter = titleBoxLeft + titleBoxWidth / 2;
		const int32 titleBoxRight = titleBoxLeft + titleBoxWidth;
		const int32 titleBoxTop = boxRect.bottom + titleOffset;
		const int32 titleBoxBottom = titleBoxTop + titleHeight;

		_engine->_interface->box(boxRect, COLOR_BRIGHT_BLUE2);

		// behaviour menu title
		const Common::Rect titleRect(titleBoxLeft, titleBoxTop, titleBoxRight, titleBoxBottom);
		_engine->_interface->box(titleRect, COLOR_BLACK);
		drawRectBorders(titleRect);

		_engine->_text->setFontColor(COLOR_WHITE);

		char dialText[256];
		_engine->_text->getMenuText(_engine->_actor->getTextIdForBehaviour(), dialText, sizeof(dialText));

		_engine->_text->drawText(titleBoxCenter - _engine->_text->sizeFont(dialText) / 2, titleBoxTop + 1, dialText);
	} else {
		_engine->_interface->box(boxRect, COLOR_BLACK);
	}

	_engine->_renderer->drawObj3D(boxRect, -600, angle, *_behaviourEntity, _moveMenu);

	_engine->_interface->restoreClip();
}

void Menu::prepareAndDrawBehaviour(int32 left, int32 top, int32 angle, HeroBehaviourType behaviour) {
	const int animIdx = _engine->_actor->_heroAnimIdx[(byte)behaviour];
	_engine->_animations->setAnimObjet(_behaviourAnimState[(byte)behaviour], _engine->_resources->_animData[animIdx], *_behaviourEntity, &_behaviourAnimData[(byte)behaviour]);
	drawBehaviour(left, top, behaviour, angle, false);
}

void Menu::drawBehaviourMenu(int32 left, int32 top, int32 angle) {
	const int32 width = 450;
	const int32 height = 190;
	const int32 right = left + width;
	const int32 bottom = top + height;

	const Common::Rect titleRect(left, top, right, bottom);
	drawRectBorders(titleRect);

	Common::Rect boxRect(titleRect);
	boxRect.grow(-1);
	_engine->_interface->shadeBox(boxRect, 2);

	prepareAndDrawBehaviour(left, top, angle, HeroBehaviourType::kNormal);
	prepareAndDrawBehaviour(left, top, angle, HeroBehaviourType::kAthletic);
	prepareAndDrawBehaviour(left, top, angle, HeroBehaviourType::kAggressive);
	prepareAndDrawBehaviour(left, top, angle, HeroBehaviourType::kDiscrete);

	_engine->copyBlockPhys(titleRect);

	drawInfoMenu(titleRect.left, titleRect.bottom + 10, titleRect.width());
}

void Menu::processBehaviourMenu(bool behaviourMenu) {
	if (_engine->_actor->_heroBehaviour == HeroBehaviourType::kProtoPack) {
		_engine->_sound->stopSamples();
		_engine->_actor->setBehaviour(HeroBehaviourType::kNormal);
	}

	_behaviourEntity = &_engine->_scene->_sceneHero->_entityDataPtr->getBody(_engine->_scene->_sceneHero->_body);

	_engine->_actor->_heroAnimIdx[(byte)HeroBehaviourType::kNormal] = _engine->_actor->_heroAnimIdxNORMAL;
	_engine->_actor->_heroAnimIdx[(byte)HeroBehaviourType::kAthletic] = _engine->_actor->_heroAnimIdxATHLETIC;
	_engine->_actor->_heroAnimIdx[(byte)HeroBehaviourType::kAggressive] = _engine->_actor->_heroAnimIdxAGGRESSIVE;
	_engine->_actor->_heroAnimIdx[(byte)HeroBehaviourType::kDiscrete] = _engine->_actor->_heroAnimIdxDISCRETE;

	_engine->_movements->initRealAngle(_engine->_scene->_sceneHero->_beta, _engine->_scene->_sceneHero->_beta - LBAAngles::ANGLE_90, LBAAngles::ANGLE_17, &_moveMenu);

	_engine->saveFrontBuffer();

	TextBankId tmpTextBank = _engine->_scene->_sceneTextBank;
	_engine->_scene->_sceneTextBank = TextBankId::None;

	_engine->_text->initDial(TextBankId::Options_and_menus);

	// quick actions to change behaviour don't show the menu in classic edition
	if (!behaviourMenu && _engine->isLba1Classic()) {
		char text[256];
		_engine->_text->getMenuText(_engine->_actor->getTextIdForBehaviour(), text, sizeof(text));
		_engine->_redraw->setRenderText(text);
	} else {
		const int32 left = _engine->width() / 2 - 220;
		const int32 top = _engine->height() / 2 - 140;
		drawBehaviourMenu(left, top, _engine->_scene->_sceneHero->_beta);

		HeroBehaviourType tmpHeroBehaviour = _engine->_actor->_heroBehaviour;

		const int animIdx = _engine->_actor->_heroAnimIdx[(byte)_engine->_actor->_heroBehaviour];
		_engine->_animations->setAnimObjet(_behaviourAnimState[(byte)_engine->_actor->_heroBehaviour], _engine->_resources->_animData[animIdx], *_behaviourEntity, &_behaviourAnimData[(byte)_engine->_actor->_heroBehaviour]);

		int32 tmpTime = _engine->timerRef;

		ScopedCursor scopedCursor(_engine);
		ScopedKeyMap scopedKeyMap(_engine, uiKeyMapId);
		while (_engine->_input->isActionActive(TwinEActionType::BehaviourMenu) || _engine->_input->isQuickBehaviourActionActive()) {
			FrameMarker frame(_engine, 50);
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				break;
			}

			if (isBehaviourHovered(left, top, HeroBehaviourType::kNormal)) {
				_engine->_actor->_heroBehaviour = HeroBehaviourType::kNormal;
			} else if (isBehaviourHovered(left, top, HeroBehaviourType::kAthletic)) {
				_engine->_actor->_heroBehaviour = HeroBehaviourType::kAthletic;
			} else if (isBehaviourHovered(left, top, HeroBehaviourType::kAggressive)) {
				_engine->_actor->_heroBehaviour = HeroBehaviourType::kAggressive;
			} else if (isBehaviourHovered(left, top, HeroBehaviourType::kDiscrete)) {
				_engine->_actor->_heroBehaviour = HeroBehaviourType::kDiscrete;
			}

			int heroBehaviour = (int)_engine->_actor->_heroBehaviour;
			if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft)) {
				heroBehaviour--;
			} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIRight)) {
				heroBehaviour++;
			}

			if (heroBehaviour < (int)HeroBehaviourType::kNormal) {
				heroBehaviour = (int)HeroBehaviourType::kDiscrete;
			} else if (heroBehaviour >= (int)HeroBehaviourType::kProtoPack) {
				heroBehaviour = (int)HeroBehaviourType::kNormal;
			}

			_engine->_actor->_heroBehaviour = (HeroBehaviourType)heroBehaviour;

			if (tmpHeroBehaviour != _engine->_actor->_heroBehaviour) {
				drawBehaviour(left, top, tmpHeroBehaviour, _engine->_scene->_sceneHero->_beta, true);
				tmpHeroBehaviour = _engine->_actor->_heroBehaviour;
				_engine->_movements->initRealAngle(_engine->_scene->_sceneHero->_beta, _engine->_scene->_sceneHero->_beta - LBAAngles::ANGLE_90, LBAAngles::ANGLE_17, &_moveMenu);
				const int tmpAnimIdx = _engine->_actor->_heroAnimIdx[(byte)_engine->_actor->_heroBehaviour];
				_engine->_animations->setAnimObjet(_behaviourAnimState[(byte)_engine->_actor->_heroBehaviour], _engine->_resources->_animData[tmpAnimIdx], *_behaviourEntity, &_behaviourAnimData[(byte)_engine->_actor->_heroBehaviour]);
			}

			drawBehaviour(left, top, _engine->_actor->_heroBehaviour, -1, true);

			// increase the timer to play the animations
			_engine->timerRef++;
			debugC(3, kDebugLevels::kDebugTimers, "Behaviour time: %i", _engine->timerRef);
		}

		_engine->timerRef = tmpTime;

		_engine->_gameState->init3DGame();
	}
	_engine->_actor->setBehaviour(_engine->_actor->_heroBehaviour);

	_engine->_scene->_sceneTextBank = tmpTextBank;
	_engine->_text->initSceneTextBank();
}

Common::Rect Menu::calcItemRect(int32 left, int32 top, int32 item, int32 *centerX, int32 *centerY) const {
	const int32 itemWidth = SIZE_INV_OBJ_X;
	const int32 itemHeight = SIZE_INV_OBJ_Y;
	const int32 itemPadding = 11;
	const int32 itemWidthHalf = itemWidth / 2;
	const int32 itemHeightHalf = itemHeight / 2;
	const int32 itemX = (item / 4) * (itemWidth + itemPadding) + left + itemWidthHalf + itemPadding - 1;
	const int32 itemY = (item % 4) * (itemHeight + itemPadding) + top + itemHeightHalf + itemPadding - 1;
	if (centerX) {
		*centerX = itemX;
	}
	if (centerY) {
		*centerY = itemY;
	}
	return Common::Rect(itemX - itemWidthHalf, itemY - itemHeightHalf, itemX + itemWidthHalf, itemY + itemHeightHalf);
}

void Menu::drawOneInventory(int32 left, int32 top, int32 item) {
	int32 itemX, itemY;
	const Common::Rect rect = calcItemRect(left, top, item, &itemX, &itemY);
	const int32 color = _inventorySelectedItem == item ? _inventorySelectedColor : COLOR_BLACK;

	_engine->_interface->box(rect, color);

	if (item < NUM_INVENTORY_ITEMS && _engine->_gameState->hasItem((InventoryItems)item) && (!_engine->_gameState->inventoryDisabled() || item == InventoryItems::kiCloverLeaf)) {
		_itemAngle[item] += LBAAngles::ANGLE_2;
		_engine->_interface->setClip(rect);
		_engine->_renderer->draw3dObject(itemX, itemY, _engine->_resources->_inventoryTable[item], _itemAngle[item], 15000);
		_engine->_interface->unsetClip();
		if (item == InventoryItems::kGasItem) {
			_engine->_text->setFontColor(COLOR_WHITE);
			const Common::String &inventoryNumGas = Common::String::format("%d", _engine->_gameState->_inventoryNumGas);
			_engine->_text->drawText(rect.left + 3, rect.bottom - 32, inventoryNumGas.c_str());
		}
	}

	drawRectBorders(rect);
}

void Menu::drawListInventory(int32 left, int32 top) {
	constexpr int w = (SIZE_INV_OBJ_X + 10) * 7 + 10;
	constexpr int h = (SIZE_INV_OBJ_Y + 10) * 4 + 10;
	const Common::Rect rect(left, top, left + w, top + h);
	_engine->_interface->shadeBox(rect, 4);
	drawRectBorders(rect);
	for (int32 item = 0; item < NUM_INVENTORY_ITEMS; item++) {
		drawOneInventory(left, top, item);
	}
	_engine->_interface->unsetClip();
}

void Menu::inventory() {
	int32 tmpAlphaLight = _engine->_scene->_alphaLight;
	int32 tmpBetaLight = _engine->_scene->_betaLight;

	_engine->saveFrontBuffer();

	_engine->_renderer->setLightVector(LBAAngles::ANGLE_315, LBAAngles::ANGLE_334, LBAAngles::ANGLE_0);

	_inventorySelectedColor = COLOR_SELECT_MENU;

	if (_engine->_gameState->_inventoryNumLeafs > 0) {
		_engine->_gameState->giveItem(InventoryItems::kiCloverLeaf);
	} else {
		_engine->_gameState->removeItem(InventoryItems::kiCloverLeaf);
	}

	const int32 left = _engine->width() / 2 - 303;
	const int32 top = _engine->height() / 2 - 230;
	drawListInventory(left, top);

	_engine->_text->initDial(TextBankId::Inventory_Intro_and_Holomap);

	_engine->_text->setFontCrossColor(COLOR_BRIGHT_BLUE);
	_engine->_text->initDialWindow();

	ProgressiveTextState dialstate = ProgressiveTextState::ContinueRunning;
	bool updateItemText = true;

	ScopedCursor scopedCursor(_engine);
	ScopedKeyMap scopedKeyMap(_engine, uiKeyMapId);
	for (;;) {
		FrameMarker frame(_engine, 66);
		_engine->readKeys();
		int32 prevSelectedItem = _inventorySelectedItem;

		if (_engine->_input->toggleAbortAction() || _engine->shouldQuit()) {
			break;
		}

		for (int32 item = 0; item < NUM_INVENTORY_ITEMS; item++) {
			const Common::Rect &rect = calcItemRect(left, top, item);
			if (_engine->_input->isMouseHovering(rect)) {
				_inventorySelectedItem = item;
				drawOneInventory(left, top, prevSelectedItem);
				updateItemText = true;
				break;
			}
		}

		const bool cursorDown = _engine->_input->toggleActionIfActive(TwinEActionType::UIDown);
		const bool cursorUp = _engine->_input->toggleActionIfActive(TwinEActionType::UIUp);
		const bool cursorLeft = _engine->_input->toggleActionIfActive(TwinEActionType::UILeft);
		const bool cursorRight = _engine->_input->toggleActionIfActive(TwinEActionType::UIRight);

		if (cursorDown) {
			_inventorySelectedItem++;
			if (_inventorySelectedItem >= NUM_INVENTORY_ITEMS) {
				_inventorySelectedItem = 0;
			}
			drawOneInventory(left, top, prevSelectedItem);
			updateItemText = true;
		} else if (cursorUp) {
			_inventorySelectedItem--;
			if (_inventorySelectedItem < 0) {
				_inventorySelectedItem = NUM_INVENTORY_ITEMS - 1;
			}
			drawOneInventory(left, top, prevSelectedItem);
			updateItemText = true;
		} else if (cursorLeft) {
			_inventorySelectedItem -= 4;
			if (_inventorySelectedItem < 0) {
				_inventorySelectedItem += NUM_INVENTORY_ITEMS;
			}
			drawOneInventory(left, top, prevSelectedItem);
			updateItemText = true;
		} else if (cursorRight) {
			_inventorySelectedItem += 4;
			if (_inventorySelectedItem >= NUM_INVENTORY_ITEMS) {
				_inventorySelectedItem -= NUM_INVENTORY_ITEMS;
			}
			drawOneInventory(left, top, prevSelectedItem);
			updateItemText = true;
		}

		if (updateItemText) {
			_engine->_text->secondInitDialWindow();
			if (_inventorySelectedItem < NUM_INVENTORY_ITEMS && _engine->_gameState->hasItem((InventoryItems)_inventorySelectedItem) && !_engine->_gameState->inventoryDisabled()) {
				_engine->_text->initInventoryText((InventoryItems)_inventorySelectedItem);
			} else {
				_engine->_text->initInventoryText(InventoryItems::MaxInventoryItems);
			}
			dialstate = ProgressiveTextState::End;
			updateItemText = false;
		}

		if (dialstate != ProgressiveTextState::NextPage) {
			dialstate = _engine->_text->nextDialChar();
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::UINextPage)) {
			// restart the item description to appear from the beginning
			if (dialstate == ProgressiveTextState::End) {
				updateItemText = true;
			}
			if (dialstate == ProgressiveTextState::NextPage) {
				_engine->_text->secondInitDialWindow();
				dialstate = ProgressiveTextState::ContinueRunning;
			}
		}

		drawOneInventory(left, top, _inventorySelectedItem);

		if (_inventorySelectedItem < NUM_INVENTORY_ITEMS && _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter) && _engine->_gameState->hasItem((InventoryItems)_inventorySelectedItem) && !_engine->_gameState->inventoryDisabled()) {
			_engine->_loopInventoryItem = _inventorySelectedItem;
			_inventorySelectedColor = COLOR_91;
			drawOneInventory(left, top, _inventorySelectedItem);
			break;
		}
	}

	_engine->_text->closeDial();

	_engine->_scene->_alphaLight = tmpAlphaLight;
	_engine->_scene->_betaLight = tmpBetaLight;

	_engine->_gameState->init3DGame();

	_engine->_text->initSceneTextBank();

	// this is a hack to 'fix' https://bugs.scummvm.org/ticket/13677
	_engine->_input->toggleActionIfActive(TwinEActionType::RecenterScreenOnTwinsen);
}

} // namespace TwinE
