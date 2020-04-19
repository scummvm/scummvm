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

#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/controllers/wait_controller.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/textview.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/location.h"
#include "common/events.h"

namespace Ultima {
namespace Ultima4 {

using namespace Std;

bool EventHandler::_controllerDone = false;
bool EventHandler::_ended = false;

EventHandler *EventHandler::_instance = nullptr;
EventHandler *EventHandler::getInstance() {
	if (_instance == nullptr)
		_instance = new EventHandler();
	return _instance;
}

void EventHandler::wait_msecs(uint msecs) {
	int msecs_per_cycle = (1000 / settings._gameCyclesPerSecond);
	int cycles = msecs / msecs_per_cycle;

	if (cycles > 0) {
		WaitController waitCtrl(cycles);
		getInstance()->pushController(&waitCtrl);
		waitCtrl.wait();
	}
	// Sleep the rest of the msecs we can't wait for
	EventHandler::sleep(msecs % msecs_per_cycle);
}

void EventHandler::wait_cycles(uint cycles) {
	WaitController waitCtrl(cycles);
	getInstance()->pushController(&waitCtrl);
	waitCtrl.wait();
}

void EventHandler::setControllerDone(bool done) {
	_controllerDone = done;
#if defined(IOS)
	if (done)
		controllerStopped_helper();
#endif
}     /**< Sets the controller exit flag for the event handler */

bool EventHandler::getControllerDone()         {
	return _controllerDone;    /**< Returns the current value of the global exit flag */
}

void EventHandler::end() {
	_ended = true;    /**< End all event processing */
}

TimedEventMgr *EventHandler::getTimer()  {
	return &_timer;
}

Controller *EventHandler::pushController(Controller *c) {
	c->setActive();
	_controllers.push_back(c);
	getTimer()->add(&Controller::timerCallback, c->getTimerInterval(), c);
	return c;
}

Controller *EventHandler::popController() {
	if (_controllers.empty())
		return nullptr;

	Controller *controller = _controllers.back();
	getTimer()->remove(&Controller::timerCallback, controller);
	_controllers.pop_back();

	controller = getController();
	if (controller)
		controller->setActive();

	return controller;
}

Controller *EventHandler::getController() const {
	if (_controllers.empty())
		return nullptr;

	return _controllers.back();
}

void EventHandler::setController(Controller *c) {
	while (popController() != nullptr) {}
	pushController(c);
}

void EventHandler::pushMouseAreaSet(const MouseArea *mouseAreas) {
	_mouseAreaSets.push_front(mouseAreas);
}

void EventHandler::popMouseAreaSet() {
	if (_mouseAreaSets.size())
		_mouseAreaSets.pop_front();
}

const MouseArea *EventHandler::getMouseAreaSet() const {
	if (_mouseAreaSets.size())
		return _mouseAreaSets.front();
	else
		return nullptr;
}

} // End of namespace Ultima4
} // End of namespace Ultima
