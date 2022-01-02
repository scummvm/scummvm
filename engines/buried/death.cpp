/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "buried/agent_evaluation.h"
#include "buried/avi_frames.h"
#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/death.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/navdata.h"
#include "buried/resources.h"
#include "buried/sound.h"

#include "common/error.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

enum {
	BUTTON_QUIT = 1,
	BUTTON_RESTORE_GAME = 2,
	BUTTON_MAIN_MENU = 3
};

DeathWindow::DeathWindow(BuriedEngine *vm, Window *parent, int deathSceneIndex, GlobalFlags globalFlags, Common::Array<int> itemArray)
		: Window(vm, parent), _deathSceneIndex(deathSceneIndex), _globalFlags(globalFlags), _itemArray(itemArray) {
	_curButton = 0;
	_deathFrameIndex = -1;
	_lightOn = false;
	_walkthroughMode = false;

	_rect = Common::Rect(0, 0, 640, 480);
	_quit = Common::Rect(27, 422, 100, 460);
	_restoreGame = Common::Rect(112, 422, 185, 460);
	_mainMenu = Common::Rect(198, 422, 271, 460);
	_agentEvaluation = new AgentEvaluation(vm, globalFlags, deathSceneIndex);
	_timer = setTimer(400);

	if (deathSceneIndex < 10) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_CASTLE_FILENAME));
	} else if (deathSceneIndex < 20) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_MAYAN_FILENAME));
	} else if (deathSceneIndex < 30) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_AGENTLAIR_FILENAME));
	} else if (deathSceneIndex < 40) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_DAVINCI_FILENAME));
	} else if (deathSceneIndex < 50) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_AILAB_FILENAME));
	} else if (deathSceneIndex < 60) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_ALIEN_FILENAME));
	} else if (deathSceneIndex < 70) {
		_deathSceneFrames = new AVIFrames(_vm->getFilePath(IDS_DEATH_FINALE_FILENAME));
	} else {
		error("Bad death scene index %d", deathSceneIndex);
	}

	// Set the frame index
	switch (deathSceneIndex) {
	case 15:
		_deathFrameIndex = 4;
		break;
	case 52:
	case 53:
		_deathFrameIndex = 1;
		break;
	case 54:
	case 55:
		_deathFrameIndex = 0;
		break;
	default:
		_deathFrameIndex = deathSceneIndex % 10;
		break;
	}

	_fontHeightA = (_vm->getLanguage() == Common::JA_JPN) ? 12 : 14;
	_textFontA = _vm->_gfx->createFont(_fontHeightA);

	_fontHeightB = 20;
	_textFontB = _vm->_gfx->createFont(_fontHeightB, true);

	_walkthroughMode = _globalFlags.generalWalkthroughMode != 0;

	_vm->_sound->setAmbientSound();
}

DeathWindow::~DeathWindow() {
	killTimer(_timer);

	delete _deathSceneFrames;

	delete _textFontA;
	delete _textFontB;
	delete _agentEvaluation;
}

void DeathWindow::onPaint() {
	Graphics::Surface *topBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_TOP);
	_vm->_gfx->blit(topBitmap, 301, 0);
	topBitmap->free();
	delete topBitmap;

	Graphics::Surface *leftBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_LEFT);
	_vm->_gfx->blit(leftBitmap, 0, 0);
	leftBitmap->free();
	delete leftBitmap;

	if (_walkthroughMode) {
		Graphics::Surface *lowerLeftBitmap = _vm->_gfx->getBitmap(IDB_DEATH_WT_LOWER_LEFT);
		_vm->_gfx->blit(lowerLeftBitmap, 0, 416);
		lowerLeftBitmap->free();
		delete lowerLeftBitmap;
	}

	Graphics::Surface *rightBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_RIGHT);
	_vm->_gfx->blit(rightBitmap, 624, 0);
	rightBitmap->free();
	delete rightBitmap;

	Graphics::Surface *bottomBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_BOTTOM);
	_vm->_gfx->blit(bottomBitmap, 301, 468);
	bottomBitmap->free();
	delete bottomBitmap;

	const Graphics::Surface *deathSceneHeader = _deathSceneFrames->getFrame(_deathFrameIndex);
	_vm->_gfx->blit(deathSceneHeader, 301, 10);

	uint32 textColor = _vm->_gfx->getColor(153, 102, 204);
	Common::String firstBlock = _vm->getString(IDS_DEATH_SCENE_MESSAGE_TEXT_BASE + _deathSceneIndex * 5);
	Common::Rect firstBlockRect(10, 54, 283, 86);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, firstBlock, firstBlockRect.left, firstBlockRect.top, firstBlockRect.width(), firstBlockRect.height(), textColor, _fontHeightA);

	Common::String secondBlock = _vm->getString(IDS_DEATH_SCENE_MESSAGE_TEXT_BASE + _deathSceneIndex * 5 + 1);
	Common::Rect secondBlockRect(10, 120, 283, 215);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, secondBlock, secondBlockRect.left, secondBlockRect.top, secondBlockRect.width(), secondBlockRect.height(), textColor, _fontHeightA);

	Common::Rect scoringDescRect(10, 248, 283, 378);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, _agentEvaluation->_scoringTextDescriptions, scoringDescRect.left, scoringDescRect.top, scoringDescRect.width(), scoringDescRect.height(), textColor, _fontHeightA);

	textColor = _vm->_gfx->getColor(212, 109, 0);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontA, _agentEvaluation->_scoringTextScores, scoringDescRect.left, scoringDescRect.top, scoringDescRect.width(), scoringDescRect.height(), textColor, _fontHeightA, kTextAlignRight);

	// CHECKME: This does center vertical alignment, so check the y coordinates
	Common::Rect finalTextScoreRect(122, 386, 283, 401);
	_vm->_gfx->renderText(_vm->_gfx->getScreen(), _textFontB, _agentEvaluation->_scoringTextFinalScore, finalTextScoreRect.left, finalTextScoreRect.top, finalTextScoreRect.width(), finalTextScoreRect.height(), textColor, _fontHeightB, kTextAlignRight);
}

bool DeathWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void DeathWindow::onTimer(uint timer) {
	_vm->_sound->timerCallback(); // clone2727 says: Don't think this is necessary

	// Flip the state of the light
	_lightOn = !_lightOn;

	Common::Rect destRect(137, 51);
	destRect.moveTo(164, 0);

	if (_lightOn) {
		// Light is now on
		Graphics::Surface *lightBitmap = _vm->_gfx->getBitmap(IDB_DEATH_ELIGHT_ON);
		_vm->_gfx->blit(lightBitmap, 164, 0);
		lightBitmap->free();
		delete lightBitmap;
	} else {
		// Light is now off
		Graphics::Surface *leftBitmap = _vm->_gfx->getBitmap(IDB_DEATH_UI_LEFT);
		_vm->_gfx->blit(leftBitmap, Common::Rect(164, 0, 164 + 137, 51), destRect);
		leftBitmap->free();
		delete leftBitmap;
	}

	// Force just that section of the screen to update
	invalidateRect(destRect, false);
	_vm->_gfx->updateScreen(false);
}

void DeathWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_quit.contains(point)) {
		Graphics::Surface *buttons = _vm->_gfx->getBitmap(_walkthroughMode ? IDB_DEATH_WT_BUTTONS_DEPRESSED : IDB_DEATH_BUTTONS_DEPRESSED);
		Common::Rect destRect(29, 424, 29 + 76, 424 + 36);
		_vm->_gfx->blit(buttons, Common::Rect(76, 36), destRect);
		buttons->free();
		delete buttons;
		invalidateRect(destRect, false);
		_vm->_gfx->updateScreen(false);
		_curButton = BUTTON_QUIT;
	} else if (_restoreGame.contains(point)) {
		Graphics::Surface *buttons = _vm->_gfx->getBitmap(_walkthroughMode ? IDB_DEATH_WT_BUTTONS_DEPRESSED : IDB_DEATH_BUTTONS_DEPRESSED);
		Common::Rect destRect(109, 424, 109 + 80, 424 + 36);
		_vm->_gfx->blit(buttons, Common::Rect(80, 0, 80 + 80, 36), destRect);
		buttons->free();
		delete buttons;
		invalidateRect(destRect, false);
		_vm->_gfx->updateScreen(false);
		_curButton = BUTTON_RESTORE_GAME;
	} else if (_mainMenu.contains(point)) {
		Graphics::Surface *buttons = _vm->_gfx->getBitmap(_walkthroughMode ? IDB_DEATH_WT_BUTTONS_DEPRESSED : IDB_DEATH_BUTTONS_DEPRESSED);
		Common::Rect destRect(195, 424, 195 + 76, 424 + 36);
		_vm->_gfx->blit(buttons, Common::Rect(166, 0, 166 + 76, 36), destRect);
		buttons->free();
		delete buttons;
		invalidateRect(destRect, false);
		_vm->_gfx->updateScreen(false);
		_curButton = BUTTON_MAIN_MENU;
	}
}

void DeathWindow::onLButtonUp(const Common::Point &point, uint flags) {
	switch (_curButton) {
	case BUTTON_QUIT:
		if (_quit.contains(point)) {
			_vm->quitGame();
			return;
		}
		break;
	case BUTTON_RESTORE_GAME:
		if (_restoreGame.contains(point)) {
			if (_walkthroughMode) {
				Location startingLocation;

				switch (_deathSceneIndex) {
				case 0:
					startingLocation = Location(1, 2, 1, 2, 1, 0);
					break;
				case 1:
					startingLocation = Location(1, 3, 2, 0, 0, 0);
					break;
				case 3:
				case 4:
					startingLocation = Location(1, 4, 7, 0, 2, 0);
					_itemArray.push_back(kItemGrapplingHook);
					break;
				case 5:
					startingLocation = Location(1, 8, 6, 3, 1, 0);
					break;
				case 10:
					startingLocation = Location(2, 1, 4, 2, 1, 0);
					break;
				case 11:
					startingLocation = Location(2, 1, 7, 3, 1, 0);
					_itemArray.push_back(kItemCeramicBowl);
					break;
				case 12:
					startingLocation = Location(2, 6, 3, 2, 0, 0);
					_itemArray.push_back(kItemPreservedHeart);
					break;
				case 14:
					startingLocation = Location(2, 4, 2, 0, 1, 0);
					break;
				case 15:
					startingLocation = Location(2, 4, 5, 2, 1, 0);
					break;
				case 20:
					startingLocation = Location(3, 2, 6, 0, 0, 0);
					_globalFlags.alRestoreSkipAgent3Initial = 1;
					break;
				case 30:
					startingLocation = Location(5, 1, 2, 0, 1, 0);
					break;
				case 31:
					startingLocation = Location(5, 5, 10, 3, 0, 0);
					break;
				case 50:
					startingLocation = Location(7, 1, 0, 2, 1, 0);
					break;
				case 51:
					startingLocation = Location(7, 1, 6, 0, 1, 1);
					break;
				case 52:
					startingLocation = Location(7, 1, 6, 0, 1, 0);
					break;
				case 53:
					startingLocation = Location(7, 1, 4, 2, 1, 0);
					break;
				case 54:
					startingLocation = Location(7, 1, 3, 3, 1, 1);
					break;
				case 55:
					startingLocation = Location(7, 1, 6, 0, 1, 2);
					break;
				case 61:
					startingLocation = Location(7, 1, 2, 0, 1, 0);
					break;
				}

				if (startingLocation.timeZone >= 0) {
					((FrameWindow *)_vm->_mainWindow)->loadFromState(startingLocation, _globalFlags, _itemArray);
					return;
				}
			} else {
				if (_vm->loadGameDialog())
					return;
			}
		}
		break;
	case BUTTON_MAIN_MENU:
		if (_mainMenu.contains(point)) {
			((FrameWindow *)_parent)->showMainMenu();
			return;
		}
		break;
	default:
		return;
	}

	_curButton = 0;
	invalidateWindow(false);
}

void DeathWindow::onMouseMove(const Common::Point &point, uint flags) {
	switch (_curButton) {
	case BUTTON_QUIT:
		if (!_quit.contains(point)) {
			_curButton = 0;
			invalidateRect(_quit, false);
		}
		break;
	case BUTTON_RESTORE_GAME:
		if (!_restoreGame.contains(point)) {
			_curButton = 0;
			invalidateRect(_restoreGame, false);
		}
		break;
	case BUTTON_MAIN_MENU:
		if (!_mainMenu.contains(point)) {
			_curButton = 0;
			invalidateRect(_mainMenu, false);
		}
		break;
	}
}

} // End of namespace Buried
