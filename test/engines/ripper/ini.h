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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/ini.h"

#include "common/formats/ini-file.h"
#include "common/memstream.h"

#include <cxxtest/TestSuite.h>

class RipperIniTestSuite : public CxxTest::TestSuite {
public:
	void testIniValuesStripCommentsAndValidateDecimalRange() {
		const char data[] =
			"[settings]\n"
			"valid=123 ; retail comment\n"
			"junk=12x\n"
			"negative=-1\n"
			"uint overflow=4294967296\n"
			"int overflow=2147483648\n"
			"empty= ; comment\n";
		Common::MemoryReadStream stream((const byte *)data, sizeof(data) - 1,
			DisposeAfterUse::NO);
		Common::INIFile ini;
		TS_ASSERT(ini.loadFromStream(stream));

		TS_ASSERT_EQUALS(Ripper::getIniString(ini, "settings", "valid"), "123");
		TS_ASSERT_EQUALS(Ripper::getIniString(
			ini, "settings", "empty", "fallback"), "fallback");
		TS_ASSERT_EQUALS(Ripper::getIniInt(ini, "settings", "valid", 7), 123);
		TS_ASSERT_EQUALS(Ripper::getIniInt(ini, "settings", "junk", 7), 7);
		TS_ASSERT_EQUALS(Ripper::getIniInt(ini, "settings", "negative", 7), 7);
		TS_ASSERT_EQUALS(Ripper::getIniInt(ini, "settings", "int overflow", 7), 7);

		uint value = 9;
		TS_ASSERT(Ripper::readIniUint(ini, "settings", "valid", value));
		TS_ASSERT_EQUALS(value, 123U);
		TS_ASSERT(!Ripper::readIniUint(ini, "settings", "uint overflow", value));
		TS_ASSERT_EQUALS(value, 123U);
	}
};
