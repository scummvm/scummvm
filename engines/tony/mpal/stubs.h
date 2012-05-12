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
 *
 */

/****************************************************************************\
*       This file contains stubs and mappings for things used by the MPAL
*		library that are handled differently under ScummVM
\****************************************************************************/

#ifndef MPAL_STUBS
#define MPAL_STUBS

#include "common/scummsys.h"
#include "common/algorithm.h"
#include "common/keyboard.h"
#include "tony/mpal/memory.h"

namespace Tony {

namespace MPAL {

/****************************************************************************\
*       Types
\****************************************************************************/

typedef uint32 (*LPTHREAD_START_ROUTINE)(void *lpThreadParameter);
typedef void (*LPTHREAD_ROUTINE)(void *lpThreadParameter);

/****************************************************************************\
*       Defines
\****************************************************************************/

#define MB_OK 1

/****************************************************************************\
*       Methods
\****************************************************************************/

extern void CopyMemory(void *dst, const void *first, int size);

extern uint16 GetAsyncKeyState(Common::KeyCode kc);

} // end of namespace MPAL

} // end of namespace Tony

#endif
