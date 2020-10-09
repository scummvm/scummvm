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

#include "common/debug.h"
#include "common/textconsole.h"
#include "engines/grim/debug.h"

#include "engines/grim/emi/costume/emiluavar_component.h"

namespace Grim {

EMILuaVarComponent::EMILuaVarComponent(Component *p, int parentID, const char *name, Component *prevComponent, tag32 t) : Component(p, parentID, name, t) {
}

EMILuaVarComponent::~EMILuaVarComponent() {
}

void EMILuaVarComponent::init() {
}

int EMILuaVarComponent::update(uint time) {
	return 0;
}

void EMILuaVarComponent::setKey(int val) {
	Debug::debug(Debug::Lua, "LuaV component: setting %s to %d", _name.c_str(), val);
	lua_pushnumber(val);
	lua_setglobal(_name.c_str());
}

void EMILuaVarComponent::reset() {
}

void EMILuaVarComponent::draw() {
}

} // end of namespace Grim
