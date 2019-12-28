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

#ifndef THEME_LAYOUT_H
#define THEME_LAYOUT_H

#include "common/array.h"
#include "common/rect.h"
#include "graphics/font.h"

#ifdef LAYOUT_DEBUG_DIALOG
namespace Graphics {
struct Surface;
}
#endif

namespace GUI {

class Widget;

class ThemeLayout {
	friend class ThemeLayoutMain;
	friend class ThemeLayoutStacked;
	friend class ThemeLayoutSpacing;
	friend class ThemeLayoutWidget;
public:
	enum LayoutType {
		kLayoutMain,
		kLayoutVertical,
		kLayoutHorizontal,
		kLayoutWidget,
		kLayoutTabWidget,
		kLayoutSpace
	};

	ThemeLayout(ThemeLayout *p) :
		_parent(p), _x(0), _y(0), _w(-1), _h(-1),
		_defaultW(-1), _defaultH(-1),
		_textHAlign(Graphics::kTextAlignInvalid) {}

	virtual ~ThemeLayout() {
		for (uint i = 0; i < _children.size(); ++i)
			delete _children[i];
	}

	virtual void reflowLayout(Widget *widgetChain) = 0;
	virtual void resetLayout();

	void addChild(ThemeLayout *child) { _children.push_back(child); }

	void setPadding(int8 left, int8 right, int8 top, int8 bottom) {
		_padding.left = left;
		_padding.right = right;
		_padding.top = top;
		_padding.bottom = bottom;
	}

protected:
	int16 getWidth() { return _w; }
	int16 getHeight() { return _h; }

	void offsetX(int newX) {
		_x += newX;
		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->offsetX(newX);
	}

	void offsetY(int newY) {
		_y += newY;
		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->offsetY(newY);
	}

	void setWidth(int16 width) { _w = width; }
	void setHeight(int16 height) { _h = height; }
	void setTextHAlign(Graphics::TextAlign align) { _textHAlign = align; }

	/**
	 * Checks if the layout element is attached to a GUI widget
	 *
	 * Layout elements that are not bound do not take space.
	 */
	virtual bool isBound(Widget *widgetChain) const { return true; }

	virtual LayoutType getLayoutType() const = 0;

	virtual ThemeLayout *makeClone(ThemeLayout *newParent) = 0;

public:
	virtual bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h);

	virtual Graphics::TextAlign getWidgetTextHAlign(const Common::String &name);

	void importLayout(ThemeLayout *layout);

	Graphics::TextAlign getTextHAlign() { return _textHAlign; }

#ifdef LAYOUT_DEBUG_DIALOG
	void debugDraw(Graphics::Surface *screen, const Graphics::Font *font);

	virtual const char *getName() const = 0;
#endif

protected:
	ThemeLayout *_parent;
	int16 _x, _y, _w, _h;
	Common::Rect _padding;
	Common::Array<ThemeLayout *> _children;
	int16 _defaultW, _defaultH;
	Graphics::TextAlign _textHAlign;
};

class ThemeLayoutMain : public ThemeLayout {
public:
	ThemeLayoutMain(const Common::String &name, const Common::String &overlays, bool enabled, int inset) :
			ThemeLayout(nullptr),
			_name(name),
			_overlays(overlays),
			_enabled(enabled),
			_inset(inset) {
		_w = _defaultW = -1;
		_h = _defaultH = -1;
		_x = _defaultX = -1;
		_y = _defaultY = -1;
	}
	void reflowLayout(Widget *widgetChain) override;

	void resetLayout() {
		ThemeLayout::resetLayout();
		_x = _defaultX;
		_y = _defaultY;
	}

	const char *getName() const { return _name.c_str(); }

protected:
	LayoutType getLayoutType() const override { return kLayoutMain; }
	ThemeLayout *makeClone(ThemeLayout *newParent) { assert(!"Do not copy Main Layouts!"); return 0; }

	int16 _defaultX;
	int16 _defaultY;

	Common::String _name;
	Common::String _overlays;
	bool _enabled;
	int _inset;
};

class ThemeLayoutStacked : public ThemeLayout {
public:
	ThemeLayoutStacked(ThemeLayout *p, LayoutType type, int spacing, bool center) :
		ThemeLayout(p), _type(type), _centered(center) {
		assert((type == kLayoutVertical) || (type == kLayoutHorizontal));
		_spacing = spacing;
		_centered = center;
	}

	void reflowLayout(Widget *widgetChain) override {
		if (_type == kLayoutVertical)
			reflowLayoutVertical(widgetChain);
		else
			reflowLayoutHorizontal(widgetChain);
	}

	void reflowLayoutHorizontal(Widget *widgetChain);
	void reflowLayoutVertical(Widget *widgetChain);

#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const {
		return (_type == kLayoutVertical)
			? "Vertical Layout" : "Horizontal Layout";
	}
#endif

protected:
	int16 getParentWidth();
	int16 getParentHeight();

	LayoutType getLayoutType() const override { return _type; }

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayoutStacked *n = new ThemeLayoutStacked(*this);
		n->_parent = newParent;

		for (uint i = 0; i < n->_children.size(); ++i)
			n->_children[i] = n->_children[i]->makeClone(n);

		return n;
	}

	const LayoutType _type;
	bool _centered;
	int8 _spacing;
};

class ThemeLayoutWidget : public ThemeLayout {
public:
	ThemeLayoutWidget(ThemeLayout *p, const Common::String &name, int16 w, int16 h, Graphics::TextAlign align) : ThemeLayout(p), _name(name) {
		_w = _defaultW = w;
		_h = _defaultH = h;

		setTextHAlign(align);
	}

	bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h);
	Graphics::TextAlign getWidgetTextHAlign(const Common::String &name);

	void reflowLayout(Widget *widgetChain) override;

	virtual const char *getName() const { return _name.c_str(); }

protected:
	LayoutType getLayoutType() const override { return kLayoutWidget; }

	bool isBound(Widget *widgetChain) const override;
	Widget *getWidget(Widget *widgetChain) const;

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayout *n = new ThemeLayoutWidget(*this);
		n->_parent = newParent;
		return n;
	}

	Common::String _name;
};

class ThemeLayoutTabWidget : public ThemeLayoutWidget {
	int _tabHeight;

public:
	ThemeLayoutTabWidget(ThemeLayout *p, const Common::String &name, int16 w, int16 h, Graphics::TextAlign align, int tabHeight):
		ThemeLayoutWidget(p, name, w, h, align) {
		_tabHeight = tabHeight;
	}

	void reflowLayout(Widget *widgetChain) override {
		for (uint i = 0; i < _children.size(); ++i) {
			_children[i]->reflowLayout(widgetChain);
		}
	}

	virtual bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) {
		if (ThemeLayoutWidget::getWidgetData(name, x, y, w, h)) {
			h -= _tabHeight;
			return true;
		}

		return false;
	}

protected:
	LayoutType getLayoutType() const override { return kLayoutTabWidget; }

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayoutTabWidget *n = new ThemeLayoutTabWidget(*this);
		n->_parent = newParent;
		return n;
	}
};

class ThemeLayoutSpacing : public ThemeLayout {
public:
	ThemeLayoutSpacing(ThemeLayout *p, int size) : ThemeLayout(p) {
		if (p->getLayoutType() == kLayoutHorizontal) {
			_w = _defaultW = size;
			_h = _defaultH = 1;
		} else if (p->getLayoutType() == kLayoutVertical) {
			_w = _defaultW = 1;
			_h = _defaultH = size;
		}
	}

	bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h) { return false; }
	void reflowLayout(Widget *widgetChain) override {}
#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const { return "SPACE"; }
#endif

protected:
	LayoutType getLayoutType() const override { return kLayoutSpace; }

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayout *n = new ThemeLayoutSpacing(*this);
		n->_parent = newParent;
		return n;
	}
};

}

#endif
