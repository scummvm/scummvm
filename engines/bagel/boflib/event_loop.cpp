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
#include "bagel/boflib/event_loop.h"
#include "bagel/bagel.h"

namespace Bagel {

EventLoop::EventLoop(Mode mode) : _limiter(g_system, 60, false),
	_mode(mode) {
}

bool EventLoop::frame() {
	Common::Event e;

	// Handle pending events
	while (g_system->getEventManager()->pollEvent(e)) {
		if (g_engine->shouldQuit() || (e.type == Common::EVENT_LBUTTONDOWN) ||
			(e.type == Common::EVENT_KEYDOWN && e.kbd.keycode == Common::KEYCODE_ESCAPE))
			return true;
	}

	_limiter.delayBeforeSwap();

	// Update the screen
	if (_mode == FORCE_REPAINT)
		CBagMasterWin::forcePaintScreen();
	g_engine->_screen->update();

	_limiter.startFrame();

	return false;
}

} // namespace Bagel
