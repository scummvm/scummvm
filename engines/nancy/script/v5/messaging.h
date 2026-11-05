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

#ifndef NANCY_SCRIPT_V5_MESSAGING_H
#define NANCY_SCRIPT_V5_MESSAGING_H

#include "common/str.h"

struct lua_State;

namespace Nancy {
namespace Script {

class Record;

// Nancy26's records talk to each other by name. A record takes names with
// RegisterAlias(), and other records, in any scene, send it messages:
//
//   themeManager:RegisterAlias("ThemeManager")
//   ar:Send("ThemeManager", "Stop")             -- calls themeManager:Receive(sender, "Stop")
//   local photos = ar:Send("PhotoManager", "Get", "camera")
//
// Whatever goes along with a message, and whatever comes back from it, is copied
// from the Lua state of one scene to the one of the other: numbers, texts,
// booleans, tables and the math types. Every record also answers "Enable",
// "Disable", "Show", "Hide", "Play" and "Pause" by itself, before its Receive
// function gets them. A stream answers messages sent to its name.

// Returns false when the name is taken by another record
bool registerAlias(const Common::String &alias, Record *record);
void deregisterAlias(const Common::String &alias, Record *record);

// Every name of a record that is going away
void deregisterAliases(Record *record);

// The record that goes by a name; null for none
Record *findAlias(const Common::String &alias);

// Sends a message from the state L, whose numArgs arguments start at firstArg,
// and leaves what the receiver returned on the stack of L. Returns the number of
// results, which is 0 when nobody goes by the name.
int sendMessage(lua_State *L, const Common::String &sender, const Common::String &recipient,
	const Common::String &action, int firstArg, int numArgs);

// Copies the value at an index of one state onto the stack of another
void copyValue(lua_State *from, int index, lua_State *to);

// The functions records have for messages
int recordSend(lua_State *L);
int recordSendAsAlias(lua_State *L);
int recordRegisterAlias(lua_State *L);
int recordDeregisterAlias(lua_State *L);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_V5_MESSAGING_H
