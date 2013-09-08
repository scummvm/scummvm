/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1995 Presto Studios, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/config-manager.h"
#include "common/events.h"

#include "buried/buried.h"
#include "buried/credits.h"
#include "buried/frame_window.h"
#include "buried/graphics.h"
#include "buried/main_menu.h"
#include "buried/message.h"
#include "buried/overview.h"
#include "buried/resources.h"
#include "buried/sound.h"
#include "buried/title_sequence.h"
#include "buried/demo/demo_menu.h"

namespace Buried {

FrameWindow::FrameWindow(BuriedEngine *vm) : Window(vm, 0) {
	// Initialize member variables
	_mainChildWindow = 0;
	_controlDown = false;
	_cacheFrames = false;
	_cycleDefault = false;
	_transitionSpeed = 2;
	_gameInProgress = false;
	_atMainMenu = true;

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

bool FrameWindow::showMainMenu() {
	_gameInProgress = false;
	_atMainMenu = true;

	// If we still have a child window, delete it now
	delete _mainChildWindow;
	_mainChildWindow = 0;

	_vm->_sound->restart();

	// Create and show the main menu window
	if (_vm->isDemo()) {
		_mainChildWindow = new DemoMainMenuWindow(_vm, this);
		((DemoMainMenuWindow *)_mainChildWindow)->showWithSplash();
	} else {
		_mainChildWindow = new MainMenuWindow(_vm, this);
		((MainMenuWindow *)_mainChildWindow)->showMainMenu();
	}

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
	_mainChildWindow = 0;

	// Create and show the main menu window
	if (_vm->isDemo()) {
		_mainChildWindow = new DemoMainMenuWindow(_vm, this);
		_mainChildWindow->showWindow(kWindowShow);
	} else {
		_mainChildWindow = new MainMenuWindow(_vm, this);
		((MainMenuWindow *)_mainChildWindow)->showMainMenu();
	}

	// Empty input queue
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
	_mainChildWindow = 0;

	// Create the window
	_mainChildWindow = new TitleSequenceWindow(_vm, this);

	// Show the window
	_mainChildWindow->showWindow(kWindowShow);
	_mainChildWindow->setFocus();

	// Start the the title sequence
	((TitleSequenceWindow *)_mainChildWindow)->playTitleSequence();

	// Empty the input queue
	_vm->removeMouseMessages(this);
	_vm->removeKeyboardMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);
	_vm->removeKeyboardMessages(_mainChildWindow);

	return true;
}

bool FrameWindow::startNewGame(bool walkthrough, bool introMovie) {
	_gameInProgress = true;
	_atMainMenu = false;

	_vm->removeMouseMessages(this);

	delete _mainChildWindow;
	_mainChildWindow = 0;

	// TODO: Create and show UI, then begin the game

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);
	return true;
}

bool FrameWindow::startNewGame(const Common::String &fileName) {
	_gameInProgress = true;
	_atMainMenu = false;

	_vm->removeMouseMessages(this);

	delete _mainChildWindow;
	_mainChildWindow = 0;

	// TODO: Create and show UI, then begin the game

	_vm->removeMouseMessages(this);
	_vm->removeMouseMessages(_mainChildWindow);
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
		// TODO
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
		// TODO: Possibly quit
	}
}

void FrameWindow::onKeyUp(const Common::KeyState &key, uint flags) {
	_controlDown = (key.flags & Common::KBD_CTRL) != 0;

	if (_mainChildWindow)
		_mainChildWindow->sendMessage(new KeyUpMessage(key, flags));
}

void FrameWindow::onTimer(uint timer) {
	// Call the sound manager maintence callback function to refresh the buffers
	_vm->_sound->timerCallback();
}

void FrameWindow::onSetFocus(Window *oldWindow) {
	_controlDown = false;
}

void FrameWindow::onKillFocus(Window *newWindow) {
	_controlDown = false;
}

} // End of namespace Buried
