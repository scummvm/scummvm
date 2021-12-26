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
#include "ags/shared/debugging/assert.h"
#include "ags/shared/util/version.h"

namespace AGS3 {

using AGS::Shared::Version;

void Test_Version() {
	// Old version format
	Version test_ver1 = Version("2.06.101");
	Version test_ver2 = Version("2.56.654");
	Version test_ver3 = Version("2.7.722");
	Version test_ver4 = Version("2.72.872NMP");
	Version test_ver5 = Version("3.2.0");
	Version test_ver6 = Version("3.21.1115");
	Version test_ver7 = Version("3.21.1115NMP");
	// New version format
	Version test_ver8 = Version("3.3.0.1130");
	Version test_ver9 = Version("3.3.0.1130 BETA");

	assert(test_ver1.Major == 2);
	assert(test_ver1.Minor == 0);
	assert(test_ver1.Release == 6);
	assert(test_ver1.Revision == 101);
	assert(strcmp(test_ver1.LongString.GetCStr(), "2.0.6.101") == 0);
	assert(strcmp(test_ver1.BackwardCompatibleString.GetCStr(), "2.06.101") == 0);
	assert(strcmp(test_ver1.ShortString.GetCStr(), "2.0") == 0);

	assert(test_ver2.Major == 2);
	assert(test_ver2.Minor == 5);
	assert(test_ver2.Release == 6);
	assert(test_ver2.Revision == 654);
	assert(strcmp(test_ver2.LongString.GetCStr(), "2.5.6.654") == 0);
	assert(strcmp(test_ver2.BackwardCompatibleString.GetCStr(), "2.56.654") == 0);
	assert(strcmp(test_ver2.ShortString.GetCStr(), "2.5") == 0);

	assert(test_ver3.Major == 2);
	assert(test_ver3.Minor == 7);
	assert(test_ver3.Release == 0);
	assert(test_ver3.Revision == 722);
	assert(strcmp(test_ver3.LongString.GetCStr(), "2.7.0.722") == 0);
	assert(strcmp(test_ver3.BackwardCompatibleString.GetCStr(), "2.70.722") == 0);
	assert(strcmp(test_ver3.ShortString.GetCStr(), "2.7") == 0);

	assert(test_ver4.Major == 2);
	assert(test_ver4.Minor == 7);
	assert(test_ver4.Release == 2);
	assert(test_ver4.Revision == 872);
	assert(strcmp(test_ver4.LongString.GetCStr(), "2.7.2.872 NMP") == 0);
	assert(strcmp(test_ver4.BackwardCompatibleString.GetCStr(), "2.72.872NMP") == 0);
	assert(strcmp(test_ver4.ShortString.GetCStr(), "2.7") == 0);

	assert(test_ver5.Major == 3);
	assert(test_ver5.Minor == 2);
	assert(test_ver5.Release == 0);
	assert(test_ver5.Revision == 0);
	assert(strcmp(test_ver5.LongString.GetCStr(), "3.2.0.0") == 0);
	assert(strcmp(test_ver5.BackwardCompatibleString.GetCStr(), "3.20.0") == 0);
	assert(strcmp(test_ver5.ShortString.GetCStr(), "3.2") == 0);

	assert(test_ver6.Major == 3);
	assert(test_ver6.Minor == 2);
	assert(test_ver6.Release == 1);
	assert(test_ver6.Revision == 1115);
	assert(strcmp(test_ver6.LongString.GetCStr(), "3.2.1.1115") == 0);
	assert(strcmp(test_ver6.BackwardCompatibleString.GetCStr(), "3.21.1115") == 0);
	assert(strcmp(test_ver6.ShortString.GetCStr(), "3.2") == 0);

	assert(test_ver7.Major == 3);
	assert(test_ver7.Minor == 2);
	assert(test_ver7.Release == 1);
	assert(test_ver7.Revision == 1115);
	assert(strcmp(test_ver7.LongString.GetCStr(), "3.2.1.1115 NMP") == 0);
	assert(strcmp(test_ver7.BackwardCompatibleString.GetCStr(), "3.21.1115NMP") == 0);
	assert(strcmp(test_ver7.ShortString.GetCStr(), "3.2") == 0);

	assert(test_ver8.Major == 3);
	assert(test_ver8.Minor == 3);
	assert(test_ver8.Release == 0);
	assert(test_ver8.Revision == 1130);
	assert(strcmp(test_ver8.LongString.GetCStr(), "3.3.0.1130") == 0);
	assert(strcmp(test_ver8.BackwardCompatibleString.GetCStr(), "3.30.1130") == 0);
	assert(strcmp(test_ver8.ShortString.GetCStr(), "3.3") == 0);

	assert(test_ver9.Major == 3);
	assert(test_ver9.Minor == 3);
	assert(test_ver9.Release == 0);
	assert(test_ver9.Revision == 1130);
	assert(strcmp(test_ver9.LongString.GetCStr(), "3.3.0.1130 BETA") == 0);
	assert(strcmp(test_ver9.BackwardCompatibleString.GetCStr(), "3.30.1130BETA") == 0);
	assert(strcmp(test_ver9.ShortString.GetCStr(), "3.3") == 0);
}

} // namespace AGS3
