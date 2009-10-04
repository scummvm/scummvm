/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

#include "common/system.h"
#include "gui/object.h"
#include "gui/widget.h"
#include "gui/GuiManager.h"
#include "gui/ThemeEval.h"

namespace GUI {

GuiObject::GuiObject(const Common::String &name) : _firstWidget(0) {
	_name = name;
}

GuiObject::~GuiObject() {
	delete _firstWidget;
	_firstWidget = 0;
}

uint32 GuiObject::getMillis() {
	return g_system->getMillis();
}

void GuiObject::reflowLayout() {
	if (!_name.empty()) {
		if (!g_gui.xmlEval()->getWidgetData(_name, _x, _y, _w, _h)) {
			error("Could not load widget position for '%s'", _name.c_str());
		}

		if (_x < 0)
			error("Widget <%s> has x < 0 (%d)", _name.c_str(), _x);
		if (_x >= g_system->getOverlayWidth())
			error("Widget <%s> has x > %d (%d)", _name.c_str(), g_system->getOverlayWidth(), _x);
		if (_x + _w > g_system->getOverlayWidth())
			error("Widget <%s> has x + w > %d (%d)", _name.c_str(), g_system->getOverlayWidth(), _x + _w);
		if (_y < 0)
			error("Widget <%s> has y < 0 (%d)", _name.c_str(), _y);
		if (_y >= g_system->getOverlayHeight())
			error("Widget <%s> has y > %d (%d)", _name.c_str(), g_system->getOverlayHeight(), _y);
		if (_y + _h > g_system->getOverlayHeight())
			error("Widget <%s> has y + h > %d (%d)", _name.c_str(), g_system->getOverlayHeight(), _y + _h);
	}
}

} // End of namespace GUI
