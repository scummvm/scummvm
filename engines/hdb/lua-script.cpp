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

#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/lua/lualib.h"
#include "common/debug.h"

#include "hdb/lua-script.h"

namespace HDB {

LuaScript::LuaScript() {
	_state = NULL;
	_systemInit = false;
}

LuaScript::~LuaScript() {
	if (_state) {
		lua_close(_state);
	}
}

#if 0
bool LuaScript::init() {
	// Create new lua_State and initialize the Std Libs
	_state = luaL_newstate();
	if (!_state || !registerStdLibs()) {
		error("Cannot initialize Lua");
		return false;
	}

	_systemInit = true;

	return true;
}
#endif

/*
	Called from Lua, this will pop into the menu
*/

static int gotoMenu(lua_State *L) {
	g_hdb->changeGameState();
	return 0;
}

struct VarInit {
	char *realName;
	char *luaName;
} luaGlobalStrings[] = {
	{ "Map00",	"MAP00"},
	{NULL, NULL}
};

// For AI States, to be implemented
struct NumberInit {
	int value;
	char *luaName;
} luaGlobalValues[] = {
	{NULL, NULL}
};

struct FuncInit {
	char *luaName;
	int (*function) (lua_State *L);
} luaFuncs[] = {
	{"GotoMenu", gotoMenu},
	{NULL, NULL}
};

bool LuaScript::initScript(Common::SeekableReadStream *stream, const char *name, int32 length) {

	if (_systemInit) {
		return false;
	}

	// Initialize Lua Environment
	_state = lua_open();
	if (_state == NULL) {
		error("Couldn't initialize Lua script.");
		return false;
	}
	luaL_openlibs(_state);

	_systemInit = true;

	// Register Extensions
	for (int i = 0; luaFuncs[i].luaName; i++) {
		lua_register(_state, luaFuncs[i].luaName, luaFuncs[i].function);
	}

	// Register Lua Globals

	for (int i = 0; luaGlobalStrings[i].realName; i++) {
		lua_pushstring(_state, luaGlobalStrings[i].realName);
		lua_setglobal(_state, luaGlobalStrings[i].luaName);
	}

	for (int j = 0; luaGlobalValues[j].luaName; j++) {
		lua_pushnumber(_state, luaGlobalValues[j].value);
		lua_setglobal(_state, luaGlobalValues[j].luaName);
	}

	/*
		TODO: Set the last mapName as a global
		after implementing the map-manager.
	*/

	// Set the lowest printable line
	lua_pushnumber(_state, 480 - 14);
	lua_setglobal(_state, "BOTTOM_Y");

	/*
		TODO: Load the sound names and entity
		spawn names into Lua once they are implemented.
	*/

	/*
		TODO: Find what from where the global.lua
		is to be loaded, and execute it.
	*/

	// Load script and execute it

	if (!executeMPC(stream, "level code", length)) {
		error("LuaScript::initScript: 'level code' failed to execute");
		return false;
	}

	lua_getglobal(_state, "level_init");
	lua_pcall(_state, 0, 0, 0);

	return true;
}

bool LuaScript::executeMPC(Common::SeekableReadStream *stream, const char *name, int32 length) {

	if (!_systemInit) {
		return false;
	}

	char *chunk = new char[length];
	stream->read((void *)chunk, length);

	// Remove C-Style comments from script
	stripComments(chunk);

	if (!executeChunk(chunk, length, name)) {
		delete[] chunk;
		
		return false;
	}

	delete[] chunk;

	return true;
}

#if 0
bool LuaScript::executeFile(const Common::String &filename) {
	
	if (!_systemInit) {
		return false;
	}

	Common::File *file = new Common::File;

	if (!file->open(filename)) {
		error("Cannot find \"%s\"", filename.c_str());
	}

	uint fileSize = file->size();
	const char *fileData = new char[fileSize];
	file->read((void *)fileData, fileSize);

	if (!executeChunk(fileData, fileSize, filename)) {
		delete[] fileData;
		delete file;

		return false;
	}

	delete[] fileData;
	delete file;

	return true;
}
#endif

bool LuaScript::executeChunk(const char *chunk, uint chunkSize, const Common::String &chunkName) const {
	
	if (!_systemInit) {
		return false;
	}
	
	// Compile Chunk
	if (luaL_loadbuffer(_state, chunk, chunkSize, chunkName.c_str())) {
		error("Couldn't compile \"%s\": %s", chunkName.c_str(), lua_tostring(_state, -1));
		lua_pop(_state, -1);
		
		return false;
	}

	// Execute Chunk
	if (lua_pcall(_state, 0, 0, 0)) {
		error("An error occured while executing \"%s\": %s.", chunkName.c_str(), lua_tostring(_state, -1));
		lua_pop(_state, -1);
		
		return false;
	}

	return true;
}

void LuaScript::stripComments(char *chunk) {
	uint32 offset = 0;

	while (chunk[offset]) {
		if (chunk[offset] == '/' && chunk[offset + 1] == '/') {
			while (chunk[offset] != 0x0d) {
				chunk[offset++] = ' ';
			}
		}
		offset++;
	}
}
}
