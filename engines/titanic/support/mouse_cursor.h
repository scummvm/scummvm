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

#ifndef TITANIC_MOUSE_CURSOR_H
#define TITANIC_MOUSE_CURSOR_H

#include "common/scummsys.h"
#include "graphics/managed_surface.h"
#include "titanic/support/rect.h"

namespace Titanic {

#define NUM_CURSORS 15

enum CursorId {
	CURSOR_ARROW = 1,
	CURSOR_MOVE_LEFT = 2,
	CURSOR_MOVE_RIGHT = 3,
	CURSOR_MOVE_FORWARD = 4,
	CURSOR_LOOK_UP = 5,
	CURSOR_LOOK_DOWN = 6,
	CURSOR_MOVE_THROUGH = 7,
	CURSOR_HAND = 8,
	CURSOR_ACTIVATE = 9,
	CURSOR_INVALID = 10,
	CURSOR_MAGNIFIER = 11,
	CURSOR_IGNORE = 12,
	CURSOR_BACKWARDS = 13,
	CURSOR_DOWN = 14,
	CURSOR_HOURGLASS = 15
};

class CScreenManager;
class CVideoSurface;

class CMouseCursor {
	struct CursorEntry {
		Graphics::ManagedSurface *_surface;
		Common::Point _centroid;

		CursorEntry() : _surface(nullptr) {}
		~CursorEntry();
	};
private:
	CScreenManager *_screenManager;
	CursorId _cursorId;
	CursorEntry _cursors[NUM_CURSORS];
	uint _setCursorCount;
	int _hideCounter;
	int _busyCount;
	bool _cursorSuppressed;
	int _fieldE8;
	Common::Point _moveStartPos;
	Common::Point _moveDestPos;
	uint32 _moveStartTime, _moveEndTime;

	/**
	 * Load the images for each cursor
	 */
	void loadCursorImages();
public:
	bool _inputEnabled;
public:
	CMouseCursor(CScreenManager *screenManager);
	~CMouseCursor();

	/**
	 * Increment the busy count for the cursor, showing an hourglass
	 */
	void incBusyCount();

	/**
	 * Decrements the busy count, resetting back to an arrow cursor
	 * when the count reaches zero
	 */
	void decBusyCount();

	/**
	 * Decrements the hide counter, and shows the mouse if
	 * it's reached zero
	 */
	void incHideCounter();

	/**
	 * Increments the hide counter, hiding the mouse if it's the first call
	 */
	void decHideCounter();

	/**
	 * Suppresses the cursor. When suppressed, the cursor isn't drawn,
	 * even if it's not otherwise being hidden
	 */
	void suppressCursor();

	/**
	 * Unflags the cursor as being suppressed, allowing it to be drawn
	 * again if it's enabled
	 */
	void unsuppressCursor();

	/**
	 * Set the cursor
	 */
	void setCursor(CursorId cursorId);

	/**
	 * Updates the mouse cursor
	 */
	void update();

	/**
	 * Returns the number of times the cursor has been set
	 */
	uint getChangeCount() const { return _setCursorCount; }

	/**
	 * Disables user control of the mouse
	 */
	void disableControl();

	/**
	 * Re-enables user control of the mouse
	 */
	void enableControl();

	/**
	 * Move the mouse to a new position
	 */
	void setPosition(const Point &pt, double duration);
};


} // End of namespace Titanic

#endif /* TITANIC_MOUSE_CURSOR_H */
