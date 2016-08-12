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
		_priorFrameTime(0) {
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
		_mousePos = event.mouse;
		eventTarget()->leftButtonDown(_mousePos);
		break;
	case Common::EVENT_LBUTTONUP:
		_mousePos = event.mouse;
		eventTarget()->leftButtonUp(_mousePos);
		break;
	case Common::EVENT_MBUTTONDOWN:
		_mousePos = event.mouse;
		eventTarget()->middleButtonDown(_mousePos);
		break;
	case Common::EVENT_MBUTTONUP:
		_mousePos = event.mouse;
		eventTarget()->middleButtonUp(_mousePos);
		break;
	case Common::EVENT_RBUTTONDOWN:
		_mousePos = event.mouse;
		eventTarget()->rightButtonDown(_mousePos);
		break;
	case Common::EVENT_RBUTTONUP:
		_mousePos = event.mouse;
		eventTarget()->rightButtonUp(_mousePos);
		break;
	case Common::EVENT_KEYDOWN:
		eventTarget()->keyDown(event.kbd);
		break;
	case Common::EVENT_KEYUP:
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
	return g_system->getMillis();
}

void Events::sleep(uint time) {
	uint32 delayEnd = g_system->getMillis() + time;

	while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd)
		pollEventsAndWait();
}

bool Events::waitForPress(uint expiry) {
	uint32 delayEnd = g_system->getMillis() + expiry;

	while (!_vm->shouldQuit() && g_system->getMillis() < delayEnd) {
		g_system->delayMillis(10);
		checkForNextFrameCounter();

		Common::Event event;
		if (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
			case Common::EVENT_MBUTTONDOWN:
			case Common::EVENT_KEYDOWN:
				return true;
			default:
				break;
			}
		}
	}

	return false;
}

} // End of namespace Titanic
