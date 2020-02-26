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

#include "titanic/events.h"
#include "titanic/debugger.h"
#include "titanic/game_manager.h"
#include "titanic/main_game_window.h"
#include "titanic/star_control/star_control.h"
#include "titanic/titanic.h"
#include "common/events.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/screen.h"

namespace Titanic {

Events::Events(TitanicEngine *vm): _vm(vm), _frameCounter(1),
		_totalFrames(0), _priorFrameTime(0), _specialButtons(0) {
}

#define MOVE_CHECK if (moved) eventTarget()->mouseMove(_mousePos)

void Events::pollEvents() {
	checkForNextFrameCounter();

	bool moved = false;
	Common::Event event;
	while (!_vm->shouldQuit() && g_system->getEventManager()->pollEvent(event)) {
		if (event.type != Common::EVENT_MOUSEMOVE) {
			MOVE_CHECK;
		}

		switch (event.type) {
		case Common::EVENT_MOUSEMOVE:
			_mousePos = event.mouse;
			moved = true;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_specialButtons |= MK_LBUTTON;
			_mousePos = event.mouse;
			eventTarget()->leftButtonDown(_mousePos);
			return;
		case Common::EVENT_LBUTTONUP:
			_specialButtons &= ~MK_LBUTTON;
			_mousePos = event.mouse;
			eventTarget()->leftButtonUp(_mousePos);
			return;
		case Common::EVENT_MBUTTONDOWN:
			_specialButtons |= MK_MBUTTON;
			_mousePos = event.mouse;
			eventTarget()->middleButtonDown(_mousePos);
			return;
		case Common::EVENT_MBUTTONUP:
			_specialButtons &= ~MK_MBUTTON;
			_mousePos = event.mouse;
			eventTarget()->middleButtonUp(_mousePos);
			return;
		case Common::EVENT_RBUTTONDOWN:
			_specialButtons |= MK_LBUTTON | MK_SHIFT;
			_mousePos = event.mouse;
			eventTarget()->leftButtonDown(_mousePos);
			return;
		case Common::EVENT_RBUTTONUP:
			_specialButtons &= ~(MK_RBUTTON | MK_SHIFT);
			_mousePos = event.mouse;
			eventTarget()->leftButtonUp(_mousePos);
			return;
		case Common::EVENT_WHEELUP:
		case Common::EVENT_WHEELDOWN:
			_mousePos = event.mouse;
			eventTarget()->mouseWheel(_mousePos, event.type == Common::EVENT_WHEELUP);
			return;
		case Common::EVENT_KEYDOWN:
			handleKbdSpecial(event.kbd);
			eventTarget()->keyDown(event.kbd);
			return;
		case Common::EVENT_KEYUP:
			handleKbdSpecial(event.kbd);
			eventTarget()->keyUp(event.kbd);
			return;
		default:
			break;
		}
	}

	MOVE_CHECK;
}

#undef MOVE_CHECK

void Events::pollEventsAndWait() {
	pollEvents();
	g_system->delayMillis(10);

	CGameManager *gameManager = g_vm->_window->_gameManager;
	if (gameManager) {
		// Regularly update the sound mixer
		gameManager->_sound.updateMixer();

		// WORKAROUND: If in the Star Control view, update the camera
		// frequently, to accomodate that the original had a higher
		// draw rate than the ScummVM implementation does
		CViewItem *view = gameManager->getView();
		if (view->getFullViewName() == "Bridge.Node 4.N") {
			CStarControl *starControl = dynamic_cast<CStarControl *>(
				view->findChildInstanceOf(CStarControl::_type));
			if (starControl && starControl->_visible)
				starControl->updateCamera();
		}
	}
}

bool Events::checkForNextFrameCounter() {
	// Check for next game frame
	uint32 milli = g_system->getMillis();
	if ((milli - _priorFrameTime) >= GAME_FRAME_TIME) {
		++_frameCounter;
		++_totalFrames;
		_priorFrameTime = milli;

		// Handle any idle updates
		eventTarget()->onIdle();

		// Display the frame
		_vm->_screen->update();

		return true;
	}

	return false;
}

uint32 Events::getTicksCount() const {
	return _frameCounter * GAME_FRAME_TIME;
}

uint32 Events::getTotalPlayTicks() const {
	return _totalFrames;
}

void Events::setTotalPlayTicks(uint frames) {
	_totalFrames = frames;
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
