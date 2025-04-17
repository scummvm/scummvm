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

#ifndef AWE_UTIL_H
#define AWE_UTIL_H

#include "common/textconsole.h"
#include "awe/intern.h"

namespace Awe {

enum {
	DBG_SCRIPT = 1 << 0,
	DBG_BANK = 1 << 1,
	DBG_VIDEO = 1 << 2,
	DBG_SND = 1 << 3,
	DBG_SER = 1 << 4,
	DBG_INFO = 1 << 5,
	DBG_PAK = 1 << 6,
	DBG_RESOURCE = 1 << 7,
};

extern uint16_t g_debugMask;

extern void string_lower(char *p);
extern void string_upper(char *p);

} // namespace Awe

#endif
