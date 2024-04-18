
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

#ifndef BAGEL_BOFLIB_LOG_H
#define BAGEL_BOFLIB_LOG_H

#include "common/str.h"
#include "common/stream.h"
#include "bagel/boflib/boffo.h"
#include "bagel/boflib/object.h"
#include "bagel/boflib/stdinc.h"

namespace Bagel {

/**
 * Builds a string like sprintf()
 * @return      Pointer to new (temporary) buffer.
 */
const char *BuildString(const char *pszFormat, ...);

#define LOG_FATAL 0x00010000 // show fatal errors
#define LOG_ERROR 0x00020000 // show non-fatal errors
#define LOG_WARN 0x00040000  // show warnings
#define LOG_INFO 0x00080000  // show information

#define LOG_ALL_ERRORS (LOG_FATAL | LOG_ERROR | LOG_WARN | LOG_INFO)

#define LOG_SOURCE 0x00100000 // include source file and line # info
#define LOG_WINDOW 0x00200000 // send log info to Debug Output Window
#define LOG_FILE 0x00400000   // send log info to output file

#define LOG_FUTURE9 0x00800000
#define LOG_FUTURE8 0x01000000
#define LOG_FUTURE7 0x02000000
#define LOG_FUTURE6 0x04000000
#define LOG_FUTURE5 0x08000000
#define LOG_FUTURE4 0x10000000
#define LOG_FUTURE3 0x20000000
#define LOG_FUTURE2 0x40000000
#define LOG_FUTURE1 0x80000000

extern void LogInfo(const char *msg);
extern void LogWarning(const char *msg);
extern void LogError(const char *msg);
extern void LogFatal(const char *msg);

} // namespace Bagel

#endif
