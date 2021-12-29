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

#include "buried/buried.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/main_menu.h"
#include "buried/resources.h"
#include "buried/sound.h"
#include "buried/video_window.h"

#include "common/error.h"
#include "graphics/surface.h"

namespace Buried {

enum {
	BUTTON_INTERFACE_OVERVIEW = 1,
	BUTTON_NEW_GAME = 2,
	BUTTON_RESTORE_GAME = 3,
	BUTTON_CREDITS = 4,
	BUTTON_QUIT = 5
};

MainMenuWindow::MainMenuWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_curButton = 0;
	_showIntro = true;
	_walkthrough = false;
	_buttonDrawnDown = false;
	_disableDrawing = false;

	Common::Rect parentRect = _parent->getClientRect();
	_rect.left = (parentRect.right - 640) / 2;
	_rect.top = (parentRect.bottom - 480) / 2;
	_rect.right = parentRect.left + 640;
	_rect.bottom = parentRect.top + 480;

	_buttons[0] = Common::Rect(335, 95, 610, 155);
	_buttons[1] = Common::Rect(335, 168, 610, 228);
	_buttons[2] = Common::Rect(335, 309, 610, 369);
	_buttons[3] = Common::Rect(344, 375, 471, 407);
	_buttons[4] = Common::Rect(471, 375, 598, 407);

	_playMode[0] = Common::Rect(345, 235, 465, 263);
	_playMode[1] = Common::Rect(478, 235, 598, 263);

	_intro[0] = Common::Rect(345, 268, 465, 296);
	_intro[1] = Common::Rect(478, 268, 598, 296);

	_background = _vm->_gfx->getBitmap(IDB_MAINMENU_NORMAL);
	_depressedPlayMode = _vm->_gfx->getBitmap(IDB_MAINMENU_DEPRESSED_PLAY_MODE);
	_depressedPlayIntro = _vm->_gfx->getBitmap(IDB_MAINMENU_DEPRESSED_PLAY_INRO);
	_depressedOverview = _vm->_gfx->getBitmap(IDB_MAINMENU_DEPRESSED_OVERVIEW);
	_depressedNewGame = _vm->_gfx->getBitmap(IDB_MAINMENU_DEPRESSED_NEW_GAME);
	_depressedRestoreGame = _vm->_gfx->getBitmap(IDB_MAINMENU_DEPRESSED_RESTORE_GAME);
	_depressedCredits = _vm->_gfx->getBitmap(IDB_MAINMENU_DEPRESSED_CREDITS);
	_depressedQuit = _vm->_gfx->getBitmap(IDB_MAINMENU_DEPRESSED_QUIT);
}

MainMenuWindow::~MainMenuWindow() {
	_background->free();
	delete _background;

	_depressedPlayMode->free();
	delete _depressedPlayMode;

	_depressedPlayIntro->free();
	delete _depressedPlayIntro;

	_depressedOverview->free();
	delete _depressedOverview;

	_depressedNewGame->free();
	delete _depressedNewGame;

	_depressedRestoreGame->free();
	delete _depressedRestoreGame;

	_depressedCredits->free();
	delete _depressedCredits;

	_depressedQuit->free();
	delete _depressedQuit;
}

bool MainMenuWindow::showMainMenu() {
	_vm->_sound->restart();
	_vm->_sound->setAmbientSound("BITDATA/COMMON/MAINMENU.BTA");

	_vm->removeKeyboardMessages(this);
	_vm->removeKeyboardMessages(_parent);

	showWindow(kWindowShow);
	invalidateWindow();
	return true;
}

void MainMenuWindow::onPaint() {
	if (!_disableDrawing) {
		_vm->_gfx->blit(_background, 0, 0);

		if (_walkthrough)
			_vm->_gfx->blit(_depressedPlayMode, 343, 232);

		if (!_showIntro)
			_vm->_gfx->blit(_depressedPlayIntro, 343, 266);

		switch (_curButton) {
		case 1:
			_vm->_gfx->blit(_depressedOverview, 334, 94);
			break;
		case 2:
			_vm->_gfx->blit(_depressedNewGame, 334, 167);
			break;
		case 3:
			_vm->_gfx->blit(_depressedRestoreGame, 334, 308);
			break;
		case 4:
			_vm->_gfx->blit(_depressedCredits, 342, 374);
			break;
		case 5:
			_vm->_gfx->blit(_depressedQuit, 471, 374);
			break;
		}
	}
}

bool MainMenuWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void MainMenuWindow::onLButtonDown(const Common::Point &point, uint flags) {
	if (_playMode[0].contains(point) && _walkthrough) {
		_walkthrough = false;
		invalidateRect(_playMode[0], false);
		invalidateRect(_playMode[1], false);
		return;
	}

	if (_playMode[1].contains(point) && !_walkthrough) {
		_walkthrough = true;
		invalidateRect(_playMode[0], false);
		invalidateRect(_playMode[1], false);
		return;
	}

	if (_intro[0].contains(point) && !_showIntro) {
		_showIntro = true;
		invalidateRect(_intro[0], false);
		invalidateRect(_intro[1], false);
		return;
	}

	if (_intro[1].contains(point) && _showIntro) {
		_showIntro = false;
		invalidateRect(_intro[0], false);
		invalidateRect(_intro[1], false);
		return;
	}

	for (int i = 0; i < 5; i++) {
		if (_buttons[i].contains(point)) {
			_curButton = i + 1;
			invalidateRect(_buttons[i], false);
			return;
		}
	}
}

void MainMenuWindow::onLButtonUp(const Common::Point &point, uint flags) {
	if (_curButton > 0 && _buttons[_curButton - 1].contains(point)) {
		switch (_curButton) {
		case BUTTON_INTERFACE_OVERVIEW:
			_vm->_sound->setAmbientSound();
			((FrameWindow *)_parent)->showOverview();
			return;
		case BUTTON_NEW_GAME:
			if (_vm->isControlDown()) {
				// Easter egg
				_disableDrawing = true;
				invalidateWindow(false);

				// Play the intro movie
				VideoWindow *video = new VideoWindow(_vm, this);

				if (video->openVideo("BITDATA/INTRO/INTRO_O.BTV")) {
					video->setWindowPos(nullptr, 104, 145, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder);
					video->enableWindow(false);
					video->showWindow(kWindowShow);
					_vm->_sound->stop();
					video->playVideo();

					while (!_vm->shouldQuit() && video->getMode() != VideoWindow::kModeStopped)
						_vm->yield(video, -1);

					_vm->_sound->restart();
				}

				delete video;

				if (_vm->shouldQuit())
					return;

				_disableDrawing = false;
				invalidateWindow(false);
			} else {
				((FrameWindow *)_parent)->startNewGame(_walkthrough, _showIntro);
			}
			return;
		case BUTTON_RESTORE_GAME:
			_vm->loadGameDialog();
			return;
		case BUTTON_CREDITS:
			((FrameWindow *)_parent)->showCredits();
			return;
		case BUTTON_QUIT:
			_vm->quitGame();
			return;
		}

		invalidateRect(_buttons[_curButton - 1], false);
		_curButton = 0;
	}
}

void MainMenuWindow::onMouseMove(const Common::Point &point, uint flags) {
	// If we mouse-downed on a button, check to see if we moved outside of its bounds
	if (_curButton > 0 && !_buttons[_curButton - 1].contains(point)) {
		invalidateRect(_buttons[_curButton - 1], false);
		_curButton = 0;
	}
}

} // End of namespace Buried
