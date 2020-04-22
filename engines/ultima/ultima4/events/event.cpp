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
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/game/textview.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/map/location.h"
#include "common/events.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

using namespace Std;

bool EventHandler::_controllerDone = false;
bool EventHandler::_ended = false;

EventHandler *EventHandler::_instance = nullptr;

EventHandler::EventHandler() : _timer(settings._eventTimerGranularity), _updateScreen(nullptr) {
}

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

void EventHandler::sleep(uint msec) {
	g_system->delayMillis(msec);
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
}

bool EventHandler::getControllerDone() {
	return _controllerDone;
}

void EventHandler::end() {
	// End all event processing
	_ended = true;
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

void EventHandler::run() {
	if (_updateScreen)
		(*_updateScreen)();
	g_screen->update();

	while (!_ended && !_controllerDone) {
		Common::Event event;
		g_system->getEventManager()->pollEvent(event);

		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			handleKeyDownEvent(event, getController(), _updateScreen);
			break;

		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_MBUTTONDOWN:
			handleMouseButtonDownEvent(event, getController(), _updateScreen);
			break;

		case Common::EVENT_MOUSEMOVE:
			handleMouseMotionEvent(event);
			continue;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			getController()->keybinder((KeybindingAction)event.customType);
			break;

		case Common::EVENT_QUIT:
			_ended = true;
			return;

		default:
			break;
		}

		// Brief delay
		g_system->delayMillis(10);

		// Poll the timer manager
		_timer.poll();
	}
}

void EventHandler::setScreenUpdate(void (*updateScreen)(void)) {
	_updateScreen = updateScreen;
}

void EventHandler::pushKeyHandler(KeyHandler kh) {
	KeyHandler *new_kh = new KeyHandler(kh);
	KeyHandlerController *khc = new KeyHandlerController(new_kh);
	pushController(khc);
}

void EventHandler::popKeyHandler() {
	if (_controllers.empty())
		return;

	popController();
}

KeyHandler *EventHandler::getKeyHandler() const {
	if (_controllers.empty())
		return nullptr;

	KeyHandlerController *khc = dynamic_cast<KeyHandlerController *>(_controllers.back());
	ASSERT(khc != nullptr, "EventHandler::getKeyHandler called when controller wasn't a keyhandler");
	if (khc == nullptr)
		return nullptr;

	return khc->getKeyHandler();
}

void EventHandler::setKeyHandler(KeyHandler kh) {
	while (popController() != nullptr) {
	}
	pushKeyHandler(kh);
}

const MouseArea *EventHandler::mouseAreaForPoint(int x, int y) {
	int i;
	const MouseArea *areas = getMouseAreaSet();

	if (!areas)
		return nullptr;

	for (i = 0; areas[i]._nPoints != 0; i++) {
		if (g_screen->screenPointInMouseArea(x, y, &(areas[i]))) {
			return &(areas[i]);
		}
	}
	return nullptr;
}

void EventHandler::handleMouseMotionEvent(const Common::Event &event) {
	if (!settings._mouseOptions._enabled)
		return;

	const MouseArea *area;
	area = eventHandler->mouseAreaForPoint(event.mouse.x, event.mouse.y);
	if (area)
		g_screen->setMouseCursor(area->_cursor);
	else
		g_screen->setMouseCursor(MC_DEFAULT);
}

void EventHandler::handleMouseButtonDownEvent(const Common::Event &event, Controller *controller, updateScreenCallback updateScreen) {
	if (!settings._mouseOptions._enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return;

	const MouseArea *area = eventHandler->mouseAreaForPoint(event.mouse.x, event.mouse.y);
	if (!area)
		return;
	controller->keybinder(KEYBIND_INTERACT);

	if (updateScreen)
		(*updateScreen)();
	g_screen->update();
}

void EventHandler::handleKeyDownEvent(const Common::Event &event, Controller *controller, updateScreenCallback updateScreen) {
	int key;
	bool processed;

	key = event.kbd.ascii;
	if (!key)
		return;
	key += event.kbd.flags << 16;

	debug(1, "key event: sym = %d, mod = %d; translated = %d",
		event.kbd.keycode, event.kbd.flags, key);

	// handle the keypress
	processed = controller->notifyKeyPressed(key);

	if (processed) {
		if (updateScreen)
			(*updateScreen)();
		g_screen->update();
	}
}

} // End of namespace Ultima4
} // End of namespace Ultima
