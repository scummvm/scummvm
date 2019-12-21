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

#include "ultima/ultima6/core/events.h"
#include "common/algorithm.h"
#include "common/system.h"
#include "engines/engine.h"

namespace Ultima {
namespace Ultima6 {

Events::Events() : _buttonsDown(0) {
	g_events = this;
}

Events::~Events() {
	g_events = nullptr;
}

void Events::setButtonDown(MouseButton button, bool isDown) {
	assert(button != BUTTON_NONE);
	if (isDown)
		_buttonsDown |= 1 << ((int)button - 1);
	else
		_buttonsDown &= ~(1 << ((int)button - 1));
}


bool Events::pollEvent(Common::Event &event) {
	if (g_system->getEventManager()->pollEvent(event)) {
		if (isMouseDownEvent(event.type))
			setButtonDown(whichButton(event.type), true);
		else if (isMouseUpEvent(event.type))
			setButtonDown(whichButton(event.type), false);

		return true;
	}

	return false;
}

bool isMouseDownEvent(Common::EventType type) {
	return type == Common::EVENT_LBUTTONDOWN || type == Common::EVENT_RBUTTONDOWN
		|| type == Common::EVENT_MBUTTONDOWN;
}

bool isMouseUpEvent(Common::EventType type) {
	return type == Common::EVENT_LBUTTONUP || type == Common::EVENT_RBUTTONUP
		|| type == Common::EVENT_MBUTTONUP;
}

MouseButton whichButton(Common::EventType type) {
	if (type == Common::EVENT_LBUTTONDOWN || type == Common::EVENT_LBUTTONUP)
		return BUTTON_LEFT;
	else if (type == Common::EVENT_RBUTTONDOWN || type == Common::EVENT_RBUTTONUP)
		return BUTTON_RIGHT;
	else if (type == Common::EVENT_MBUTTONDOWN || type == Common::EVENT_MBUTTONUP)
		return BUTTON_MIDDLE;
	else
		return BUTTON_NONE;
}

bool shouldQuit() {
	return g_engine->shouldQuit();
}

} // End of namespace Ultima6
} // End of namespace Ultima
