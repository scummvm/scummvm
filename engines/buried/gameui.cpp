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

#include "buried/biochip_right.h"
#include "buried/buried.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/invdata.h"
#include "buried/inventory_window.h"
#include "buried/livetext.h"
#include "buried/message.h"
#include "buried/navarrow.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/video_window.h"

#include "common/error.h"
#include "common/keyboard.h"
#include "common/system.h"
#include "graphics/surface.h"

namespace Buried {

GameUIWindow::GameUIWindow(BuriedEngine *vm, Window *parent) : Window(vm, parent) {
	_currentDateDisplay = -1;
	_warningLightDisplayed = false;
	_rect = Common::Rect(0, 0, 640, 480);
	_doNotDraw = true;

	_navArrowWindow = new NavArrowWindow(_vm, this);
	_liveTextWindow = new LiveTextWindow(_vm, this);
	_sceneViewWindow = new SceneViewWindow(_vm, this);
	_inventoryWindow = new InventoryWindow(_vm, this);
	_bioChipRightWindow = new BioChipRightWindow(_vm, this);
}

GameUIWindow::~GameUIWindow() {
	delete _navArrowWindow;
	delete _liveTextWindow;
	delete _inventoryWindow;
	delete _bioChipRightWindow;
	delete _sceneViewWindow;
}

bool GameUIWindow::startNewGame(bool walkthrough) {
	_doNotDraw = false;
	invalidateWindow(false);

	_navArrowWindow->showWindow(kWindowShow);
	_liveTextWindow->showWindow(kWindowShow);
	_inventoryWindow->showWindow(kWindowShow);
	_bioChipRightWindow->showWindow(kWindowShow);
	_sceneViewWindow->showWindow(kWindowShow);
	_sceneViewWindow->startNewGame(walkthrough);

	return true;
}

bool GameUIWindow::startNewGameIntro(bool walkthrough) {
	_doNotDraw = true;
	_vm->_sound->setAmbientSound();

	VideoWindow *video = new VideoWindow(_vm, this);

	if (!video->openVideo(_vm->getFilePath(IDS_INTRO_FILENAME)))
		error("Failed to load intro video");

	video->setWindowPos(nullptr, 104, 145, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder);
	video->enableWindow(false);
	video->showWindow(kWindowShow);
	_vm->_sound->stop();
	video->playVideo();

	while (!_vm->shouldQuit() && video->getMode() != VideoWindow::kModeStopped)
		_vm->yield(video, -1);

	delete video;

	if (_vm->shouldQuit())
		return false;

	_vm->_sound->restart();
	_doNotDraw = false;
	invalidateWindow(false);

	_navArrowWindow->showWindow(kWindowShow);
	_liveTextWindow->showWindow(kWindowShow);
	_inventoryWindow->showWindow(kWindowShow);
	_bioChipRightWindow->showWindow(kWindowShow);
	_sceneViewWindow->showWindow(kWindowShow);
	_sceneViewWindow->startNewGameIntro(walkthrough);

	return true;
}

bool GameUIWindow::startNewGame(const Location &startingLocation) {
	_doNotDraw = false;

	showWindow(kWindowShow);
	_navArrowWindow->showWindow(kWindowShow);
	_liveTextWindow->showWindow(kWindowShow);
	_sceneViewWindow->showWindow(kWindowShow);
	_inventoryWindow->showWindow(kWindowShow);
	_bioChipRightWindow->showWindow(kWindowShow);
	_sceneViewWindow->startNewGame(startingLocation);
	invalidateWindow(false);

	return true;
}

bool GameUIWindow::changeCurrentDate(int timeZoneID) {
	switch (timeZoneID) {
	case 1:
		_currentDateDisplay = 1;
		break;
	case 2:
		_currentDateDisplay = 3;
		break;
	case 3:
	case 4:
	case 7:
		_currentDateDisplay = 4;
		break;
	case 5:
		_currentDateDisplay = 5;
		break;
	case 6:
		_currentDateDisplay = 0;
		break;
	case 10:
		_currentDateDisplay = 2;
		break;
	default:
		_currentDateDisplay = -1;
		break;
	}

	invalidateRect(Common::Rect(50, 330, 200, 360), false);
	return true;
}

bool GameUIWindow::flashWarningLight() {
	Common::Rect redrawRect(185, 318, 275, 359);

	if (_warningLightDisplayed) {
		_warningLightDisplayed = false;
		invalidateRect(redrawRect, false);
		_vm->_gfx->updateScreen();
	}

	uint32 startTime = g_system->getMillis();
	while (!_vm->shouldQuit() && (startTime + 200) > g_system->getMillis()) {
		_vm->yield(nullptr, -1);
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit())
		return false;

	_warningLightDisplayed = true;
	invalidateRect(redrawRect, false);
	_vm->_gfx->updateScreen();

	_vm->_sound->playInterfaceSound("BITDATA/COMMON/MSGBEEP.BTA");

	startTime = g_system->getMillis();
	while (!_vm->shouldQuit() && (startTime + 250) > g_system->getMillis()) {
		_vm->yield(nullptr, -1);
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit())
		return false;

	_warningLightDisplayed = false;
	invalidateRect(redrawRect, false);
	_vm->_gfx->updateScreen();

	startTime = g_system->getMillis();
	while (!_vm->shouldQuit() && (startTime + 250) > g_system->getMillis()) {
		_vm->yield(nullptr, -1);
		_vm->_sound->timerCallback();
	}

	if (_vm->shouldQuit())
		return false;

	_warningLightDisplayed = true;
	invalidateRect(redrawRect, false);
	_vm->_gfx->updateScreen();

	_vm->_sound->playInterfaceSound("BITDATA/COMMON/MSGBEEP.BTA");
	return true;
}

bool GameUIWindow::setWarningState(bool newState) {
	if (newState != _warningLightDisplayed) {
		Common::Rect redrawRect(185, 318, 275, 359);
		_warningLightDisplayed = newState;
		_vm->_sound->playInterfaceSound("BITDATA/COMMON/MSGBEEP.BTA");
		invalidateRect(redrawRect, false);
	}

	return true;
}

void GameUIWindow::onPaint() {
	Common::Rect topUI(0, 0, 639, 127);
	Common::Rect leftUI(0, 128, 64, 316);
	Common::Rect bottomUI(0, 317, 639, 479);
	Common::Rect rightUI(495, 128, 639, 316);

	if (topUI.intersects(_vm->_gfx->getDirtyRect()) && !_doNotDraw) {
		Graphics::Surface *topBitmap = _vm->_gfx->getBitmap(IDB_UI_TOP);
		_vm->_gfx->blit(topBitmap, 0, 0);
		topBitmap->free();
		delete topBitmap;
	}

	if (leftUI.intersects(_vm->_gfx->getDirtyRect()) && !_doNotDraw) {
		Graphics::Surface *leftBitmap = _vm->_gfx->getBitmap(IDB_UI_LEFT);
		_vm->_gfx->blit(leftBitmap, 0, 128);
		leftBitmap->free();
		delete leftBitmap;
	}

	if (rightUI.intersects(_vm->_gfx->getDirtyRect()) && !_doNotDraw) {
		Graphics::Surface *rightBitmap = _vm->_gfx->getBitmap(IDB_UI_RIGHT);
		_vm->_gfx->blit(rightBitmap, 496, 128);
		rightBitmap->free();
		delete rightBitmap;
	}

	if (bottomUI.intersects(_vm->_gfx->getDirtyRect()) && !_doNotDraw) {
		Graphics::Surface *bottomBitmap = _vm->_gfx->getBitmap(IDB_UI_BOTTOM);
		_vm->_gfx->blit(bottomBitmap, 0, 317);
		bottomBitmap->free();
		delete bottomBitmap;

		if (!_vm->isDemo()) {
			if (_currentDateDisplay >= 0) {
				Graphics::Surface *dateBitmap = _vm->_gfx->getBitmap(IDB_UI_DATE_BASE + _currentDateDisplay);
				_vm->_gfx->blit(dateBitmap, 62, 332);
				dateBitmap->free();
				delete dateBitmap;
			}

			if (_warningLightDisplayed) {
				Graphics::Surface *warningLight = _vm->_gfx->getBitmap(IDB_UI_WARNING_LIGHT);
				_vm->_gfx->blit(warningLight, 189, 321);
				warningLight->free();
				delete warningLight;
			}
		}
	}
}

void GameUIWindow::onEnable(bool enable) {
	// Pass the enable message to all child windows
	_inventoryWindow->enableWindow(enable);
	_navArrowWindow->enableWindow(enable);
	_liveTextWindow->enableWindow(enable);
	_sceneViewWindow->enableWindow(enable);
	_bioChipRightWindow->enableWindow(enable);

	// If we're re-enabling, clear out the message queue of any mouse messages
	if (enable)
		_vm->removeMouseMessages(this);
}

void GameUIWindow::onKeyUp(const Common::KeyState &key, uint flags) {
	const bool cloakingDisabled = _sceneViewWindow->getGlobalFlags().bcCloakingEnabled != 1;
	const bool interfaceMenuActive = (_bioChipRightWindow->getCurrentBioChip() == kItemBioChipInterface);

	switch (key.keycode) {
	case Common::KEYCODE_KP4:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_KP6:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_KP2:
	case Common::KEYCODE_DOWN:
	case Common::KEYCODE_KP8:
	case Common::KEYCODE_UP:
	case Common::KEYCODE_KP5:
		if (_navArrowWindow->isWindowEnabled())
			_navArrowWindow->sendMessage(new KeyUpMessage(key, flags));
		break;
	case Common::KEYCODE_s:
		if ((key.flags & Common::KBD_CTRL) && cloakingDisabled && !interfaceMenuActive) {
			_vm->handleSaveDialog();
		} else if (_sceneViewWindow)
			_sceneViewWindow->sendMessage(new KeyUpMessage(key, flags));
		break;
	case Common::KEYCODE_o:
	case Common::KEYCODE_l:
		if ((key.flags & Common::KBD_CTRL) && cloakingDisabled && !interfaceMenuActive) {
			_vm->handleRestoreDialog();
		} else if (_sceneViewWindow)
			_sceneViewWindow->sendMessage(new KeyUpMessage(key, flags));
		break;
	case Common::KEYCODE_p:
		if ((key.flags & Common::KBD_CTRL) && cloakingDisabled && !interfaceMenuActive)
			_vm->pauseGame();
		else if (_sceneViewWindow)
			_sceneViewWindow->sendMessage(new KeyUpMessage(key, flags));
		break;
	default:
		if (_sceneViewWindow)
			_sceneViewWindow->sendMessage(new KeyUpMessage(key, flags));
		break;
	}
}

} // End of namespace Buried
