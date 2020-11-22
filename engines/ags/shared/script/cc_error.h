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
 */

//=============================================================================
//
// 'C'-style script compiler
//
//=============================================================================

#ifndef AGS_SHARED_SCRIPT_ERROR_H
#define AGS_SHARED_SCRIPT_ERROR_H

#include "ags/shared/util/string.h"

namespace AGS3 {

extern void cc_error(const char *, ...);

// error reporting
extern int ccError;             // set to non-zero if error occurs
extern int ccErrorLine;         // line number of the error
extern AGS::Shared::String ccErrorString; // description of the error
extern AGS::Shared::String ccErrorCallStack; // callstack where error happened
extern bool ccErrorIsUserError;
extern const char *ccCurScriptName; // name of currently compiling script

} // namespace AGS3

#endif
