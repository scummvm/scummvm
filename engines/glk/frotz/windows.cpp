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

#include "glk/frotz/windows.h"
#include "glk/frotz/frotz.h"

namespace Glk {
namespace Frotz {

Windows::Windows() : _lower(_windows[0]), _upper(_windows[1]) {
	for (size_t idx = 0; idx < 8; ++idx)
		_windows[idx]._windows = this;
}

size_t Windows::size() const {
	return (g_vm->h_version < 6) ? 2 : 8;
}

Window &Windows::operator[](uint idx) {
	assert(idx < size());
	return _windows[idx];
}

/*--------------------------------------------------------------------------*/

winid_t Window::getWindow() {
	if (!_win) {
		// Window doesn't exist, so create it
		// TODO
	}

	return _win;
}

void Window::setSize(const Point &newSize) {
	winid_t win = getWindow();

/* TODO
	y_size = zargs[1];
	_wp[win].x_size = zargs[2];

	// Keep the cursor within the window
	if (wp[win].y_cursor > zargs[1] || wp[win].x_cursor > zargs[2])
		reset_cursor(win);

	os_window_height(win, _wp[win].y_size);
	*/
}

void Window::setPosition(const Point &newPos) {
	winid_t win = getWindow();

	/* TODO
	if (win == cwin)
		update_cursor();
	*/
}


} // End of namespace Frotz
} // End of namespace Glk
