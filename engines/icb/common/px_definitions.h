/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

namespace ICB {

#define BUILD_OUTPUT_EXTENSION "build"                // Extension for the file containing the build results
#define OBJECT_DEFINITION_EXTENSION "objdef"          // Extension for an object definition
#define COMPILED_OBJECT_EXTENSION "compobj"           // Extension for a compiled object
#define COMPILED_SCRIPT_EXTENSION "scrobj"            // Extension for a compiled script
#define COMBINED_OBJECT_EXTENSION "cbdobj"            // Extension for a combined object
#define COMPILED_FN_ROUTINES_EXTENSION "fn_dat"       // Extension for the compiled fn routine data
#define COMPILED_SPEECH_SCRIPT_EXTENSION "spchscrobj" // Compiled speech script

// Makefile macro names
#define NMAKE_LOCAL_ROOT_MACRO "$(LOCALROOT)"
#define NMAKE_COMMON_ROOT_MACRO "$(COMMONROOT)"

// Resource gauge definitions
#define RESOURCE_VARIABLE_SEARCH_PATTERN "Resource Value:"
#define RESOURCE_VARIABLE_SEARCH_PATTERN_LEN 15

// Converter program versions
#define PXVER_FN_ROUTINES 1    // fn_routines converter version.
#define PXVER_SCRIPTCOMPILER 1 // fn_routines converter version.

} // End of namespace ICB
