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

#include "common/system.h"
#include "common/file.h"

#include "hdb/hdb.h"

#ifndef HDB_LUA_SCRIPT_H
#define HDB_LUA_SCRIPT_H

struct lua_State;

namespace HDB {

class LuaScript {
public:
	LuaScript();
	~LuaScript();

	bool init();
	bool initScript(Common::SeekableReadStream *stream, const char *scriptName, int32 length);
	bool callFunction(const char *name, int returns);
	bool executeMPC(Common::SeekableReadStream *stream, const char *name, const char *scriptName, int32 length);
	bool executeFile(const Common::String &filename);
	void checkParameters(const char *func, int params);

	const char *getStringOffStack();

private:
	lua_State *_state;
	int _pcallErrorhandlerRegistryIndex;

	Common::SeekableReadStream* _globalLuaStream;
	int32 _globalLuaLength;
	bool _systemInit;

	bool registerExtensions();
	bool executeChunk(Common::String &chunk, uint chunkSize, const Common::String &chunkName) const;
	void stripComments(char *chunk);
	void addPatches(Common::String &chunk, const char *scriptName);
};

}

#endif // !HDB_LUA_SCRIPT_H
