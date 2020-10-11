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

#include "common/textconsole.h"

#include "gui/object.h"
#include "gui/widget.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"

namespace GUI {

GuiObject::GuiObject(const Common::String &name)
	: _x(-1000), _y(-1000), _w(0), _h(0), _useRTL(true), _name(name), _firstWidget(nullptr) {
}

GuiObject::~GuiObject() {
	delete _firstWidget;
	_firstWidget = nullptr;
}

void GuiObject::reflowLayout() {
	if (!_name.empty()) {
		int16 w, h;
		bool useRTL = true;
//		if (!g_gui.xmlEval()->getWidgetData(_name, _x, _y, w, h, useRTL) || w == -1 || h == -1) {
//			warning("widget h: %d	w: %d", h, w);
//			error("Unable to load widget position for '%s'. Please check your theme files", _name.c_str());
//		}
	
		//symbian fix!!!
	g_gui.xmlEval()->getWidgetData(_name, _x, _y, w, h, useRTL);
	if (w == -1 || h == -1) {
		error("Unable to load widget position for '%s'. Please check your theme files", _name.c_str());
	}
		_w = w;
		_h = h;
		_useRTL = useRTL;
	}
}

void GuiObject::removeWidget(Widget *del) {
	if (del == _firstWidget) {
		Widget *del_next = del->next();
		del->setNext(nullptr);
		_firstWidget = del_next;
		return;
	}

	Widget *w = _firstWidget;
	while (w) {
		if (w->next() == del) {
			Widget *del_next = del->next();
			del->setNext(nullptr);
			w->setNext(del_next);
			return;
		}
		w = w->next();
	}
}

Common::Rect GuiObject::getClipRect() const {
	return Common::Rect(getAbsX(), getAbsY(), getAbsX() + getWidth(), getAbsY() + getHeight());
}

} // End of namespace GUI
