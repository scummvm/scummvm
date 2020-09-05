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

#include "ultima/ultima4/controllers/key_handler_controller.h"
#include "ultima/ultima4/core/utils.h"
#include "ultima/ultima4/events/event_handler.h"
#include "ultima/ultima4/game/context.h"
#include "ultima/ultima4/ultima4.h"

namespace Ultima {
namespace Ultima4 {


KeyHandler::KeyHandler(Callback func, void *d, bool asyncronous) :
	_handler(func),
	_async(asyncronous),
	_data(d) {
}

bool KeyHandler::globalHandler(int key) {
	if (key == Common::KEYCODE_F5) {
		(void)g_ultima->saveGameDialog();
	} else if (key == Common::KEYCODE_F7) {
		(void)g_ultima->loadGameDialog();
	}

	return false;
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
	case Common::KEYCODE_RSHIFT:
	case Common::KEYCODE_LSHIFT:
	case Common::KEYCODE_RCTRL:
	case Common::KEYCODE_LCTRL:
	case Common::KEYCODE_RALT:
	case Common::KEYCODE_LALT:
	case Common::KEYCODE_RMETA:
	case Common::KEYCODE_LMETA:
	case Common::KEYCODE_TAB:
		return true;
	default:
		return false;
	}
}

bool KeyHandler::operator==(Callback cb) const {
	return (_handler == cb) ? true : false;
}

/*-------------------------------------------------------------------*/

KeyHandlerController::KeyHandlerController(KeyHandler *handler) {
	this->_handler = handler;
}

KeyHandlerController::~KeyHandlerController() {
	delete _handler;
}

bool KeyHandlerController::keyPressed(int key) {
	assertMsg(_handler != nullptr, "key handler must be initialized");
	return _handler->handle(key);
}

KeyHandler *KeyHandlerController::getKeyHandler() {
	return _handler;
}

} // End of namespace Ultima4
} // End of namespace Ultima
