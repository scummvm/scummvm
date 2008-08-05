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

#include "gui/ThemeRenderer.h"
#include "gui/ThemeParser.h"
#include "gui/ThemeEval.h"

namespace GUI {
	
bool ThemeLayoutWidget::getWidgetData(const Common::String &name, int16 &x, int16 &y, int16 &w, int16 &h) {
	if (name == _name) {
		x = _x; y = _y;
		w = _w; h = _h;
		return true;
	}
	
	return false;
}

bool ThemeLayout::getWidgetData(const Common::String &name, int16 &x, int16 &y, int16 &w, int16 &h) {
	for (uint i = 0; i < _children.size(); ++i) {
		if (_children[i]->getWidgetData(name, x, y, w, h))
			return true;
	}
	
	return false;
}

void ThemeLayoutMain::reflowLayout() {
	assert(_children.size() <= 1);
	
	if (_children.size()) {
		_children[0]->setWidth(_w);
		_children[0]->setHeight(_h);
		_children[0]->reflowLayout();
	}
}

void ThemeLayoutVertical::reflowLayout() {
	int curX, curY;
	
	curX = _paddingLeft;
	curY = _paddingTop;
	_h = _paddingTop + _paddingBottom;
	
	for (uint i = 0; i < _children.size(); ++i) {
		assert(_children[i]->getLayoutType() != kLayoutVertical);
	
		_children[i]->reflowLayout();
	
		if (i != _children.size() - 1)
			assert(_children[i]->getHeight() != -1);
		
		if (i == 0)
			assert(_children[i]->getWidth() != -1);
			
		_children[i]->setX(curX);
		_children[i]->setY(curY);
	
		if (_children[i]->getWidth() == -1)
			_children[i]->setWidth(_w - _paddingLeft - _paddingRight);
			
		if (_children[i]->getHeight() == -1)
			_children[i]->setHeight(getParentH() - _h - _spacing);

		if (_reverse) {
			for (int j = i - 1; j >= 0; --j)
				_children[j]->setY(_children[i]->getHeight() + _spacing);
		} else {
			curY += _children[i]->getHeight() + _spacing;
		}
		
		_w = MAX(_w, (int16)(_children[i]->getWidth() + _paddingLeft + _paddingRight));
		_h += _children[i]->getHeight() + _spacing;
	}
}

void ThemeLayoutHorizontal::reflowLayout() {
	int curX, curY;

	curX = _paddingLeft;
	curY = _paddingTop;
	_w = _paddingLeft + _paddingRight;
		
	for (uint i = 0; i < _children.size(); ++i) {
		assert(_children[i]->getLayoutType() != kLayoutHorizontal);
	
		_children[i]->reflowLayout();
	
		if (i != _children.size() - 1)
			assert(_children[i]->getWidth() != -1);
			
		if (i == 0)
			assert(_children[i]->getHeight() != -1);
		
		_children[i]->setX(curX);
		_children[i]->setY(curY);
	
		if (_children[i]->getHeight() == -1)
			_children[i]->setHeight(_h - _paddingTop - _paddingBottom);

		if (_children[i]->getWidth() == -1)
			_children[i]->setWidth(getParentW() - _w - _spacing);
			
		if (_reverse) {
			for (int j = i - 1; j >= 0; --j)
				_children[j]->setX(_children[i]->getWidth() + _spacing);
		} else {
			curX += (_children[i]->getWidth() + _spacing);
		}

		_w += _children[i]->getWidth() + _spacing;
		_h = MAX(_h, (int16)(_children[i]->getHeight() + _paddingTop + _paddingBottom));
	}
}


void ThemeEval::addWidget(const Common::String &name, int w, int h) {
	ThemeLayoutWidget *widget = new ThemeLayoutWidget(_curLayout.top(), name);
	
	widget->setWidth(w);
	widget->setHeight(h);
	
	_curLayout.top()->addChild(widget);
}

void ThemeEval::addDialog(const Common::String &name) {
	ThemeLayout *layout = new ThemeLayoutMain();
	_layouts[name] = layout;
	
	layout->setX(0);
	layout->setY(0);
	layout->setWidth(g_system->getOverlayWidth());
	layout->setHeight(g_system->getOverlayHeight());
	
	layout->setPadding(
		getVar("Globals.Padding.Left", 0),
		getVar("Globals.Padding.Right", 0),
		getVar("Globals.Padding.Top", 0),
		getVar("Globals.Padding.Bottom", 0)
		);
	
	_curLayout.push(layout);
}

void ThemeEval::addLayout(ThemeLayout::LayoutType type, bool reverse) {
	ThemeLayout *layout = 0;
	
	if (type == ThemeLayout::kLayoutVertical)
		layout = new ThemeLayoutVertical(_curLayout.top(), getVar("Globals.Layout.Spacing", 4), reverse);
	else if (type == ThemeLayout::kLayoutHorizontal)
		layout = new ThemeLayoutHorizontal(_curLayout.top(), getVar("Globals.Layout.Spacing", 4), reverse);
	
	layout->setPadding(
		getVar("Globals.Padding.Left", 0),
		getVar("Globals.Padding.Right", 0),
		getVar("Globals.Padding.Top", 0),
		getVar("Globals.Padding.Bottom", 0)
		);
	
	layout->setSpacing(4);
	
	_curLayout.top()->addChild(layout);
	_curLayout.push(layout);
}

void ThemeEval::addSpacing(int size) {
	ThemeLayout *space = new ThemeLayoutSpacing(_curLayout.top(), size);
	_curLayout.top()->addChild(space);
}

}
