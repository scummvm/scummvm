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

/* OS-layer functions and macros.
 *
 * This file does not introduce any curses (or other screen-API)
 * dependencies; it can be used for both the interpreter as well as the
 * compiler.
 */

/* Text IO Buffering
 */

#ifndef GLK_TADS_OS_BUFFER
#define GLK_TADS_OS_BUFFER

#include "common/scummsys.h"

namespace Glk {
namespace TADS {

extern void os_put_buffer(const char *buf, size_t len);

extern void os_get_buffer(char *buf, size_t len, size_t init);

extern char *os_fill_buffer(char *buf, size_t len);

} // End of namespace TADS
} // End of namespace Glk

#endif
