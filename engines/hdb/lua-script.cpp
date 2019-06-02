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
#include "hdb/lua-script.h"

namespace HDB {

LuaScript::LuaScript() {
	_state = NULL;
}

LuaScript::~LuaScript() {
	if (_state) {
		lua_close(_state);
	}
}

bool LuaScript::init() {
	// Create new lua_State and initialize the Std Libs
	_state = luaL_newstate();
	if (!_state || !registerStdLibs()) {
		error("Cannot initialize Lua");
		return false;
	}

	return true;
}

bool LuaScript::executeFile(const Common::String &filename) {
	
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

bool LuaScript::registerStdLibs() {
	luaL_openlibs(_state);
	
	return true;
}

bool LuaScript::executeChunk(const char *chunk, uint chunkSize, const Common::String &chunkName) const {
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

}