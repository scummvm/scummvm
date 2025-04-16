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

/**
 * Automatically scroll a GUI menu or similar graphical element between an active and an idle position
 * Movement in one dimension; idle & active positions specified as int16.
 * Movement is at constant speed determined by period, specified in ms.
 * If active/idle status is changed mid-transition, will scroll from current position to the appropriate position.
 * LinearScroller also be used to reversibly scroll animation frames.
 */

class LinearScroller {
public:
	LinearScroller(const int16 activePos, const int16 idlePos, const int16 period = 500);
	~LinearScroller();
	void reset();  ///< Set idle and immediately jump to idle position
	bool update(uint32 deltatime);  ///< Calculate updated position of scrolled graphics; return true if redraw is necessary.
	void setActive(bool active);	  ///< Set active or idle & scroll at set speed from current position to that position.
	bool isMoving();
	int16 getPos();

	int16 _pos;
	int16 _prevPos;

private:
	bool _active = false;
	bool _moving = false;
	const int16 _activePos;
	const int16 _idlePos;
	const int16 _deltaPos;
	const int16 _period;
};

/**
 * Automatically scroll a GUI menu or similar graphical element between an active and an idle position
 * Movement in two dimensions; idle & active positions specified as Common::Point
 * Movement is at constant speed determined by period, specified in ms.
 * If active/idle status is changed mid-transition, will scroll from current position to the appropriate position.
 */

class Scroller {
public:
	Scroller(const Common::Point &activePos, const Common::Point &idlePos, const int16 period = 500);
	~Scroller();
	void reset();  ///< Set idle and immediately jump to idle position
	bool update(uint32 deltatime);  ///< Calculate updated position of scrolled graphics; return true if redraw is necessary.
	void setActive(bool active);  ///< Set active or idle & scroll at set speed from current position to that position.
	bool isMoving();
	
	Common::Point _pos;

private:
	LinearScroller _xScroller, _yScroller;
};

} // End of namespace ZVision

#endif
