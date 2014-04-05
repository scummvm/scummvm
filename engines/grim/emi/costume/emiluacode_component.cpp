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
#include "engines/grim/debug.h"

#include "engines/grim/emi/costume/emiluacode_component.h"

namespace Grim {

EMILuaCodeComponent::EMILuaCodeComponent(Component *p, int parentID, const char *name, Component *prevComponent, tag32 t) : Component(p, parentID, name, t) {
}

EMILuaCodeComponent::~EMILuaCodeComponent() {
}

void EMILuaCodeComponent::init() {
}

int EMILuaCodeComponent::update(uint time) {
	return 0;
}

void EMILuaCodeComponent::setKey(int val) {
	Debug::debug(Debug::Lua, "LuaC component: executing code [%s]", _name.c_str());
	lua_dostring(_name.c_str());
}

void EMILuaCodeComponent::reset() {
}

void EMILuaCodeComponent::draw() {
}

} // end of namespace Grim
