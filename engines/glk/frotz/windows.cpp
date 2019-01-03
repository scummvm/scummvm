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
#include "glk/window_pair.h"
#include "glk/conf.h"

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

Window::Window() : _windows(nullptr), _win(nullptr), _tempVal(0) {}

winid_t Window::getWindow() {
	if (!_win) {
		// Window doesn't exist, so create it
		// TODO: For now I'm assuming all the extra created windows will be graphics, since Glk requires
		// us to specify it at creation time. Not sure if it's true or not for all V6 games
		_win = g_vm->glk_window_open(g_vm->glk_window_get_root(), winmethod_Arbitrary | winmethod_Fixed,
			0, wintype_Graphics, 0);
	}

	return _win;
}

void Window::setSize(const Point &newSize) {
	winid_t win = getWindow();
	checkRepositionLower();

	Point s(newSize.x * g_conf->_monoInfo._cellW, newSize.y * g_conf->_monoInfo._cellH);
	win->setSize(s);
/* TODO

	// Keep the cursor within the window
	if (wp[win].y_cursor > zargs[1] || wp[win].x_cursor > zargs[2])
		reset_cursor(win);

	os_window_height(win, _wp[win].y_size);
	*/
}

void Window::setPosition(const Point &newPos) {
	winid_t win = getWindow();
	checkRepositionLower();

	Point pos((newPos.x - 1) * g_conf->_monoInfo._cellW, (newPos.y - 1) * g_conf->_monoInfo._cellH);
	win->setPosition(pos);
}

const uint16 &Window::operator[](WindowProperty propType) {
	_tempVal = getProperty(propType);
	return _tempVal;
}

uint16 Window::getProperty(WindowProperty propType) {
	winid_t win = getWindow();
	Point pt;

	switch (propType) {
	case Y_POS:
		return win->_bbox.top / g_conf->_monoInfo._cellH;
	case X_POS:
		return win->_bbox.left / g_conf->_monoInfo._cellW;
	case Y_SIZE:
		return win->_bbox.height() / g_conf->_monoInfo._cellH;
	case X_SIZE:
		return win->_bbox.width() / g_conf->_monoInfo._cellW;
	case Y_CURSOR:
		return win->getCursor().y;
	case X_CURSOR:
		return win->getCursor().x;

	default:
		error("Read of an unimplemented property");
		/*
			LEFT_MARGIN = 6, RIGHT_MARGIN = 7, NEWLINE_INTERRUPT = 8, INTERRUPT_COUNTDOWN = 9,
			TEXT_STYLE = 10, COLOUR_DATA = 11, FONT_NUMBER = 12, FONT_SIZE = 13, ATTRIBUTES = 14,
			LINE_COUNT = 15, TRUE_FG_COLOR = 16, TRUE_BG_COLOR = 17

	case TRUE_FG_COLOR:
		store(os_to_true_colour(lo(wp[winarg0()].colour)));

	case TRUE_BG_COLOR:
		zword bg = hi(wp[winarg0()].colour);

		if (bg == TRANSPARENT_COLOUR)
			store((zword)-4);
		else
			store(os_to_true_colour(bg));
			*/
	}
}

void Window::setProperty(WindowProperty propType, uint16 value) {
	// TODO
}

void Window::checkRepositionLower() {
	if (&_windows->_lower == this) {
		PairWindow *parent = dynamic_cast<PairWindow *>(_win->_parent);
		if (!parent)
			error("Parent was not a pair window");

		// Ensure the parent pair window is flagged as having children at arbitrary positions,
		// just in case it isn't already
		parent->_dir = winmethod_Arbitrary;
	}
}

} // End of namespace Frotz
} // End of namespace Glk
