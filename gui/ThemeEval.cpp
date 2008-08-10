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

#include "gui/ThemeRenderer.h"
#include "gui/ThemeParser.h"
#include "gui/ThemeEval.h"

namespace GUI {
	
bool ThemeLayoutWidget::getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) {
	if (name == _name) {
		x = _x; y = _y;
		w = _w; h = _h;
		return true;
	}
	
	return false;
}

bool ThemeLayout::getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) {
	for (uint i = 0; i < _children.size(); ++i) {
		if (_children[i]->getWidgetData(name, x, y, w, h))
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
		
//		_children[0]->setX(_x);
//		_children[0]->setY(_y);
		
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
	int autoWidget = -1;
	
	curX = _paddingLeft;
	curY = _paddingTop;
	_h = _paddingTop + _paddingBottom;
	
	for (uint i = 0; i < _children.size(); ++i) {
	
		_children[i]->resetLayout();
		_children[i]->reflowLayout();

		if (_children[i]->getWidth() == -1)
			_children[i]->setWidth((_w == -1 ? getParentW() : _w) - _paddingLeft - _paddingRight);
			
		if (_children[i]->getHeight() == -1) {
			if (autoWidget != -1)
				error("Cannot expand automatically two different widgets.");
				
			autoWidget = i;
			_children[i]->setHeight(getParentH() - _h - _spacing);
		}
			
		_children[i]->setY(curY);
		
		if (_centered && _children[i]->getWidth() < _w)
			_children[i]->setX((_w >> 1) - (_children[i]->getWidth() >> 1));
		else
			_children[i]->setX(curX);

		if (_reverse) {
			for (int j = i - 1; j >= 0; --j)
				_children[j]->setY(_children[i]->getHeight() + _spacing);
		} else {
			curY += _children[i]->getHeight() + _spacing;
		}
		
		_w = MAX(_w, (int16)(_children[i]->getWidth() + _paddingLeft + _paddingRight));
		
		if (autoWidget != -1 && autoWidget != (int)i) {
			_children[autoWidget]->setHeight(_children[autoWidget]->getHeight() - (_children[i]->getHeight() + _spacing));
			
			if (_reverse) for (int j = autoWidget - 1; j >= 0; --j)
				_children[j]->setY(-(_children[i]->getHeight() + _spacing));
			else
				_children[i]->setY(-2 * (_children[i]->getHeight() + _spacing));
		} else {
			_h += _children[i]->getHeight() + _spacing;
		}
	}
}

void ThemeLayoutHorizontal::reflowLayout() {
	int curX, curY;
	int autoWidget = -1;

	curX = _paddingLeft;
	curY = _paddingTop;
	_w = _paddingLeft + _paddingRight;
		
	for (uint i = 0; i < _children.size(); ++i) {
	
		_children[i]->resetLayout();
		_children[i]->reflowLayout();
	
		if (_children[i]->getHeight() == -1)
			_children[i]->setHeight((_h == -1 ? getParentH() : _h) - _paddingTop - _paddingBottom);

		if (_children[i]->getWidth() == -1) {
			if (autoWidget != -1)
				error("Cannot expand automatically two different widgets.");
				
			autoWidget = i;
			_children[i]->setWidth(getParentW() - _w - _spacing);
		}
			
		_children[i]->setX(curX);
		
		if (_centered && _children[i]->getHeight() < _h)
			_children[i]->setY((_h >> 1) - (_children[i]->getHeight() >> 1));
		else
			_children[i]->setY(curY);
			
		if (_reverse) {
			for (int j = i - 1; j >= 0; --j)
				_children[j]->setX(_children[i]->getWidth() + _spacing);
		} else {
			curX += (_children[i]->getWidth() + _spacing);
		}

		if (autoWidget != -1 && autoWidget != (int)i) {
			_children[autoWidget]->setWidth(_children[autoWidget]->getWidth() - (_children[i]->getWidth() + _spacing));
			
			if (_reverse) for (int j = autoWidget - 1; j >= 0; --j)
				_children[j]->setX(-(_children[i]->getWidth() + _spacing));
			else
				_children[i]->setX(-2 * (_children[i]->getWidth() + _spacing));
		} else {
			_w += _children[i]->getWidth() + _spacing;
		}
		
		_h = MAX(_h, (int16)(_children[i]->getHeight() + _paddingTop + _paddingBottom));
	}
}

ThemeEval::~ThemeEval() {
	reset();
}

void ThemeEval::buildBuiltinVars() {
	_builtin["kThumbnailWidth"] = kThumbnailWidth;
	_builtin["kThumbnailHeight"] = kThumbnailHeight1;
	_builtin["kThumbnailHeight2"] = kThumbnailHeight2;
	
	_builtin["kButtonWidth"] = GUI::kButtonWidth;
	_builtin["kButtonHeight"] = GUI::kButtonHeight;
	_builtin["kSliderWidth"] = GUI::kSliderWidth;
	_builtin["kSliderHeight"] = GUI::kSliderHeight;
	_builtin["kBigButtonWidth"] = GUI::kBigButtonWidth;
	_builtin["kBigButtonHeight"] = GUI::kBigButtonHeight;
	_builtin["kBigSliderWidth"] = GUI::kBigSliderWidth;
	_builtin["kBigSliderWidth"] = GUI::kBigSliderWidth;
	_builtin["kBigSliderHeight"] = GUI::kBigSliderHeight;
	
	_builtin["kNormalWidgetSize"] = GUI::kNormalWidgetSize;
	_builtin["kBigWidgetSize"] = GUI::kBigWidgetSize;
}


void ThemeEval::addWidget(const Common::String &name, int w, int h, const Common::String &type, bool enabled) {	
	int typeW = -1;
	int typeH = -1;
	
	if (!type.empty()) {
		typeW = getVar("Globals." + type + ".Width", -1);
		typeH = getVar("Globals." + type + ".Height", -1);
	}
	
	ThemeLayoutWidget *widget = new ThemeLayoutWidget(_curLayout.top(), name, 
								typeW == -1 ? w : typeW, 
								typeH == -1 ? h : typeH);
	
	_curLayout.top()->addChild(widget);
	setVar(_curDialog + "." + name + ".Enabled", enabled ? 1 : 0);
}

void ThemeEval::addDialog(const Common::String &name, const Common::String &overlays, bool enabled) {
	int16 x, y;
	uint16 w, h;
	
	ThemeLayout *layout = 0;
	
	if (overlays == "screen") {
		layout = new ThemeLayoutMain(0, 0, g_system->getOverlayWidth(), g_system->getOverlayHeight());
	} else if (overlays == "screen_center") {
		layout = new ThemeLayoutMain(-1, -1, -1, -1);
	} else if (getWidgetData(overlays, x, y, w, h)) {
		layout = new ThemeLayoutMain(x, y, w, h);
	}
	
	if (!layout)
		error("Error when loading dialog position for '%s'", overlays.c_str());
		
	if (_layouts.contains(name))
		delete _layouts[name];
	
	_layouts[name] = layout;

	layout->setPadding(
		getVar("Globals.Padding.Left", 0),
		getVar("Globals.Padding.Right", 0),
		getVar("Globals.Padding.Top", 0),
		getVar("Globals.Padding.Bottom", 0)
		);
	
	_curLayout.push(layout);
	_curDialog = name;
	setVar(name + ".Enabled", enabled ? 1 : 0);
}

void ThemeEval::addLayout(ThemeLayout::LayoutType type, int spacing, bool reverse, bool center) {
	ThemeLayout *layout = 0;
	
	if (spacing == -1)
		spacing = getVar("Globals.Layout.Spacing", 4);
	
	if (type == ThemeLayout::kLayoutVertical)
		layout = new ThemeLayoutVertical(_curLayout.top(), spacing, reverse, center);
	else if (type == ThemeLayout::kLayoutHorizontal)
		layout = new ThemeLayoutHorizontal(_curLayout.top(), spacing, reverse, center);
	
	layout->setPadding(
		getVar("Globals.Padding.Left", 0),
		getVar("Globals.Padding.Right", 0),
		getVar("Globals.Padding.Top", 0),
		getVar("Globals.Padding.Bottom", 0)
		);
	
	_curLayout.top()->addChild(layout);
	_curLayout.push(layout);
}

void ThemeEval::addSpace(int size) {
	ThemeLayout *space = new ThemeLayoutSpacing(_curLayout.top(), size);
	_curLayout.top()->addChild(space);
}

bool ThemeEval::addImportedLayout(const Common::String &name) {
	if (!_layouts.contains(name))
		return false;
		
	_curLayout.top()->importLayout(_layouts[name]);
	return true;
}

}
