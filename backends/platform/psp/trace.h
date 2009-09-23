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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */


#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>
#include <psptypes.h>
#include <pspkernel.h>
#include <stdarg.h>
#include <pspdebug.h>

// Use these defines for debugging

//#define __PSP_DEBUG__
//#define __PSP_DEBUG_SUSPEND__

void PSPDebugTrace (const char *filename, const char *format, ...);
void PSPDebugTrace (const char *format, ...);

#ifdef __PSP_DEBUG_SUSPEND__
#define PSPDebugSuspend(format,...)		PSPDebugTrace(format, ## __VA_ARGS__)
#else
#define PSPDegbugSuspend(x)
#define PSPDebugSuspend(format,...)
#endif /* __PSP_DEBUG_SUSPEND__ */


#endif // TRACE_H

