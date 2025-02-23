
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

#ifndef BAGEL_BOFLIB_EVENT_LOOP_H
#define BAGEL_BOFLIB_EVENT_LOOP_H

#include "graphics/framelimiter.h"

namespace Bagel {

class EventLoop {
public:
	enum Mode {
		NO_UPDATES = 0, FORCE_REPAINT = 1
	};
private:
	Graphics::FrameLimiter _limiter;
	Mode _mode;

public:
	EventLoop(Mode mode = NO_UPDATES);

	/**
	 * Processes pending events and does a frame output.
	 * @returns		True if Escape was pressed to abort loop
	 */
	bool frame();
};

} // namespace Bagel

#endif
