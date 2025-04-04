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

class LinearScroller {
public:
  LinearScroller(const int16 activePos, const int16 idlePos, const int16 period = 500);
  ~LinearScroller();
  void reset();
  bool update(uint32 deltatime);  //Calculate updated position of scrolled graphics; return true if redraw is necessary.
  void setActive(bool active);
  bool isMoving();
  int16 getPos();

	int16 Pos;
	int16 prevPos;

private:
  bool _active = false;
  bool moving = false;
  const int16 _activePos;
  const int16 _idlePos;
	const int16 deltaPos;
  const int16 _period;
};

class Scroller {
public:
  Scroller(const Common::Point &activePos, const Common::Point &idlePos, const int16 period = 500);
  ~Scroller();
  void reset();
  bool update(uint32 deltatime);  //Calculate updated position of scrolled graphics; return true if redraw is necessary.
  void setActive(bool active);
  bool isMoving();
	Common::Point Pos;

private:
  LinearScroller Xscroller, Yscroller;
};

} // End of namespace ZVision

#endif
