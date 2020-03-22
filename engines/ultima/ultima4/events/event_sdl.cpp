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

#include "ultima/ultima4/ultima4.h"
#include "ultima/ultima4/events/event.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/core/debug.h"
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/game/screen.h"
#include "ultima/ultima4/game/settings.h"
#include "common/debug.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

extern bool verbose, quit;
extern int eventTimerGranularity;

KeyHandler::KeyHandler(Callback func, void *d, bool asyncronous) :
	_handler(func),
	_async(asyncronous),
	_data(d) {
}

/**
 * Sets the key-repeat characteristics of the keyboard.
 */
int KeyHandler::setKeyRepeat(int delay, int interval) {
#ifdef TODO
	return SDL_EnableKeyRepeat(delay, interval);
#else
	return 0;
#endif
}

/**
 * Handles any and all keystrokes.
 * Generally used to exit the application, switch applications,
 * minimize, maximize, etc.
 */
bool KeyHandler::globalHandler(int key) {
	switch (key) {
#if defined(MACOSX)
	case U4_META + 'q': /* Cmd+q */
	case U4_META + 'x': /* Cmd+x */
#endif
	case U4_ALT + 'x': /* Alt+x */
#if defined(WIN32)
	case U4_ALT + U4_FKEY + 3:
#endif
		quit = true;
		EventHandler::end();
		return true;
	default:
		return false;
	}
}

/**
 * A default key handler that should be valid everywhere
 */
bool KeyHandler::defaultHandler(int key, void *data) {
	bool valid = true;

	switch (key) {
	case '`':
		if (g_context && g_context->_location)
			debug(1, "x = %d, y = %d, level = %d, tile = %d (%s)\n", g_context->_location->_coords.x, g_context->_location->_coords.y, g_context->_location->_coords.z, g_context->_location->_map->translateToRawTileIndex(*g_context->_location->_map->tileAt(g_context->_location->_coords, WITH_OBJECTS)), g_context->_location->_map->tileTypeAt(g_context->_location->_coords, WITH_OBJECTS)->getName().c_str());
		break;
	default:
		valid = false;
		break;
	}

	return valid;
}

/**
 * A key handler that ignores keypresses
 */
bool KeyHandler::ignoreKeys(int key, void *data) {
	return true;
}

/**
 * Handles a keypress.
 * First it makes sure the key combination is not ignored
 * by the current key handler. Then, it passes the keypress
 * through the global key handler. If the global handler
 * does not process the keystroke, then the key handler
 * handles it itself by calling its handler callback function.
 */
bool KeyHandler::handle(int key) {
	bool processed = false;
	if (!isKeyIgnored(key)) {
		processed = globalHandler(key);
		if (!processed)
			processed = _handler(key, _data);
	}

	return processed;
}

/**
 * Returns true if the key or key combination is always ignored by xu4
 */
bool KeyHandler::isKeyIgnored(int key) {
	switch (key) {
	case U4_RIGHT_SHIFT:
	case U4_LEFT_SHIFT:
	case U4_RIGHT_CTRL:
	case U4_LEFT_CTRL:
	case U4_RIGHT_ALT:
	case U4_LEFT_ALT:
	case U4_RIGHT_META:
	case U4_LEFT_META:
	case U4_TAB:
		return true;
	default:
		return false;
	}
}

bool KeyHandler::operator==(Callback cb) const {
	return (_handler == cb) ? true : false;
}

KeyHandlerController::KeyHandlerController(KeyHandler *handler) {
	this->_handler = handler;
}

KeyHandlerController::~KeyHandlerController() {
	delete _handler;
}

bool KeyHandlerController::keyPressed(int key) {
	ASSERT(_handler != NULL, "key handler must be initialized");
	return _handler->handle(key);
}

KeyHandler *KeyHandlerController::getKeyHandler() {
	return _handler;
}

/**
 * Constructs a timed event manager object.
 * Adds a timer callback to the SDL subsystem, which
 * will drive all of the timed events that this object
 * controls.
 */
TimedEventMgr::TimedEventMgr(int i) : _baseInterval(i) {
	/* start the SDL timer */
#ifdef TODO
	if (instances == 0) {
		if (u4_SDL_InitSubSystem(SDL_INIT_TIMER) < 0)
			errorFatal("unable to init SDL: %s", SDL_GetError());
	}

	id = static_cast<void *>(SDL_AddTimer(i, &TimedEventMgr::callback, this));
#endif
	_instances++;
}

/**
 * Destructs a timed event manager object.
 * It removes the callback timer and un-initializes the
 * SDL subsystem if there are no other active TimedEventMgr
 * objects.
 */
TimedEventMgr::~TimedEventMgr() {
#ifdef TODO
	SDL_RemoveTimer(static_cast<SDL_TimerID>(id));
	id = NULL;

	if (instances == 1)
		u4_SDL_QuitSubSystem(SDL_INIT_TIMER);
#endif
	if (_instances > 0)
		_instances--;
}

/**
 * Adds an SDL timer event to the message queue.
 */
unsigned int TimedEventMgr::callback(unsigned int interval, void *param) {
#ifdef TODO
	Common::Event event;

	event.type = SDL_USEREVENT;
	event.user.code = 0;
	event.user.data1 = param;
	event.user.data2 = NULL;
	SDL_PushEvent(&event);
#endif
	return interval;
}

/**
 * Re-initializes the timer manager to a new timer granularity
 */
void TimedEventMgr::reset(unsigned int interval) {
	_baseInterval = interval;
	stop();
	start();
}

void TimedEventMgr::stop() {
#ifdef TODO
	if (id) {
		SDL_RemoveTimer(static_cast<SDL_TimerID>(id));
		id = NULL;
	}
#endif
}

void TimedEventMgr::start() {
#ifdef TODO
	if (!id)
		id = static_cast<void *>(SDL_AddTimer(baseInterval, &TimedEventMgr::callback, this));
#endif
}

/**
 * Constructs an event handler object.
 */
EventHandler::EventHandler() : _timer(eventTimerGranularity), _updateScreen(NULL) {
}

static void handleMouseMotionEvent(const Common::Event &event) {
	if (!settings._mouseOptions.enabled)
		return;

	const MouseArea *area;
	area = eventHandler->mouseAreaForPoint(event.mouse.x, event.mouse.y);
	if (area)
		screenSetMouseCursor(area->_cursor);
	else
		screenSetMouseCursor(MC_DEFAULT);
}

static void handleActiveEvent(const Common::Event &event, updateScreenCallback updateScreen) {
#ifdef TODO
	if (event.active.state & SDL_APPACTIVE) {
		// application was previously iconified and is now being restored
		if (event.active.gain) {
			if (updateScreen)
				(*updateScreen)();
			screenRedrawScreen();
		}
	}
#endif
}

static void handleMouseButtonDownEvent(const Common::Event &event, Controller *controller, updateScreenCallback updateScreen) {
	int button = 0;
	if (event.type == Common::EVENT_RBUTTONDOWN)
		button = 1;
	else if (event.type == Common::EVENT_MBUTTONDOWN)
		button = 2;

	if (!settings._mouseOptions.enabled)
		return;

	if (button > 2)
		button = 0;
	const MouseArea *area = eventHandler->mouseAreaForPoint(event.mouse.x, event.mouse.y);
	if (!area || area->_command[button] == 0)
		return;
	controller->keyPressed(area->_command[button]);
	if (updateScreen)
		(*updateScreen)();
	screenRedrawScreen();
}

static void handleKeyDownEvent(const Common::Event &event, Controller *controller, updateScreenCallback updateScreen) {
	int processed;
	int key = event.kbd.keycode;

	if (event.kbd.flags & Common::KBD_ALT)
		key += U4_ALT;

	if (event.kbd.flags & Common::KBD_META)
		key += U4_META;

	if (event.kbd.keycode == Common::KEYCODE_UP)
		key = U4_UP;
	else if (event.kbd.keycode == Common::KEYCODE_DOWN)
		key = U4_DOWN;
	else if (event.kbd.keycode == Common::KEYCODE_LEFT)
		key = U4_LEFT;
	else if (event.kbd.keycode == Common::KEYCODE_RIGHT)
		key = U4_RIGHT;
	else if (event.kbd.keycode == Common::KEYCODE_BACKSPACE ||
	         event.kbd.keycode == Common::KEYCODE_DELETE)
		key = U4_BACKSPACE;

	if (verbose)
		debug(1, "key event: sym = %d, mod = %d; translated = %d",
		      event.kbd.keycode,  event.kbd.flags,  key);

	/* handle the keypress */
	processed = controller->notifyKeyPressed(key);

	if (processed) {
		if (updateScreen)
			(*updateScreen)();
		screenRedrawScreen();
	}

}

static uint32 sleepTimerCallback(uint32 interval, void *) {
#ifdef TODO
	Common::Event stopEvent;
	stopEvent.type = SDL_USEREVENT;
	stopEvent.user.code = 1;
	stopEvent.user.data1 = 0;
	stopEvent.user.data2 = 0;
	SDL_PushEvent(&stopEvent);
#endif
	return 0;
}

/**
 * Delays program execution for the specified number of milliseconds.
 * This doesn't actually stop events, but it stops the user from interacting
 * While some important event happens (e.g., getting hit by a cannon ball or a spell effect).
 */
void EventHandler::sleep(unsigned int msec) {
	g_system->delayMillis(msec);
}

void EventHandler::run() {
	if (_updateScreen)
		(*_updateScreen)();
	screenRedrawScreen();

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
			break;
#ifdef TODO
		case SDL_USEREVENT:
			eventHandler->getTimer()->tick();
			break;

		case SDL_ACTIVEEVENT:
			handleActiveEvent(event, updateScreen);
			break;
#endif
		case Common::EVENT_QUIT:
			return;

		default:
			break;
		}
	}

}

void EventHandler::setScreenUpdate(void (*updateScreen)(void)) {
	this->_updateScreen = updateScreen;
}

/**
 * Returns true if the queue is empty of events that match 'mask'.
 */
bool EventHandler::timerQueueEmpty() {
#ifdef TODO
	Common::Event event;

	if (SDL_PeepEvents(&event, 1, SDL_PEEKEVENT, SDL_EVENTMASK(SDL_USEREVENT)))
		return false;
	else
		return true;
#else
	return false;
#endif
}


/**
 * Adds a key handler to the stack.
 */
void EventHandler::pushKeyHandler(KeyHandler kh) {
	KeyHandler *new_kh = new KeyHandler(kh);
	KeyHandlerController *khc = new KeyHandlerController(new_kh);
	pushController(khc);
}

/**
 * Pops a key handler off the stack.
 * Returns a pointer to the resulting key handler after
 * the current handler is popped.
 */
void EventHandler::popKeyHandler() {
	if (_controllers.empty())
		return;

	popController();
}

/**
 * Returns a pointer to the current key handler.
 * Returns NULL if there is no key handler.
 */
KeyHandler *EventHandler::getKeyHandler() const {
	if (_controllers.empty())
		return NULL;

	KeyHandlerController *khc = dynamic_cast<KeyHandlerController *>(_controllers.back());
	ASSERT(khc != NULL, "EventHandler::getKeyHandler called when controller wasn't a keyhandler");
	if (khc == NULL)
		return NULL;

	return khc->getKeyHandler();
}

/**
 * Eliminates all key handlers and begins stack with new handler.
 * This pops all key handlers off the stack and adds
 * the key handler provided to the stack, making it the
 * only key handler left. Use this function only if you
 * are sure the key handlers in the stack are disposable.
 */
void EventHandler::setKeyHandler(KeyHandler kh) {
	while (popController() != NULL) {}
	pushKeyHandler(kh);
}

const MouseArea *EventHandler::mouseAreaForPoint(int x, int y) {
	int i;
	const MouseArea *areas = getMouseAreaSet();

	if (!areas)
		return NULL;

	for (i = 0; areas[i]._nPoints != 0; i++) {
		if (screenPointInMouseArea(x, y, &(areas[i]))) {
			return &(areas[i]);
		}
	}
	return NULL;
}

} // End of namespace Ultima4
} // End of namespace Ultima
