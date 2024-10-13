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

#include "m4/riddle/rooms/section.h"
#include "m4/gui/gui_vmng.h"
#include "m4/riddle/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Section::global_room_init() {
	digi_preload("950_s51", 950);
}

void Section::tick() {
	int oldTrigger = _G(kernel).trigger;
	_G(kernel).trigger = kCALLED_EACH_LOOP;
	g_engine->game_daemon_code();
	_G(kernel).trigger = oldTrigger;
}

void Section::daemon() {
	_G(kernel).continue_handling_trigger = true;
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
