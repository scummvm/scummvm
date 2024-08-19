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

#include "common/debug.h"

#include "qdengine/qdengine.h"
#include "qdengine/qd_fwd.h"
#include "qdengine/qdcore/util/ResourceDispatcher.h"


namespace QDEngine {

int ResourceUser::IDs;

void ResourceDispatcher::do_start() {
	if (_start_log) {
		_start_log = false;
		_syncro_timer.setTime(1);
		for (UserList::iterator i = users.begin(); i != users.end(); ++i)
			(*i)->time = _syncro_timer();
	}
}

void ResourceDispatcher::reset() {
	for (UserList::iterator i = users.begin(); i != users.end(); ++i)
		(*i)->time = _syncro_timer();
}

void ResourceDispatcher::quant() {
	debugC(9, kDebugQuant, "ResourceDispatcher::quant()");
	if (users.empty())
		return;

	do_start();

	_syncro_timer.next_frame();

	for (;;) {
		time_type t_min = users.front()->time;
		ResourceUser *user_min = users.front();
		for (UserList::iterator i = users.begin(); i != users.end(); ++i) {
			ResourceUser &u = **i;
			if (t_min > u.time) {
				t_min = u.time;
				user_min = &u;
			}
		}
		if (t_min < _syncro_timer()) {
			if (!user_min->quant()) {
				debugC(3, kDebugQuant, "ResourceDispatcher::quant() user_min->time = %d", user_min->time);
				detach(user_min);
			} else
				user_min->time += user_min->time_step();
		} else
			break;
	}
}
} // namespace QDEngine
