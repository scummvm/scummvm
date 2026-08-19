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

#ifndef SCUMM_INSANE_REBEL_TOUCH_H
#define SCUMM_INSANE_REBEL_TOUCH_H

#include "common/scummsys.h"

namespace Scumm {

// Double rather than single, because a single tap is the gameplay fire button.
class RebelTouchTapDetector {
public:
	RebelTouchTapDetector();

	bool addTap(int16 x, int16 y, uint32 now);
	void reset();

private:
	uint32 _lastTapTime;
	int16 _lastTapX;
	int16 _lastTapY;
};

} // End of namespace Scumm

#endif
