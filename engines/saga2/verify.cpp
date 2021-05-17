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
#include "saga2/errclass.h"

namespace Saga2 {

/****** errclass.cpp/VERIFY *******************************
*
*   NAME
*       VERIFY -- GTools-compatible assertion macro
*
*   SYNOPSIS
*       VERIFY( condition );
*       verify( condition );
*
*   FUNCTION
*       Throws a gError exception if condition not met. It prints
*       a message indicating the condition that was not met and
*       the source file name and line number of the macro.
*
*   SEE ALSO
*       gError::gError
*       gError::warn
*       class gError
*
*******************************************************************/

void __verify(char *expr, char *file, long line, char *msg) {
	if (msg) {
		error("Error in %s, line %d: %s\n",
		             file,
		             line,
		             msg);
	} else {
		error("VERIFY failed in %s, line %d: (%s)\n",
		             file,
		             line,
		             expr);
	}
}

//
// This value is set if the program is in the debugger & the extensions
// have initialized properly
//

volatile signed short int DebugExtensionsEnabled = 0;

//
// When DebugBreakNow is set to 1 a breakpoint is triggered
//

volatile signed short int DebugBreakNow = 0;

//
// Override these calls in an OBJ file to disable /enable stuff
//

void PauseTimers(void) { }
void ResumeTimers(void) { }

//
// Force a breakpoint
//

void debug_breakpoint(const int, const char []) {
	PauseTimers();
	DebugBreakNow = 1;
}

//
// When DebugDumpNow is set, anything in DebugDumpText will get
// dumped to the debugger LOG screen
//

volatile signed short int DebugDumpNow = 0;
char DebugDumpText[80] = "Run Time Messages Enabled\0";

//
// Set up & trigger a run-time error message
//

void debug_dumptext(const char text[]) {
	int l;
	l = strlen(text);
	if (l > 78)
		l = 78;
	strncpy(DebugDumpText, text, l);
	DebugDumpText[l] = '\0';
	PauseTimers();
	DebugDumpNow = 1;
	ResumeTimers();
}


extern "C" {

	void cebug_breakpoint(const int linenumber, const char filename[]) {
		debug_breakpoint(linenumber, filename);
	}

	void cebug_dumptext(const char text[]) {
		debug_dumptext(text);
	}

};

} // end of namespace Saga2
