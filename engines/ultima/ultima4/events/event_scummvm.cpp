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
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/gfx/screen.h"
#include "ultima/ultima4/core/settings.h"
#include "ultima/ultima4/meta_engine.h"
#include "common/debug.h"
#include "common/system.h"

namespace Ultima {
namespace Ultima4 {

EventHandler::EventHandler() : _timer(settings._eventTimerGranularity), _updateScreen(nullptr)  {
}

static void handleMouseMotionEvent(const Common::Event &event) {
	if (!settings._mouseOptions._enabled)
		return;

	const MouseArea *area;
	area = eventHandler->mouseAreaForPoint(event.mouse.x, event.mouse.y);
	if (area)
		g_screen->setMouseCursor(area->_cursor);
	else
		g_screen->setMouseCursor(MC_DEFAULT);
}

static void handleMouseButtonDownEvent(const Common::Event &event, Controller *controller, updateScreenCallback updateScreen) {
	int button = 0;
	if (event.type == Common::EVENT_MBUTTONDOWN)
		button = 1;
	else if (event.type == Common::EVENT_RBUTTONDOWN)
		button = 2;

	if (!settings._mouseOptions._enabled)
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
	int key;
	bool processed;

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
	else {
		key = event.kbd.ascii;
		if (!key)
			return;

		if (event.kbd.flags & Common::KBD_ALT)
			key += U4_ALT;

		if (event.kbd.flags & Common::KBD_META)
			key += U4_META;
	}

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

void EventHandler::sleep(uint msec) {
	g_system->delayMillis(msec);
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
	this->_updateScreen = updateScreen;
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
	while (popController() != nullptr) {}
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

} // End of namespace Ultima4
} // End of namespace Ultima
