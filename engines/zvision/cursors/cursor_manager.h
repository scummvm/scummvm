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

#ifndef ZVISION_CURSOR_MANAGER_H
#define ZVISION_CURSOR_MANAGER_H

#include "zvision/cursors/cursor.h"

#include "common/str.h"


namespace Graphics {
struct PixelFormat;
}

namespace ZVision {

class ZVision;

/**
 * Class to manage cursor changes. The actual changes have to be done
 * through CursorMan. Otherwise the cursor will disappear after GMM
 * or debug console.
 * TODO: Figure out a way to get rid of the extraneous data copying due to having to use CursorMan
 */
class CursorManager {
public:
	CursorManager(ZVision *engine, const Graphics::PixelFormat *pixelFormat);

private:
	enum {
		NUM_CURSORS = 18,
		// WARNING: The index 11 is hardcoded. If you change the order of _cursorNames/_zgiCursorFileNames/_zNemCursorFileNames, you HAVE to change the index accordingly
		IDLE_CURSOR_INDEX = 11
	};

	ZVision *_engine;
	const Graphics::PixelFormat *_pixelFormat;
	ZorkCursor _idleCursor;
	Common::String _currentCursor;
	bool _cursorIsPushed;

	static const char *_cursorNames[];
	static const char *_zgiCursorFileNames[];
	static const char *_zNemCursorFileNames[];

public:
	/** Creates the idle cursor and shows it */
	void initialize();

	/**
	 * Parses a cursor name into a cursor file then creates and shows that cursor.
	 * It will use the current _isCursorPushed state to choose the correct cursor
	 *
	 * @param cursorName    The name of a cursor. This *HAS* to correspond to one of the entries in _cursorNames[]
	 */
	void changeCursor(const Common::String &cursorName);
	/**
	 * Parses a cursor name into a cursor file then creates and shows that cursor.
	 *
	 * @param cursorName    The name of a cursor. This *HAS* to correspond to one of the entries in _cursorNames[]
	 * @param pushed        Should the cursor be pushed (true) or not pushed (false) (Another way to say it: down or up)
	 */
	void changeCursor(const Common::String &cursorName, bool pushed);
	/**
	 * Change the cursor to a certain push state. If the cursor is already in the specified push state, nothing will happen.
	 *
	 * @param pushed    Should the cursor be pushed (true) or not pushed (false) (Another way to say it: down or up)
	 */
	void cursorDown(bool pushed);

	/** Set the cursor to 'Left Arrow'. It will retain the current _isCursorPushed state */
	void setLeftCursor();
	/** Set the cursor to 'Right Arrow'. It will retain the current _isCursorPushed state */
	void setRightCursor();
	/** Set the cursor to 'Up Arrow'. It will retain the current _isCursorPushed state */
	void setUpCursor();
	/** Set the cursor to 'Down Arrow'. It will retain the current _isCursorPushed state */
	void setDownCursor();

	/** Set the cursor to 'Idle'. It will retain the current _isCursorPushed state */
	void revertToIdle();

private:
	/**
	 * Calls CursorMan.replaceCursor() using the data in cursor
	 *
	 * @param cursor    The cursor to show
	 */
	void changeCursor(const ZorkCursor &cursor);
};

} // End of namespace ZVision

#endif
