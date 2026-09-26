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

#include "ripper/diagnostics/screen_presenter.h"

#include <cxxtest/TestSuite.h>

class RipperScreenPresenterTestSuite : public CxxTest::TestSuite {
public:
	void testMessagesAreQueuedForFiveSecondLifetimes() {
		Ripper::ScreenMessageQueue queue;
		queue.enqueue("Subtitles: ON");
		queue.enqueue("Subtitle autoscroll: OFF");
		TS_ASSERT_EQUALS(queue.pendingCount(), 2U);
		TS_ASSERT(queue.update(100));
		TS_ASSERT_EQUALS(queue.message(), "Subtitles: ON");
		TS_ASSERT_EQUALS(queue.pendingCount(), 1U);
		TS_ASSERT_EQUALS(queue.opacity(100), 0);
		TS_ASSERT_EQUALS(queue.opacity(350), 255);
		TS_ASSERT_EQUALS(queue.opacity(4850), 255);
		TS_ASSERT(queue.opacity(5099) < 255);

		TS_ASSERT(queue.update(5100));
		TS_ASSERT_EQUALS(queue.message(), "Subtitle autoscroll: OFF");
		TS_ASSERT_EQUALS(queue.pendingCount(), 0U);
		TS_ASSERT(queue.update(10100));
		TS_ASSERT(!queue.hasActiveMessage());
	}

	void testClearRemovesTheActiveMessageAndQueue() {
		Ripper::ScreenMessageQueue queue;
		queue.enqueue("Milestone 212: Eddie Photo [set]");
		queue.enqueue("Milestone 213: Magnotta Photo [set]");
		queue.update(200);

		queue.clear();
		TS_ASSERT(!queue.hasActiveMessage());
		TS_ASSERT_EQUALS(queue.pendingCount(), 0U);
		TS_ASSERT(queue.message().empty());
	}

	void testPauseDoesNotConsumeTheActiveMessageLifetime() {
		Ripper::ScreenMessageQueue queue;
		queue.enqueue("Milestone 44: Prologue Newsroom OPEN [set]");
		queue.update(100);
		queue.pause(1100);
		TS_ASSERT(!queue.update(11100));
		TS_ASSERT(queue.hasActiveMessage());
		queue.resume(11100);
		TS_ASSERT(!queue.update(15099));
		TS_ASSERT(queue.hasActiveMessage());
		TS_ASSERT(queue.update(15100));
		TS_ASSERT(!queue.hasActiveMessage());
	}
};
