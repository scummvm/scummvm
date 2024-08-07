/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/textconsole.h"

#include "qdengine/system/input/mouse_input.h"
#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

mouseDispatcher::mouseDispatcher() : _events(0), _active_events(0), _mouse_x(0), _mouse_y(0), _button_status(0) {
	for (int i = 0; i < EV_MOUSE_MOVE + 1; i++)
		_event_handlers[i] = 0;
}

mouseDispatcher::~mouseDispatcher() {
}

mouseDispatcher *mouseDispatcher::instance() {
	static mouseDispatcher dsp;
	return &dsp;
}

bool mouseDispatcher::handle_event(mouseEvent ev, int x, int y, int flags) {
	if (x >= grDispatcher::instance()->get_SizeX())
		x = grDispatcher::instance()->get_SizeX() - 1;
	if (y >= grDispatcher::instance()->get_SizeY())
		y = grDispatcher::instance()->get_SizeY() - 1;

	if (_event_handlers[ev])
		(*_event_handlers[ev])(x, y, flags);

	switch (ev) {
	case EV_LEFT_DOWN:
		_button_status |= 1 << (ID_BUTTON_LEFT);
		break;
	case EV_LEFT_UP:
		_button_status  &= ~(1 << ID_BUTTON_LEFT);
		break;
	case EV_RIGHT_DOWN:
		_button_status |= 1 << (ID_BUTTON_RIGHT);
		break;
	case EV_RIGHT_UP:
		_button_status  &= ~(1 << ID_BUTTON_RIGHT);
		break;
	default:
		break;
	}

	toggle_event(ev);
	_mouse_x = x;
	_mouse_y = y;

	return true;
}

} // namespace QDEngine
