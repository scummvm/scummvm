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

#include "twine/menu/menu.h"
#include "audio/mixer.h"
#include "backends/audiocd/audiocd.h"
#include "backends/keymapper/keymapper.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/util.h"
#include "graphics/cursorman.h"
#include "twine/scene/actor.h"
#include "twine/scene/animations.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/scene/gamestate.h"
#include "twine/scene/grid.h"
#include "twine/resources/hqr.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/menu/menuoptions.h"
#include "twine/scene/movements.h"
#include "twine/renderer/redraw.h"
#include "twine/renderer/renderer.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/scene.h"
#include "twine/shared.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

static const uint32 kPlasmaEffectFilesize = 262176;

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
	kWallCollision = 10
};
}

#define checkMenuQuit(callMenu)      \
	if ((callMenu) == kQuitEngine) { \
		return kQuitEngine;          \
	}
#define kBackground 9999

namespace _priv {

static MenuSettings createMainMenu() {
	MenuSettings settings;
	settings.setButtonsBoxHeight(200);
	settings.addButton(TextId::kNewGame);
	settings.addButton(TextId::kContinueGame);
	settings.addButton(TextId::kOptions);
	settings.addButton(TextId::kQuit);
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

static MenuSettings createOptionsMenu() {
	MenuSettings settings;
	settings.addButton(TextId::kReturnMenu);
	settings.addButton(TextId::kVolumeSettings);
	settings.addButton(TextId::kSaveManage);
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
		const TextId textId = getButtonTextId(buttonIndex);
		char dialText[256] = "";
		text->getMenuText(textId, dialText, sizeof(dialText));
		_buttonTexts[buttonIndex] = dialText;
	}
	return _buttonTexts[buttonIndex].c_str();
}

Menu::Menu(TwinEEngine *engine) {
	_engine = engine;

	optionsMenuState = _priv::createOptionsMenu();
	giveUpMenuWithSaveState = _priv::createGiveUpSaveMenu();
	volumeMenuState = _priv::createVolumeMenu();
	saveManageMenuState = _priv::createSaveManageMenu();
	giveUpMenuState = _priv::createGiveUpMenu();
	mainMenuState = _priv::createMainMenu();
	advOptionsMenuState = _priv::createAdvancedOptionsMenu();

	Common::fill(&behaviourAnimState[0], &behaviourAnimState[4], 0);
	Common::fill(&itemAngle[0], &itemAngle[NUM_INVENTORY_ITEMS], 0);
}

Menu::~Menu() {
	free(plasmaEffectPtr);
}

void Menu::plasmaEffectRenderFrame() {
	for (int32 j = 1; j < PLASMA_HEIGHT - 1; j++) {
		for (int32 i = 1; i < PLASMA_WIDTH - 1; i++) {
			/* Here we calculate the average of all 8 neighbour pixel values */

			int16 c;
			c = plasmaEffectPtr[(i - 1) + (j - 1) * PLASMA_WIDTH];  //top-left
			c += plasmaEffectPtr[(i + 0) + (j - 1) * PLASMA_WIDTH]; //top
			c += plasmaEffectPtr[(i + 1) + (j - 1) * PLASMA_WIDTH]; //top-right

			c += plasmaEffectPtr[(i - 1) + (j + 0) * PLASMA_WIDTH]; //left
			c += plasmaEffectPtr[(i + 1) + (j + 0) * PLASMA_WIDTH]; //right

			c += plasmaEffectPtr[(i - 1) + (j + 1) * PLASMA_WIDTH]; // bottom-left
			c += plasmaEffectPtr[(i + 0) + (j + 1) * PLASMA_WIDTH]; // bottom
			c += plasmaEffectPtr[(i + 1) + (j + 1) * PLASMA_WIDTH]; // bottom-right

			/* And the 2 least significant bits are used as a
			 * randomizing parameter for statistically fading the flames */
			c = (c >> 3) | ((c & 0x0003) << 13);

			if (!(c & 0x6500) &&
			    (j >= (PLASMA_HEIGHT - 4) || c > 0)) {
				c--; /*fade this pixel*/
			}

			/* plot the pixel using the calculated color */
			plasmaEffectPtr[i + (PLASMA_HEIGHT + j) * PLASMA_WIDTH] = (uint8)c;
		}
	}

	// flip the double-buffer while scrolling the effect vertically:
	const uint8 *src = plasmaEffectPtr + (PLASMA_HEIGHT + 1) * PLASMA_WIDTH;
	memcpy(plasmaEffectPtr, src, PLASMA_HEIGHT * PLASMA_WIDTH);
}

void Menu::processPlasmaEffect(const Common::Rect &rect, int32 color) {
	const int32 max_value = color + 15;

	plasmaEffectRenderFrame();

	const uint8 *in = plasmaEffectPtr + 5 * PLASMA_WIDTH;
	uint8 *out = (uint8 *)_engine->imageBuffer.getBasePtr(0, 0);

	for (int32 y = 0; y < PLASMA_HEIGHT / 2; y++) {
		int32 yOffset = y * _engine->imageBuffer.w;
		const uint8 *colPtr = &in[y * PLASMA_WIDTH];
		for (int32 x = 0; x < PLASMA_WIDTH; x++) {
			const uint8 c = MIN(*colPtr / 2 + color, max_value);
			/* 2x2 squares sharing the same pixel color: */
			const int32 target = 2 * yOffset;
			out[target + 0] = c;
			out[target + 1] = c;
			out[target + _engine->imageBuffer.w + 0] = c;
			out[target + _engine->imageBuffer.w + 1] = c;
			++colPtr;
			++yOffset;
		}
	}
	const Common::Rect prect(0, 0, PLASMA_WIDTH, PLASMA_HEIGHT);
	_engine->frontVideoBuffer.blitFrom(_engine->imageBuffer, prect, rect);
}

void Menu::drawBox(const Common::Rect &rect, int32 colorLeftTop, int32 colorRightBottom) {
	_engine->_interface->drawLine(rect.left, rect.top, rect.right, rect.top, colorLeftTop);           // top line
	_engine->_interface->drawLine(rect.left, rect.top, rect.left, rect.bottom, colorLeftTop);         // left line
	_engine->_interface->drawLine(rect.right, rect.top + 1, rect.right, rect.bottom, colorRightBottom);   // right line
	_engine->_interface->drawLine(rect.left + 1, rect.bottom, rect.right, rect.bottom, colorRightBottom); // bottom line
}

void Menu::drawBox(int32 left, int32 top, int32 right, int32 bottom, int32 colorLeftTop, int32 colorRightBottom) {
	drawBox(Common::Rect(left, top, right, bottom), colorLeftTop, colorLeftTop);
}

void Menu::drawButtonGfx(const MenuSettings *menuSettings, const Common::Rect &rect, int32 buttonId, const char *dialText, bool hover) {
	if (hover) {
		if (menuSettings == &volumeMenuState && buttonId <= MenuButtonTypes::kSpeechVolume && buttonId >= MenuButtonTypes::kMusicVolume) {
			int32 newWidth = 0;
			switch (buttonId) {
			case MenuButtonTypes::kMusicVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
				newWidth = _engine->_screens->crossDot(rect.left, rect.right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			case MenuButtonTypes::kSoundVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType);
				newWidth = _engine->_screens->crossDot(rect.left, rect.right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			case MenuButtonTypes::kCDVolume: {
				const AudioCDManager::Status status = _engine->_system->getAudioCDManager()->getStatus();
				newWidth = _engine->_screens->crossDot(rect.left, rect.right, Audio::Mixer::kMaxMixerVolume, status.volume);
				break;
			}
			case MenuButtonTypes::kSpeechVolume: {
				const int volume = _engine->_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType);
				newWidth = _engine->_screens->crossDot(rect.left, rect.right, Audio::Mixer::kMaxMixerVolume, volume);
				break;
			}
			}

			processPlasmaEffect(rect, COLOR_80);
			if (!(_engine->getRandomNumber() % 5)) {
				plasmaEffectPtr[_engine->getRandomNumber() % 140 * 10 + 1900] = 255;
			}
			_engine->_interface->drawFilledRect(Common::Rect(newWidth, rect.top, rect.right, rect.bottom), COLOR_68);
		} else {
			processPlasmaEffect(rect, COLOR_64);
			if (!(_engine->getRandomNumber() % 5)) {
				plasmaEffectPtr[_engine->getRandomNumber() % PLASMA_WIDTH * 10 + 6400] = 255;
			}
		}
	} else {
		_engine->_interface->blitBox(rect, _engine->workVideoBuffer, _engine->frontVideoBuffer);
		_engine->_interface->drawTransparentBox(rect, 4);
	}

	drawBox(rect);

	_engine->_text->setFontColor(COLOR_WHITE);
	_engine->_text->setFontParameters(2, 8);
	const int32 textSize = _engine->_text->getTextSize(dialText);
	_engine->_text->drawText((_engine->width() / 2) - (textSize / 2), rect.top + 7, dialText);

	_engine->copyBlockPhys(rect);
}

int16 Menu::drawButtons(MenuSettings *menuSettings, bool hover) {
	int16 buttonNumber = menuSettings->getActiveButton();
	const int32 maxButton = menuSettings->getButtonCount();
	int32 topHeight = menuSettings->getButtonBoxHeight();

	if (topHeight == 0) {
		topHeight = 35;
	} else {
		topHeight = topHeight - (((maxButton - 1) * 6) + (maxButton * 50)) / 2;
	}

	if (maxButton <= 0) {
		return -1;
	}

	int16 mouseActiveButton = -1;

	for (int16 i = 0; i < maxButton; ++i) {
		if (menuSettings == &advOptionsMenuState) {
			int16 id = menuSettings->getButtonState(i);
			switch (id) {
			case MenuButtonTypes::kAggressiveMode:
				if (_engine->_actor->autoAggressive) {
					menuSettings->setButtonTextId(i, TextId::kBehaviourAggressiveAuto);
				} else {
					menuSettings->setButtonTextId(i, TextId::kBehaviourAggressiveManual);
				}
				break;
			case MenuButtonTypes::kPolygonDetails:
				if (_engine->cfgfile.PolygonDetails == 0) {
					menuSettings->setButtonTextId(i, TextId::kDetailsPolygonsLow);
				} else if (_engine->cfgfile.PolygonDetails == 1) {
					menuSettings->setButtonTextId(i, TextId::kDetailsPolygonsMiddle);
				} else {
					menuSettings->setButtonTextId(i, TextId::kDetailsPolygonsHigh);
				}
				break;
			case MenuButtonTypes::kShadowSettings:
				if (_engine->cfgfile.ShadowMode == 0) {
					menuSettings->setButtonTextId(i, TextId::kShadowsDisabled);
				} else if (_engine->cfgfile.ShadowMode == 1) {
					menuSettings->setButtonTextId(i, TextId::kShadowsFigures);
				} else {
					menuSettings->setButtonTextId(i, TextId::kDetailsShadowHigh);
				}
				break;
			case MenuButtonTypes::kSceneryZoom:
				if (_engine->cfgfile.SceZoom) {
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
		}
		const int32 menuItemId = menuSettings->getButtonState(i);
		const char *text = menuSettings->getButtonText(_engine->_text, i);
		const int32 border = 45;
		const int32 mainMenuButtonHeightHalf = 25;
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

		topHeight += 56; // increase button top height
	}
	return mouseActiveButton;
}

int32 Menu::processMenu(MenuSettings *menuSettings, bool showCredits) {
	int16 currentButton = menuSettings->getActiveButton();
	bool buttonsNeedRedraw = true;
	const int32 numEntry = menuSettings->getButtonCount();
	int32 maxButton = numEntry - 1;
	Common::Point mousepos = _engine->_input->getMousePositions();
	bool useMouse = true;

	_engine->_input->enableKeyMap(uiKeyMapId);

	// if we are running the game already, the buttons are just rendered on top of the scene
	if (_engine->_scene->isGameRunning()) {
		_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	} else {
		_engine->_screens->loadMenuImage(false);
	}
	uint32 startMillis = _engine->_system->getMillis();
	do {
		FrameMarker frame(_engine);
		const uint32 loopMillis = _engine->_system->getMillis();
		_engine->readKeys();

		Common::Point newmousepos = _engine->_input->getMousePositions();
		if (mousepos != newmousepos) {
			useMouse = true;
			mousepos = newmousepos;
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::UIDown)) {
			currentButton++;
			if (currentButton == numEntry) { // if current button is the last, than next button is the first
				currentButton = 0;
			}
			useMouse = false;
			buttonsNeedRedraw = true;
			startMillis = loopMillis;
		} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIUp)) {
			currentButton--;
			if (currentButton < 0) { // if current button is the first, than previous button is the last
				currentButton = maxButton;
			}
			useMouse = false;
			buttonsNeedRedraw = true;
			startMillis = loopMillis;
		}

		const int16 id = menuSettings->getActiveButtonState();
		if (menuSettings == &advOptionsMenuState) {
			switch (id) {
			case MenuButtonTypes::kAggressiveMode:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft) || _engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					_engine->_actor->autoAggressive = !_engine->_actor->autoAggressive;
					startMillis = loopMillis;
				}
				break;
			case MenuButtonTypes::kPolygonDetails:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft)) {
					_engine->cfgfile.PolygonDetails--;
					_engine->cfgfile.PolygonDetails %= 3;
					startMillis = loopMillis;
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					_engine->cfgfile.PolygonDetails++;
					_engine->cfgfile.PolygonDetails %= 3;
					startMillis = loopMillis;
				}
				break;
			case MenuButtonTypes::kShadowSettings:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft)) {
					_engine->cfgfile.ShadowMode--;
					_engine->cfgfile.ShadowMode %= 3;
					startMillis = loopMillis;
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					_engine->cfgfile.ShadowMode++;
					_engine->cfgfile.ShadowMode %= 3;
					startMillis = loopMillis;
				}
				break;
			case MenuButtonTypes::kSceneryZoom:
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft) || _engine->_input->toggleActionIfActive(TwinEActionType::UIRight) || _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					_engine->cfgfile.SceZoom = !_engine->cfgfile.SceZoom;
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
		} else if (menuSettings == &volumeMenuState) {
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
			if (useMouse && mouseButtonHovered != -1) {
				currentButton = mouseButtonHovered;
			}
			menuSettings->setActiveButton(currentButton);
		}

		// draw plasma effect for the current selected button
		const int16 mouseButtonHovered = drawButtons(menuSettings, true);
		if (useMouse && mouseButtonHovered != -1) {
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
		if (showCredits && loopMillis - startMillis > 11650) {
			_engine->_menuOptions->showCredits();
			// TODO the original game also performs these actions:
			// play FLA_DRAGON3 fla
			// display RESSHQR_INTROSCREEN1IMG
			// display RESSHQR_INTROSCREEN2IMG
			// display RESSHQR_INTROSCREEN3IMG
			// play FLA_BATEAU fla
			// if version == EUROPE_VERSION display RESSHQR_LBAIMG else display RESSHQR_RELLENTIMG
			// display adeline logo
			// pressing any key during these actions will abort everything and return to the menu
			startMillis = _engine->_system->getMillis();
			_engine->_screens->loadMenuImage(false);
		}
	} while (!_engine->_input->toggleActionIfActive(TwinEActionType::UIEnter));

	return (int32)menuSettings->getActiveButtonTextId();
}

int32 Menu::advoptionsMenu() {
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (processMenu(&advOptionsMenuState)) {
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
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (processMenu(&saveManageMenuState)) {
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

int32 Menu::volumeMenu() {
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (processMenu(&volumeMenuState)) {
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

void Menu::inGameOptionsMenu() {
	_engine->_text->initTextBank(TextBankId::Options_and_menus);
	optionsMenuState.setButtonTextId(0, TextId::kReturnGame);
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
	optionsMenu();
	_engine->_text->initSceneTextBank();
	optionsMenuState.setButtonTextId(0, TextId::kReturnMenu);
}

int32 Menu::optionsMenu() {
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);

	_engine->_sound->stopSamples();
	_engine->_music->playTrackMusic(9); // LBA's Theme

	ScopedCursor scoped(_engine);
	for (;;) {
		switch (processMenu(&optionsMenuState)) {
		case (int32)TextId::kReturnGame:
		case (int32)TextId::kReturnMenu: {
			return 0;
		}
		case (int32)TextId::kVolumeSettings: {
			checkMenuQuit(volumeMenu()) break;
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
	// load menu effect file only once
	plasmaEffectPtr = (uint8 *)malloc(kPlasmaEffectFilesize);
	memset(plasmaEffectPtr, 0, kPlasmaEffectFilesize);

	CursorMan.pushCursor(cursorArrow, 11, 16, 1, 1, 3);
	CursorMan.pushCursorPalette(cursorPalette, 0, 2);
	return HQR::getEntry(plasmaEffectPtr, Resources::HQR_RESS_FILE, RESSHQR_PLASMAEFFECT) > 0;
}

EngineState Menu::run() {
	FrameMarker frame(_engine);
	_engine->_text->initTextBank(TextBankId::Options_and_menus);

	_engine->_music->playTrackMusic(9); // LBA's Theme
	_engine->_sound->stopSamples();

	ScopedCursor scoped(_engine);
	switch (processMenu(&mainMenuState)) {
	case (int32)TextId::kNewGame: {
		if (_engine->_menuOptions->newGameMenu()) {
			return EngineState::GameLoop;
		}
		break;
	}
	case (int32)TextId::kContinueGame: {
		if (_engine->_menuOptions->continueGameMenu()) {
			return EngineState::LoadedGame;
		}
		break;
	}
	case (int32)TextId::kOptions: {
		optionsMenu();
		break;
	}
	case kBackground: {
		_engine->_screens->loadMenuImage();
		break;
	}
	case (int32)TextId::kQuit:
	case kQuitEngine:
		debug("quit the game");
		return EngineState::QuitGame;
	}
	return EngineState::Menu;
}

int32 Menu::giveupMenu() {
	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
	_engine->_sound->pauseSamples();

	MenuSettings *localMenu;
	if (_engine->cfgfile.UseAutoSaving) {
		localMenu = &giveUpMenuState;
	} else {
		localMenu = &giveUpMenuWithSaveState;
	}

	ScopedCursor scoped(_engine);

	int32 menuId;
	do {
		FrameMarker frame(_engine);
		_engine->_text->initTextBank(TextBankId::Options_and_menus);
		menuId = processMenu(localMenu);
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
	_engine->_grid->drawSprite(left, top + 3, _engine->_resources->spriteData[SPRITEHQR_LIFEPOINTS]);
	const int32 barLeft = left + barLeftPadding;
	const int32 healthBarRight = _engine->_screens->crossDot(barLeft, right, 50, _engine->_scene->sceneHero->life);
	const int32 barBottom = top + barHeight;
	_engine->_interface->drawFilledRect(Common::Rect(barLeft, top, healthBarRight, barBottom), COLOR_91);
	drawBox(Common::Rect(barLeft, top, right, barBottom));
}

void Menu::drawCloverLeafs(int32 newBoxLeft, int32 boxRight, int32 top) {
	// Clover leaf boxes
	for (int32 i = 0; i < _engine->_gameState->inventoryNumLeafsBox; i++) {
		const int32 leftSpritePos = _engine->_screens->crossDot(newBoxLeft, boxRight, 10, i);
		_engine->_grid->drawSprite(leftSpritePos, top + 58, _engine->_resources->spriteData[SPRITEHQR_CLOVERLEAFBOX]);
	}

	// Clover leafs
	for (int32 i = 0; i < _engine->_gameState->inventoryNumLeafs; i++) {
		const int32 leftSpritePos = _engine->_screens->crossDot(newBoxLeft, boxRight, 10, i);
		_engine->_grid->drawSprite(leftSpritePos + 2, top + 60, _engine->_resources->spriteData[SPRITEHQR_CLOVERLEAF]);
	}
}

void Menu::drawMagicPointsBar(int32 left, int32 right, int32 top, int32 barLeftPadding, int32 barHeight) {
	if (_engine->_gameState->inventoryDisabled()) {
		return;
	}
	if (!_engine->_gameState->hasItem(InventoryItems::kiTunic)) {
		return;
	}
	_engine->_grid->drawSprite(left, top + 1, _engine->_resources->spriteData[SPRITEHQR_MAGICPOINTS]);
	if (_engine->_gameState->magicLevelIdx <= 0) {
		return;
	}
	const int32 barLeft = left + barLeftPadding;
	const int32 barBottom = top + barHeight;
	const int32 barRight = _engine->_screens->crossDot(barLeft, right, 80, _engine->_gameState->inventoryMagicPoints);
	const Common::Rect pointsRect(barLeft, top, barRight, barBottom);
	_engine->_interface->drawFilledRect(pointsRect, COLOR_75);
	drawBox(barLeft, top, barLeft + _engine->_gameState->magicLevelIdx * 80, barBottom);
}

void Menu::drawSpriteAndString(int32 left, int32 top, const SpriteData &spriteData, const Common::String &str, int32 color) {
	_engine->_grid->drawSprite(left, top + 15, spriteData);
	_engine->_text->setFontColor(color);
	_engine->_text->drawText(left + 30, top + 5, str.c_str());
}

void Menu::drawCoins(int32 left, int32 top) {
	const Common::String &inventoryNumKashes = Common::String::format("%d", _engine->_gameState->inventoryNumKashes);
	drawSpriteAndString(left, top, _engine->_resources->spriteData[SPRITEHQR_KASHES], inventoryNumKashes);
}

void Menu::drawKeys(int32 left, int32 top) {
	const Common::String &inventoryNumKeys = Common::String::format("%d", _engine->_gameState->inventoryNumKeys);
	drawSpriteAndString(left, top, _engine->_resources->spriteData[SPRITEHQR_KEY], inventoryNumKeys);
}

void Menu::drawInfoMenu(int16 left, int16 top, int16 width) {
	_engine->_interface->resetClip();
	const int16 height = 80;
	const Common::Rect rect(left, top, left + width, top + height);
	drawBox(rect);
	Common::Rect filledRect(rect);
	filledRect.grow(-1);
	_engine->_interface->drawFilledRect(filledRect, COLOR_BLACK);

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
	const Common::Rect &boxRect = calcBehaviourRect(left, top, behaviour);
	return _engine->_input->isMouseHovering(boxRect);
}

void Menu::drawBehaviour(int32 left, int32 top, HeroBehaviourType behaviour, int32 angle, bool cantDrawBox, Common::Rect &dirtyRect) {
	const Common::Rect &boxRect = calcBehaviourRect(left, top, behaviour);

	const int animIdx = _engine->_actor->heroAnimIdx[(byte)behaviour];
	const AnimData &currentAnimData = _engine->_resources->animData[animIdx];

	uint currentAnimState = behaviourAnimState[(byte)behaviour];

	if (_engine->_animations->setModelAnimation(currentAnimState, currentAnimData, *behaviourEntity, &behaviourAnimData[(byte)behaviour])) {
		currentAnimState++; // keyframe
		if (currentAnimState >= currentAnimData.getNumKeyframes()) {
			currentAnimState = currentAnimData.getLoopFrame();
		}
		behaviourAnimState[(byte)behaviour] = currentAnimState;
	}

	if (!cantDrawBox) {
		Common::Rect boxRectCopy(boxRect);
		boxRectCopy.grow(1);
		drawBox(boxRectCopy);
	}

	_engine->_interface->saveClip();
	_engine->_interface->resetClip();

	if (behaviour == _engine->_actor->heroBehaviour) {
		const int titleOffset = 10;
		const int titleHeight = 40;
		const int32 titleBoxLeft = left + 10;
		const int32 titleBoxWidth = 430;
		const int32 titleBoxCenter = titleBoxLeft + titleBoxWidth / 2;
		const int32 titleBoxRight = titleBoxLeft + titleBoxWidth;
		const int32 titleBoxTop = boxRect.bottom + titleOffset;
		const int32 titleBoxBottom = titleBoxTop + titleHeight;

		_engine->_interface->drawFilledRect(boxRect, COLOR_BRIGHT_BLUE2);

		// behaviour menu title
		const Common::Rect titleRect(titleBoxLeft, titleBoxTop, titleBoxRight, titleBoxBottom);
		_engine->_interface->drawFilledRect(titleRect, COLOR_BLACK);
		drawBox(titleRect);

		_engine->_text->setFontColor(COLOR_WHITE);

		char dialText[256];
		_engine->_text->getMenuText(_engine->_actor->getTextIdForBehaviour(), dialText, sizeof(dialText));

		_engine->_text->drawText(titleBoxCenter - _engine->_text->getTextSize(dialText) / 2, titleBoxTop + 1, dialText);
		_engine->copyBlockPhys(titleRect);
	} else {
		_engine->_interface->drawFilledRect(boxRect, COLOR_BLACK);
	}

	_engine->_renderer->renderBehaviourModel(boxRect, -600, angle, *behaviourEntity);

	if (dirtyRect.isEmpty()) {
		dirtyRect = boxRect;
	} else {
		dirtyRect.extend(boxRect);
	}

	_engine->_interface->loadClip();
}

void Menu::prepareAndDrawBehaviour(int32 left, int32 top, int32 angle, HeroBehaviourType behaviour, Common::Rect &dirtyRect) {
	const int animIdx = _engine->_actor->heroAnimIdx[(byte)behaviour];
	_engine->_animations->setAnimAtKeyframe(behaviourAnimState[(byte)behaviour], _engine->_resources->animData[animIdx], *behaviourEntity, &behaviourAnimData[(byte)behaviour]);
	drawBehaviour(left, top, behaviour, angle, false, dirtyRect);
}

void Menu::drawBehaviourMenu(int32 left, int32 top, int32 angle) {
	const int32 width = 450;
	const int32 height = 190;
	const int32 right = left + width;
	const int32 bottom = top + height;

	const Common::Rect titleRect(left, top, right, bottom);
	drawBox(titleRect);

	Common::Rect boxRect(titleRect);
	boxRect.grow(-1);
	_engine->_interface->drawTransparentBox(boxRect, 2);

	Common::Rect ignoreRect;
	prepareAndDrawBehaviour(left, top, angle, HeroBehaviourType::kNormal, ignoreRect);
	prepareAndDrawBehaviour(left, top, angle, HeroBehaviourType::kAthletic, ignoreRect);
	prepareAndDrawBehaviour(left, top, angle, HeroBehaviourType::kAggressive, ignoreRect);
	prepareAndDrawBehaviour(left, top, angle, HeroBehaviourType::kDiscrete, ignoreRect);

	_engine->copyBlockPhys(titleRect);

	drawInfoMenu(titleRect.left, titleRect.bottom + 10, titleRect.width());
}

void Menu::processBehaviourMenu() {
	_engine->exitSceneryView();
	if (_engine->_actor->heroBehaviour == HeroBehaviourType::kProtoPack) {
		_engine->_sound->stopSamples();
		_engine->_actor->setBehaviour(HeroBehaviourType::kNormal);
	}

	behaviourEntity = &_engine->_resources->bodyData[_engine->_scene->sceneHero->entity];

	_engine->_actor->heroAnimIdx[(byte)HeroBehaviourType::kNormal] = _engine->_actor->heroAnimIdxNORMAL;
	_engine->_actor->heroAnimIdx[(byte)HeroBehaviourType::kAthletic] = _engine->_actor->heroAnimIdxATHLETIC;
	_engine->_actor->heroAnimIdx[(byte)HeroBehaviourType::kAggressive] = _engine->_actor->heroAnimIdxAGGRESSIVE;
	_engine->_actor->heroAnimIdx[(byte)HeroBehaviourType::kDiscrete] = _engine->_actor->heroAnimIdxDISCRETE;

	_engine->_movements->setActorAngleSafe(_engine->_scene->sceneHero->angle, _engine->_scene->sceneHero->angle - ANGLE_90, ANGLE_17, &moveMenu);

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	TextBankId tmpTextBank = _engine->_scene->sceneTextBank;
	_engine->_scene->sceneTextBank = TextBankId::None;

	_engine->_text->initTextBank(TextBankId::Options_and_menus);

	const int32 left = _engine->width() / 2 - 220;
	const int32 top = _engine->height() / 2 - 140;
	drawBehaviourMenu(left, top, _engine->_scene->sceneHero->angle);

	HeroBehaviourType tmpHeroBehaviour = _engine->_actor->heroBehaviour;

	const int animIdx = _engine->_actor->heroAnimIdx[(byte)_engine->_actor->heroBehaviour];
	_engine->_animations->setAnimAtKeyframe(behaviourAnimState[(byte)_engine->_actor->heroBehaviour], _engine->_resources->animData[animIdx], *behaviourEntity, &behaviourAnimData[(byte)_engine->_actor->heroBehaviour]);

	int32 tmpTime = _engine->lbaTime;

#if 0
	ScopedCursor scopedCursor(_engine);
#endif
	ScopedKeyMap scopedKeyMap(_engine, uiKeyMapId);
	while (_engine->_input->isActionActive(TwinEActionType::BehaviourMenu) || _engine->_input->isQuickBehaviourActionActive()) {
		FrameMarker frame(_engine, 50);
		_engine->readKeys();

#if 0
		if (isBehaviourHovered(HeroBehaviourType::kNormal)) {
			_engine->_actor->heroBehaviour = HeroBehaviourType::kNormal;
		} else if (isBehaviourHovered(HeroBehaviourType::kAthletic)) {
			_engine->_actor->heroBehaviour = HeroBehaviourType::kAthletic;
		} else if (isBehaviourHovered(HeroBehaviourType::kAggressive)) {
			_engine->_actor->heroBehaviour = HeroBehaviourType::kAggressive;
		} else if (isBehaviourHovered(HeroBehaviourType::kDiscrete)) {
			_engine->_actor->heroBehaviour = HeroBehaviourType::kDiscrete;
		}
#endif

		int heroBehaviour = (int)_engine->_actor->heroBehaviour;
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

		_engine->_actor->heroBehaviour = (HeroBehaviourType)heroBehaviour;

		Common::Rect dirtyRect;
		if (tmpHeroBehaviour != _engine->_actor->heroBehaviour) {
			drawBehaviour(left, top, tmpHeroBehaviour, _engine->_scene->sceneHero->angle, true, dirtyRect);
			tmpHeroBehaviour = _engine->_actor->heroBehaviour;
			_engine->_movements->setActorAngleSafe(_engine->_scene->sceneHero->angle, _engine->_scene->sceneHero->angle - ANGLE_90, ANGLE_17, &moveMenu);
			const int tmpAnimIdx = _engine->_actor->heroAnimIdx[(byte)_engine->_actor->heroBehaviour];
			_engine->_animations->setAnimAtKeyframe(behaviourAnimState[(byte)_engine->_actor->heroBehaviour], _engine->_resources->animData[tmpAnimIdx], *behaviourEntity, &behaviourAnimData[(byte)_engine->_actor->heroBehaviour]);
		}

		drawBehaviour(left, top, _engine->_actor->heroBehaviour, -1, true, dirtyRect);
		if (!dirtyRect.isEmpty()) {
			_engine->copyBlockPhys(dirtyRect);
		}

		_engine->lbaTime++;
	}

	_engine->lbaTime = tmpTime;

	_engine->_actor->setBehaviour(_engine->_actor->heroBehaviour);
	_engine->_gameState->initEngineProjections();

	_engine->_scene->sceneTextBank = tmpTextBank;
	_engine->_text->initSceneTextBank();
}

void Menu::drawItem(int32 left, int32 top, int32 item, Common::Rect &dirtyRect) {
	const int32 itemWidth = 74;
	const int32 itemHeight = 64;
	const int32 itemPadding = 11;
	const int32 itemWidthHalf = itemWidth / 2;
	const int32 itemHeightHalf = itemHeight / 2;
	const int32 itemX = (item / 4) * (itemWidth + itemPadding) + left + itemWidthHalf + itemPadding - 1;
	const int32 itemY = (item % 4) * (itemHeight + itemPadding) + top + itemHeightHalf + itemPadding - 1;
	const Common::Rect rect(itemX - itemWidthHalf, itemY - itemHeightHalf, itemX + itemWidthHalf, itemY + itemHeightHalf);
	const int32 color = inventorySelectedItem == item ? inventorySelectedColor : COLOR_BLACK;

	_engine->_interface->drawFilledRect(rect, color);

	if (item < NUM_INVENTORY_ITEMS && _engine->_gameState->hasItem((InventoryItems)item) && (!_engine->_gameState->inventoryDisabled() || item == InventoryItems::kiCloverLeaf)) {
		itemAngle[item] += ANGLE_2;
		_engine->_interface->setClip(rect);
		_engine->_renderer->renderInventoryItem(itemX, itemY, _engine->_resources->inventoryTable[item], itemAngle[item], 15000);
		_engine->_interface->resetClip();
		if (item == InventoryItems::kGasItem) {
			_engine->_text->setFontColor(COLOR_WHITE);
			const Common::String &inventoryNumGas = Common::String::format("%d", _engine->_gameState->inventoryNumGas);
			_engine->_text->drawText(rect.left + 3, rect.bottom - 32, inventoryNumGas.c_str());
		}
	}

	drawBox(rect);
	if (dirtyRect.isEmpty()) {
		dirtyRect = rect;
	} else {
		dirtyRect.extend(rect);
	}
}

void Menu::drawInventoryItems(int32 left, int32 top) {
	const Common::Rect rect(left, top, left + 605, top + 310);
	_engine->_interface->drawTransparentBox(rect, 4);
	drawBox(rect);
	drawBox(left + 93, top + 8, left + 93 + 78, top + 8 + 293, COLOR_75, COLOR_75);
	_engine->copyBlockPhys(rect);

	Common::Rect dirtyRect;
	for (int32 item = 0; item < NUM_INVENTORY_ITEMS; item++) {
		drawItem(left, top, item, dirtyRect);
	}
	if (!dirtyRect.isEmpty()) {
		_engine->copyBlockPhys(dirtyRect);
	}
}

void Menu::processInventoryMenu() {
	int32 tmpAlphaLight = _engine->_scene->alphaLight;
	int32 tmpBetaLight = _engine->_scene->betaLight;

	_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);

	_engine->_renderer->setLightVector(ANGLE_315, ANGLE_334, ANGLE_0);

	inventorySelectedColor = COLOR_68;

	if (_engine->_gameState->inventoryNumLeafs > 0) {
		_engine->_gameState->giveItem(InventoryItems::kiCloverLeaf);
		// TODO: shouldn't this get reset? } else {
		//	_engine->_gameState->removeItem(InventoryItems::kiCloverLeaf);
	}

	const int32 left = _engine->width() / 2 - 303;
	const int32 top = _engine->height() / 2 - 210;
	drawInventoryItems(left, top);

	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);

	_engine->_text->setFontCrossColor(COLOR_BRIGHT_BLUE);
	_engine->_text->initDialogueBox();

	ProgressiveTextState textState = ProgressiveTextState::ContinueRunning;
	bool updateItemText = true;

	//ScopedCursor scopedCursor(_engine);
	ScopedKeyMap scopedKeyMap(_engine, uiKeyMapId);
	for (;;) {
		FrameMarker frame(_engine, 66);
		_engine->readKeys();
		int32 prevSelectedItem = inventorySelectedItem;

		if (_engine->_input->toggleAbortAction() || _engine->shouldQuit()) {
			break;
		}

		const bool cursorDown = _engine->_input->toggleActionIfActive(TwinEActionType::UIDown);
		const bool cursorUp = _engine->_input->toggleActionIfActive(TwinEActionType::UIUp);
		const bool cursorLeft = _engine->_input->toggleActionIfActive(TwinEActionType::UILeft);
		const bool cursorRight = _engine->_input->toggleActionIfActive(TwinEActionType::UIRight);

		Common::Rect dirtyRect;
		if (cursorDown) {
			inventorySelectedItem++;
			if (inventorySelectedItem >= NUM_INVENTORY_ITEMS) {
				inventorySelectedItem = 0;
			}
			drawItem(left, top, prevSelectedItem, dirtyRect);
			updateItemText = true;
		} else if (cursorUp) {
			inventorySelectedItem--;
			if (inventorySelectedItem < 0) {
				inventorySelectedItem = NUM_INVENTORY_ITEMS - 1;
			}
			drawItem(left, top, prevSelectedItem, dirtyRect);
			updateItemText = true;
		} else if (cursorLeft) {
			inventorySelectedItem -= 4;
			if (inventorySelectedItem < 0) {
				inventorySelectedItem += NUM_INVENTORY_ITEMS;
			}
			drawItem(left, top, prevSelectedItem, dirtyRect);
			updateItemText = true;
		} else if (cursorRight) {
			inventorySelectedItem += 4;
			if (inventorySelectedItem >= NUM_INVENTORY_ITEMS) {
				inventorySelectedItem -= NUM_INVENTORY_ITEMS;
			}
			drawItem(left, top, prevSelectedItem, dirtyRect);
			updateItemText = true;
		}

		if (updateItemText) {
			_engine->_text->initInventoryDialogueBox();
			if (inventorySelectedItem < NUM_INVENTORY_ITEMS && _engine->_gameState->hasItem((InventoryItems)inventorySelectedItem) && !_engine->_gameState->inventoryDisabled()) {
				_engine->_text->initInventoryText((InventoryItems)inventorySelectedItem);
			} else {
				_engine->_text->initInventoryText(InventoryItems::MaxInventoryItems);
			}
			textState = ProgressiveTextState::ContinueRunning;
			updateItemText = false;
		}

		if (textState == ProgressiveTextState::ContinueRunning) {
			textState = _engine->_text->updateProgressiveText();
		} else {
			_engine->_text->fadeInRemainingChars();
		}

		if (_engine->_input->toggleActionIfActive(TwinEActionType::UINextPage)) {
			// restart the item description to appear from the beginning
			if (textState == ProgressiveTextState::End) {
				updateItemText = true;
			}
			if (textState == ProgressiveTextState::NextPage) {
				_engine->_text->initInventoryDialogueBox();
				textState = ProgressiveTextState::ContinueRunning;
			}
		}

		drawItem(left, top, inventorySelectedItem, dirtyRect);

		if (inventorySelectedItem < NUM_INVENTORY_ITEMS && _engine->_input->toggleActionIfActive(TwinEActionType::UIEnter) && _engine->_gameState->hasItem((InventoryItems)inventorySelectedItem) && !_engine->_gameState->inventoryDisabled()) {
			_engine->loopInventoryItem = inventorySelectedItem;
			inventorySelectedColor = COLOR_91;
			drawItem(left, top, inventorySelectedItem, dirtyRect);
			if (!dirtyRect.isEmpty()) {
				_engine->copyBlockPhys(dirtyRect);
			}
			break;
		}

		if (!dirtyRect.isEmpty()) {
			_engine->copyBlockPhys(dirtyRect);
		}
	}

	_engine->_text->_hasValidTextHandle = false;

	_engine->_scene->alphaLight = tmpAlphaLight;
	_engine->_scene->betaLight = tmpBetaLight;

	_engine->_gameState->initEngineProjections();

	_engine->_text->initSceneTextBank();
}

} // namespace TwinE
