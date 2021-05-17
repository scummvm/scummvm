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
 * aint32 with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_VERSION_H
#define SAGA2_VERSION_H

namespace Saga2 {

#define VERSINT_MAJOR 1
#define VERSINT_MINOR 00
#define VERSINT_LEVEL 02   // 1=ALPHA 2=BETA 3=GREEN 4=GOLD/RC 5=RELEASE
#define VERSINT_BUILD 28
#define VERSION_MAJOR "1"
#define VERSION_MINOR "00"
#define VERSION_LEVEL "02"   // 1=ALPHA 2=BETA 3=GREEN 4=GOLD/RC 5=RELEASE
#define VERSION_BUILD "28"

#if DEBUG
#define VERSION_DEBUG " ("__DATE__"@"__TIME__")"
#define VERSION_DISTRIB "For internal distribution only"
#else
#define VERSION_DEBUG
#define VERSION_DISTRIB "General distribution"
#endif

#ifdef __WATCOMC__
#define VERSION_OPSYS "DOS"
#else
#define VERSION_OPSYS "W95"
#endif

/* ===================================================================== *
    Target variables
 * ===================================================================== */

#define VERSINT_SHOWN VERSINT_MAJOR,VERSINT_MINOR
#define VERSINT_STAMP VERSINT_MAJOR,VERSINT_MINOR,VERSINT_LEVEL,VERSINT_BUILD
#define VERSION_SHOWN VERSION_MAJOR "." VERSION_MINOR "\0"
#define VERSION_STAMP VERSION_MAJOR "." VERSION_MINOR "." VERSION_LEVEL "." VERSION_BUILD VERSION_DEBUG "\0"
#define VERSION_NOTES VERSION_DISTRIB

} // end of namespace Saga2

#endif
