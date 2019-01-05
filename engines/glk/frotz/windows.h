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
#include "glk/frotz/frotz_types.h"

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

	/**
	 * Stub class for accessing window properties via the square brackets operator
	 */
	class PropertyAccessor {
	private:
		Window *_owner;
		WindowProperty _prop;
	public:
		/**
		 * Constructor
		 */
		PropertyAccessor(Window *owner, WindowProperty prop) : _owner(owner), _prop(prop) {}

		/**
		 * Get
		 */
		operator zword() const {
			return _owner->getProperty(_prop);
		}

		/**
		 * Set
		 */
		PropertyAccessor &operator=(zword val) {
			_owner->setProperty(_prop, val);
			return *this;
		}
	};
private:
	Windows *_windows;
	winid_t _win;
	zword _properties[TRUE_BG_COLOR + 1];
private:
	/**
	 * Get a property value
	 */
	const zword &getProperty(WindowProperty propType);

	/**
	 * Set a property value
	 */
	void setProperty(WindowProperty propType, zword value);

	/**
	 * Called when trying to reposition or resize windows. Does special handling for the lower window
	 */
	void checkRepositionLower();

	/**
	 * Updates the local window properties based on an attached Glk window
	 */
	void update();
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
	operator winid_t() const {
		assert(_win);
		return _win;
	}

	/**
	 * Cast operator for testing if the window has a proper Glk window attached to it
	 */
	operator bool() const { return _win != nullptr; }

	/**
	 * Property accessor
	 */
	PropertyAccessor operator[](WindowProperty propType) { return PropertyAccessor(this, propType); }

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
	winid_t _background;
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

	/**
	 * Setup the screen
	 */
	void setup(bool isVersion6);
};

} // End of namespace Frotz
} // End of namespace Glk

#endif
