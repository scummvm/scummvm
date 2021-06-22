/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "math/matrix3.h"

#include "engines/grim/debug.h"
#include "engines/grim/lua_v1.h"
#include "engines/grim/actor.h"
#include "engines/grim/grim.h"
#include "engines/grim/set.h"
#include "engines/grim/hotspot.h"

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lapi.h"

namespace Grim {

void Lua_V1::RegisterHotspot() {
	Math::Vector3d pos (lua_getnumber(lua_getparam(2)),
						lua_getnumber(lua_getparam(3)),
						lua_getnumber(lua_getparam(4)));
	Common::String scene = lua_getstring(lua_getparam(5));
	int n = g_grim->getHotspotMan()->addHotspot(lua_getstring(lua_getparam(1)), pos, scene);
	lua_pushnumber(n);
}

void Lua_V1::ActivateHotspot() {
	int num = lua_getnumber(lua_getparam(1));
	if (num < 0) {
		g_grim->getHotspotMan()->disableAll();
	} else {
		HotObject &hs = g_grim->getHotspotMan()->getObject(num);
		hs._active = getbool(2);
	}
}

void Lua_V1::UpdateHotspot() {
	Common::String id = lua_getstring(lua_getparam(1));
	Math::Vector3d pos (lua_getnumber(lua_getparam(2)),
						lua_getnumber(lua_getparam(3)),
						lua_getnumber(lua_getparam(4)));
	int visible = lua_getnumber(lua_getparam(5));
	g_grim->getHotspotMan()->updateHotspot(id, pos, visible);
}

void Lua_V1::SwitchControlMode() {
	int mode = lua_getnumber(lua_getparam(1));
	g_grim->getHotspotMan()->switchMode(mode);
	if (mode == 1 || mode == 5) {
		if (!lua_isnumber(lua_getparam(2)))
			return;
		int lines = lua_getnumber(lua_getparam(2));
		int x0 = lua_getnumber(lua_getparam(3));
		int y0 = lua_getnumber(lua_getparam(4));
		int width = lua_getnumber(lua_getparam(5));
		int height = lua_getnumber(lua_getparam(6));
		g_grim->getHotspotMan()->setupDialog(x0, y0, width, height, lines, 1);
	} else if (mode == 3) {
		if (!lua_isnumber(lua_getparam(2)))
			return;
		Math::Vector3d axis(lua_getnumber(lua_getparam(2)),
							lua_getnumber(lua_getparam(3)),
							lua_getnumber(lua_getparam(4)));
		g_grim->getHotspotMan()->setAxis(axis, lua_getnumber(lua_getparam(5)));
	} else if (mode == 4) {
		g_grim->getHotspotMan()->setupDialog(20, 40, 100, 100, 4, 6);
	}
}

void Lua_V1::RegisterInventory() {
	Common::String id = lua_getstring(lua_getparam(1));
	if (id == "reset") {
		g_grim->getHotspotMan()->resetInventory();
	} else {
		Common::String pic = lua_getstring(lua_getparam(2));
		g_grim->getHotspotMan()->addInventory(id, pic);
	}
}

void Lua_V1::MouseDown() {
	int state = g_grim->getEventManager()->getButtonState();
	lua_pushnumber(state);
}

void Lua_V1::RenameHotspot() {
	int id = lua_getnumber(lua_getparam(1));
	Common::String name = lua_getstring(lua_getparam(2));
	g_grim->getHotspotMan()->renameHotspot(id, name);
}

void Lua_V1::GetControlMode() {
	int mode = g_grim->getHotspotMan()->getCtrlMode();
	lua_pushnumber(mode);
}

void Lua_V1::SetOptionMode() {
	Common::String name = lua_getstring(lua_getparam(1));
	Common::String old = g_grim->getHotspotMan()->getOptionMode();

	g_grim->getHotspotMan()->setOptionMode(name);
	lua_pushstring(old.c_str());
}

} // end of namespace Grim
