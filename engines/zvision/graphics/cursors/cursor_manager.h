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

#include "zvision/graphics/cursors/cursor.h"

#include "common/str.h"

namespace Graphics {
struct PixelFormat;
}

namespace ZVision {

class ZVision;

/**
 * Mostly usable cursors
 */
enum CursorIndex {
	CursorIndex_Active = 0,
	CursorIndex_DownArr = 3,
	CursorIndex_HandPu = 6,
	CursorIndex_Idle = 11,
	CursorIndex_Left = 12,
	CursorIndex_Right = 13,
	CursorIndex_UpArr = 17,
	CursorIndex_ItemIdle = 18,
	CursorIndex_ItemAct = 19
};

/**
 * Class to manage cursor changes. The actual changes have to be done
 * through CursorMan. Otherwise the cursor will disappear after GMM
 * or debug console.
 * TODO: Figure out a way to get rid of the extraneous data copying due to having to use CursorMan
 */
class CursorManager {
public:
	CursorManager(ZVision *engine, const Graphics::PixelFormat pixelFormat);

private:
	static const int NUM_CURSORS = 18;

	// 18 default cursors in up/down states, +2 for items idle/act cursors
	ZorkCursor _cursors[NUM_CURSORS + 2][2];

	ZVision *_engine;
	const Graphics::PixelFormat _pixelFormat;
	bool _cursorIsPushed;
	int _item;
	int _lastitem;
	int _currentCursor;

	static const char *_cursorNames[];
	static const char *_zgiCursorFileNames[];
	static const char *_zNemCursorFileNames[];

public:
	/** Creates the idle cursor and shows it */
	void initialize();

	/**
	 * Change cursor to specified cursor ID. If item setted to not 0 and cursor id idle/acrive/handpu change cursor to item.
	 *
	 * @param id    Wanted cursor id.
	 */

	void changeCursor(int id);

	/**
	 * Return founded id for string contains cursor name
	 *
	 * @param name    Cursor name
	 * @return        Id of cursor or idle cursor id if not found
	 */

	int getCursorId(const Common::String &name);

	/**
	 * Load cursor for item by id, and try to change cursor to item cursor if it's not 0
	 *
	 * @param id    Item id or 0 for no item cursor
	 */

	void setItemID(int id);

	/**
	 * Change the cursor to a certain push state. If the cursor is already in the specified push state, nothing will happen.
	 *
	 * @param pushed    Should the cursor be pushed (true) or not pushed (false) (Another way to say it: down or up)
	 */
	void cursorDown(bool pushed);

	/**
	 * Show or hide mouse cursor.
	 *
	 * @param vis    Should the cursor be showed (true) or hide (false)
	 */
	void showMouse(bool vis);

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
