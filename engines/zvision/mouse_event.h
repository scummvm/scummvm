/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 *
 */

#ifndef ZVISION_MOUSE_EVENT_H
#define ZVISION_MOUSE_EVENT_H

#include "common/types.h"
#include "common/rect.h"
#include "common/str.h"

namespace ZVision {

class ZVision;

class MouseEvent {
public:
	MouseEvent() : _key(0) {}
	MouseEvent(uint32 key) : _key(key) {}
	virtual ~MouseEvent() {}

public:
	uint32 _key;

public:
	/**
	 * Called when LeftMouse is pushed.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	virtual void onMouseDown(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos) = 0;
	/**
	 * Called when LeftMouse is lifted.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 */
	virtual void onMouseUp(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos) = 0;
	/**
	 * Called on every MouseMove.
	 *
	 * @param screenSpacePos             The position of the mouse in screen space
	 * @param backgroundImageSpacePos    The position of the mouse in background image space
	 * @return                           Was the cursor changed?
	 */
	virtual bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point backgroundImageSpacePos) = 0;
};

} // End of namespace ZVision

#endif
