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
	CURSOR_MOVE_UP = 5,
	CURSOR_MOVE_DOWN1 = 6,
	CURSOR_MOVE_FORWARD2 = 7,
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
		CVideoSurface *_videoSurface;
		Graphics::ManagedSurface *_frameSurface;
		Common::Point _centroid;

		CursorEntry() : _videoSurface(nullptr), _frameSurface(nullptr) {}
		~CursorEntry();
	};
private:
	CScreenManager *_screenManager;
	CursorId _cursorId;
	CursorEntry _cursors[NUM_CURSORS];
	uint _setCursorCount;
	int _fieldE4;
	int _fieldE8;

	/**
	 * Load the images for each cursor
	 */
	void loadCursorImages();
public:
	CMouseCursor(CScreenManager *screenManager);
	~CMouseCursor();

	/**
	 * Make the mouse cursor visible
	 */
	void show();

	/**
	 * Hide the mouse cursor
	 */
	void hide();

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

	void lockE4();
	void unlockE4();

	/**
	 * Sets the mouse to a new position
	 */
	void setPosition(const Point &pt, double rate);
};


} // End of namespace Titanic

#endif /* TITANIC_MOUSE_CURSOR_H */
