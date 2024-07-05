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

#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "common/textconsole.h"
#include "qdengine/qd_precomp.h"
#include "qdengine/system/input/mouse_input.h"
#include "qdengine/system/graphics/gr_dispatcher.h"


namespace QDEngine {

mouseDispatcher::mouseDispatcher() : events_(0), active_events_(0), mouse_x_(0), mouse_y_(0), button_status_(0) {
	for (int i = 0; i < EV_MOUSE_MOVE + 1; i++)
		event_handlers_[i] = 0;
}

mouseDispatcher::~mouseDispatcher() {
}

mouseDispatcher *mouseDispatcher::instance() {
	static mouseDispatcher dsp;
	return &dsp;
}

bool mouseDispatcher::handle_event(mouseEvent ev, int x, int y, int flags) {
	if (x >= grDispatcher::instance()->Get_SizeX())
		x = grDispatcher::instance()->Get_SizeX() - 1;
	if (y >= grDispatcher::instance()->Get_SizeY())
		y = grDispatcher::instance()->Get_SizeY() - 1;

	if (event_handlers_[ev])
		(*event_handlers_[ev])(x, y, flags);

	switch (ev) {
	case EV_LEFT_DOWN:
		button_status_ |= 1 << (ID_BUTTON_LEFT);
		break;
	case EV_LEFT_UP:
		button_status_  &= ~(1 << ID_BUTTON_LEFT);
		break;
	case EV_RIGHT_DOWN:
		button_status_ |= 1 << (ID_BUTTON_RIGHT);
		break;
	case EV_RIGHT_UP:
		button_status_  &= ~(1 << ID_BUTTON_RIGHT);
		break;
	default:
		break;
	}

	toggle_event(ev);
	mouse_x_ = x;
	mouse_y_ = y;

	return true;
}

} // namespace QDEngine
