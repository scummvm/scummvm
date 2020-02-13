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

#ifndef ARCHETYPE_SYS_OBJECT
#define ARCHETYPE_SYS_OBJECT

#include "glk/archetype/string.h"
#include "glk/archetype/interpreter.h"

namespace Glk {
namespace Archetype {

extern void sys_object_init();

/**
 * Is the receiver of all "system calls" and the only object that receives
 * messages in the form of strings rather than message constants.
 *
 * Notes: Uses a global variable called sys_state to keep track of its state
 * between calls.
 */
extern void send_to_system(int transport, String &strmsg, ResultType &result, ContextType &context);

} // End of namespace Archetype
} // End of namespace Glk

#endif
