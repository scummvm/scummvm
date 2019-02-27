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
#include "glk/window_text_buffer.h"
#include "glk/conf.h"

namespace Glk {
namespace Frotz {

Windows::Windows() : _lower(_windows[0]), _upper(_windows[1]), _background(nullptr) {
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

void Windows::setup(bool isVersion6) {
	if (isVersion6) {
		// For graphic games we have a background window covering the entire screen for greater
		// flexibility of wher we draw pictures, and the lower and upper areas sit on top of them
		_background = g_vm->glk_window_open(0, 0, 0, wintype_Graphics, 0);
		_background->setBackgroundColor(0xffffff);

		MonoFontInfo &fi = g_vm->_conf->_monoInfo;
		_lower = g_vm->glk_window_open(g_vm->glk_window_get_root(),
			winmethod_Arbitrary | winmethod_Fixed, 0, wintype_TextBuffer, 0);
		_upper = g_vm->glk_window_open(g_vm->glk_window_get_root(),
			winmethod_Arbitrary | winmethod_Fixed, 0, wintype_TextGrid, 0);
		_upper.setPosition(Point(1, 1));
		_upper.setSize(Point(g_system->getWidth() / fi._cellW, 1));
		_lower.setPosition(Point(1, 2));
		_lower.setSize(Point(g_system->getWidth() / fi._cellW, g_system->getHeight() / fi._cellH - 1));

	} else {
		_lower = g_vm->glk_window_open(0, 0, 0, wintype_TextBuffer, 0);
		_upper = g_vm->glk_window_open(_lower, winmethod_Above | winmethod_Fixed, 0, wintype_TextGrid, 0);
	}

	_lower.update();
	_upper.update();
	g_vm->glk_set_window(_lower);
}

/*--------------------------------------------------------------------------*/

Window::Window() : _windows(nullptr), _win(nullptr) {
	Common::fill(_properties, _properties + TRUE_BG_COLOR + 1, 0);
	_properties[Y_POS] = _properties[X_POS] = 1;
	_properties[Y_CURSOR] = _properties[X_CURSOR] = 1;
	_properties[FONT_NUMBER] = TEXT_FONT;
	_properties[FONT_SIZE] = 12;
}

void Window::update() {
	assert(_win);

	_properties[X_POS] = _win->_bbox.left / g_conf->_monoInfo._cellW + 1;
	_properties[Y_POS] = _win->_bbox.top / g_conf->_monoInfo._cellH + 1;
	_properties[X_SIZE] = _win->_bbox.width() / g_conf->_monoInfo._cellW;
	_properties[Y_SIZE] = _win->_bbox.height() / g_conf->_monoInfo._cellH;

	Point pt = _win->getCursor();
	_properties[X_CURSOR] = (g_vm->h_version != V6) ? pt.x + 1 : pt.x / g_conf->_monoInfo._cellW + 1;
	_properties[Y_CURSOR] = (g_vm->h_version != V6) ? pt.y + 1 : pt.y / g_conf->_monoInfo._cellH + 1;

	TextBufferWindow *win = dynamic_cast<TextBufferWindow *>(_win);
	_properties[LEFT_MARGIN] = (win ? win->_ladjw : 0) / g_conf->_monoInfo._cellW;
	_properties[RIGHT_MARGIN] = (win ? win->_radjw : 0) / g_conf->_monoInfo._cellW;
	_properties[FONT_SIZE] = g_conf->_monoInfo._size;
}

void Window::setSize(const Point &newSize) {
	checkRepositionLower();

	_properties[X_SIZE] = newSize.x;
	_properties[Y_SIZE] = newSize.y;

	if (_win)
		_win->setSize(Point(newSize.x * g_conf->_monoInfo._cellW, newSize.y * g_conf->_monoInfo._cellH));
}

void Window::setPosition(const Point &newPos) {
	checkRepositionLower();

	_properties[X_POS] = newPos.x;
	_properties[Y_POS] = newPos.y;

	if (_win)
		_win->setPosition(Point((newPos.x - 1) * g_conf->_monoInfo._cellW, (newPos.y - 1) * g_conf->_monoInfo._cellH));
}

void Window::setCursor(const Point &newPos) {
	int x = newPos.x, y = newPos.y;

	if (y < 0) {
		// Cursor on/off
		if (y == -2)
			g_vm->_events->showMouseCursor(true);
		else if (y == -1)
			g_vm->_events->showMouseCursor(false);
		return;
	}

	if (!x || !y) {
		update();

		if (!x)
			x = _properties[X_CURSOR];
		if (!y)
			y = _properties[Y_CURSOR];
	}

	g_vm->glk_window_move_cursor(_win, x - 1, y - 1);
}


const uint &Window::getProperty(WindowProperty propType) {
	if (_win)
		update();

	return _properties[propType];
}

void Window::setProperty(WindowProperty propType, uint value) {
	switch (propType) {
	case TRUE_FG_COLOR:
	case TRUE_BG_COLOR:
		_properties[propType] = value;
		if (_win && _win->_stream)
			_win->_stream->setZColors(_properties[TRUE_FG_COLOR], _properties[TRUE_BG_COLOR]);
		break;

	default:
		warning("Setting window property %d not yet supported", (int)propType);
	}
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
