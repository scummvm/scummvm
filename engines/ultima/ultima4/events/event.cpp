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
#include "ultima/ultima4/context.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/map/location.h"
#include "ultima/ultima4/filesys/savegame.h"
#include "ultima/ultima4/screen.h"
#include "ultima/ultima4/settings.h"
#include "ultima/ultima4/textview.h"
#include "common/events.h"

namespace Ultima {
namespace Ultima4 {

using namespace Std;

int eventTimerGranularity = 250;

extern bool quit;
bool EventHandler::_controllerDone = false;
bool EventHandler::_ended = false;
unsigned int TimedEventMgr::instances = 0;

EventHandler *EventHandler::_instance = NULL;
EventHandler *EventHandler::getInstance() {
	if (_instance == NULL)
		_instance = new EventHandler();
	return _instance;
}

/**
 * Waits a given number of milliseconds before continuing
 */
void EventHandler::wait_msecs(unsigned int msecs) {
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

/**
 * Waits a given number of game cycles before continuing
 */
void EventHandler::wait_cycles(unsigned int cycles) {
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
	_controllers.push_back(c);
	getTimer()->add(&Controller::timerCallback, c->getTimerInterval(), c);
	return c;
}

Controller *EventHandler::popController() {
	if (_controllers.empty())
		return NULL;

	Controller *controller = _controllers.back();
	getTimer()->remove(&Controller::timerCallback, controller);
	_controllers.pop_back();

	return getController();
}


Controller *EventHandler::getController() const {
	if (_controllers.empty())
		return NULL;

	return _controllers.back();
}

void EventHandler::setController(Controller *c) {
	while (popController() != NULL) {}
	pushController(c);
}


/* TimedEvent functions */
TimedEvent::TimedEvent(TimedEvent::Callback cb, int i, void *d) :
	callback(cb),
	data(d),
	interval(i),
	current(0) {
}

TimedEvent::Callback TimedEvent::getCallback() const    {
	return callback;
}
void *TimedEvent::getData()                             {
	return data;
}

/**
 * Advances the timed event forward a tick.
 * When (current >= interval), then it executes its callback function.
 */
void TimedEvent::tick() {
	if (++current >= interval) {
		(*callback)(data);
		current = 0;
	}
}

/**
 * Returns true if the event queue is locked
 */
bool TimedEventMgr::isLocked() const {
	return locked;
}

/**
 * Adds a timed event to the event queue.
 */
void TimedEventMgr::add(TimedEvent::Callback callback, int interval, void *data) {
	events.push_back(new TimedEvent(callback, interval, data));
}

/**
 * Removes a timed event from the event queue.
 */
TimedEventMgr::List::iterator TimedEventMgr::remove(List::iterator i) {
	if (isLocked()) {
		deferredRemovals.push_back(*i);
		return i;
	} else {
		delete *i;
		return events.erase(i);
	}
}

void TimedEventMgr::remove(TimedEvent *event) {
	List::iterator i;
	for (i = events.begin(); i != events.end(); i++) {
		if ((*i) == event) {
			remove(i);
			break;
		}
	}
}

void TimedEventMgr::remove(TimedEvent::Callback callback, void *data) {
	List::iterator i;
	for (i = events.begin(); i != events.end(); i++) {
		if ((*i)->getCallback() == callback && (*i)->getData() == data) {
			remove(i);
			break;
		}
	}
}

/**
 * Runs each of the callback functions of the TimedEvents associated with this manager.
 */
void TimedEventMgr::tick() {
	List::iterator i;
	lock();

	for (i = events.begin(); i != events.end(); i++)
		(*i)->tick();

	unlock();

	// Remove events that have been deferred for removal
	for (i = deferredRemovals.begin(); i != deferredRemovals.end(); i++)
		events.remove(*i);
}

void TimedEventMgr::lock()      {
	locked = true;
}
void TimedEventMgr::unlock()    {
	locked = false;
}

void EventHandler::pushMouseAreaSet(MouseArea *mouseAreas) {
	_mouseAreaSets.push_front(mouseAreas);
}

void EventHandler::popMouseAreaSet() {
	if (_mouseAreaSets.size())
		_mouseAreaSets.pop_front();
}

/**
 * Get the currently active mouse area set off the top of the stack.
 */
MouseArea *EventHandler::getMouseAreaSet() const {
	if (_mouseAreaSets.size())
		return _mouseAreaSets.front();
	else
		return NULL;
}

/**
 * @param maxlen the maximum length of the Common::String
 * @param screenX the screen column where to begin input
 * @param screenY the screen row where to begin input
 * @param accepted_chars a Common::String characters to be accepted for input
 */
ReadStringController::ReadStringController(int maxlen, int screenX, int screenY, const Common::String &accepted_chars) {
	this->maxlen = maxlen;
	this->screenX = screenX;
	this->screenY = screenY;
	this->view = NULL;
	this->accepted = accepted_chars;
}

ReadStringController::ReadStringController(int maxlen, TextView *view, const Common::String &accepted_chars) {
	this->maxlen = maxlen;
	this->screenX = view->getCursorX();
	this->screenY = view->getCursorY();
	this->view = view;
	this->accepted = accepted_chars;
}

bool ReadStringController::keyPressed(int key) {
	int valid = true,
	    len = _value.size();
	size_t pos = Common::String::npos;

#ifdef TODO
	if (key < U4_ALT)
#endif
		pos = accepted.findFirstOf(key);

	if (pos != Common::String::npos) {
		if (key == Common::KEYCODE_BACKSPACE) {
			if (len > 0) {
				/* remove the last character */
				_value.erase(len - 1, 1);

				if (view) {
					view->textAt(screenX + len - 1, screenY, " ");
					view->setCursorPos(screenX + len - 1, screenY, true);
				} else {
					screenHideCursor();
					screenTextAt(screenX + len - 1, screenY, " ");
					screenSetCursorPos(screenX + len - 1, screenY);
					screenShowCursor();
				}
			}
		} else if (key == '\n' || key == '\r') {
			doneWaiting();
		} else if (len < maxlen) {
			/* add a character to the end */
			_value += key;

			if (view) {
				view->textAt(screenX + len, screenY, "%c", key);
			} else {
				screenHideCursor();
				screenTextAt(screenX + len, screenY, "%c", key);
				screenSetCursorPos(screenX + len + 1, screenY);
				g_context->col = len + 1;
				screenShowCursor();
			}
		}
	} else valid = false;

	return valid || KeyHandler::defaultHandler(key, NULL);
}

Common::String ReadStringController::get(int maxlen, int screenX, int screenY, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	ReadStringController ctrl(maxlen, screenX, screenY);
	eh->pushController(&ctrl);
	return ctrl.waitFor();
}

Common::String ReadStringController::get(int maxlen, TextView *view, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	ReadStringController ctrl(maxlen, view);
	eh->pushController(&ctrl);
	return ctrl.waitFor();
}

ReadIntController::ReadIntController(int maxlen, int screenX, int screenY) : ReadStringController(maxlen, screenX, screenY, "0123456789 \n\r\010") {}

int ReadIntController::get(int maxlen, int screenX, int screenY, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	ReadIntController ctrl(maxlen, screenX, screenY);
	eh->pushController(&ctrl);
	ctrl.waitFor();
	return ctrl.getInt();
}

int ReadIntController::getInt() const {
	return static_cast<int>(strtol(_value.c_str(), NULL, 10));
}

ReadChoiceController::ReadChoiceController(const Common::String &choices) {
	this->choices = choices;
}

bool ReadChoiceController::keyPressed(int key) {
	// Common::isUpper() accepts 1-byte characters, yet the modifier keys
	// (ALT, SHIFT, ETC) produce values beyond 255
	if ((key <= 0x7F) && (Common::isUpper(key)))
		key = tolower(key);

	_value = key;

	if (choices.empty() || choices.findFirstOf(_value) < choices.size()) {
		// If the value is printable, display it
		if (!Common::isSpace(key))
			screenMessage("%c", toupper(key));
		doneWaiting();
		return true;
	}

	return false;
}

char ReadChoiceController::get(const Common::String &choices, EventHandler *eh) {
	if (!eh)
		eh = eventHandler;

	ReadChoiceController ctrl(choices);
	eh->pushController(&ctrl);
	return ctrl.waitFor();
}

ReadDirController::ReadDirController() {
	_value = DIR_NONE;
}

bool ReadDirController::keyPressed(int key) {
	Direction d = keyToDirection(key);
	bool valid = (d != DIR_NONE);

	switch (key) {
	case Common::KEYCODE_ESCAPE:
	case Common::KEYCODE_SPACE:
	case Common::KEYCODE_RETURN:
		_value = DIR_NONE;
		doneWaiting();
		return true;

	default:
		if (valid) {
			_value = d;
			doneWaiting();
			return true;
		}
		break;
	}

	return false;
}

WaitController::WaitController(unsigned int c) : Controller(), cycles(c), current(0) {}

void WaitController::timerFired() {
	if (++current >= cycles) {
		current = 0;
		eventHandler->setControllerDone(true);
	}
}

bool WaitController::keyPressed(int key) {
	return true;
}

void WaitController::wait() {
	Controller_startWait();
}

void WaitController::setCycles(int c) {
	cycles = c;
}

} // End of namespace Ultima4
} // End of namespace Ultima
