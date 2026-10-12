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

#ifndef NANCY_SCRIPT_API_H
#define NANCY_SCRIPT_API_H

#include "common/keyboard.h"
#include "common/scummsys.h"
#include "common/str.h"

struct lua_State;

namespace Common {
class WriteStream;
}

namespace Nancy {

struct CVTX;
struct ENVS;
struct EVNT;
struct INVD;
struct PCUI;
struct VIEW;
struct ScriptBootData;

namespace Script {

// Puts everything scripts can use into a new state
void registerAPI(lua_State *L);

// The tables the boot scripts fill. A script can only fill the ones that are set
// while it runs.
struct BootTables {
	EVNT *flags = nullptr;
	INVD *inventory = nullptr;
	ENVS *environments = nullptr;
	PCUI *players = nullptr;
	VIEW *viewport = nullptr;
	CVTX *texts = nullptr;
	ScriptBootData *boot = nullptr;
	int playerIndex = -1;	// the character whose own boot script is running
};

// Runs one of the boot scripts. Returns false when there is no such script.
bool runBootScript(const char *name, const BootTables &tables);

void registerBootAPI(lua_State *L);

// Inventory items are handed to scripts by name ("Inventory.INV_Board") as objects
// of their own, which hold the id of the item
void registerInventoryAPI(lua_State *L);
void pushInventoryItem(lua_State *L, int16 itemID);
int16 checkInventoryItem(lua_State *L, int index);

// The objects scripts reach the state of the game through: Flags, VarTable,
// Player, Scene, Sound, Cursor...
void registerGameAPI(lua_State *L);

// Flags go by name. Both return false when there is no flag of that name.
bool setFlagByName(const Common::String &name, bool value);
bool getFlagByName(const Common::String &name, bool &value);

// Puts the journal entries and the tasks the character being played was given
// into the UI, once it has been made
void showPlayerEntries();

// Reads a cursor a script picked with "Cursor.<name>", "Cursor.<name>.hot"...
// Returns false when the value is something else.
bool testCursor(lua_State *L, int index, uint &cursorType, bool &isHot);
void pushCursor(lua_State *L, uint cursorType, bool isHot);

// Keys, as picked with "Key.<name>"
void pushKey(lua_State *L, Common::KeyCode key);
Common::KeyCode checkKey(lua_State *L, int index);

// The text kept under a key: what a sound says, or a text of the UI.
// Returns false when there is none.
bool findText(const Common::String &key, Common::String &text);

// The panels and controls scripts build the UI out of
void registerUIAPI(lua_State *L);

// Writes out how a text gets drawn, the way an element of an NDUI chunk has it:
// in a font and a color, and placed as the "font" field of the table at an index says
void writeTextElement(lua_State *L, int tableIndex, const Common::String &fontName, bool isBold, uint32 color, Common::WriteStream &out);

// The images scripts look colors up in
void registerRefAPI(lua_State *L);

// Sound channels, as picked with "SoundChannel.<name>"
void pushSoundChannel(lua_State *L, uint16 channel);
uint16 checkSoundChannel(lua_State *L, int index);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_API_H
