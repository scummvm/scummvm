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

#ifndef GLK_FROTZ_WINDOWS
#define GLK_FROTZ_WINDOWS

#include "glk/windows.h"

namespace Glk {
namespace Frotz {

#include "glk/windows.h"
#include "glk/utils.h"

enum WindowProperty {
	Y_POS = 0, X_POS = 1, Y_SIZE = 2, X_SIZE = 3, Y_CURSOR = 4, X_CURSOR = 5,
	LEFT_MARGIN = 6, RIGHT_MARGIN = 7, NEWLINE_INTERRUPT = 8, INTERRUPT_COUNTDOWN = 9,
	TEXT_STYLE = 10, COLOUR_DATA = 11, FONT_NUMBER = 12, FONT_SIZE = 13, ATTRIBUTES = 14,
	LINE_COUNT = 15, TRUE_FG_COLOR = 16, TRUE_BG_COLOR = 17
};

class Windows;

/**
 * Represents one of the virtual windows
 */
class Window {
	friend class Windows;
private:
	Windows *_windows;
	winid_t _win;
	uint16 _tempVal;		///< used in calls to square brackets operator
private:
	/**
	 * Gets a reference to the window, creating a new one if one doesn't already exist
	 */
	winid_t getWindow();

	/**
	 * Get a property value
	 */
	uint16 getProperty(WindowProperty propType);

	/**
	 * Set a property value
	 */
	void setProperty(WindowProperty propType, uint16 value);
public:
	/**
	 * Constructor
	 */
	Window();

	/**
	 * Assignment operator
	 */
	Window &operator=(winid_t win) {
		_win = win;
		return *this;
	}

	/**
	 * Cast operator for getting a Glk window
	 */
	operator winid_t() const { return _win; }

	/**
	 * Cast operator for testing if the window is valid (present)
	 */
	operator bool() const { return _win != nullptr; }

	/**
	 * Property accessor
	 */
	const uint16 &operator[](WindowProperty propType);

	/**
	 * Set the window size
	 */
	void setSize(const Point &newSize);

	/**
	 * Set the position of a window
	 */
	void setPosition(const Point &newPos);
};

/**
 * Windows manager
 */
class Windows {
private:
	Window _windows[8];
public:
	Window &_lower;
	Window &_upper;
public:
	/**
	 * Constructor
	 */
	Windows();

	/**
	 * Returns the number of allowable windows
	 */
	size_t size() const;

	/**
	 * Array access
	 */
	Window &operator[](uint idx);
};

} // End of namespace Frotz
} // End of namespace Glk

#endif
