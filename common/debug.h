/* Residual - Virtual machine to run LucasArts' 3D adventure games
 * Copyright (C) 2003-2006 The ScummVM-Residual Team (www.scummvm.org)
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

#include "bits.h"

#ifndef DEBUG_H
#define DEBUG_H

enum enDebugLevels {
	DEBUG_NONE,
	DEBUG_NORMAL,
	DEBUG_WARN,
	DEBUG_ERROR,
	DEBUG_FUNC,
	DEBUG_BITMAPS,
	DEBUG_MODEL,
	DEBUG_STUB,
	DEBUG_SMUSH,
	DEBUG_IMUSE,
	DEBUG_CHORES,
	DEBUG_ALL
};
extern enDebugLevels debugLevel;
extern const char *debug_levels[];
extern const char *debug_descriptions[];

// Hacky toggles for experimental / debug code (defined/set in main.cpp)
extern bool ZBUFFER_GLOBAL, SHOWFPS_GLOBAL;

void warning(const char *fmt, ...);
void error(const char *fmt, ...);

const char *tag2str(uint32 tag);
void hexdump(const byte * data, int len, int bytesPerLine);

#endif
