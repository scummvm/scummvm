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

#include "common/events.h"
#include "common/system.h"
#include "harold/bcb/bcb.h"
#include "harold/bcb/script.h"
#include "video/avi_decoder.h"

namespace BCB {

static const StaticRect kMainSceneMciControlHitRect = {16, 48, 416, 248};
static const StaticRect kTitleMciControlHitRect = {120, 100, 520, 300};
static const StaticRect kMovieQueueControlHitRect = {341, 414, 407, 452};
static const StaticRect kSystemMenuControlHitRect = {265, 414, 330, 452};
static const uint32 kCursorRefreshMillis = 10;
static const int kMovieControlStripNormalSourceX = 0;
static const int kMovieControlStripSkipPressedSourceX = 0x98;
static const int kMovieControlStripPausePressedSourceX = 0x130;

void BCBEngine::handleLeftButtonDown(const Common::Point &mousePosition) {
}

bool BCBEngine::pollEvents() {
	return pollEvents(true);
}

bool BCBEngine::pollEvents(bool handleClicks) {
	Common::Event event;

	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_QUIT:
		case Common::EVENT_RETURN_TO_LAUNCHER:
			quitGame();
			return false;
		case Common::EVENT_MOUSEMOVE:
			_mousePosition = event.mouse;
			break;
		case Common::EVENT_LBUTTONDOWN:
			_mousePosition = event.mouse;
			if (handleClicks && !_suppressClickHandling)
				handleLeftButtonDown(_mousePosition);
			break;
		default:
			break;
		}
	}

	return !shouldQuit();
}

void BCBEngine::serviceCursorRefresh(bool screenUpdated) {
	const uint32 now = g_system->getMillis();

	if (screenUpdated) {
		_lastCursorRefreshMillis = now;
		return;
	}

	if (now - _lastCursorRefreshMillis >= kCursorRefreshMillis) {
		_lastCursorRefreshMillis = now;
		g_system->updateScreen();
	}
}

void BCBEngine::delayWithEventPolling(uint32 millis) {
	delayWithEventPolling(millis, true);
}

void BCBEngine::delayWithEventPolling(uint32 millis, bool handleClicks) {
	const uint32 start = g_system->getMillis();

	while (!shouldQuit() && g_system->getMillis() - start < millis) {
		pollEvents(handleClicks);
		serviceCursorRefresh();

		const uint32 elapsed = g_system->getMillis() - start;
		if (elapsed >= millis)
			break;

		g_system->delayMillis(MIN<uint32>(10, millis - elapsed));
	}
}

} // End of namespace BCB
