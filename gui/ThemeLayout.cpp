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
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/xmlparser.h"
#include "graphics/scaler.h"

#include "gui/ThemeEval.h"
#include "gui/ThemeLayout.h"

namespace GUI {

void ThemeLayout::importLayout(ThemeLayout *layout) {
	assert(layout->getLayoutType() == kLayoutMain);
	
	if (layout->_children.size() == 0)
		return;
	
	layout = layout->_children[0];
	
	if (getLayoutType() == layout->getLayoutType()) {
		for (uint i = 0; i < layout->_children.size(); ++i)
			_children.push_back(layout->_children[i]->buildCopy()); 
	} else {
		_children.push_back(layout->buildCopy()); 
	}
}

bool ThemeLayout::getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) {
	for (uint i = 0; i < _children.size(); ++i) {
		if (_children[i]->getWidgetData(name, x, y, w, h))
			return true;
	}
	
	return false;
}

int16 ThemeLayout::getParentW() {
	ThemeLayout *p = _parent;
	int width = 0;
	
	while (p && p->getLayoutType() != kLayoutMain) {
		width += p->_paddingRight + p->_paddingLeft;
		if (p->getLayoutType() == kLayoutHorizontal) {
			for (uint i = 0; i < p->_children.size(); ++i)
				width += p->_children[i]->getHeight() + p->_spacing;
		}
		p = p->_parent;
	}
	
	return p->getWidth() - width;
}

int16 ThemeLayout::getParentH() {
	ThemeLayout *p = _parent;
	int height = 0;
	
	while (p && p->getLayoutType() != kLayoutMain) {
		height += p->_paddingBottom + p->_paddingTop;
		if (p->getLayoutType() == kLayoutVertical) {
			for (uint i = 0; i < p->_children.size(); ++i)
				height += p->_children[i]->getHeight() + p->_spacing;
		}
		p = p->_parent;
	}
	
	return p->getHeight() - height;
}


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

void ThemeLayoutVertical::reflowLayout() {
	int curX, curY;
	int resize[8];
	int rescount = 0;
	
	curX = _paddingLeft;
	curY = _paddingTop;
	_h = _paddingTop + _paddingBottom;
	
	for (uint i = 0; i < _children.size(); ++i) {
	
		_children[i]->resetLayout();
		_children[i]->reflowLayout();

		if (_children[i]->getWidth() == -1)
			_children[i]->setWidth((_w == -1 ? getParentW() : _w) - _paddingLeft - _paddingRight);
			
		if (_children[i]->getHeight() == -1) {
			resize[rescount++] = i;
			_children[i]->setHeight(0);
		}
			
		_children[i]->setY(curY);
		
		if (_centered && _children[i]->getWidth() < _w && _w != -1) {
			_children[i]->setX((_w >> 1) - (_children[i]->getWidth() >> 1));
		}
		else
			_children[i]->setX(curX);

		curY += _children[i]->getHeight() + _spacing;	
		_w = MAX(_w, (int16)(_children[i]->getWidth() + _paddingLeft + _paddingRight));
		_h += _children[i]->getHeight() + _spacing;
	}
	
	_h -= _spacing;
	
	if (rescount) {
		int newh = (getParentH() - _h - _paddingBottom) / rescount;
		
		for (int i = 0; i < rescount; ++i) {
			_children[resize[i]]->setHeight(newh);
			_h += newh;
			for (uint j = resize[i] + 1; j < _children.size(); ++j)
				_children[j]->setY(newh);
		}
	}
}

void ThemeLayoutHorizontal::reflowLayout() {
	int curX, curY;
	int resize[8];
	int rescount = 0;

	curX = _paddingLeft;
	curY = _paddingTop;
	_w = _paddingLeft + _paddingRight;
		
	for (uint i = 0; i < _children.size(); ++i) {
	
		_children[i]->resetLayout();
		_children[i]->reflowLayout();
	
		if (_children[i]->getHeight() == -1)
			_children[i]->setHeight((_h == -1 ? getParentH() : _h) - _paddingTop - _paddingBottom);

		if (_children[i]->getWidth() == -1) {
			resize[rescount++] = i;
			_children[i]->setWidth(0);
		}
			
		_children[i]->setX(curX);
		
		if (_centered && _children[i]->getHeight() < _h && _h != -1)
			_children[i]->setY((_h >> 1) - (_children[i]->getHeight() >> 1));
		else
			_children[i]->setY(curY);
			
		curX += (_children[i]->getWidth() + _spacing);
		_w += _children[i]->getWidth() + _spacing;
		_h = MAX(_h, (int16)(_children[i]->getHeight() + _paddingTop + _paddingBottom));
	}
	
	_w -= _spacing;
	
	if (rescount) {
		int neww = (getParentW() - _w - _paddingRight) / rescount;
		
		for (int i = 0; i < rescount; ++i) {
			_children[resize[i]]->setWidth(neww);
			_w += neww;
			for (uint j = resize[i] + 1; j < _children.size(); ++j)
				_children[j]->setX(neww);
		}
	}
}


}
