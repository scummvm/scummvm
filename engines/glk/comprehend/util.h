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

#ifndef GLK_COMPREHEND_UTIL_H
#define GLK_COMPREHEND_UTIL_H

#include "common/scummsys.h"

namespace Glk {
namespace Comprehend {

#define DEBUG_IMAGE_DRAW	(1 << 0)
#define DEBUG_GAME_STATE	(1 << 1)
#define DEBUG_FUNCTIONS		(1 << 2)
#define DEBUG_ALL		(~0U)

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#define fatal_error error

void __fatal_error(const char *func, unsigned line, const char *fmt, ...);
void fatal_strerror(int err, const char *fmt, ...);
void *xmalloc(size_t size);
char *xstrndup(const char *str, size_t size);

void debug_printf(unsigned flags, const char *fmt, ...);
void debug_enable(unsigned flags);
void debug_disable(unsigned flags);
bool debugging_enabled(void);

} // namespace Comprehend
} // namespace Glk

#endif
