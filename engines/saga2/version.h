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
 * aint32 with this program; if not, write to the Free Software
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_VERSION_H
#define SAGA2_VERSION_H

namespace Saga2 {

#define VERSION_MAJOR "1"
#define VERSION_MINOR "00"
#define VERSION_LEVEL "02"   // 1=ALPHA 2=BETA 3=GREEN 4=GOLD/RC 5=RELEASE
#define VERSION_BUILD "28"

#ifdef __WATCOMC__
#define VERSION_OPSYS "DOS"
#else
#define VERSION_OPSYS "W95"
#endif

/* ===================================================================== *
    Target variables
 * ===================================================================== */

#define VERSION_STAMP VERSION_MAJOR "." VERSION_MINOR "." VERSION_LEVEL "." VERSION_BUILD "\0"

} // end of namespace Saga2

#endif
