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
#include "ultima/ultima4/core/error.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "common/debug.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

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
		g_ultima->quitGame();
		EventHandler::end();
		return true;
	default:
		return false;
	}
}

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

bool KeyHandler::ignoreKeys(int key, void *data) {
	return true;
}

bool KeyHandler::handle(int key) {
	bool processed = false;
	if (!isKeyIgnored(key)) {
		processed = globalHandler(key);
		if (!processed)
			processed = _handler(key, _data);
	}

	return processed;
}

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

EventHandler::EventHandler() : _timer(settings._eventTimerGranularity), _updateScreen(NULL),
		_lastTickTime(0) {
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
			g_screen->update();
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
	g_screen->update();
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

	debug(1, "key event: sym = %d, mod = %d; translated = %d",
		    event.kbd.keycode,  event.kbd.flags,  key);

	/* handle the keypress */
	processed = controller->notifyKeyPressed(key);

	if (processed) {
		if (updateScreen)
			(*updateScreen)();
		g_screen->update();
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

void EventHandler::sleep(unsigned int msec) {
	g_system->delayMillis(msec);
}

void EventHandler::run() {
	const int FRAME_TIME = 1000 / settings._gameCyclesPerSecond;

	if (_updateScreen)
		(*_updateScreen)();
	g_screen->update();

	while (!_ended && !_controllerDone) {
		uint32 time = g_system->getMillis();
		if (time >= (_lastTickTime + FRAME_TIME)) {
			_lastTickTime = time;
			eventHandler->getTimer()->tick();
			g_screen->update();
		}

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

		case Common::EVENT_QUIT:
			_ended = true;
			return;

		default:
			break;
		}
	}
}

void EventHandler::setScreenUpdate(void (*updateScreen)(void)) {
	this->_updateScreen = updateScreen;
}

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
		return NULL;

	KeyHandlerController *khc = dynamic_cast<KeyHandlerController *>(_controllers.back());
	ASSERT(khc != NULL, "EventHandler::getKeyHandler called when controller wasn't a keyhandler");
	if (khc == NULL)
		return NULL;

	return khc->getKeyHandler();
}

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
