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

#include "saga2/std.h"

namespace Saga2 {

static bool         loopCheck = FALSE;

/****** exit.cpp/normalExit *******************************
*
*   NAME
*       normalExit -- exits from program
*
*   SYNOPSIS
*       normalExit();
*
*       void normalExit( void );
*
*   FUNCTION
*       This function exits the program. The DOS error state is
*       set to "No error". The "atexit" cleanup handler is called.
*       If, for some reason, the cleanup handler should call exit,
*       the call will be ignored.
*
*   INPUTS
*       none
*
*   SEE ALSO
*       failExit()
*
*******************************************************************/

void normalExit(void) {
	if (loopCheck == FALSE) {
		loopCheck = TRUE;
		exit(EXIT_SUCCESS);
	}
}


/****** exit.cpp/failExit *******************************
*
*   NAME
*       failExit -- exits from program
*
*   SYNOPSIS
*       failExit();
*
*       void failExit( void );
*
*   FUNCTION
*       This function exits the program. The DOS error state is
*       set to EXIT_FAILURE. The "atexit" cleanup handler is called.
*       If, for some reason, the cleanup handler should call exit,
*       the call will be ignored.
*
*   INPUTS
*       none
*
*   SEE ALSO
*       normalExit()
*
*******************************************************************/

void failExit(void) {
	if (loopCheck == FALSE) {
		loopCheck = TRUE;
		exit(EXIT_FAILURE);
	}
}


} // end of namespace Saga
