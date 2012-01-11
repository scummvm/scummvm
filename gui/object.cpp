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
 */

#include "common/textconsole.h"
#include "common/tokenizer.h"

#include "gui/object.h"
#include "gui/widget.h"
#include "gui/gui-manager.h"
#include "gui/ThemeEval.h"

namespace GUI {

GuiObject::GuiObject(const Common::String &name)
	: _x(-1000), _y(-1000), _w(0), _h(0), _name(name), _firstWidget(0) {
	reflowLayout();
}

GuiObject::~GuiObject() {
	delete _firstWidget;
	_firstWidget = 0;
}

void GuiObject::reflowLayout() {
	if (!_name.empty()) {

		if (_name.hasPrefix("GameOptions_Engine")) {
			// Find the y position of this widget from its name
			Common::StringTokenizer tokenizer(_name, ".");
			tokenizer.nextToken();	// skip the tab name
			tokenizer.nextToken();	// skip the widget name
			uint id = atoi(tokenizer.nextToken().c_str());

			// The values below are set so that up to 10 options can fit in a
			// 320x200 screen.
			_x = 6;
			_y = (kLineHeight + 6) * id + kLineHeight - 6;
			// FIXME: This fixed width isn't pretty, but at this point, we
			// don't have information about the widget's parent.
			_w = 240;
			_h = kLineHeight;
			return;
		}

		if (!g_gui.xmlEval()->getWidgetData(_name, _x, _y, _w, _h)) {
			error("Could not load widget position for '%s'", _name.c_str());
		}

		if (_x < 0)
			error("Widget <%s> has x < 0 (%d)", _name.c_str(), _x);
		if (_x >= g_gui.getWidth())
			error("Widget <%s> has x > %d (%d)", _name.c_str(), g_gui.getWidth(), _x);
		if (_x + _w > g_gui.getWidth())
			error("Widget <%s> has x + w > %d (%d)", _name.c_str(), g_gui.getWidth(), _x + _w);
		if (_y < 0)
			error("Widget <%s> has y < 0 (%d)", _name.c_str(), _y);
		if (_y >= g_gui.getHeight())
			error("Widget <%s> has y > %d (%d)", _name.c_str(), g_gui.getHeight(), _y);
		if (_y + _h > g_gui.getHeight())
			error("Widget <%s> has y + h > %d (%d)", _name.c_str(), g_gui.getHeight(), _y + _h);
	}
}

} // End of namespace GUI
