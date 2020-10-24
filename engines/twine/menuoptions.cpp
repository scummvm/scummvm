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
#include "common/system.h"
#include "twine/flamovies.h"
#include "twine/gamestate.h"
#include "twine/input.h"
#include "twine/interface.h"
#include "twine/menu.h"
#include "twine/music.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/text.h"
#include "twine/twine.h"

namespace TwinE {

/** Main menu continue game option key */
#define MAINMENU_CONTINUEGAME 21
/** Main menu enter players name */
#define MAINMENU_ENTERPLAYERNAME 42

static const char allowedCharIndex[] = " ABCDEFGHIJKLM.NOPQRSTUVWXYZ-abcdefghijklm?nopqrstuvwxyz!0123456789\040\b\r\0";

void MenuOptions::newGame() {
#if TWINE_PLAY_INTROS
	_engine->_music->stopMusic();

	int32 tmpFlagDisplayText = _engine->cfgfile.FlagDisplayText;
	_engine->cfgfile.FlagDisplayText = true;

	// intro screen 1 - twinsun
	_engine->_screens->loadImage(RESSHQR_INTROSCREEN1IMG);

	_engine->_text->newGameVar4 = 0;
	_engine->_text->newGameVar5 = 1;

	_engine->_text->initTextBank(2);
	_engine->_text->textClipFull();
	_engine->_text->setFontCrossColor(15);

	_engine->_text->drawTextFullscreen(150);
	_engine->readKeys();

	// intro screen 2
	_engine->_screens->loadImage(RESSHQR_INTROSCREEN2IMG);
	_engine->_text->drawTextFullscreen(151);
	_engine->readKeys();

	_engine->_screens->loadImage(RESSHQR_INTROSCREEN3IMG);
	_engine->_text->drawTextFullscreen(152);

	_engine->_text->newGameVar5 = 0;
	_engine->_text->textClipSmall();
	_engine->_text->newGameVar4 = 1;

	_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBACustom);
	_engine->_screens->clearScreen();
	_engine->flip();

	_engine->_music->playMidiMusic(1);
	_engine->_flaMovies->playFlaMovie(FLA_INTROD);

	_engine->_screens->clearScreen();
	_engine->flip();

	// set main palette back
	_engine->setPalette(_engine->_screens->paletteRGBA);

	_engine->cfgfile.FlagDisplayText = tmpFlagDisplayText;
#endif
}

void MenuOptions::showCredits() {
	canShowCredits = true;
	int32 tmpShadowMode = _engine->cfgfile.ShadowMode;
	_engine->cfgfile.ShadowMode = 0;
	_engine->_gameState->initEngineVars();
	_engine->_scene->currentSceneIdx = 119;
	_engine->_scene->needChangeScene = 119;

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

void MenuOptions::drawSelectableCharacter(int32 x, int32 y, int32 arg) {
	char buffer[256];
	int32 centerX, left, top, centerY, bottom, right, right2;

	buffer[0] = allowedCharIndex[y + x * 14];

	centerX = y * 45 + 25;
	left = centerX - 20;
	right = centerX + 20;
	top = x * 56 + 200 - 25;
	buffer[1] = 0;
	centerY = x * 56 + 200;
	bottom = x * 56 + 200 + 25;

	if (arg != 0) {
		_engine->_interface->drawSplittedBox(left, top, right, bottom, 91);
	} else {
		_engine->_interface->blitBox(left, top, right, bottom, (const int8 *)_engine->workVideoBuffer.getPixels(), left, top, (int8 *)_engine->frontVideoBuffer.getPixels());
		right2 = right;
		_engine->_interface->drawTransparentBox(left, top, right2, bottom, 4);
	}

	_engine->_menu->drawBox(left, top, right, bottom);
	right2 = right;

	_engine->_text->setFontColor(15);
	_engine->_text->drawText(centerX - _engine->_text->getTextSize(buffer) / 2, centerY - 18, buffer);

	_engine->copyBlockPhys(left, top, right2, bottom);
}

void MenuOptions::drawSelectableCharacters() {
	for (int8 x = 0; x < 5; x++) {
		for (int8 y = 0; y < 14; y++) {
			drawSelectableCharacter(x, y, 0);
		}
	}
}

// 0001F18C
void MenuOptions::drawPlayerName(int32 centerx, int32 top, int32 type) {
	const int left = _engine->_text->dialTextBoxLeft;
	const int right = _engine->_text->dialTextBoxRight;
	if (type == 1) {
		_engine->_menu->processPlasmaEffect(left, top, right, 1);
	}

	const int bottom = _engine->_text->dialTextBoxBottom;
	_engine->_menu->drawBox(left, top, right, bottom);
	_engine->_interface->drawTransparentBox(left + 1, top + 1, right - 1, bottom - 1, 3);

	_engine->_text->drawText(centerx - _engine->_text->getTextSize(playerName) / 2, top, playerName);

	_engine->flip();
	// TODO: _engine->copyBlockPhys(left, top, right, bottom);
}

int32 MenuOptions::enterPlayerName(int32 textIdx) {
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip();
	playerName[0] = '\0'; // TODO: read from settings?
	while (1) {
		_engine->_text->initTextBank(0);
		char buffer[256];
		_engine->_text->getMenuText(textIdx, buffer, sizeof(buffer));
		_engine->_text->setFontColor(15);
		const int halfScreenWidth = (SCREEN_WIDTH / 2);
		_engine->_text->drawText(halfScreenWidth - (_engine->_text->getTextSize(buffer) / 2), 20, buffer);
		_engine->copyBlockPhys(0, 0, SCREEN_WIDTH - 1, 99);
		drawPlayerName(halfScreenWidth, 100, 1);
		drawSelectableCharacters();
		_engine->flip();

		// we don't want custom events here - as we are entering the player name
		ScopedKeyMapperDisable scopedKeyMapperDisable;
		for (;;) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_KEYDOWN) {
					if (event.kbd.keycode == Common::KEYCODE_KP_ENTER || event.kbd.keycode == Common::KEYCODE_RETURN) {
						return 1;
					}
					const size_t size = strlen(playerName);
					if (size >= sizeof(playerName) - 1) {
						return 1;
					}
					playerName[size] = event.kbd.ascii;
					playerName[size + 1] = '\0';
					debug("name: %s", playerName);

					drawPlayerName(halfScreenWidth, 100, 1);
					_engine->flip();
				}
			}
			if (_engine->shouldQuit()) {
				break;
			}
			_engine->_system->delayMillis(1);
		};
	}
	return 1;
}

void MenuOptions::newGameMenu() {
	// TODO: process players name
	if (enterPlayerName(MAINMENU_ENTERPLAYERNAME)) {
		_engine->_gameState->initEngineVars();
		newGame();

		if (_engine->gameEngineLoop()) {
			showCredits();
		}
	}
}

void MenuOptions::continueGameMenu() {
	//TODO: get list of saved games
	//if(chooseSave(MAINMENU_CONTINUEGAME))
	{
		_engine->_gameState->initEngineVars();
		_engine->_gameState->loadGame();
		if (_engine->_scene->newHeroX == -1) {
			_engine->_scene->heroPositionType = ScenePositionType::kNoPosition;
		}
		if (_engine->_gameState->gameChapter == 0 && _engine->_scene->currentSceneIdx == 0) {
			newGame();
		} else {
			_engine->_text->newGameVar5 = 0;
			_engine->_text->textClipSmall();
			_engine->_text->newGameVar4 = 1;
		}

		if (_engine->gameEngineLoop()) {
			showCredits();
		}

		_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
		do {
			_engine->readKeys();
			_engine->_system->delayMillis(1);
		} while (!_engine->shouldQuit() && !_engine->_input->toggleAbortAction());
	}
}

} // namespace TwinE
