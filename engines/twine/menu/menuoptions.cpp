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

#include "twine/menu/menuoptions.h"
#include "common/error.h"
#include "common/keyboard.h"
#include "common/str-array.h"
#include "common/system.h"
#include "common/util.h"
#include "savestate.h"
#include "twine/audio/music.h"
#include "twine/audio/sound.h"
#include "twine/movies.h"
#include "twine/scene/gamestate.h"
#include "twine/input.h"
#include "twine/menu/interface.h"
#include "twine/menu/menu.h"
#include "twine/renderer/screens.h"
#include "twine/resources/resources.h"
#include "twine/scene/scene.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

static const char allowedCharIndex[] = " ABCDEFGHIJKLM.NOPQRSTUVWXYZ-abcdefghijklm?nopqrstuvwxyz!0123456789\040\b\r\0";

void MenuOptions::newGame() {
	_engine->setTotalPlayTime(0);
	_engine->_music->stopMusic();
	_engine->_sound->stopSamples();

	if (_engine->isLBA1()) {
		int32 tmpFlagDisplayText = _engine->_cfgfile.FlagDisplayText;
		_engine->_cfgfile.FlagDisplayText = true;

		// intro screen 1 - twinsun
		_engine->_screens->loadImage(TwineImage(Resources::HQR_RESS_FILE, 15, 16));

		_engine->_text->_flagMessageShade = false;
		_engine->_text->_renderTextTriangle = true;

		_engine->_text->initDial(TextBankId::Inventory_Intro_and_Holomap);
		_engine->_text->bigWinDial();
		_engine->_text->setFontCrossColor(COLOR_WHITE);

		bool aborted = _engine->_text->drawTextProgressive(TextId::kIntroText1);

		// intro screen 2
		if (!aborted) {
			_engine->_screens->loadImage(TwineImage(Resources::HQR_RESS_FILE, 17, 18));
			aborted |= _engine->_text->drawTextProgressive(TextId::kIntroText2);

			if (!aborted) {
				_engine->_screens->loadImage(TwineImage(Resources::HQR_RESS_FILE, 19, 20));
				aborted |= _engine->_text->drawTextProgressive(TextId::kIntroText3);
			}
		}
		_engine->_cfgfile.FlagDisplayText = tmpFlagDisplayText;

		_engine->_screens->fadeToBlack(_engine->_screens->_palettePcx);
		_engine->_screens->clearScreen();

		if (!aborted) {
			_engine->_music->playMidiFile(1);
			_engine->_movie->playMovie(FLA_INTROD);
		}

		_engine->_text->normalWinDial();
	} else {
		_engine->_movie->playMovie(ACF_INTRO);
	}
	_engine->_screens->clearScreen();

	_engine->_text->_flagMessageShade = true;
	_engine->_text->_renderTextTriangle = false;

	// set main palette back
	_engine->setPalette(_engine->_screens->_ptrPal);
}

// TODO: dotemu has credits_<lang>.txt files
void MenuOptions::showCredits() {
	const int32 tmpShadowMode = _engine->_cfgfile.ShadowMode;
	_engine->_cfgfile.ShadowMode = 0;
	_engine->_gameState->initEngineVars();
	_engine->_scene->_numCube = LBA1SceneId::Credits_List_Sequence;
	_engine->_scene->_newCube = LBA1SceneId::Credits_List_Sequence;
	_engine->_screens->clearScreen();

	flagCredits = true;
	_engine->mainLoop();
	_engine->_scene->stopRunningGame();
	flagCredits = false;

	_engine->_cfgfile.ShadowMode = tmpShadowMode;

	_engine->_screens->clearScreen();

	_engine->_input->enableKeyMap(uiKeyMapId);
}

void MenuOptions::showEndSequence() {
	_engine->_movie->playMovie(FLA_THEEND);

	_engine->_screens->clearScreen();
	_engine->setPalette(_engine->_screens->_ptrPal);
}

void MenuOptions::drawSelectableCharacter(int32 x, int32 y) {
	const int32 borderTop = 200;
	const int32 borderLeft = _engine->width() / 2 - 295;
	const int32 halfButtonHeight = 25;
	const int32 halfButtonWidth = 20;
	const int32 buttonDistanceX = halfButtonWidth * 2 + 5;
	const int32 buttonDistanceY = halfButtonHeight * 2 + 5;
	const int32 centerX = x * buttonDistanceX + borderLeft;
	const int32 centerY = y * buttonDistanceY + borderTop;
	const Common::Rect rect(centerX - halfButtonWidth, centerY - halfButtonHeight, centerX + halfButtonWidth, centerY + halfButtonHeight);

	if (_engine->_input->isMouseHovering(rect)) {
		setOnScreenKeyboard(x, y);
	}

	const int idx = x + y * ONSCREENKEYBOARD_WIDTH;
	if (_onScreenKeyboardDirty[idx] == 0) {
		return;
	}

	--_onScreenKeyboardDirty[idx];

	const char buffer[]{allowedCharIndex[idx], '\0'};

	const bool selected = _onScreenKeyboardX == x && _onScreenKeyboardY == y;
	if (selected) {
		_engine->_interface->box(rect, COLOR_91);
	} else {
		_engine->blitWorkToFront(rect);
		_engine->_interface->shadeBox(rect, 4);
	}

	_engine->_menu->drawRectBorders(rect);

	_engine->_text->setFontColor(COLOR_WHITE);
	const uint8 character = (uint8)allowedCharIndex[idx];
	const int32 textX = centerX - _engine->_text->getCharWidth(character) / 2;
	const int32 textY = centerY - _engine->_text->getCharHeight(character) / 2;
	_engine->_text->drawText(textX, textY, buffer);
}

void MenuOptions::setOnScreenKeyboard(int x, int y) {
	if (x < 0) {
		x = ONSCREENKEYBOARD_WIDTH - 1;
	} else if (x >= ONSCREENKEYBOARD_WIDTH) {
		x = 0;
	}

	if (y < 0) {
		y = ONSCREENKEYBOARD_HEIGHT - 1;
	} else if (y >= ONSCREENKEYBOARD_HEIGHT) {
		y = 0;
	}

	if (_onScreenKeyboardX == x && _onScreenKeyboardY == y) {
		return;
	}

	++_onScreenKeyboardDirty[_onScreenKeyboardX + _onScreenKeyboardY * ONSCREENKEYBOARD_WIDTH];
	++_onScreenKeyboardDirty[x + y * ONSCREENKEYBOARD_WIDTH];

	_onScreenKeyboardX = x;
	_onScreenKeyboardY = y;

	_onScreenKeyboardLeaveViaOkButton = true;
}

void MenuOptions::drawSelectableCharacters() {
	for (int8 x = 0; x < ONSCREENKEYBOARD_WIDTH; x++) {
		for (int8 y = 0; y < ONSCREENKEYBOARD_HEIGHT; y++) {
			drawSelectableCharacter(x, y);
		}
	}
}

void MenuOptions::drawInputText(int32 centerx, int32 top, int32 type, const char *text) {
	const int32 left = 10;
	const int right = _engine->width() - left;
	const int bottom = top + PLASMA_HEIGHT;
	const Common::Rect rect(left, top, right, bottom);
	if (type == 1) {
		_engine->_menu->processPlasmaEffect(rect, 32);
	}

	Common::Rect rectBox(rect);
	rectBox.grow(-1);
	_engine->_menu->drawRectBorders(rect);
	_engine->_interface->shadeBox(rectBox, 3);

	_engine->_text->drawText(centerx - _engine->_text->sizeFont(text) / 2, top + 6, text);
	_engine->copyBlockPhys(rect);
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

bool MenuOptions::enterText(TextId textIdx, char *textTargetBuf, size_t bufSize) {
	_engine->_text->initDial(TextBankId::Options_and_menus);
	char buffer[256];
	_engine->_text->getMenuText(textIdx, buffer, sizeof(buffer));
	_engine->_text->setFontColor(COLOR_WHITE);
	const int halfScreenWidth = (_engine->width() / 2);
	_engine->_text->drawText(halfScreenWidth - (_engine->_text->sizeFont(buffer) / 2), 20, buffer);
	_engine->copyBlockPhys(0, 0, _engine->width() - 1, 99);

	Common::fill(&_onScreenKeyboardDirty[0], &_onScreenKeyboardDirty[ARRAYSIZE(_onScreenKeyboardDirty)], 1);
	ScopedFeatureState scopedVirtualKeyboard(OSystem::kFeatureVirtualKeyboard, true);
	for (;;) {
		FrameMarker frame(_engine);
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
							if (textTargetBuf[0] == '\0') {
								continue;
							}
							return true;
						}
						const size_t size = strlen(textTargetBuf);
						if (_onScreenKeyboardX == ONSCREENKEYBOARD_WIDTH - 2 && _onScreenKeyboardY == ONSCREENKEYBOARD_HEIGHT - 1) {
							if (size >= 1) {
								textTargetBuf[size - 1] = '\0';
							}
							continue;
						}
						const char chr = allowedCharIndex[_onScreenKeyboardX + _onScreenKeyboardY * ONSCREENKEYBOARD_WIDTH];
						textTargetBuf[size] = chr;
						textTargetBuf[size + 1] = '\0';
						if (size + 1 >= bufSize - 1) {
							return true;
						}
						continue;
					}
					if (textTargetBuf[0] == '\0') {
						continue;
					}

					return true;
				}
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UIAbort)) {
					return false;
				}
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UILeft)) {
					setOnScreenKeyboard(_onScreenKeyboardX - 1, _onScreenKeyboardY);
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIRight)) {
					setOnScreenKeyboard(_onScreenKeyboardX + 1, _onScreenKeyboardY);
				}
				if (_engine->_input->toggleActionIfActive(TwinEActionType::UIUp)) {
					setOnScreenKeyboard(_onScreenKeyboardX, _onScreenKeyboardY - 1);
				} else if (_engine->_input->toggleActionIfActive(TwinEActionType::UIDown)) {
					setOnScreenKeyboard(_onScreenKeyboardX, _onScreenKeyboardY + 1);
				}

				break;
			case Common::EVENT_KEYDOWN: {
				const size_t size = strlen(textTargetBuf);
				if (!Common::isPrint(event.kbd.ascii)) {
					if (event.kbd.keycode == Common::KEYCODE_BACKSPACE) {
						if (size >= 1) {
							textTargetBuf[size - 1] = '\0';
							_onScreenKeyboardLeaveViaOkButton = false;
						}
					}
					continue;
				}
				if (size >= bufSize - 1) {
					return true;
				}
				if (strchr(allowedCharIndex, event.kbd.ascii)) {
					textTargetBuf[size] = event.kbd.ascii;
					textTargetBuf[size + 1] = '\0';
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
		drawInputText(halfScreenWidth, 100, 1, textTargetBuf);
		drawSelectableCharacters();
	}
	return false;
}

bool MenuOptions::newGameMenu() {
	_engine->restoreFrontBuffer();
	_saveGameName[0] = '\0';
	if (!enterText(TextId::kEnterYourName, _saveGameName, sizeof(_saveGameName))) {
		return false;
	}
	_engine->_gameState->initEngineVars();
	newGame();
	return true;
}

int MenuOptions::chooseSave(TextId textIdx, bool showEmptySlots) {
	const SaveStateList &savegames = _engine->getSaveSlots();
	if (savegames.empty() && !showEmptySlots) {
		return -1;
	}

	_engine->_text->initDial(TextBankId::Options_and_menus);

	MenuSettings saveFiles;
	saveFiles.addButton(TextId::kReturnMenu);

	const int maxButtons = _engine->getMetaEngine()->getMaximumSaveSlot() + 1;
	uint savesIndex = 0;
	for (int i = 1; i < maxButtons; ++i) {
		if (savesIndex < savegames.size()) {
			const SaveStateDescriptor &savegame = savegames[savesIndex];
			if (savegame.getSaveSlot() == i - 1) {
				// manually creating a savegame should not overwrite the autosave slot
				if (textIdx != TextId::kCreateSaveGame || i > 1) {
					saveFiles.addButton(savegame.getDescription().encode().c_str(), i);
				}
				++savesIndex;
			} else if (showEmptySlots) {
				saveFiles.addButton("EMPTY", i);
			}
		} else if (showEmptySlots) {
			saveFiles.addButton("EMPTY", i);
		}
	}

	const int32 id = _engine->_menu->doGameMenu(&saveFiles);
	switch (id) {
	case kQuitEngine:
	case (int32)TextId::kReturnMenu:
		return -1;
	default:
		const int slot = saveFiles.getButtonState(id) - 1;
		debug("Selected savegame slot %d", slot);
		return slot;
	}

	return -1;
}

bool MenuOptions::continueGameMenu() {
	_engine->restoreFrontBuffer();
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
	_engine->restoreFrontBuffer();
	const int slot = chooseSave(TextId::kDeleteSaveGame);
	if (slot >= 0) {
		_engine->wipeSaveSlot(slot);
		return true;
	}
	return false;
}

bool MenuOptions::saveGameMenu() {
	if (!_engine->_scene->isGameRunning()) {
		return false;
	}
	_engine->restoreFrontBuffer();
	const int slot = chooseSave(TextId::kCreateSaveGame, true);
	if (slot >= 0) {
		char buf[30];
		strncpy(buf, _engine->_gameState->_sceneName, sizeof(buf));
		buf[sizeof(buf) - 1] = '\0';
		_engine->restoreFrontBuffer();
		enterText(TextId::kEnterYourNewName, buf, sizeof(buf));
		// may not be empty
		if (buf[0] == '\0') {
			Common::strlcpy(buf, _engine->_gameState->_sceneName, sizeof(buf));
		}
		Common::Error state = _engine->saveGameState(slot, buf, false);
		if (state.getCode() != Common::kNoError) {
			error("Failed to save slot %i", slot);
			return false;
		}

		return true;
	}
	return false;
}

} // namespace TwinE
