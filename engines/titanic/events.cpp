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

#include "common/scummsys.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "titanic/events.h"
#include "titanic/titanic.h"
#include "titanic/main_game_window.h"

namespace Titanic {

Events::Events(TitanicEngine *vm): _vm(vm), _specialButtons(0),
		_frameCounter(1), _priorFrameTime(0), _priorLeftDownTime(0),
		_priorMiddleDownTime(0), _priorRightDownTime(0) {
}

void Events::pollEvents() {
	checkForNextFrameCounter();

	Common::Event event;
	if (!g_system->getEventManager()->pollEvent(event))
		return;

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		_mousePos = event.mouse;
		mouseMove();
		break;
	case Common::EVENT_LBUTTONDOWN:
		_mousePos = event.mouse;
		leftButtonDown();
		break;
	case Common::EVENT_LBUTTONUP:
		_mousePos = event.mouse;
		leftButtonUp();
		break;
	case Common::EVENT_MBUTTONDOWN:
		_mousePos = event.mouse;
		middleButtonDown();
		break;
	case Common::EVENT_MBUTTONUP:
		_mousePos = event.mouse;
		middleButtonUp();
		break;
	case Common::EVENT_KEYDOWN:
		keyDown(event.kbd);
		break;
	case Common::EVENT_KEYUP:
		keyUp(event.kbd);
		break;
	default:
		break;
	}
}

void Events::pollEventsAndWait() {
	pollEvents();
	g_system->delayMillis(10);
}

bool Events::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		++_frameCounter;
		_priorFrameTime = milli;

		// Handle any idle updates
		onIdle();

		// Give time to the debugger
		_vm->_debugger->onFrame();

		// Display the frame
		_vm->_screen->update();

		return true;
	}

	return false;
}

uint32 Events::getTicksCount() const {
	return g_system->getMillis();
}

void Events::onIdle() {
	if (!_vm->_window->_inputAllowed)
		return;
	CGameManager *gameManager = _vm->_window->_gameManager;
	if (!gameManager)
		return;

	// Let the game manager perform any game updates
	gameManager->update();

	if (gameManager->_gameState._quitGame) {
		// Game needs to shut down
		_vm->quitGame();
	}
}

#define HANDLE_MESSAGE(METHOD) 	if (_vm->_window->_inputAllowed) { \
	_vm->_window->_gameManager->_inputTranslator.METHOD(_specialButtons, Point(_mousePos.x, _mousePos.y)); \
	_vm->_window->mouseChanged(); \
	}


void Events::mouseMove() {
	HANDLE_MESSAGE(mouseMove)
}

void Events::leftButtonDown() {
	_specialButtons |= MK_LBUTTON;

	if ((getTicksCount() - _priorLeftDownTime) < DOUBLE_CLICK_TIME) {
		_priorLeftDownTime = 0;
		leftButtonDoubleClick();
	} else {
		_priorLeftDownTime = getTicksCount();
		HANDLE_MESSAGE(leftButtonDown)
	}
}

void Events::leftButtonUp() {
	_specialButtons &= ~MK_LBUTTON;
	HANDLE_MESSAGE(leftButtonUp)
}

void Events::leftButtonDoubleClick() {
	HANDLE_MESSAGE(leftButtonDoubleClick)
}

void Events::middleButtonDown() {
	_specialButtons |= MK_MBUTTON;

	if ((getTicksCount() - _priorMiddleDownTime) < DOUBLE_CLICK_TIME) {
		_priorMiddleDownTime = 0;
		middleButtonDoubleClick();
	} else {
		_priorMiddleDownTime = getTicksCount();
		HANDLE_MESSAGE(middleButtonDown)
	}
}

void Events::middleButtonUp() {
	_specialButtons &= ~MK_MBUTTON;
	HANDLE_MESSAGE(middleButtonUp)
}

void Events::middleButtonDoubleClick() {
	HANDLE_MESSAGE(middleButtonDoubleClick)
}

void Events::rightButtonDown() {
	_specialButtons |= MK_RBUTTON;

	if ((getTicksCount() - _priorRightDownTime) < DOUBLE_CLICK_TIME) {
		_priorRightDownTime = 0;
		rightButtonDoubleClick();
	} else {
		_priorRightDownTime = getTicksCount();
		HANDLE_MESSAGE(rightButtonDown)
	}
}

void Events::rightButtonUp() {
	_specialButtons &= ~MK_RBUTTON;
	HANDLE_MESSAGE(rightButtonUp)
}

void Events::rightButtonDoubleClick() {
	HANDLE_MESSAGE(rightButtonDoubleClick)
}

void Events::charPress(char c) {

}

void Events::keyDown(Common::KeyState keyState) {
	handleKbdSpecial(keyState);

	if (keyState.keycode == Common::KEYCODE_d && (keyState.flags & Common::KBD_CTRL)) {
		// Attach to the debugger
		_vm->_debugger->attach();
		_vm->_debugger->onFrame();
	}

	if (_vm->_window->_inputAllowed)
		_vm->_window->_gameManager->_inputTranslator.keyDown(keyState);
}

void Events::keyUp(Common::KeyState keyState) {
	handleKbdSpecial(keyState);
}

void Events::handleKbdSpecial(Common::KeyState keyState) {
	if (keyState.flags & Common::KBD_CTRL)
		_specialButtons |= MK_CONTROL;
	else
		_specialButtons &= ~MK_CONTROL;

	if (keyState.flags & Common::KBD_SHIFT)
		_specialButtons |= MK_SHIFT;
	else
		_specialButtons &= ~MK_SHIFT;
}

} // End of namespace Titanic
