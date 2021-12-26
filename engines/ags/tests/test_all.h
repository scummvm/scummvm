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

#include "ags/shared/core/platform.h"

namespace AGS3 {

extern void Test_DoAllTests();

// Math tests
extern void Test_Math();

// File tests
extern void Test_File();
extern void Test_IniFile();

// Graphics tests
extern void Test_Gfx();

// Memory / bit-byte operations
extern void Test_Memory();

// String tests
extern void Test_ScriptSprintf();
extern void Test_String();
extern void Test_Path();
extern void Test_Version();

} // namespace AGS3
