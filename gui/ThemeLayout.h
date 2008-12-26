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

#ifndef THEME_LAYOUT_H
#define THEME_LAYOUT_H

#ifdef LAYOUT_DEBUG_DIALOG
namespace Graphics {
	class Font;
	class Surface;
}
#endif

namespace GUI {

class ThemeLayout {
	friend class ThemeLayoutMain;
	friend class ThemeLayoutVertical;
	friend class ThemeLayoutHorizontal;
	friend class ThemeLayoutSpacing;
	friend class ThemeLayoutWidget;
public:
	enum LayoutType {
		kLayoutMain,
		kLayoutVertical,
		kLayoutHorizontal,
		kLayoutWidget
	};

	ThemeLayout(ThemeLayout *p) :
		_parent(p), _x(0), _y(0), _w(-1), _h(-1),
		_paddingLeft(0), _paddingRight(0), _paddingTop(0), _paddingBottom(0),
		_centered(false), _defaultW(-1), _defaultH(-1) { }

	virtual ~ThemeLayout() {
		for (uint i = 0; i < _children.size(); ++i)
			delete _children[i];
	}

	virtual void reflowLayout() = 0;

	virtual void resetLayout() { _x = 0; _y = 0; _w = _defaultW; _h = _defaultH; }

	void addChild(ThemeLayout *child) { _children.push_back(child); }

	void setPadding(int8 left, int8 right, int8 top, int8 bottom) {
		_paddingLeft = left;
		_paddingRight = right;
		_paddingTop = top;
		_paddingBottom = bottom;
	}

	void setSpacing(int8 spacing) {
		_spacing = spacing;
	}

protected:
	int16 getParentW();
	int16 getParentH();
	int16 getWidth() { return _w; }
	int16 getHeight() { return _h; }

	void setX(int newX) {
		_x += newX;
		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->setX(newX);
	}

	void setY(int newY) {
		_y += newY;
		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->setY(newY);
	}

	void setWidth(int16 width) { _w = width; }
	void setHeight(int16 height) { _h = height; }

	virtual LayoutType getLayoutType() = 0;

	virtual ThemeLayout *makeClone(ThemeLayout *newParent) = 0;

public:
	virtual bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h);

	virtual bool getDialogData(int16 &x, int16 &y, uint16 &w, uint16 &h) {
		assert(getLayoutType() == kLayoutMain);
		x = _x; y = _y;
		w = _w; h = _h;
		return true;
	}

	void importLayout(ThemeLayout *layout);

#ifdef LAYOUT_DEBUG_DIALOG
	void debugDraw(Graphics::Surface *screen, const Graphics::Font *font);

	virtual const char *getName() const = 0;
#endif

protected:
	ThemeLayout *_parent;
	int16 _x, _y, _w, _h;
	int8 _paddingLeft, _paddingRight, _paddingTop, _paddingBottom;
	int8 _spacing;
	Common::Array<ThemeLayout *> _children;
	bool _centered;
	int16 _defaultW, _defaultH;
};

class ThemeLayoutMain : public ThemeLayout {
public:
	ThemeLayoutMain(int16 x, int16 y, int16 w, int16 h) : ThemeLayout(0) {
		_w = _defaultW = w;
		_h = _defaultH = h;
		_x = _defaultX = x;
		_y = _defaultY = y;
	}
	void reflowLayout();

	void resetLayout() {
		ThemeLayout::resetLayout();
		_x = _defaultX;
		_y = _defaultY;
	}

#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const { return "Global Layout"; }
#endif
	LayoutType getLayoutType() { return kLayoutMain; }

	ThemeLayout *makeClone(ThemeLayout *newParent) { assert(!"Do not copy Main Layouts!"); return 0; }

protected:
	int16 _defaultX;
	int16 _defaultY;
};

class ThemeLayoutVertical : public ThemeLayout {
public:
	ThemeLayoutVertical(ThemeLayout *p, int spacing, bool center) :
		ThemeLayout(p) {
		_spacing = spacing;
		_centered = center;
	}

	void reflowLayout();
#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const { return "Vertical Layout"; }
#endif
	LayoutType getLayoutType() { return kLayoutVertical; }


	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayoutVertical *n = new ThemeLayoutVertical(*this);
		n->_parent = newParent;

		for (uint i = 0; i < n->_children.size(); ++i)
			n->_children[i] = n->_children[i]->makeClone(n);

		return n;
	}
};

class ThemeLayoutHorizontal : public ThemeLayout {
public:
	ThemeLayoutHorizontal(ThemeLayout *p, int spacing, bool center) :
		ThemeLayout(p) {
		_spacing = spacing;
		_centered = center;
	}

	void reflowLayout();
#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const { return "Horizontal Layout"; }
#endif
	LayoutType getLayoutType() { return kLayoutHorizontal; }

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayoutHorizontal *n = new ThemeLayoutHorizontal(*this);
		n->_parent = newParent;

		for (uint i = 0; i < n->_children.size(); ++ i)
			n->_children[i] = n->_children[i]->makeClone(n);

		return n;
	}
};

class ThemeLayoutWidget : public ThemeLayout {
public:
	ThemeLayoutWidget(ThemeLayout *p, const Common::String &name, int16 w, int16 h) : ThemeLayout(p), _name(name) {
		_w = _defaultW = w;
		_h = _defaultH = h;
	}

	bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h);
	void reflowLayout() {}
#ifdef LAYOUT_DEBUG_DIALOG
	virtual const char *getName() const { return _name.c_str(); }
#endif
	LayoutType getLayoutType() { return kLayoutWidget; }

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayout *n = new ThemeLayoutWidget(*this);
		n->_parent = newParent;
		return n;
	}

protected:
	Common::String _name;
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
	void reflowLayout() {}
	LayoutType getLayoutType() { return kLayoutWidget; }
#ifdef LAYOUT_DEBUG_DIALOG
	const char *getName() const { return "SPACE"; }
#endif

	ThemeLayout *makeClone(ThemeLayout *newParent) {
		ThemeLayout *n = new ThemeLayoutSpacing(*this);
		n->_parent = newParent;
		return n;
	}
};

}

#endif
