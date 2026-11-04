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

#ifndef NANCY_SCRIPT_BOOTAPI_H
#define NANCY_SCRIPT_BOOTAPI_H

#include "common/array.h"
#include "common/str.h"

#include "engines/nancy/script/api.h"

struct lua_State;

namespace Nancy {
namespace Script {

// What the "Init" objects of the boot scripts share

// The tables the boot script that runs fills; null while none runs
const BootTables *getBootTables();

// Every Create function takes the object itself and one table. A script that
// may not use the object gets an error.
void checkCreateArguments(lua_State *L, const char *typeName, bool hasTable);

// The keys of a table that are strings, in alphabetical order
void getSortedKeys(lua_State *L, int tableIndex, Common::Array<Common::String> &keys);

// A field of the table of CursorInit:Create that must be there
void requireCursorField(lua_State *L, const char *name, Common::String &value);
bool hasCursor(const ScriptBootData &boot, const Common::String &name);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_BOOTAPI_H
