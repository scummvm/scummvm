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

Events::Events(TitanicEngine *vm): _vm(vm), _frameCounter(1),
		_priorFrameTime(0), _specialButtons(0) {
}

void Events::pollEvents() {
	checkForNextFrameCounter();

	Common::Event event;
	if (!g_system->getEventManager()->pollEvent(event))
		return;

	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		_mousePos = event.mouse;
		eventTarget()->mouseMove(_mousePos);
		break;
	case Common::EVENT_LBUTTONDOWN:
		_specialButtons |= MK_LBUTTON;
		_mousePos = event.mouse;
		eventTarget()->leftButtonDown(_mousePos);
		break;
	case Common::EVENT_LBUTTONUP:
		_specialButtons &= ~MK_LBUTTON;
		_mousePos = event.mouse;
		eventTarget()->leftButtonUp(_mousePos);
		break;
	case Common::EVENT_MBUTTONDOWN:
		_specialButtons |= MK_MBUTTON;
		_mousePos = event.mouse;
		eventTarget()->middleButtonDown(_mousePos);
		break;
	case Common::EVENT_MBUTTONUP:
		_specialButtons &= ~MK_MBUTTON;
		_mousePos = event.mouse;
		eventTarget()->middleButtonUp(_mousePos);
		break;
	case Common::EVENT_RBUTTONDOWN:
		_specialButtons |= MK_RBUTTON;
		_mousePos = event.mouse;
		eventTarget()->rightButtonDown(_mousePos);
		break;
	case Common::EVENT_RBUTTONUP:
		_specialButtons &= ~MK_RBUTTON;
		_mousePos = event.mouse;
		eventTarget()->rightButtonUp(_mousePos);
		break;
	case Common::EVENT_WHEELUP:
	case Common::EVENT_WHEELDOWN:
		_mousePos = event.mouse;
		eventTarget()->mouseWheel(_mousePos, event.type == Common::EVENT_WHEELUP);
		break;
	case Common::EVENT_KEYDOWN:
		handleKbdSpecial(event.kbd);
		eventTarget()->keyDown(event.kbd);
		break;
	case Common::EVENT_KEYUP:
		handleKbdSpecial(event.kbd);
		eventTarget()->keyUp(event.kbd);
		break;
	default:
		break;
	}
}

void Events::pollEventsAndWait() {
	pollEvents();
	g_system->delayMillis(10);

	// Regularly update the sound mixer
	CGameManager *gameManager = g_vm->_window->_gameManager;
	if (gameManager)
		gameManager->_sound.updateMixer();
}

bool Events::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		++_frameCounter;
		_priorFrameTime = milli;

		// Handle any idle updates
		eventTarget()->onIdle();

		// Give time to the debugger
		_vm->_debugger->onFrame();

		// Display the frame
		_vm->_screen->update();

		return true;
	}

	return false;
}

uint32 Events::getTicksCount() const {
	return _frameCounter * GAME_FRAME_TIME;
}

void Events::sleep(uint time) {
	uint32 delayEnd = g_system->getMillis() + time;

	while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd)
		pollEventsAndWait();
}

bool Events::waitForPress(uint expiry) {
	uint32 delayEnd = g_system->getMillis() + expiry;
	CPressTarget pressTarget;
	addTarget(&pressTarget);

	while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd && !pressTarget._pressed) {
		pollEventsAndWait();
	}

	removeTarget();
	return pressTarget._pressed;
}

void Events::setMousePos(const Common::Point &pt) {
	g_system->warpMouse(pt.x, pt.y);
	_mousePos = pt;
	eventTarget()->mouseMove(_mousePos);
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
