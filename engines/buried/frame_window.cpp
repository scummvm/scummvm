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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/system.h"
#include "graphics/surface.h"

#include "buried/buried.h"
#include "buried/complete.h"
#include "buried/credits.h"
#include "buried/death.h"
#include "buried/frame_window.h"
#include "buried/gameui.h"
#include "buried/graphics.h"
#include "buried/inventory_window.h"
#include "buried/main_menu.h"
#include "buried/message.h"
#include "buried/overview.h"
#include "buried/resources.h"
#include "buried/scene_view.h"
#include "buried/sound.h"
#include "buried/title_sequence.h"
#include "buried/video_window.h"
#include "buried/demo/demo_menu.h"
#include "buried/demo/features.h"
#include "buried/demo/movie_scene.h"

namespace Buried {

FrameWindow::FrameWindow(BuriedEngine *vm) : Window(vm, nullptr) {
	// Initialize member variables
	_mainChildWindow = nullptr;
	_controlDown = false;
	_cacheFrames = false;
	_cycleDefault = true;
	_transitionSpeed = 2;
	_gameInProgress = false;
	_atMainMenu = true;
	_reviewerMode = false;

	// Retrieve the transition speed from the INI file
	Common::String transitionConfigName = _vm->isDemo() ? "TransitionSpeed" : _vm->getString(IDS_INI_KEY_TRANS_SPEED);
	if (ConfMan.hasKey(transitionConfigName))
		_transitionSpeed = ConfMan.getInt(transitionConfigName);

	// Get the INI file key for frame caching
	Common::String cacheFramesConfigName = _vm->isDemo() ? "CycleCaching" : _vm->getString(IDS_INI_KEY_CYCLE_CACHING);
	if (ConfMan.hasKey(cacheFramesConfigName))
		_cacheFrames = ConfMan.getInt(cacheFramesConfigName) != 0;

	_rect = Common::Rect(0, 0, 640, 480);
}

FrameWindow::~FrameWindow() {
	delete _mainChildWindow;
}

bool FrameWindow::showTitleSequence() {
	invalidateWindow();
	updateWindow();

	Graphics::Surface *swLogo = _vm->_gfx->getBitmap(_vm->isTrueColor() ? "MISC/24BPP/SWLOGO.BMP" : "MISC/8BPP/SWLOGO.BMP");
	uint32 x = (640 - swLogo->w) / 2;
	uint32 y = (480 - swLogo->h) / 2;
	_vm->_gfx->blit(swLogo, x, y);
	_vm->_gfx->updateScreen(false);
	swLogo->free();
	delete swLogo;

	_vm->_sound->playInterfaceSound("MISC/SWSTING.WAV");

	_vm->removeMouseMessages(this);

	uint32 startTime = g_system->getMillis();
	while (g_system->getMillis() < (startTime + 7000) && !_vm->hasMessage(this, kMessageTypeLButtonDown, kMessageTypeLButtonDown) && !_vm->shouldQuit())
		_vm->yield(nullptr, -1);

	_vm->_sound->stopInterfaceSound();
	invalidateWindow();

	VideoWindow *video = new VideoWindow(_vm, this);

	if (!video->openVideo("MISC/PRESTO.AVI"))
		error("Failed to open MISC/PRESTO.AVI");

	video->enableWindow(false);
	x = (_rect.right - video->getRect().right) / 2;
	y = (_rect.bottom - video->getRect().bottom) / 2;

	video->setWindowPos(nullptr, x, y, 0, 0, kWindowPosNoSize | kWindowPosNoZOrder | kWindowPosShowWindow);
	video->playVideo();
	enableWindow(true);

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(video);

	while (!_vm->shouldQuit() && video->getMode() != VideoWindow::kModeStopped && !_vm->hasMessage(this, kMessageTypeLButtonDown, kMessageTypeLButtonDown))
		_vm->yield(video, -1);

	delete video;

	invalidateWindow();
	return true;
}

bool FrameWindow::showMainMenu() {
	_gameInProgress = false;
	_atMainMenu = true;

	// If we still have a child window, delete it now
	delete _mainChildWindow;
	_mainChildWindow = nullptr;

	_vm->_sound->restart();

	// Create and show the main menu window
	if (_vm->isDemo()) {
		_mainChildWindow = new DemoMainMenuWindow(_vm, this);
		((DemoMainMenuWindow *)_mainChildWindow)->showWithSplash();
	} else {
		_mainChildWindow = new MainMenuWindow(_vm, this);
		((MainMenuWindow *)_mainChildWindow)->showMainMenu();
	}

	setFocus();
	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);
	return true;
}

bool FrameWindow::returnToMainMenu() {
	_gameInProgress = false;
	_atMainMenu = true;

	// Kill the ambient and restart it
	_vm->_sound->restart();
	_vm->_sound->setAmbientSound();

	// If we still have a child window, delete it now
	delete _mainChildWindow;
	_mainChildWindow = nullptr;

	// Create and show the main menu window
	if (_vm->isDemo()) {
		_mainChildWindow = new DemoMainMenuWindow(_vm, this);
		_mainChildWindow->showWindow(kWindowShow);
	} else {
		_mainChildWindow = new MainMenuWindow(_vm, this);
		((MainMenuWindow *)_mainChildWindow)->showMainMenu();
	}

	setFocus();

	// Empty input queue
	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);

	return true;
}

bool FrameWindow::playMovie(const Common::String &background, const Common::String &movie, int movieLeft, int movieTop) {
	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);

	delete _mainChildWindow;
	_mainChildWindow = new MovieDisplayWindow(_vm, this, background, movie, movieLeft, movieTop);

	((MovieDisplayWindow *)_mainChildWindow)->showMovieInWindow();

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);

	return true;
}

bool FrameWindow::showClosingScreen() {
	_gameInProgress = false;
	_atMainMenu = false;

	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);

	// If we still have a child window, delete it now
	delete _mainChildWindow;
	_mainChildWindow = nullptr;

	// Create the window
	_mainChildWindow = new TitleSequenceWindow(_vm, this);

	// Show the window
	_mainChildWindow->showWindow(kWindowShow);
	_mainChildWindow->setFocus();

	// Start the title sequence
	((TitleSequenceWindow *)_mainChildWindow)->playTitleSequence();

	// Empty the input queue
	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);
	_vm->removeKeyboardMessages(_mainChildWindow);

	return true;
}

bool FrameWindow::showFeaturesScreen() {
	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);

	delete _mainChildWindow;
	_mainChildWindow = new FeaturesDisplayWindow(_vm, this);

	_mainChildWindow->showWindow(kWindowShow);

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);
	return true;
}

bool FrameWindow::startNewGame(bool walkthrough, bool introMovie) {
	_gameInProgress = true;
	_atMainMenu = false;

	_vm->removeMouseMessages(this);

	delete _mainChildWindow;

	// Create Game UI window to kick things off
	_mainChildWindow = new GameUIWindow(_vm, this);
	_mainChildWindow->showWindow(kWindowShow);
	setFocus();

	if (introMovie)
		((GameUIWindow *)_mainChildWindow)->startNewGameIntro(walkthrough);
	else
		((GameUIWindow *)_mainChildWindow)->startNewGame(walkthrough);

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);
	return true;
}

bool FrameWindow::showDeathScene(int deathSceneIndex, GlobalFlags &globalFlags, Common::Array<int> itemArray) {
	_gameInProgress = false;
	_atMainMenu = false;

	_vm->removeMouseMessages(this);

	// Pass globalFlags by value to DeathWindow here, as they will be destroyed
	// together with _mainChildWindow (a GameUIWindow, which contains the scene
	// window, which holds the instance of the global flags)
	DeathWindow *deathWindow = new DeathWindow(_vm, this, deathSceneIndex, globalFlags, itemArray);

	delete _mainChildWindow;
	_mainChildWindow = deathWindow;
	_mainChildWindow->showWindow(kWindowShow);
	_mainChildWindow->invalidateWindow(false);

	return true;
}

bool FrameWindow::showCompletionScene(GlobalFlags &globalFlags) {
	_gameInProgress = false;
	_atMainMenu = false;

	_vm->removeMouseMessages(this);

	// Pass globalFlags by value to CompletionWindow here, as they will be destroyed
	// together with _mainChildWindow (a GameUIWindow, which contains the scene
	// window, which holds the instance of the global flags)
	CompletionWindow *completionWindow = new CompletionWindow(_vm, this, globalFlags);

	delete _mainChildWindow;
	_mainChildWindow = completionWindow;
	_mainChildWindow->showWindow(kWindowShow);
	_mainChildWindow->invalidateWindow(false);

	return true;
}

bool FrameWindow::showCredits() {
	_gameInProgress = false;
	_atMainMenu = false;

	_vm->removeMouseMessages(this);

	// If we still have a child window, delete it now
	delete _mainChildWindow;

	_mainChildWindow = new CreditsWindow(_vm, this);
	_mainChildWindow->showWindow(kWindowShow);
	_mainChildWindow->invalidateWindow(false);

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);

	return true;
}

bool FrameWindow::showOverview() {
	_gameInProgress = false;
	_atMainMenu = false;

	_vm->removeMouseMessages(this);

	// If we still have a child window, delete it now
	delete _mainChildWindow;

	_mainChildWindow = new OverviewWindow(_vm, this);
	((OverviewWindow *)_mainChildWindow)->startOverview();

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);

	return true;
}

bool FrameWindow::setTimerPause(bool pause) {
	if (_gameInProgress) {
		((GameUIWindow *)_mainChildWindow)->_sceneViewWindow->_paused = pause;
		return true;
	}

	return false;
}


bool FrameWindow::onEraseBackground() {
	_vm->_gfx->fillRect(getAbsoluteRect(), _vm->_gfx->getColor(0, 0, 0));
	return true;
}

void FrameWindow::onKeyDown(const Common::KeyState &key, uint flags) {
	_controlDown = (key.flags & Common::KBD_CTRL) != 0;

	if (key.keycode == Common::KEYCODE_ESCAPE) {
		if (_gameInProgress || !_atMainMenu) {
			// Ask if the player wants to return
			if (_vm->runQuitDialog())
				showMainMenu();
		} else {
			// Quit from the main menu
			_vm->quitGame();
		}
	}
}

void FrameWindow::onKeyUp(const Common::KeyState &key, uint flags) {
	_controlDown = (key.flags & Common::KBD_CTRL) != 0;

	if (_mainChildWindow)
		_mainChildWindow->sendMessage(new KeyUpMessage(key, flags));
}

void FrameWindow::onTimer(uint timer) {
	// Call the sound manager maintenance callback function to refresh the buffers
	_vm->_sound->timerCallback();
}

void FrameWindow::onSetFocus(Window *oldWindow) {
	_controlDown = false;
}

void FrameWindow::onKillFocus(Window *newWindow) {
	_controlDown = false;
}

void FrameWindow::setTransitionSpeed(int newSpeed) {
	_transitionSpeed = newSpeed;
	ConfMan.setInt(_vm->isDemo() ? "TransitionSpeed" : _vm->getString(IDS_INI_KEY_TRANS_SPEED), newSpeed);
}

void FrameWindow::loadFromState(const Location &location, GlobalFlags &flags, Common::Array<int> inventoryItems) {
	if (!_gameInProgress) {
		// Make the game in progress
		_atMainMenu = false;
		_gameInProgress = true;
		delete _mainChildWindow;
		_mainChildWindow = new GameUIWindow(_vm, this);
		_mainChildWindow->showWindow(kWindowShow);
		setFocus();
	}

	GameUIWindow *gameUI = (GameUIWindow *)_mainChildWindow;

	// Set the flags
	gameUI->_sceneViewWindow->getGlobalFlags() = flags;

	// Set the inventory array
	gameUI->_inventoryWindow->setItemArray(inventoryItems);
	gameUI->_inventoryWindow->rebuildPreBuffer();
	gameUI->_inventoryWindow->invalidateWindow(false);

	gameUI->startNewGame(location);
}

} // End of namespace Buried
