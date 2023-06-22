
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

#ifndef M4_PLATFORM_TIMER_H
#define M4_PLATFORM_TIMER_H

#include "m4/m4_types.h"

namespace M4 {

struct Timer_Globals {
	uint32 _timer_600_low = 0;
	uint32 _timer_60_low = 0;
	uint32 _timer_dos_low = 0;

	bool _timer_installed = false;
	uint16 _timer_600_handle = 0;
	uint16 _timer_60_handle = 0;
	uint16 _timer_dos_handle = 0;
};

extern uint32 timer_read();
extern uint32 timer_read_dos();
extern uint32 timer_read_600();
extern uint32 timer_read_60();
extern void timer_600_proc();

} // End of namespace M4

#endif
