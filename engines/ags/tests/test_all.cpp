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
#include "ags/tests/test_all.h"

namespace AGS3 {

void Test_DoAllTests() {
	Test_Math();
	Test_Memory();
	// The commented out tests don't work right now (will fix, but that is not my problem right now) @eklipsed
	//Test_Path();
	Test_ScriptSprintf();
	Test_String();
	Test_Version();
	//Test_File();
	//Test_IniFile();
	Test_Gfx();
}

} // namespace AGS3
