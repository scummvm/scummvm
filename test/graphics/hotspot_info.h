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

#include <cxxtest/TestSuite.h>
#include "graphics/hotspot_renderer.h"

class HotspotInfoTestSuite : public CxxTest::TestSuite {
public:
	void test_constructor_with_values() {
		Graphics::HotspotInfo info(Common::Point(10, 20), "Test");
		TS_ASSERT_EQUALS(info.position.x, 10);
		TS_ASSERT_EQUALS(info.position.y, 20);
		TS_ASSERT_EQUALS(info.name, "Test");
	}

	void test_default_constructor() {
		Graphics::HotspotInfo info;
		TS_ASSERT_EQUALS(info.position.x, 0);
		TS_ASSERT_EQUALS(info.position.y, 0);
		TS_ASSERT(info.name.empty());
	}

	void test_empty_name() {
		Graphics::HotspotInfo info(Common::Point(100, 200), "");
		TS_ASSERT_EQUALS(info.position.x, 100);
		TS_ASSERT_EQUALS(info.position.y, 200);
		TS_ASSERT(info.name.empty());
	}

	void test_negative_coordinates() {
		Graphics::HotspotInfo info(Common::Point(-5, -10), "Negative");
		TS_ASSERT_EQUALS(info.position.x, -5);
		TS_ASSERT_EQUALS(info.position.y, -10);
		TS_ASSERT_EQUALS(info.name, "Negative");
	}
};
