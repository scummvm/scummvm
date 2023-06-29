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

#include "m4/adv_r/adv_trigger.h"
#include "m4/core/errors.h"
#include "m4/globals.h"

namespace M4 {

int32 kernel_trigger_create(int32 trigger_num) {
	int32 new_trigger;

	if (trigger_num < 0)
		return (trigger_num);

	if (trigger_num > 0xffff) {		// if room changed, this is an invalid trigger 
		error_show(FL, 'BADT', "bad trigger. %ld > 0xffff", trigger_num);
	}

	new_trigger = trigger_num + (_G(game).room_id << 16) + (_G(kernel).trigger_mode << 28);

	return new_trigger;
}

bool kernel_trigger_dispatch(int32 trigger) {
	error("TODO: kernel_trigger_dispatch");
	return true;
}


} // End of namespace M4
