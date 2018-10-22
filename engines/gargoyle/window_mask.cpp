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

#include "gargoyle/window_mask.h"
#include "gargoyle/conf.h"
#include "gargoyle/gargoyle.h"
#include "gargoyle/windows.h"

namespace Gargoyle {

int WindowMask::_lastX;
int WindowMask::_lastY;

WindowMask::WindowMask() : _hor(0), _ver(0), _links(nullptr) {
	_lastX = _lastY = 0;
}

void WindowMask::resize(size_t x, size_t y) {
	// Deallocate old storage
	for (size_t i = 0; i < _hor; i++) {
		if (_links[i])
			delete _links[i];
	}

	delete _links;

	_hor = x + 1;
	_ver = y + 1;

	// allocate new storage
	_links = new glui32 *[_hor];
	if (!_links) {
		warning("resize_mask: out of memory");
		_hor = _ver = 0;
		return;
	}

	for (size_t i = 0; i < _hor; i++) {
		_links[i] = new glui32[_ver];
		if (!_links[i]) {
			warning("resize_mask: could not allocate new memory");
			return;
		}
	}

	_select.left = 0;
	_select.top = 0;
	_select.right = 0;
	_select.bottom = 0;
}

void WindowMask::putHyperlink(glui32 linkval, uint x0, uint y0, uint x1, uint y1) {
	uint i, k;
	size_t tx0 = x0 < x1 ? x0 : x1;
	size_t tx1 = x0 < x1 ? x1 : x0;
	size_t ty0 = y0 < y1 ? y0 : y1;
	size_t ty1 = y0 < y1 ? y1 : y0;

	if (!_hor || !_ver) {
		warning("putHyperlink: struct not initialized");
		return;
	}

	if (tx0 >= _hor
		|| tx1 >= _hor
		|| ty0 >= _ver || ty1 >= _ver
		|| !_links[tx0] || !_links[tx1]) {
		warning("putHyperlink: invalid range given");
		return;
	}

	for (i = tx0; i < tx1; i++) {
		for (k = ty0; k < ty1; k++)
			_links[i][k] = linkval;
	}
}

glui32 WindowMask::getHyperlink(const Common::Point &pos) {
	if (!_hor || !_ver) {
		warning("getHyperlink: struct not initialized");
		return 0;
	}

	if (pos.x >= (int16)_hor
		|| pos.y >= (int16)_ver
		|| !_links[pos.x]) {
		warning("getHyperlink: invalid range given");
		return 0;
	}

	return _links[pos.x][pos.y];
}

void WindowMask::startSelection(const Common::Point &pos) {
	int tx, ty;

	if (!_hor || !_ver) {
		warning("startSelection: mask not initialized");
		return;
	}

	tx = MIN(pos.x, (int16)_hor);
	ty = MIN(pos.y, (int16)_ver);

	_select.left = _lastX = tx;
	_select.top = _lastY = ty;
	_select.right = 0;
	_select.bottom = 0;

	g_vm->_windows->selectionChanged();
}

void WindowMask::moveSelection(const Common::Point &pos) {
	int tx, ty;

	if (ABS(pos.x - _lastX) < 5 && abs(pos.y - _lastY) < 5)
		return;

	if (!_hor || !_ver) {
		warning("moveSelection: mask not initialized");
		return;
	}

	tx = MIN(pos.x, (int16)_hor);
	ty = MIN(pos.y, (int16)_ver);

	_select.right = _lastX = tx;
	_select.bottom = _lastY = ty;

	g_vm->_windows->selectionChanged();
}

void WindowMask::clearSelection() {
	if (_select.left || _select.right
		|| _select.top || _select.bottom)
		Windows::_forceRedraw = true;

	_select.left = 0;
	_select.top = 0;
	_select.right = 0;
	_select.bottom = 0;
	g_vm->_windows->clearClaimSelect();
}

int WindowMask::checkSelection(uint x0, uint y0, uint x1, uint y1) {
	uint cx0, cx1, cy0, cy1;

	cx0 = _select.left < _select.right
		? _select.left
		: _select.right;

	cx1 = _select.left < _select.right
		? _select.right
		: _select.left;

	cy0 = _select.top < _select.bottom
		? _select.top
		: _select.bottom;

	cy1 = _select.top < _select.bottom
		? _select.bottom
		: _select.top;

	if (!cx0 || !cx1 || !cy0 || !cy1)
		return false;

	if (cx0 >= x0 && cx0 <= x1
		&& cy0 >= y0 && cy0 <= y1)
		return true;

	if (cx0 >= x0 && cx0 <= x1
		&& cy1 >= y0 && cy1 <= y1)
		return true;

	if (cx1 >= x0 && cx1 <= x1
		&& cy0 >= y0 && cy0 <= y1)
		return true;

	if (cx1 >= x0 && cx1 <= x1
		&& cy1 >= y0 && cy1 <= y1)
		return true;

	return false;
}

int WindowMask::getSelection(uint x0, uint y0, uint x1, uint y1, uint *rx0, uint *rx1) {
	uint row, upper, lower, above, below;
	int row_selected, found_left, found_right;
	int from_right, from_below, is_above, is_below;
	uint cx0, cx1, cy0, cy1;

	row = (y0 + y1) / 2;
	upper = row - (row - y0) / 2;
	lower = row + (y1 - row) / 2;
	above = upper - (g_conf->_leading) / 2;
	below = lower + (g_conf->_leading) / 2;

	cx0 = _select.left < _select.right
		? _select.left
		: _select.right;

	cx1 = _select.left < _select.right
		? _select.right
		: _select.left;

	cy0 = _select.top < _select.bottom
		? _select.top
		: _select.bottom;

	cy1 = _select.top < _select.bottom
		? _select.bottom
		: _select.top;

	row_selected = false;

	if ((cy0 >= upper && cy0 <= lower)
		|| (cy1 >= upper && cy1 <= lower))
		row_selected = true;

	if (row >= cy0 && row <= cy1)
		row_selected = true;

	if (!row_selected)
		return false;

	from_right = (_select.left != (int16)cx0);
	from_below = (_select.top != (int16)cy0);
	is_above = (above >= cy0 && above <= cy1);
	is_below = (below >= cy0 && below <= cy1);

	*rx0 = 0;
	*rx1 = 0;

	found_left = false;
	found_right = false;

	if (is_above && is_below) {
		*rx0 = x0;
		*rx1 = x1;
		found_left = true;
		found_right = true;
	} else if (!is_above && is_below) {
		if (from_below) {
			if (from_right) {
				*rx0 = cx0;
				*rx1 = x1;
				found_left = true;
				found_right = true;
			} else {
				*rx0 = cx1;
				*rx1 = x1;
				found_left = true;
				found_right = true;
			}
		} else {
			if (from_right) {
				*rx0 = cx1;
				*rx1 = x1;
				found_left = true;
				found_right = true;
			} else {
				*rx1 = x1;
				found_right = true;
			}
		}
	} else if (is_above && !is_below) {
		if (from_below) {
			if (from_right) {
				*rx0 = x0;
				*rx1 = cx1;
				found_left = true;
				found_right = true;
			} else {
				*rx0 = x0;
				*rx1 = cx0;
				found_left = true;
				found_right = true;
			}
		} else {
			if (from_right) {
				if (x0 > cx0)
					return false;
				*rx0 = x0;
				*rx1 = cx0;
				found_left = true;
				found_right = true;
			} else {
				*rx0 = x0;
				found_left = true;
			}
		}
	}

	if (found_left && found_right)
		return true;

	for (uint i = x0; i <= x1; i++) {
		if (i >= cx0 && i <= cx1) {
			if (!found_left) {
				*rx0 = i;
				found_left = true;
				if (found_right)
					return true;
			} else {
				if (!found_right)
					*rx1 = i;
			}
		}
	}

	if (rx0 && !rx1)
		*rx1 = x1;

	return (rx0 && rx1);
}

} // End of namespace Gargoyle
