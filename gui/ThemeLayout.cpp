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
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"
#include "common/system.h"
#include "common/events.h"
#include "common/xmlparser.h"

#include "gui/ThemeEval.h"
#include "gui/ThemeLayout.h"

#ifdef LAYOUT_DEBUG_DIALOG
#include "graphics/font.h"
#include "graphics/surface.h"
#endif

namespace GUI {

void ThemeLayout::importLayout(ThemeLayout *layout) {
	assert(layout->getLayoutType() == kLayoutMain);

	if (layout->_children.size() == 0)
		return;

	layout = layout->_children[0];

	if (getLayoutType() == layout->getLayoutType()) {
		for (uint i = 0; i < layout->_children.size(); ++i)
			_children.push_back(layout->_children[i]->makeClone(this));
	} else {
		_children.push_back(layout->makeClone(this));
	}
}

bool ThemeLayout::getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) {
	if (name.empty()) {
		assert(getLayoutType() == kLayoutMain);
		x = _x; y = _y;
		w = _w; h = _h;
		return true;
	}

	for (uint i = 0; i < _children.size(); ++i) {
		if (_children[i]->getWidgetData(name, x, y, w, h))
			return true;
	}

	return false;
}

int16 ThemeLayoutStacked::getParentW() {
	ThemeLayout *p = _parent;
	int width = 0;

	while (p && p->getLayoutType() != kLayoutMain) {
		width += p->_padding.right + p->_padding.left;
		if (p->getLayoutType() == kLayoutHorizontal) {
			for (uint i = 0; i < p->_children.size(); ++i)
				width += p->_children[i]->getWidth() + p->_spacing;
		}
		// FIXME: Do we really want to assume that any layout type different
		// from kLayoutHorizontal corresponds to width 0 ?
		p = p->_parent;
	}

	assert(p && p->getLayoutType() == kLayoutMain);
	return p->getWidth() - width;
}

int16 ThemeLayoutStacked::getParentH() {
	ThemeLayout *p = _parent;
	int height = 0;

	while (p && p->getLayoutType() != kLayoutMain) {
		height += p->_padding.bottom + p->_padding.top;
		if (p->getLayoutType() == kLayoutVertical) {
			for (uint i = 0; i < p->_children.size(); ++i)
				height += p->_children[i]->getHeight() + p->_spacing;
		}
		// FIXME: Do we really want to assume that any layout type different
		// from kLayoutVertical corresponds to height 0 ?
		p = p->_parent;
	}

	assert(p && p->getLayoutType() == kLayoutMain);
	return p->getHeight() - height;
}

#ifdef LAYOUT_DEBUG_DIALOG
void ThemeLayout::debugDraw(Graphics::Surface *screen, const Graphics::Font *font) {
	uint16 color = 0xFFFF;
	font->drawString(screen, getName(), _x, _y, _w, color, Graphics::kTextAlignRight, 0, true);
	screen->hLine(_x, _y, _x + _w, color);
	screen->hLine(_x, _y + _h, _x + _w , color);
	screen->vLine(_x, _y, _y + _h, color);
	screen->vLine(_x + _w, _y, _y + _h, color);

	for (uint i = 0; i < _children.size(); ++i)
		_children[i]->debugDraw(screen, font);
}
#endif


bool ThemeLayoutWidget::getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) {
	if (name == _name) {
		x = _x; y = _y;
		w = _w; h = _h;
		return true;
	}

	return false;
}

void ThemeLayoutMain::reflowLayout() {
	assert(_children.size() <= 1);

	if (_children.size()) {
		_children[0]->resetLayout();
		_children[0]->setWidth(_w);
		_children[0]->setHeight(_h);
		_children[0]->reflowLayout();

		if (_w == -1)
			_w = _children[0]->getWidth();

		if (_h == -1)
			_h = _children[0]->getHeight();

		if (_y == -1)
			_y = (g_system->getOverlayHeight() >> 1) - (_h >> 1);

		if (_x == -1)
			_x = (g_system->getOverlayWidth() >> 1) - (_w >> 1);
	}
}

void ThemeLayoutStacked::reflowLayoutV() {
	int curX, curY;
	int resize[8];
	int rescount = 0;

	curX = _padding.left;
	curY = _padding.top;
	_h = _padding.top + _padding.bottom;

	for (uint i = 0; i < _children.size(); ++i) {

		_children[i]->resetLayout();
		_children[i]->reflowLayout();

		if (_children[i]->getWidth() == -1)
			_children[i]->setWidth((_w == -1 ? getParentW() : _w) - _padding.left - _padding.right);

		if (_children[i]->getHeight() == -1) {
			assert(rescount < ARRAYSIZE(resize));
			resize[rescount++] = i;
			_children[i]->setHeight(0);
		}

		_children[i]->setY(curY);

		// Center child if it this has been requested *and* the space permits it.
		if (_centered && _children[i]->getWidth() < _w && _w != -1) {
			_children[i]->setX((_w >> 1) - (_children[i]->getWidth() >> 1));
		} else
			_children[i]->setX(curX);

		curY += _children[i]->getHeight() + _spacing;
		_w = MAX(_w, (int16)(_children[i]->getWidth() + _padding.left + _padding.right));
		_h += _children[i]->getHeight() + _spacing;
	}

	_h -= _spacing;

	if (rescount) {
		int newh = (getParentH() - _h - _padding.bottom) / rescount;

		for (int i = 0; i < rescount; ++i) {
			_children[resize[i]]->setHeight(newh);
			_h += newh;
			for (uint j = resize[i] + 1; j < _children.size(); ++j)
				_children[j]->setY(newh);
		}
	}
}

void ThemeLayoutStacked::reflowLayoutH() {
	int curX, curY;
	int resize[8];
	int rescount = 0;

	curX = _padding.left;
	curY = _padding.top;
	_w = _padding.left + _padding.right;

	for (uint i = 0; i < _children.size(); ++i) {

		_children[i]->resetLayout();
		_children[i]->reflowLayout();

		if (_children[i]->getHeight() == -1)
			_children[i]->setHeight((_h == -1 ? getParentH() : _h) - _padding.top - _padding.bottom);

		if (_children[i]->getWidth() == -1) {
			assert(rescount < ARRAYSIZE(resize));
			resize[rescount++] = i;
			_children[i]->setWidth(0);
		}

		_children[i]->setX(curX);

		// Center child if it this has been requested *and* the space permits it.
		if (_centered && _children[i]->getHeight() < _h && _h != -1)
			_children[i]->setY((_h >> 1) - (_children[i]->getHeight() >> 1));
		else
			_children[i]->setY(curY);

		curX += (_children[i]->getWidth() + _spacing);
		_w += _children[i]->getWidth() + _spacing;
		_h = MAX(_h, (int16)(_children[i]->getHeight() + _padding.top + _padding.bottom));
	}

	_w -= _spacing;

	if (rescount) {
		int neww = (getParentW() - _w - _padding.right) / rescount;

		for (int i = 0; i < rescount; ++i) {
			_children[resize[i]]->setWidth(neww);
			_w += neww;
			for (uint j = resize[i] + 1; j < _children.size(); ++j)
				_children[j]->setX(neww);
		}
	}
}

} // End of namespace GUI
