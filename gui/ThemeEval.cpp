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

#include "gui/ThemeEval.h"

#include "graphics/scaler.h"

#include "common/system.h"
#include "common/tokenizer.h"

namespace GUI {

ThemeEval::~ThemeEval() {
	reset();
}

void ThemeEval::buildBuiltinVars() {
	_builtin["kThumbnailWidth"] = kThumbnailWidth;
	_builtin["kThumbnailHeight"] = kThumbnailHeight1;
	_builtin["kThumbnailHeight2"] = kThumbnailHeight2;
}

void ThemeEval::reset() {
	_vars.clear();
	_curDialog.clear();
	_curLayout.clear();

	for (LayoutsMap::iterator i = _layouts.begin(); i != _layouts.end(); ++i)
		delete i->_value;

	_layouts.clear();
}

bool ThemeEval::getWidgetData(const Common::String &widget, int16 &x, int16 &y, int16 &w, int16 &h) {
	bool useRTL;

	return getWidgetData(widget, x, y, w, h, useRTL);
}

bool ThemeEval::getWidgetData(const Common::String &widget, int16 &x, int16 &y, int16 &w, int16 &h, bool &useRTL) {
	Common::StringTokenizer tokenizer(widget, ".");

	if (widget.hasPrefix("Dialog."))
		tokenizer.nextToken();

	Common::String dialogName = "Dialog." + tokenizer.nextToken();
	Common::String widgetName = tokenizer.nextToken();

	if (!_layouts.contains(dialogName))
		return false;

	return _layouts[dialogName]->getWidgetData(widgetName, x, y, w, h, useRTL);
}

Graphics::TextAlign ThemeEval::getWidgetTextHAlign(const Common::String &widget) {
	Common::StringTokenizer tokenizer(widget, ".");

	if (widget.hasPrefix("Dialog."))
		tokenizer.nextToken();

	Common::String dialogName = "Dialog." + tokenizer.nextToken();
	Common::String widgetName = tokenizer.nextToken();

	if (!_layouts.contains(dialogName))
		return Graphics::kTextAlignInvalid;

	return _layouts[dialogName]->getWidgetTextHAlign(widgetName);
}

ThemeEval &ThemeEval::addWidget(const Common::String &name, const Common::String &type, int w, int h, Graphics::TextAlign align, bool useRTL) {
	int typeW = -1;
	int typeH = -1;
	Graphics::TextAlign typeAlign = Graphics::kTextAlignInvalid;

	if (!type.empty()) {
		typeW = getVar("Globals." + type + ".Width", -1);
		typeH = getVar("Globals." + type + ".Height", -1);
		typeAlign = (Graphics::TextAlign)getVar("Globals." + type + ".Align", Graphics::kTextAlignInvalid);
	}

	ThemeLayoutWidget *widget;
	if (type == "TabWidget")
		widget = new ThemeLayoutTabWidget(_curLayout.top(), name,
									typeW == -1 ? w : typeW,
									typeH == -1 ? h : typeH,
									typeAlign == Graphics::kTextAlignInvalid ? align : typeAlign,
									getVar("Globals.TabWidget.Tab.Height", 0));
	else
		widget = new ThemeLayoutWidget(_curLayout.top(), name,
									typeW == -1 ? w : typeW,
									typeH == -1 ? h : typeH,
									typeAlign == Graphics::kTextAlignInvalid ? align : typeAlign,
									useRTL);

	_curLayout.top()->addChild(widget);

	return *this;
}

ThemeEval &ThemeEval::addDialog(const Common::String &name, const Common::String &overlays, int16 width, int16 height, int inset) {
	Common::String var = "Dialog." + name;

	ThemeLayout *layout = new ThemeLayoutMain(name, overlays, width, height, inset);

	if (_layouts.contains(var))
		delete _layouts[var];

	_layouts[var] = layout;

	layout->setPadding(
		getVar("Globals.Padding.Left", 0),
		getVar("Globals.Padding.Right", 0),
		getVar("Globals.Padding.Top", 0),
		getVar("Globals.Padding.Bottom", 0)
		);

	_curLayout.push(layout);
	_curDialog = name;

	return *this;
}

ThemeEval &ThemeEval::addLayout(ThemeLayout::LayoutType type, int spacing, ThemeLayout::ItemAlign itemAlign) {
	ThemeLayout *layout = nullptr;

	if (spacing == -1)
		spacing = getVar("Globals.Layout.Spacing", 4);

	layout = new ThemeLayoutStacked(_curLayout.top(), type, spacing, itemAlign);

	assert(layout);

	layout->setPadding(
		getVar("Globals.Padding.Left", 0),
		getVar("Globals.Padding.Right", 0),
		getVar("Globals.Padding.Top", 0),
		getVar("Globals.Padding.Bottom", 0)
		);

	_curLayout.top()->addChild(layout);
	_curLayout.push(layout);

	return *this;
}

ThemeEval &ThemeEval::addSpace(int size) {
	ThemeLayout *space = new ThemeLayoutSpacing(_curLayout.top(), size);
	_curLayout.top()->addChild(space);

	return *this;
}

bool ThemeEval::hasDialog(const Common::String &name) {
	Common::StringTokenizer tokenizer(name, ".");

	if (name.hasPrefix("Dialog."))
		tokenizer.nextToken();

	Common::String dialogName = "Dialog." + tokenizer.nextToken();
	return _layouts.contains(dialogName);
}

void ThemeEval::reflowDialogLayout(const Common::String &name, Widget *widgetChain) {
	if (!_layouts.contains("Dialog." + name)) {
		warning("No layout found for dialog '%s'", name.c_str());
		return;
	}

	_layouts["Dialog." + name]->reflowLayout(widgetChain);
}

ThemeEval &ThemeEval::addImportedLayout(const Common::String &name) {
	ThemeLayout *importedLayout = _layouts[name];
	assert(importedLayout);

	_curLayout.top()->importLayout(importedLayout);

	return *this;
}

} // End of namespace GUI
