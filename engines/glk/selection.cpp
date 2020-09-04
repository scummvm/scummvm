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

#include "glk/selection.h"
#include "glk/conf.h"
#include "glk/glk.h"
#include "glk/windows.h"
#include "common/system.h"

namespace Glk {

void Clipboard::clipboardStore(const Common::U32String &text) {
	_text = text;
}

void Clipboard::clipboardSend(ClipSource source) {
	g_system->setTextInClipboard(_text);
}

void Clipboard::clipboardReceive(ClipSource source) {
	Windows &windows = *g_vm->_windows;

	if (g_system->hasTextInClipboard()) {
		Common::U32String text = g_system->getTextFromClipboard();
		for (uint idx = 0; idx < text.size(); ++idx) {
			uint c = text[idx];
			if (c != '\r' && c != '\n' && c != '\b' && c != '\t')
				windows.inputHandleKey(c);
		}
	}
}

/*--------------------------------------------------------------------------*/

WindowMask::WindowMask() : _hor(0), _ver(0), _links(nullptr) {
	_last.x = _last.y = 0;
	resize(g_system->getWidth(), g_system->getHeight());
}

WindowMask::~WindowMask() {
	clear();
}

void WindowMask::clear() {
	for (size_t i = 0; i < _hor; i++) {
		if (_links[i])
			delete[] _links[i];
	}

	delete[] _links;
}

void WindowMask::resize(size_t x, size_t y) {
	clear();

	_hor = x + 1;
	_ver = y + 1;

	// allocate new storage
	_links = new uint *[_hor];
	if (!_links) {
		warning("resize_mask: out of memory");
		_hor = _ver = 0;
		return;
	}

	for (size_t i = 0; i < _hor; i++) {
		_links[i] = new uint[_ver];
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

void WindowMask::putHyperlink(uint linkval, uint x0, uint y0, uint x1, uint y1) {
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

uint WindowMask::getHyperlink(const Point &pos) const {
	if (!_hor || !_ver) {
		warning("getHyperlink: struct not initialized");
		return 0;
	}

	if (pos.x >= (int16)_hor || pos.y >= (int16)_ver || !_links[pos.x]) {
		warning("getHyperlink: invalid range given");
		return 0;
	}

	return _links[pos.x][pos.y];
}

/*--------------------------------------------------------------------------*/

void Selection::startSelection(const Point &pos) {
	int tx, ty;

	if (!_hor || !_ver) {
		warning("startSelection: mask not initialized");
		return;
	}

	tx = MIN(pos.x, (int16)_hor);
	ty = MIN(pos.y, (int16)_ver);

	_select.left = _select.right = _last.x = tx;
	_select.top = _select.bottom = _last.y = ty;

	g_vm->_windows->selectionChanged();
}

void Selection::moveSelection(const Point &pos) {
	int tx, ty;

	if (ABS(pos.x - _last.x) < 5 && ABS(pos.y - _last.y) < 5)
		return;

	if (!_hor || !_ver) {
		warning("moveSelection: mask not initialized");
		return;
	}

	tx = MIN(pos.x, (int16)_hor);
	ty = MIN(pos.y, (int16)_ver);

	_select.right = _last.x = tx;
	_select.bottom = _last.y = ty;

	g_vm->_windows->selectionChanged();
}

void Selection::clearSelection() {
	if (!_select.isEmpty())
		Windows::_forceRedraw = true;

	_select = Rect();
	g_vm->_windows->clearClaimSelect();
}

bool Selection::checkSelection(const Rect &r) const {
	Rect select(MIN(_select.left, _select.right), MIN(_select.top, _select.bottom),
		MAX(_select.left, _select.right), MAX(_select.top, _select.bottom));
	if (select.isEmpty())
		return false;

	return select.intersects(r);
}

bool Selection::getSelection(const Rect &r, int *rx0, int *rx1) const {
	uint row, upper, lower, above, below;
	bool row_selected, found_left, found_right;
	int from_right, from_below, is_above, is_below;
	uint cx0, cx1, cy0, cy1;
	uint x0 = r.left, y0 = r.top, x1 = r.right, y1 = r.bottom;

	row = (y0 + y1) / 2;
	upper = row - (row - y0) / 2;
	lower = row + (y1 - row) / 2;
	above = upper - (g_conf->_propInfo._leading) / 2;
	below = lower + (g_conf->_propInfo._leading) / 2;

	cx0 = MIN(_select.left, _select.right);
	cx1 = MAX(_select.left, _select.right);
	cy0 = MIN(_select.top, _select.bottom);
	cy1 = MAX(_select.top, _select.bottom);

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

} // End of namespace Glk
