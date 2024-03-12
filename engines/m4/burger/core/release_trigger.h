
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

#ifndef M4_BURGER_CORE_RELEASE_TRIGGER_H
#define M4_BURGER_CORE_RELEASE_TRIGGER_H

#include "m4/m4_types.h"

namespace M4 {
namespace Burger {

struct ReleaseTrigger_Globals {
	uint32 _released_digi_trigger = 0;
	uint32 _check_channels = 0, _match_value = 0;
};

void release_trigger_on_digi_state(int32 trigger, int32 checkChannels, int32 matchValue = 0);
void release_trigger_digi_check();

} // namespace Burger
} // namespace M4

#endif
