/* Residual - Virtual machine to run LucasArts' 3D adventure games
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 * $URL$
 * $Id$
 *
 */

#include "common/sys.h"

#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

enum enDebugLevels {
	DEBUG_NONE,
	DEBUG_NORMAL,
	DEBUG_WARN,
	DEBUG_ERROR,
	DEBUG_LUA,
	DEBUG_BITMAPS,
	DEBUG_MODEL,
	DEBUG_STUB,
	DEBUG_SMUSH,
	DEBUG_IMUSE,
	DEBUG_CHORES,
	DEBUG_ALL
};

namespace Common {
	class String;
}

extern enDebugLevels gDebugLevel;

extern const char *debug_levels[];
extern const char *debug_descriptions[];

void error(const char *fmt, ...);

#ifdef DISABLE_TEXT_CONSOLE

inline int printf(const char *s, ...) { return 0; }

inline void warning(const char *s, ...) {}

#else

void warning(const char *fmt, ...);
void debug(int level, const char *s, ...);
void debug(const char *s, ...);

#endif

#endif
