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

#ifndef GUI_THEME_EVAL
#define GUI_THEME_EVAL

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
	
class ThemeLayout {

public:
	enum LayoutType {
		kLayoutMain,
		kLayoutVertical,
		kLayoutHorizontal,
		kLayoutWidget
	};
	
	ThemeLayout(ThemeLayout *p, const Common::String &name) : 
		_parent(p), _name(name), _x(0), _y(0), _w(-1), _h(-1), _reverse(false),
		_paddingLeft(0), _paddingRight(0), _paddingTop(0), _paddingBottom(0), 
		_centered(false), _defaultW(-1), _defaultH(-1) { }
		
	virtual ~ThemeLayout() {
		for (uint i = 0; i < _children.size(); ++i)
			delete _children[i];
	}
		
	virtual void reflowLayout() = 0;
	
	virtual void resetLayout() {
		_x = 0;
		_y = 0;
		_w = _defaultW;
		_h = _defaultH;
	}
	
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

	int16 getParentX() { return _parent ? _parent->_x : 0; }
	int16 getParentY() { return _parent ? _parent->_y : 0; }
	
	int16 getParentW() {
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
	
	int16 getParentH() {
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
	
	int16 getX() { return _x; }
	int16 getY() { return _y; }
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
	
	void debugDraw(Graphics::Surface *screen, const Graphics::Font *font) {
		uint16 color = 0xFFFF;
		font->drawString(screen, getName(), _x, _y, _w, color, Graphics::kTextAlignRight, 0, true);
		screen->hLine(_x, _y, _x + _w, color);
		screen->hLine(_x, _y + _h, _x + _w , color);
		screen->vLine(_x, _y, _y + _h, color);
		screen->vLine(_x + _w, _y, _y + _h, color);

		for (uint i = 0; i < _children.size(); ++i)
			_children[i]->debugDraw(screen, font);
	}
	
	virtual LayoutType getLayoutType() = 0;
	virtual const char *getName() { return _name.c_str(); }
	
	virtual bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h);

	virtual bool getDialogData(int16 &x, int16 &y, uint16 &w, uint16 &h) {
		assert(getLayoutType() == kLayoutMain);
		x = _x; y = _y;
		w = _w; h = _h;
		return true;
	}
	
	virtual ThemeLayout *buildCopy() = 0;
	
	void importLayout(ThemeLayout *layout) {
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
	
protected:
	int16 _x, _y, _w, _h;
	int16 _defaultW, _defaultH;
	int8 _paddingTop, _paddingBottom, _paddingLeft, _paddingRight;
	int8 _spacing;
	Common::Array<ThemeLayout*> _children;
	ThemeLayout *_parent;
	bool _reverse;
	bool _centered;
	Common::String _name;
};

class ThemeLayoutMain : public ThemeLayout {
public:
	ThemeLayoutMain(int16 x, int16 y, int16 w, int16 h) : ThemeLayout(0, "") {
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
	
	const char *getName() { return "Global Layout"; }
	LayoutType getLayoutType() { return kLayoutMain; }
	
	ThemeLayout *buildCopy() { assert(!"Do not copy Main Layouts!"); return 0; }
	
protected:
	int16 _defaultX;
	int16 _defaultY;
};	

class ThemeLayoutVertical : public ThemeLayout {
public:
	ThemeLayoutVertical(ThemeLayout *p, int spacing, bool reverse, bool center) :
	 	ThemeLayout(p, "") {
		_spacing = spacing;
		_reverse = reverse;
		_centered = center;
	}
		
	void reflowLayout();
	const char *getName() { return "Vertical Layout"; }
	LayoutType getLayoutType() { return kLayoutVertical; }
	
	
	ThemeLayout *buildCopy() { 
		ThemeLayoutVertical *n = new ThemeLayoutVertical(*this);
		
		for (uint i = 0; i < n->_children.size(); ++ i)
			n->_children[i] = n->_children[i]->buildCopy();
		
		return n;
	}
};

class ThemeLayoutHorizontal : public ThemeLayout {
public:
	ThemeLayoutHorizontal(ThemeLayout *p, int spacing, bool reverse, bool center) : 
		ThemeLayout(p, "") {
		_spacing = spacing;
		_reverse = reverse;
		_centered = center;
	}
		
	void reflowLayout();
	const char *getName() { return "Horizontal Layout"; }
	LayoutType getLayoutType() { return kLayoutHorizontal; }
	
	ThemeLayout *buildCopy() { 
		ThemeLayoutHorizontal *n = new ThemeLayoutHorizontal(*this);
		
		for (uint i = 0; i < n->_children.size(); ++ i)
			n->_children[i] = n->_children[i]->buildCopy();
		
		return n;
	}
};

class ThemeLayoutWidget : public ThemeLayout {
public:
	ThemeLayoutWidget(ThemeLayout *p, const Common::String &name, int16 w, int16 h) : ThemeLayout(p, name) {
		_w = _defaultW = w;
		_h = _defaultH = h;
	}
	
	bool getWidgetData(const Common::String &name, int16 &x, int16 &y, uint16 &w, uint16 &h);
	void reflowLayout() {}
	LayoutType getLayoutType() { return kLayoutWidget; }
	
	ThemeLayout *buildCopy() { return new ThemeLayoutWidget(*this); }
};

class ThemeLayoutSpacing : public ThemeLayout {
public:
	ThemeLayoutSpacing(ThemeLayout *p, int size) : ThemeLayout(p, "") {
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
	const char *getName() { return "SPACE"; }
	
	ThemeLayout *buildCopy() { return new ThemeLayoutSpacing(*this); }
};
	
class ThemeEval {

	typedef Common::HashMap<Common::String, int> VariablesMap;
	typedef Common::HashMap<Common::String, ThemeLayout*> LayoutsMap;
	
public:
	ThemeEval() {
		buildBuiltinVars();
	}
	
	~ThemeEval();
	
	void buildBuiltinVars();
	
	int getVar(const Common::String &s) {
		if (_vars.contains(s))
			return _vars[s];
			
		if (_builtin.contains(s))
			return _builtin[s];

		error("CRITICAL: Missing variable: '%s'", s.c_str());
		return -13375; //EVAL_UNDEF_VAR 
	}
	
	int getVar(const Common::String &s, int def) {
		if (_vars.contains(s))
			return _vars[s];
			
		if (_builtin.contains(s))
			return _builtin[s];

		return def;
	}
	
	void setVar(const String &name, int val) { _vars[name] = val; }
	
	bool hasVar(const Common::String &name) { return _vars.contains(name) || _builtin.contains(name); }
	
	void addDialog(const Common::String &name, const Common::String &overlays, bool enabled = true);
	void addLayout(ThemeLayout::LayoutType type, int spacing, bool reverse, bool center = false);
	void addWidget(const Common::String &name, int w, int h, const Common::String &type, bool enabled = true);
	bool addImportedLayout(const Common::String &name);
	void addSpace(int size);
	
	void addPadding(int16 l, int16 r, int16 t, int16 b) {
		_curLayout.top()->setPadding(l, r, t, b);
	}
	
	void closeLayout() { _curLayout.pop(); }
	void closeDialog() { _curLayout.pop()->reflowLayout(); _curDialog.clear(); }
	
	bool getWidgetData(const Common::String &widget, int16 &x, int16 &y, uint16 &w, uint16 &h) {
		Common::StringTokenizer tokenizer(widget, ".");
		
		if (widget.hasPrefix("Dialog."))
			tokenizer.nextToken();
	
		Common::String dialogName = "Dialog." + tokenizer.nextToken();
		Common::String widgetName = tokenizer.nextToken();
		
		if (!_layouts.contains(dialogName)) 
			return false;

		if (widgetName.empty())
			return _layouts[dialogName]->getDialogData(x, y, w, h);
			
		return _layouts[dialogName]->getWidgetData(widgetName, x, y, w, h);
	}
	
	void debugPrint() {
		printf("Debug variable list:\n");
		
		VariablesMap::const_iterator i;
		for (i = _vars.begin(); i != _vars.end(); ++i) {
			printf("  '%s' = %d\n", i->_key.c_str(), i->_value);
		}
	}

	void debugDraw(Graphics::Surface *screen, const Graphics::Font *font) {
		_layouts["Dialog.Browser"]->debugDraw(screen, font);
//		_layouts["Dialog.GameOptions_Graphics"]->debugDraw(screen, font);
	}
	
	void reset() {
		_vars.clear();
		_builtin.clear();
		_curDialog.clear();
		_curLayout.clear();
		
		for (LayoutsMap::iterator i = _layouts.begin(); i != _layouts.end(); ++i)
			delete i->_value;
	}
	
private:
	VariablesMap _vars;
	VariablesMap _builtin;
	
	LayoutsMap _layouts;
	Common::Stack<ThemeLayout*> _curLayout;
	Common::String _curDialog;
};


}

#endif
