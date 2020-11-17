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

#include "twine/menuoptions.h"
#include "common/error.h"
#include "common/keyboard.h"
#include "common/str-array.h"
#include "common/system.h"
#include "common/util.h"
#include "savestate.h"
#include "twine/flamovies.h"
#include "twine/gamestate.h"
#include "twine/input.h"
#include "twine/interface.h"
#include "twine/menu.h"
#include "twine/music.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/sound.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

#define ONSCREENKEYBOARD_WIDTH 14
#define ONSCREENKEYBOARD_HEIGHT 5

static const char allowedCharIndex[] = " ABCDEFGHIJKLM.NOPQRSTUVWXYZ-abcdefghijklm?nopqrstuvwxyz!0123456789\040\b\r\0";

void MenuOptions::newGame() {
	_engine->_music->stopMusic();
	_engine->_sound->stopSamples();

	int32 tmpFlagDisplayText = _engine->cfgfile.FlagDisplayText;
	_engine->cfgfile.FlagDisplayText = true;

	// intro screen 1 - twinsun
	_engine->_screens->loadImage(RESSHQR_INTROSCREEN1IMG);

	_engine->_text->newGameVar4 = 0;
	_engine->_text->newGameVar5 = 1;

	_engine->_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
	_engine->_text->textClipFull();
	_engine->_text->setFontCrossColor(15);

	bool aborted = _engine->_text->drawTextFullscreen(150);

	// intro screen 2
	if (!aborted) {
		_engine->_screens->loadImage(RESSHQR_INTROSCREEN2IMG);
		aborted |= _engine->_text->drawTextFullscreen(151);

		if (!aborted) {
			_engine->_screens->loadImage(RESSHQR_INTROSCREEN3IMG);
			aborted |= _engine->_text->drawTextFullscreen(152);
		}
	}
	_engine->cfgfile.FlagDisplayText = tmpFlagDisplayText;

	_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBACustom);
	_engine->_screens->clearScreen();
	_engine->flip();

	if (!aborted) {
		// _engine->_music->playMidiMusic(1);
		_engine->_flaMovies->playFlaMovie(FLA_INTROD);
	}

	_engine->_text->textClipSmall();
	_engine->_screens->clearScreen();
	_engine->flip();

	_engine->_text->newGameVar4 = 1;
	_engine->_text->newGameVar5 = 0;

	// set main palette back
	_engine->setPalette(_engine->_screens->paletteRGBA);
}

void MenuOptions::showCredits() {
	canShowCredits = true;
	int32 tmpShadowMode = _engine->cfgfile.ShadowMode;
	_engine->cfgfile.ShadowMode = 0;
	_engine->_gameState->initEngineVars();
	_engine->_scene->currentSceneIdx = LBA1SceneId::Credits_List_Sequence;
	_engine->_scene->needChangeScene = LBA1SceneId::Credits_List_Sequence;

	_engine->gameEngineLoop();

	canShowCredits = false;
	_engine->cfgfile.ShadowMode = tmpShadowMode;

	_engine->_screens->clearScreen();
	_engine->flip();

	_engine->_flaMovies->playFlaMovie(FLA_THEEND);

	_engine->_screens->clearScreen();
	_engine->flip();
	_engine->setPalette(_engine->_screens->paletteRGBA);
}

void MenuOptions::drawSelectableCharacter(int32 x, int32 y, bool selected) {
	char buffer[2];

	buffer[0] = allowedCharIndex[y + x * ONSCREENKEYBOARD_WIDTH];
	buffer[1] = '\0';

	const int32 centerX = y * 45 + 25;
	const int32 left = centerX - 20;
	const int32 right = centerX + 20;
	const int32 top = x * 56 + 200 - 25;
	const int32 centerY = x * 56 + 200;
	const int32 bottom = x * 56 + 200 + 25;

	if (selected) {
		_engine->_interface->drawSplittedBox(left, top, right, bottom, 91);
	} else {
		_engine->_interface->blitBox(left, top, right, bottom, _engine->workVideoBuffer, left, top, _engine->frontVideoBuffer);
		_engine->_interface->drawTransparentBox(left, top, right, bottom, 4);
	}

	_engine->_menu->drawBox(left, top, right, bottom);

	_engine->_text->setFontColor(15);
	_engine->_text->drawText(centerX - _engine->_text->getTextSize(buffer) / 2, centerY - 18, buffer);

	_engine->copyBlockPhys(left, top, right, bottom);
}

void MenuOptions::drawSelectableCharacters() {
	for (int8 x = 0; x < ONSCREENKEYBOARD_HEIGHT; x++) {
		for (int8 y = 0; y < ONSCREENKEYBOARD_WIDTH; y++) {
			drawSelectableCharacter(x, y, _onScreenKeyboardY == x && _onScreenKeyboardX == y);
		}
	}
}

void MenuOptions::drawPlayerName(int32 centerx, int32 top, int32 type) {
	const int32 left = 10;
	if (type == 1) {
		_engine->_menu->processPlasmaEffect(left, top, 32);
	}

	const int right = SCREEN_WIDTH - left;
	const int bottom = top + PLASMA_HEIGHT;
	_engine->_menu->drawBox(left, top, right, bottom);
	_engine->_interface->drawTransparentBox(left + 1, top + 1, right - 1, bottom - 1, 3);

	_engine->_text->drawText(centerx - _engine->_text->getTextSize(playerName) / 2, top + 6, playerName);
	_engine->copyBlockPhys(left, top, right, bottom);
}

/**
 * @brief Toggle a given @c OSystem::Feature and restore the previous state on destruction
 */
class ScopedFeatureState {
private:
	OSystem::Feature _feature;
	bool _changeTo;
public:
	ScopedFeatureState(OSystem::Feature feature, bool enable) : _feature(feature), _changeTo(enable) {
		if (g_system->getFeatureState(feature) != enable) {
			g_system->setFeatureState(feature, enable);
			_changeTo = !g_system->getFeatureState(feature);
		}
	}
	~ScopedFeatureState() {
		g_system->setFeatureState(_feature, _changeTo);
	}
};

bool MenuOptions::enterPlayerName(int32 textIdx) {
	playerName[0] = '\0'; // TODO: read from settings?
	_engine->_text->initTextBank(TextBankId::Options_and_menus);
	char buffer[256];
	_engine->_text->getMenuText(textIdx, buffer, sizeof(buffer));
	_engine->_text->setFontColor(15);
	const int halfScreenWidth = (SCREEN_WIDTH / 2);
	_engine->_text->drawText(halfScreenWidth - (_engine->_text->getTextSize(buffer) / 2), 20, buffer);
	_engine->copyBlockPhys(0, 0, SCREEN_WIDTH - 1, 99);
	_engine->flip();

	ScopedFeatureState scopedVirtualKeyboard(OSystem::kFeatureVirtualKeyboard, true);
	for (;;) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
				_engine->_input->processCustomEngineEventEnd(event);
				break;
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				_engine->_input->processCustomEngineEventStart(event);

				if (_engine->_input->toggleActionIfActive(TwinEActionType::UIEnter)) {
					if (_onScreenKeyboardLeaveViaOkButton) {
						if (_onScreenKeyboardX == ONSCREENKEYBOARD_WIDTH - 1 && _onScreenKeyboardY == ONSCREENKEYBOARD_HEIGHT - 1) {
							if (playerName[0] == '\0') {
								continue;
							}
							return true;
						}
						const size_t size = strlen(playerName);
						if (_onScreenKeyboardX == ONSCREENKEYBOARD_WIDTH - 2 && _onScreenKeyboardY == ONSCREENKEYBOARD_HEIGHT - 1) {
							if (size >= 1) {
								playerName[size - 1] = '\0';
							}
							continue;
						}
						const char chr = allowedCharIndex[_onScreenKeyboardX + _onScreenKeyboardY * ONSCREENKEYBOARD_WIDTH];
						playerName[size] = chr;
						playerName[size + 1] = '\0';
						if (size + 1 >= sizeof(playerName) - 1) {
							return true;
						}
						continue;
					}
					if (playerName[0] == '\0') {
						continue;
					}

					return true;
				}
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UIAbort)) {
					return false;
				}
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft)) {
					--_onScreenKeyboardX;
					if (_onScreenKeyboardX < 0) {
						_onScreenKeyboardX = ONSCREENKEYBOARD_WIDTH - 1;
					}
					_onScreenKeyboardLeaveViaOkButton = true;
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIRight)) {
					++_onScreenKeyboardX;
					if (_onScreenKeyboardX >= ONSCREENKEYBOARD_WIDTH) {
						_onScreenKeyboardX = 0;
					}
					_onScreenKeyboardLeaveViaOkButton = true;
				}
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UIUp)) {
					--_onScreenKeyboardY;
					if (_onScreenKeyboardY < 0) {
						_onScreenKeyboardY = ONSCREENKEYBOARD_HEIGHT - 1;
					}
					_onScreenKeyboardLeaveViaOkButton = true;
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIDown)) {
					++_onScreenKeyboardY;
					if (_onScreenKeyboardY >= ONSCREENKEYBOARD_HEIGHT) {
						_onScreenKeyboardY = 0;
					}
					_onScreenKeyboardLeaveViaOkButton = true;
				}

				break;
			case Common::EVENT_KEYDOWN: {
				const size_t size = strlen(playerName);
				if (!Common::isPrint(event.kbd.ascii)) {
					if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
						if (size >= 1) {
							playerName[size - 1] = '\0';
							_onScreenKeyboardLeaveViaOkButton = false;
						}
					}
					continue;
				}
				if (size >= sizeof(playerName) - 1) {
					return true;
				}
				if (strchr(allowedCharIndex, event.kbd.ascii)) {
					playerName[size] = event.kbd.ascii;
					playerName[size + 1] = '\0';
					_onScreenKeyboardLeaveViaOkButton = false;
				}

				break;
			}
			default:
				break;
			}
		}
		if (_engine->shouldQuit()) {
			break;
		}
		drawPlayerName(halfScreenWidth, 100, 1);
		drawSelectableCharacters();
		_engine->_system->delayMillis(1);
	}
	return false;
}

bool MenuOptions::newGameMenu() {
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();
	if (!enterPlayerName(TextId::kEnterYourName)) {
		return false;
	}
	_engine->_gameState->initEngineVars();
	newGame();
	return true;
}

int MenuOptions::chooseSave(int textIdx, bool showEmptySlots) {
	const SaveStateList& savegames = _engine->getSaveSlots();
	if (savegames.empty() && !showEmptySlots) {
		return -1;
	}

	_engine->_text->initTextBank(TextBankId::Options_and_menus);

	MenuSettings saveFiles;
	saveFiles.addButton(TextId::kReturnMenu);

	const int maxButtons = _engine->getMetaEngine().getMaximumSaveSlot() + 1;
	for (const SaveStateDescriptor& savegame : savegames) {
		saveFiles.addButton(savegame.getDescription().encode().c_str(), savegame.getSaveSlot());
		if (saveFiles.getButtonCount() >= maxButtons) {
			break;
		}
	}

	if (showEmptySlots) {
		while (saveFiles.getButtonCount() < maxButtons) {
			// the first button is the back button - to subtract that one again to get the real slot index
			saveFiles.addButton("EMPTY", saveFiles.getButtonCount() - 1);
		}
	}

	for (;;) {
		const int32 id = _engine->_menu->processMenu(&saveFiles);
		switch (id) {
		case kQuitEngine:
		case TextId::kReturnMenu:
			return -1;
		default:
			const int16 slot = saveFiles.getButtonState(id);
			debug("Selected slot %d for saving", slot);
			return slot;
		}
	}

	return -1;
}

bool MenuOptions::continueGameMenu() {
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();
	const int slot = chooseSave(TextId::kContinueGame);
	if (slot >= 0) {
		debug("Load slot %i", slot);
		Common::Error state = _engine->loadGameState(slot);
		if (state.getCode() != Common::kNoError) {
			error("Failed to load slot %i", slot);
			return false;
		}

		return true;
	}
	return false;
}

bool MenuOptions::deleteSaveMenu() {
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();
	const int slot = chooseSave(TextId::kDeleteSaveGame);
	if (slot >= 0) {
		_engine->wipeSaveSlot(slot);
		return true;
	}
	return false;
}

bool MenuOptions::saveGameMenu() {
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();
	const int slot = chooseSave(TextId::kCreateSaveGame, true);
	if (slot >= 0) {
		// TODO: enter description
		Common::Error state = _engine->saveGameState(slot, "description", false);
		if (state.getCode() != Common::kNoError) {
			error("Failed to save slot %i", slot);
			return false;
		}

		return true;
	}
	return false;
}

} // namespace TwinE
