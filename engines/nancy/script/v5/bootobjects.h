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

#ifndef NANCY_SCRIPT_V5_BOOTOBJECTS_H
#define NANCY_SCRIPT_V5_BOOTOBJECTS_H

#include "engines/nancy/enginedata.h"

struct lua_State;

namespace Nancy {
namespace Script {

// Nancy26's forms of InventoryInit:Create and CursorInit:Create, and the
// FontsInit:Create the file of a font made of pictures calls
int createNancy26Inventory(lua_State *L, INVD &inventory);
int createNancy26Cursors(lua_State *L, ScriptBootData &boot);
int createBitmapFont(lua_State *L, ScriptBootData::BitmapFont &font);

// The "Init" objects only Nancy26's boot scripts have: FontsBoot,
// FontColorsInit, AutotextBoot and BuildNumberInit
void registerNancy26BootAPI(lua_State *L);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_BOOTOBJECTS_H
