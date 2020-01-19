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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA_SHARED_CORE_MOUSE_CURSOR_H
#define ULTIMA_SHARED_CORE_MOUSE_CURSOR_H

#include "common/scummsys.h"
#include "ultima/shared/core/rect.h"
#include "common/stream.h"

namespace Ultima {
namespace Shared {

#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16
/*
namespace Early {

enum CursorId {
	CURSOR_ARROW = 0
};

} // End of namespace Early

namespace Later {
namespace Xanth {

enum CursorId {
	CURSOR_BIG_ARROW = 0,
	CURSOR_SMALL_ARROW = 1,
	CURSOR_TICK = 2,
	CURSOR_HOURGLASS = 3
};

} // End of namespace Xanth
} // End of namespace Later
*/

/**
 * Handles the dislay and management of the on-screen mouse cursor
 */
class MouseCursor {
	struct MouseCursorData {
		Point _hotspot;
		uint16 _pixels[16];
		uint16 _mask[16];

		void load(Common::SeekableReadStream &s);
	};
private:
	Common::Array<MouseCursorData> _cursors;
	int _cursorId;
private:
	/**
	 * Loads the mouse cursors
	 */
	void loadCursors();
public:
	MouseCursor();

	/**
	 * Sets the active cursor
	 */
	void setCursor(int cursorId);

	/**
	 * Returns the curent cursor
	 */
	int getCursor() const { return _cursorId; }

	/**
	 * Make the mouse cursor visible
	 */
	void show();

	/**
	 * Hide the mouse cursor
	 */
	void hide();
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
