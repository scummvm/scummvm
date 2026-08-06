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

#include "ripper/diagnostics/milestone_overlay.h"

#include <cxxtest/TestSuite.h>

class RipperMilestoneOverlayTestSuite : public CxxTest::TestSuite {
public:
	void testNotificationsAreQueuedForFiveSecondLifetimes() {
		Ripper::MilestoneOverlayQueue queue;
		queue.enqueue(6, "Burton", true);
		queue.update(100);
		TS_ASSERT(!queue.hasActiveNotification());

		queue.setEnabled(true);
		queue.enqueue(6, "Burton", true);
		queue.enqueue(84, "Audio Editor", false);
		TS_ASSERT_EQUALS(queue.pendingCount(), 2U);
		TS_ASSERT(queue.update(100));
		TS_ASSERT_EQUALS(queue.message(), "Milestone 6: Burton [set]");
		TS_ASSERT_EQUALS(queue.pendingCount(), 1U);
		TS_ASSERT_EQUALS(queue.opacity(100), 0);
		TS_ASSERT_EQUALS(queue.opacity(350), 255);
		TS_ASSERT_EQUALS(queue.opacity(4850), 255);
		TS_ASSERT(queue.opacity(5099) < 255);

		TS_ASSERT(queue.update(5100));
		TS_ASSERT_EQUALS(queue.message(), "Milestone 84: Audio Editor [unset]");
		TS_ASSERT_EQUALS(queue.pendingCount(), 0U);
		TS_ASSERT(queue.update(10100));
		TS_ASSERT(!queue.hasActiveNotification());
	}

	void testDisablingClearsTheActiveNotificationAndQueue() {
		Ripper::MilestoneOverlayQueue queue;
		queue.setEnabled(true);
		queue.enqueue(212, "Eddie Photo", true);
		queue.enqueue(213, "Magnotta Photo", true);
		queue.update(200);

		queue.setEnabled(false);
		TS_ASSERT(!queue.isEnabled());
		TS_ASSERT(!queue.hasActiveNotification());
		TS_ASSERT_EQUALS(queue.pendingCount(), 0U);
		TS_ASSERT(queue.message().empty());
	}
};
