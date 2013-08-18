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
	MouseEvent(uint32 key, const Common::Rect &hotspot, const Common::String hoverCursor);

	/** The Control key */
	uint32 _key;
	/** 
	 * The area that will trigger the event
	 * This is in image space coordinates, NOT screen space 
	 */
	Common::Rect _hotspot;
	/** The cursor to use when hovering over _hotspot */
	Common::String _hoverCursor;

	/**
	 * Does a simple Rect::contains() using _hotspot
	 *
	 * @param point    The point to check against _hotspot
	 * @return         The point is inside _hotspot (true) or not (false)
	 */
	bool withinHotspot(const Common::Point &point);
	/**
	 * Calls ScriptManager::setStateValue(_key, 1)
	 *
	 * @param engine    The base engine
	 */
	void onClick(ZVision *engine);
	/**
	 * Gets the name of the cursor that should be displayed when withinHotspot returns true
	 *
	 * @return    The name of the cursor. This should correspond to one of the names in CursorManager::_cursorNames[]
	 */
	const Common::String getHoverCursor() { return _hoverCursor; }
};

} // End of namespace ZVision

#endif
