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
	int16 x, y, w, h;
	int paddingTop, paddingBottom, paddingLeft, paddingRight;
	int spacing;
	Common::Array<ThemeLayout*> children;
	ThemeLayout *parent;

	uint16 debugcolor;
	
	enum LayoutType {
		kLayoutNone,
		kLayoutVertical,
		kLayoutHorizontal,
		kLayoutWidget
	};
	
	enum LayoutParsing {
		kLayoutParseDefault,
		kLayoutParseTop2Bottom,
		kLayoutParseBottom2Top,
		kLayoutParseLeft2Right,
		kLayoutParseRight2Left
	} parsingMode;
	
	virtual LayoutType getLayoutType() { return kLayoutNone; }
	virtual void reflowLayout() {
		assert(children.size() <= 1);
		
		if (children.size()) {
			children[0]->w = w;
			children[0]->h = h;
			children[0]->reflowLayout();
			children[0]->setX(0);
			children[0]->setY(0);
		}
	}
	
	virtual const char *getName() { return "Global Layout"; }
	
	int16 getParentW() { return parent ? parent->w - parent->paddingLeft - parent->paddingRight : g_system->getOverlayWidth(); }
	int16 getParentH() { return parent ? parent->h - parent->paddingTop - parent->paddingBottom : g_system->getOverlayHeight(); }
	int16 getParentX() { return parent ? parent->x : 0; }
	int16 getParentY() { return parent ? parent->y : 0; }
	
	void setX(int newX) {
		x += newX;
		for (uint i = 0; i < children.size(); ++i)
			children[i]->setX(newX);
	}
	
	void setY(int newY) {
		y += newY;
		for (uint i = 0; i < children.size(); ++i)
			children[i]->setY(newY);
	}
	
	ThemeLayout(ThemeLayout *p) : parent(p), x(0), y(0), w(-1), h(-1) { debugcolor = rand() % 0xFFFF; }
	
	virtual void debugPrintIndent(int indent) {
		while (indent--)
			printf("    ");
	}

	void debugDraw(Graphics::Surface *screen, const Graphics::Font *font) {
		uint16 color = debugcolor;
		font->drawString(screen, getName(), x, y, w, color, Graphics::kTextAlignRight, 0, true);
		screen->hLine(x, y, x + w, color);
		screen->hLine(x, y + h, x + w, color);
		screen->vLine(x, y, y + h, color);
		screen->vLine(x + w, y, y + h, color);

		for (uint i = 0; i < children.size(); ++i)
			children[i]->debugDraw(screen, font);
	}
	
	virtual void debugPrint(int indent = 0) {
		debugPrintIndent(indent);
		switch (getLayoutType()) {
			case kLayoutNone:
				printf("Dialog Layout  ::  ");
				break;
			
			case kLayoutVertical:
				printf("Vertical Layout  ::  ");
				break;
				
			case kLayoutHorizontal:
				printf("Horizontal Layout  ::  ");
				break;
				
			case kLayoutWidget:
				printf("WIDGET (%s)  ::  ", getName());
				break;
		}
	
		printf("X: %d / Y: %d / W: %d / H: %d\n", x, y, w, h);
		
		for (uint i = 0; i < children.size(); ++i)
			children[i]->debugPrint(indent + 1);
	}
	
	virtual ~ThemeLayout() {
		children.clear();
	}
};

class ThemeLayoutVertical : public ThemeLayout {
public:
	LayoutType getLayoutType() { return kLayoutVertical; }
	
	ThemeLayoutVertical(ThemeLayout *p) : ThemeLayout(p) {}

	const char *getName() { return "Vertical Layout"; }
	
	void reflowLayout() {
		int curX, curY, mul;
		
		if (parsingMode == kLayoutParseTop2Bottom) {
			curX = paddingLeft;
			curY = paddingTop;
			mul = 1;
		} else {
			curX = paddingLeft;
			curY = getParentH() - paddingBottom;
			mul = -1;
		}

		h = paddingTop + paddingBottom;
		
		for (uint i = 0; i < children.size(); ++i) {
			assert(children[i]->getLayoutType() != kLayoutVertical);
		
			children[i]->reflowLayout();
		
			if (i != children.size() - 1)
				assert(children[i]->h != -1);
			
			if (i == 0)
				assert(children[i]->w != -1);
				
			children[i]->setX(curX);
			children[i]->setY((parsingMode == kLayoutParseBottom2Top) ? curY - children[i]->h : curY);
		
			if (children[i]->w == -1)
				children[i]->w = w - paddingLeft - paddingRight;
				
			w = MAX(w, (int16)(children[i]->w + paddingLeft + paddingRight));
				
			if (children[i]->h == -1)
				children[i]->h = 32;
			
			h += children[i]->h + spacing;
	
			curY += (children[i]->h + spacing) * mul;
		}
		
		
	}
};

class ThemeLayoutHorizontal : public ThemeLayout {
public:
	LayoutType getLayoutType() { return kLayoutHorizontal; }
	
	ThemeLayoutHorizontal(ThemeLayout *p) : ThemeLayout(p) {}

	const char *getName() { return "Horizontal Layout"; }
	
	void reflowLayout() {
		int curX, curY;
	
		curX = paddingLeft;
		curY = paddingTop;
		w = paddingLeft + paddingRight;
			
		for (uint i = 0; i < children.size(); ++i) {
			assert(children[i]->getLayoutType() != kLayoutHorizontal);
		
			children[i]->reflowLayout();
		
			if (i != children.size() - 1)
				assert(children[i]->w != -1);
				
			if (i == 0)
				assert(children[i]->h != -1);
			
			
			children[i]->setX(curX);
			children[i]->setY(curY);
		
			if (children[i]->h == -1)
				children[i]->h = h - paddingTop - paddingBottom;

			if (children[i]->w == -1)
				children[i]->w = getParentW() - w - spacing;
				
			h = MAX(h, (int16)(children[i]->h + paddingTop + paddingBottom));

			if (parsingMode == kLayoutParseRight2Left) {
				for (int j = i - 1; j >= 0; --j)
					children[j]->setX(children[i]->w + spacing);
			} else {
				curX += (children[i]->w + spacing);
			}

			w += children[i]->w + spacing;
		}
	}
};

class ThemeLayoutWidget : public ThemeLayout {
public:
	LayoutType getLayoutType() { return kLayoutWidget; }
	void reflowLayout() {
		
	}
	ThemeLayoutWidget(ThemeLayout *p, const Common::String &name) : ThemeLayout(p), widgetName(name) {}
	
	const char *getName() { return widgetName.c_str(); }
	
	Common::String widgetName;
};
	
class ThemeEval {

	typedef Common::HashMap<Common::String, int> VariablesMap;
	typedef Common::HashMap<Common::String, ThemeLayout*> LayoutsMap;
	
public:
	ThemeEval() {}
	~ThemeEval() {}
	
	int getVar(const Common::String &s) {
		if (!_vars.contains(s)) {
			warning("Missing variable: '%s'", s.c_str());
			return -1;
		} 
		
		return _vars[s];
	}
	
	int getVar(const Common::String &s, int def) {
		return (_vars.contains(s)) ? _vars[s] : def;
	}
	
	void setVar(const String &name, int val) { _vars[name] = val; }
	
	bool hasVar(const Common::String &name) { return _vars.contains(name); }
	
	void addDialog(const Common::String &name) {
		ThemeLayout *layout = new ThemeLayout(0);
		_layouts[name] = layout;
		
		layout->x = 0;
		layout->y = 0;
		layout->w = g_system->getOverlayWidth();
		layout->h = g_system->getOverlayHeight();

		layout->paddingBottom = getVar("Globals.Padding.Bottom", 0);
		layout->paddingTop = getVar("Globals.Padding.Top", 0);
		layout->paddingRight = getVar("Globals.Padding.Right", 0);
		layout->paddingLeft = getVar("Globals.Padding.Left", 0);
		
		_curLayout.push(layout);
	}
	
	void addLayout(ThemeLayout::LayoutType type, ThemeLayout::LayoutParsing parsingMode) {
		ThemeLayout *layout = 0;
		ThemeLayout::LayoutParsing def = ThemeLayout::kLayoutParseDefault;
		
		if (type == ThemeLayout::kLayoutVertical) {
			layout = new ThemeLayoutVertical(_curLayout.top());
			def = ThemeLayout::kLayoutParseTop2Bottom;
		} else if (type == ThemeLayout::kLayoutHorizontal) {
			layout = new ThemeLayoutHorizontal(_curLayout.top());
			def = ThemeLayout::kLayoutParseLeft2Right;
		}
		
		layout->parsingMode = (parsingMode == ThemeLayout::kLayoutParseDefault) ? def : parsingMode;
		layout->paddingBottom = getVar("Globals.Padding.Bottom", 0);
		layout->paddingTop = getVar("Globals.Padding.Top", 0);
		layout->paddingRight = getVar("Globals.Padding.Right", 0);
		layout->paddingLeft = getVar("Globals.Padding.Left", 0);

		layout->spacing = 4;
		
		_curLayout.top()->children.push_back(layout);
		_curLayout.push(layout);
	}
	
	void closeLayout() {
		_curLayout.pop();
	}
	
	void closeDialog() {
		_curLayout.top()->reflowLayout();
		printf("DEBUG LAYOUT PRINT:\n");
		
		_curLayout.top()->debugPrint();
	}
	
	void addWidget(const Common::String &name, int w, int h) {
		ThemeLayoutWidget *widget = new ThemeLayoutWidget(_curLayout.top(), name);
		
		widget->w = w;
		widget->h = h;
		
		_curLayout.top()->children.push_back(widget);
	}
	
	void debugPrint() {
		printf("Debug variable list:\n");
		
		VariablesMap::const_iterator i;
		for (i = _vars.begin(); i != _vars.end(); ++i) {
			printf("  '%s' = %d\n", i->_key.c_str(), i->_value);
		}
	}

	void debugDraw(Graphics::Surface *screen, const Graphics::Font *font) {
		_curLayout.top()->debugDraw(screen, font);
	}
	
private:
	VariablesMap _vars;
	LayoutsMap _layouts;
	Common::Stack<ThemeLayout*> _curLayout;
};


}

#endif
