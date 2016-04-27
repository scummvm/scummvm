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

#ifndef TITANIC_TEXT_CURSOR_H
#define TITANIC_TEXT_CURSOR_H

#include "common/scummsys.h"
#include "titanic/support/rect.h"

namespace Titanic {

class CScreenManager;
class CVideoSurface;

class CTextCursor {
private:
	CScreenManager *_screenManager;
	Point _pos;
	Rect _bounds;
	uint _priorTicks;
	int _field24;
	Point _size;
	Point _screenTopLeft;
	int _field44;
	int _field48;
	int _field4C;
	CVideoSurface *_surface;
	int _field54;
public:
	bool _active;
public:
	CTextCursor(CScreenManager *screenManager);
	~CTextCursor();

	/**
	 * Sets the position of the cursor
	 */
	void setPos(const Point &pt) { _pos = pt; }

	/**
	 * Sets the size of the cursor
	 */
	void setSize(const Point &size) { _size = size; }

	/**
	 * Returns the bounds for the cursor
	 */
	Rect getCursorBounds() const {
		return Rect(_pos.x, _pos.y, _pos.x + _size.x, _pos.y + _size.y);
	}

	/**
	 * Set bounds
	 */
	void setBounds(const Rect &r) { _bounds = r; }

	/**
	 * Clear the bounds
	 */
	void clearBounds() { _bounds.clear(); }

	/**
	 * Set the prior ticks
	 */
	void setTicks(uint ticks) { _priorTicks = ticks; }

	/**
	 * Returns whether the text cursor is active
	 */
	bool isActive() const { return _active; }

	int get54() const { return _field54; }
};

} // End of namespace Titanic

#endif /* TITANIC_TEXT_CURSOR_H */
