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
#include "twine/flamovies.h"
#include "twine/gamestate.h"
#include "twine/interface.h"
#include "twine/keyboard.h"
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
	int32 tmpFlagDisplayText;

	_engine->_music->stopMusic();

	tmpFlagDisplayText = _engine->cfgfile.FlagDisplayText;
	_engine->cfgfile.FlagDisplayText = 1;

	// intro screen 1 - twinsun
	_engine->_screens->loadImage(RESSHQR_INTROSCREEN1IMG);

	_engine->_text->newGameVar4 = 0;
	_engine->_text->newGameVar5 = 1;

	_engine->_text->initTextBank(2);
	_engine->_text->textClipFull();
	_engine->_text->setFontCrossColor(15);

	_engine->_text->drawTextFullscreen(150);
	_engine->readKeys();

	if (_engine->_keyboard.skipIntro != 1) {
		// intro screen 1 - twinsun
		_engine->_screens->loadImage(RESSHQR_INTROSCREEN2IMG);
		_engine->_text->drawTextFullscreen(151);
		_engine->readKeys();

		if (_engine->_keyboard.skipIntro != 1) {
			_engine->_screens->loadImage(RESSHQR_INTROSCREEN3IMG);
			_engine->_text->drawTextFullscreen(152);
		}
	}

	_engine->_text->newGameVar5 = 0;
	_engine->_text->textClipSmall();
	_engine->_text->newGameVar4 = 1;

	_engine->_screens->fadeToBlack(_engine->_screens->paletteRGBCustom);
	_engine->_screens->clearScreen();
	_engine->flip();

	_engine->_music->playMidiMusic(1, 0);
	_engine->_flaMovies->playFlaMovie(FLA_INTROD);

	_engine->_screens->clearScreen();
	_engine->flip();

	// set main palette back
	_engine->setPalette(_engine->_screens->paletteRGB);

	_engine->cfgfile.FlagDisplayText = tmpFlagDisplayText;
}

void MenuOptions::showCredits() {
	int32 tmpShadowMode, tmpLanguageCDIdx;

	canShowCredits = 1;
	tmpShadowMode = _engine->cfgfile.ShadowMode;
	tmpLanguageCDIdx = _engine->cfgfile.LanguageCDId;
	_engine->cfgfile.ShadowMode = 0;
	_engine->cfgfile.LanguageCDId = 0;
	_engine->_gameState->initEngineVars();
	_engine->_scene->currentSceneIdx = 119;
	_engine->_scene->needChangeScene = 119;

	_engine->gameEngineLoop();

	canShowCredits = 0;
	_engine->cfgfile.ShadowMode = tmpShadowMode;
	_engine->cfgfile.LanguageCDId = tmpLanguageCDIdx;

	_engine->_screens->clearScreen();
	_engine->flip();

	_engine->_flaMovies->playFlaMovie(FLA_THEEND);

	_engine->_screens->clearScreen();
	_engine->flip();
	_engine->setPalette(_engine->_screens->paletteRGB);
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
		_engine->_interface->blitBox(left, top, right, bottom, (int8 *)_engine->workVideoBuffer, left, top, (int8 *)_engine->frontVideoBuffer);
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
	int8 x, y;

	for (x = 0; x < 5; x++) {
		for (y = 0; y < 14; y++) {
			drawSelectableCharacter(x, y, 0);
		}
	}
}

// 0001F18C
void MenuOptions::drawPlayerName(int32 centerx, int32 top, int8 * /*playerName*/, int32 type) {
	/*
	int v4; // ebp@0
  int v6; // [sp+0h] [bp-14h]@0
  int v7; // [sp+0h] [bp-14h]@4
  int v8; // [sp+4h] [bp-10h]@0
  int v9; // [sp+4h] [bp-10h]@4

  LOWORD(v8) = a1 - buttonDrawVar1 / 2;
  if ( !a4 )
  {
    v6 = (signed __int16)(a2 + 25);
    blitRectangle(v4);
    drawBoxInsideTrans(v4);
  }
  if ( a4 == 1 )
  {
    makeFireEffect(v4);
    if ( !(rand(v6, v8) % 5) )
      *(_BYTE *)(10 * rand(v7, v9) % 320 + bufSpeak + 6400) = -1;
  }
  if ( a4 == 2 )
    Box(v4);
  DrawCadre();
  CoulFont(0xFu);
  SizeFont(a3);
  Font(v4);
  return CopyBlockPhys(v4);
	*/

	// TODO: implement the other types (don't seam to be used)
	/*if (type == 1) {
		processPlasmaEffect(top, 1);
	}

	drawBox(x, top, dialTextBoxRight, dialTextBoxBottom);
	drawTransparentBox(dialTextBoxLeft + 1, dialTextBoxTop + 1, dialTextBoxRight - 1, dialTextBoxBottom - 1, 3);

	setFontColor(15);
	drawText(centerX - getTextSize(playerName) / 2, top, playerName);

	copyBlockPhys(x, y, x + 320, y + 25);*/
}

int32 MenuOptions::enterPlayerName(int32 textIdx) {
	char buffer[256];

	while (1) {
		_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
		_engine->flip(); //frontVideoBuffer
		_engine->_text->initTextBank(0);
		_engine->_text->getMenuText(textIdx, buffer, sizeof(buffer));
		_engine->_text->setFontColor(15);
		_engine->_text->drawText(320 - (_engine->_text->getTextSize(buffer) / 2), 20, buffer);
		_engine->copyBlockPhys(0, 0, 639, 99);
		playerName[0] = enterPlayerNameVar1;
		drawPlayerName(320, 100, playerName, 1);
		drawSelectableCharacters();

		do {
			_engine->readKeys();
			do {
				_engine->readKeys();
				if (_engine->shouldQuit()) {
					break;
				}
			} while (_engine->_keyboard.skipIntro);
			if (_engine->shouldQuit()) {
				break;
			}
		} while (_engine->_keyboard.skippedKey);

		enterPlayerNameVar2 = 1;

		do {
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				break;
			}
		} while (_engine->_keyboard.pressedKey);

		while (!_engine->_keyboard.skipIntro) {
			_engine->readKeys();
			if (_engine->shouldQuit()) {
				break;
			}
			// TODO
			drawPlayerName(320, 100, playerName, 1);
		}

		// FIXME: remove this lines after implementing everything
		if (_engine->_keyboard.skipIntro)
			break;
	}

	enterPlayerNameVar2 = 0;
	_engine->_screens->copyScreen(_engine->workVideoBuffer, _engine->frontVideoBuffer);
	_engine->flip(); // frontVideoBuffer

	return 1;
}

void MenuOptions::newGameMenu() {
	//TODO: process players name
	if (enterPlayerName(MAINMENU_ENTERPLAYERNAME)) {
		_engine->_gameState->initEngineVars();
		newGame();

		if (_engine->gameEngineLoop()) {
			showCredits();
		}

		_engine->_screens->copyScreen(_engine->frontVideoBuffer, _engine->workVideoBuffer);
		// TODO: recheck this
		do {
			_engine->readKeys();
			do {
				_engine->readKeys();
				if (_engine->shouldQuit()) {
					break;
				}
			} while (_engine->_keyboard.skippedKey != 0);
			if (_engine->shouldQuit()) {
				break;
			}
		} while (_engine->_keyboard.skipIntro != 0);
	}
}

void MenuOptions::continueGameMenu() {
	//TODO: get list of saved games
	//if(chooseSave(MAINMENU_CONTINUEGAME))
	{
		_engine->_gameState->initEngineVars();
		_engine->_gameState->loadGame();
		if (_engine->_scene->newHeroX == -1) {
			_engine->_scene->heroPositionType = kNoPosition;
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
		// TODO: recheck this
		do {
			_engine->readKeys();
			do {
				_engine->readKeys();
				if (_engine->shouldQuit()) {
					break;
				}
			} while (_engine->_keyboard.skippedKey != 0);
			if (_engine->shouldQuit()) {
				break;
			}
		} while (_engine->_keyboard.skipIntro != 0);
	}
}

} // namespace TwinE
