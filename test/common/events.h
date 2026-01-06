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
#include "common/events.h"

class EventsTestSuite : public CxxTest::TestSuite {
public:
	void test_engine_action_hotspot_toggle() {
		TS_ASSERT_EQUALS(Common::kEngineActionHotspotToggle, 1);
	}

	void test_custom_engine_action_event() {
		Common::Event event;
		event.type = Common::EVENT_CUSTOM_ENGINE_ACTION_START;
		event.customType = Common::kEngineActionHotspotToggle;

		TS_ASSERT_EQUALS(event.type, Common::EVENT_CUSTOM_ENGINE_ACTION_START);
		TS_ASSERT_EQUALS(event.customType, Common::kEngineActionHotspotToggle);
	}

	void test_custom_engine_action_end_event() {
		Common::Event event;
		event.type = Common::EVENT_CUSTOM_ENGINE_ACTION_END;
		event.customType = Common::kEngineActionHotspotToggle;

		TS_ASSERT_EQUALS(event.type, Common::EVENT_CUSTOM_ENGINE_ACTION_END);
		TS_ASSERT_EQUALS(event.customType, Common::kEngineActionHotspotToggle);
	}
};
