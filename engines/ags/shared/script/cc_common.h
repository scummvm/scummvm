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

// Script options and error reporting.

#ifndef AGS_SHARED_SCRIPT_CC_COMMON_H
#define AGS_SHARED_SCRIPT_CC_COMMON_H

#include "ags/shared/util/string.h"

namespace AGS3 {

#define SCOPT_EXPORTALL      1   // export all functions automatically
#define SCOPT_SHOWWARNINGS   2   // printf warnings to console
#define SCOPT_LINENUMBERS    4   // include line numbers in compiled code
#define SCOPT_AUTOIMPORT     8   // when creating instance, export funcs to other scripts
#define SCOPT_DEBUGRUN    0x10   // write instructions as they are procssed to log file
#define SCOPT_NOIMPORTOVERRIDE 0x20 // do not allow an import to be re-declared
#define SCOPT_LEFTTORIGHT 0x40   // left-to-right operator precedance
#define SCOPT_OLDSTRINGS  0x80   // allow old-style strings
#define SCOPT_UTF8        0x100  // UTF-8 text mode

extern void ccSetOption(int, int);
extern int ccGetOption(int);

// error reporting

struct ScriptError {
	bool HasError = false; // set if error occurs
	bool IsUserError = false; // marks script use errors
	AGS::Shared::String ErrorString; // description of the error
	int Line = 0;  // line number of the error
	AGS::Shared::String CallStack; // callstack where error happened
};

void cc_clear_error();
bool cc_has_error();
const ScriptError &cc_get_error();
// Returns callstack of the last recorded script error, or a callstack
// of a current execution point, if no script error is currently saved in memory.
AGS::Shared::String cc_get_err_callstack(int max_lines = INT_MAX);
void cc_error(const char *, ...);
void cc_error(const ScriptError &err);
// Project-dependent script error formatting
AGS::Shared::String cc_format_error(const AGS::Shared::String &message);

} // namespace AGS3

#endif
