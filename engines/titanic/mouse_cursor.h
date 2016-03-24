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
#include "common/rect.h"

namespace Titanic {

#define NUM_CURSORS 15

enum CursorId {
	CURSOR_1 = 1,
	CURSOR_2 = 3,
	CURSOR_3 = 3,
	CURSOR_4 = 4,
	CURSOR_5 = 5,
	CURSOR_6 = 6,
	CURSOR_7 = 7,
	CURSOR_8 = 8,
	CURSOR_9 = 9,
	CURSOR_10 = 10,
	CURSOR_11 = 11,
	CURSOR_12 = 12,
	CURSOR_13 = 13,
	CURSOR_14 = 14,
	CURSOR_15 = 15
};

class CScreenManager;
class CVideoSurface;

class CMouseCursor {
	struct CursorEntry {
		CVideoSurface *_videoSurface;
		void *_ptrUnknown;
		Common::Point _centroid;
	};
private:
	CScreenManager *_screenManager;
	CursorId _cursorId;
	CursorEntry _cursors[NUM_CURSORS];

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
};


} // End of namespace Titanic

#endif /* TITANIC_MOUSE_CURSOR_H */
