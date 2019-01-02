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

namespace Glk {
namespace Frotz {

/**
 * Represents one of the virtual windows
 */
class Window {
private:
	winid_t _win;
public:
	/**
	 * Constructor
	 */
	Window() : _win(nullptr) {}

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
	 * Property access. There are the following properties defined by the spec:
	 * 0  y coordinate    6   left margin size            12  font number
	 * 1  x coordinate    7   right margin size           13  font size
	 * 2  y size          8   newline interrupt routine   14  attributes
	 * 3  x size          9   interrupt countdown         15  line count
	 * 4  y cursor        10  text style                  16 true foreground colour
	 * 5  x cursor        11  colour data                 17 true background colour
	 */
};

/**
 * The Z-machine has 8 virtual windows
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
	Windows() : _lower(_windows[0]), _upper(_windows[1]) {}

	/**
	 * Array access
	 */
	Window &operator[](uint idx) {
		assert(idx < 8);
		return _windows[idx];
	}
};

} // End of namespace Frotz
} // End of namespace Glk

#endif
