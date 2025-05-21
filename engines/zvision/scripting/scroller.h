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

#ifndef ZVISION_SCROLLER_H
#define ZVISION_SCROLLER_H

#include "common/rect.h"
#include "zvision/zvision.h"

namespace ZVision {

class Scroller {
public:
  Scroller(const Common::Point &activePos, const Common::Point &idlePos, const int16 period = 500);
  ~Scroller();
  bool update(uint32 deltatime);  //Calculate updated position of scrolled graphics; return true if redraw is necessary.

	Common::Point Pos;
	Common::Point prevPos;

  bool active = false;
	bool scrolled = false;  //True when menu is in full active position
private:
  const Common::Point _activePos;
  const Common::Point _idlePos;
	Common::Point deltaPos;
  const int16 _period;
};

} // End of namespace ZVision

#endif
