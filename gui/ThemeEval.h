/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef GUI_THEME_EVAL_H
#define GUI_THEME_EVAL_H

#include "common/scummsys.h"
#include "common/hashmap.h"
#include "common/hash-str.h"
#include "common/stack.h"
#include "common/str.h"
#include "common/textconsole.h"
#include "graphics/font.h"

#include "gui/ThemeLayout.h"

namespace GUI {

class ThemeEval {

	typedef Common::HashMap<Common::String, int> VariablesMap;
	typedef Common::HashMap<Common::String, ThemeLayout *> LayoutsMap;

public:
	ThemeEval() : _scaleFactor(1.0f) {
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

	void setScaleFactor(float s) { _scaleFactor = s; }

	void setVar(const Common::String &name, int val) { _vars[name] = val; }

	bool hasVar(const Common::String &name) { return _vars.contains(name) || _builtin.contains(name); }

	ThemeEval &addDialog(const Common::String &name, const Common::String &overlays, int16 maxWidth = -1, int16 maxHeight = -1, int inset = 0);
	ThemeEval &addLayout(ThemeLayout::LayoutType type, int spacing = -1, ThemeLayout::ItemAlign itemAlign = ThemeLayout::kItemAlignStart);
	ThemeEval &addWidget(const Common::String &name, const Common::String &type, int w = -1, int h = -1, Graphics::TextAlign align = Graphics::kTextAlignStart, bool useRTL = true);
	ThemeEval &addImportedLayout(const Common::String &name);
	ThemeEval &addSpace(int size = -1);

	ThemeEval &addPadding(int16 l, int16 r, int16 t, int16 b);

	ThemeEval &closeLayout() { _curLayout.pop(); return *this; }
	ThemeEval &closeDialog() { _curLayout.pop(); _curDialog.clear(); return *this; }

	bool hasDialog(const Common::String &name);

	void reflowDialogLayout(const Common::String &name, Widget *widgetChain);
	bool getWidgetData(const Common::String &widget, int16 &x, int16 &y, int16 &w, int16 &h);
	bool getWidgetData(const Common::String &widget, int16 &x, int16 &y, int16 &w, int16 &h, bool &useRTL);

	Graphics::TextAlign getWidgetTextHAlign(const Common::String &widget);

#ifdef LAYOUT_DEBUG_DIALOG
	void debugDraw(Graphics::ManagedSurface *screen, const Graphics::Font *font) {
		if (_layouts.contains(LAYOUT_DEBUG_DIALOG)) {
			_layouts[LAYOUT_DEBUG_DIALOG]->debugDraw(screen, font);
		} else {
			Common::String list;

			for (auto l = _layouts.begin(); l != _layouts.end(); ++l)
				list += " " + l->_key;

			warning("debugDraw: Unknown layout %s\nList:%s", LAYOUT_DEBUG_DIALOG, list.c_str());
		}
	}
#endif

	void reset();

private:
	VariablesMap _vars;
	VariablesMap _builtin;

	LayoutsMap _layouts;
	Common::Stack<ThemeLayout *> _curLayout;
	Common::String _curDialog;

	float _scaleFactor;
};

} // End of namespace GUI

#endif
