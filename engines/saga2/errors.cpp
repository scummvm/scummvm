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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#include "saga2/std.h"
#include "saga2/errlist.h"
#include "saga2/errtype.h"

namespace Saga2 {

// ------------------------------------------------------------------
// FTA2 error text

const char *FTA2ProgramErrors[] = {
	"No Error",
	"Unknown command line argument",


	"Minimum CPU not found",
	"Get a real machine",
	"Get a real machine",
	"Not in a DOS Box",
	"This program requires 8 Megabytes of RAM to run properly",
	"You do not have sufficient memory available",
	"This program requires more virtual memory to run properly",
	"You do not have sufficient virtual memory available",
	"No DPMI support",   // Minimum memory


	"You should run SETSOUND before running this program for the first time",
	"You should run SETSOUND and set up a digital sound driver",
	"You should run SETSOUND and set up a music driver",
	"Unaccelerated video card detected",
	"Get a real machine",
	"This game requires a mouse. No mouse driver was detected.",


	"Unable to allocate fault handler",
	"Direct X does not recognize your display. You may have inappropriate drivers.",
	"Could not initialize the audio",
	"Could not initialize the game clock",


	"A severe internal error has occurred",


	"A program file cannot be opened",
	"A necessary file can't be found. You may need to insert the game CD",
	"The game CD is required to play a video.",
	"Sound driver not detected. Continue anyway?",
	"Music driver not detected. Continue anyway?",

	"A program file is in use and cannot be opened",
	"A program file may be corrupt or wrong size",
	"A program file could not be read",
	"A program file could not be written",
	"A program file could not be closed",

	"A saved game file could not be read",
	"A saved game file could not be written",


	"The display is in use by another program",


	"An internal program error has been detected",


	"Are you sure you want to exit",
	"Direct X does not recognize your display. You may have inappropriate drivers. Emulation will allow you to continue, but perfomance will be severly degraded. Do you want to continue?",
	"Game heap overflowed continue?",

	""
};

SequentialErrorList programErrors(etiFTA2ErrorList, FTA2ProgramErrors);

}
