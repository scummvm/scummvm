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

#include "chewy/events.h"
#include "chewy/global.h"

namespace Chewy {

EventsManager *g_events;

EventsManager::EventsManager(Graphics::Screen *screen) :
		EventsBase(screen) {
	g_events = this;
	init_life_handler();
}

EventsManager::~EventsManager() {
	g_events = nullptr;
}

void EventsManager::init_life_handler() {
	life_anz = false;
	life_flag = false;
	life_handler = true;

	addTimer(life_line, (uint32)(1000 / 18.2));
}

void EventsManager::life_line() {
	if (!life_flag) {
		life_flag = true;
		if (life_y > 190) {
			life_y = 0;
			out->cls();
		}
		int x = strlen(life_str);
		out->printxy(life_x, life_y, 255, 0, scr_width, life_str);
		for (int i = 0; i < life_anz; i++)
			out->printchar('.', 255, 0, scr_width);
		++life_anz;
		if (life_anz > 50 - x) {
			life_anz = 0;

			out->box_fill(x * fvorx6x8, life_y, 319, life_y + 8, 0);

		}
		life_flag = false;
	}
}

} // namespace Chewy
