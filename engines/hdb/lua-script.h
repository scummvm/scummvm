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

#ifndef HDB_LUA_SCRIPT_H
#define HDB_LUA_SCRIPT_H

struct lua_State;

namespace HDB {

struct Global {
	char	global[32];			// name of global variable
	int		valueOrString;	// value = 0, string = 1
	double	value;				// value
	char	string[32];			// string

	Global() : valueOrString(0), value(0) {
		global[0] = 0;
		string[0] = 0;
	}
};

class LuaScript {
public:
	LuaScript();
	~LuaScript();

	bool loadLua(const char *name);
	void saveGlobalNumber(const char *global, double value);
	void saveGlobalString(const char *global, const char *string);
	void loadGlobal(const char *global);
	void purgeGlobals();
	void save(Common::OutSaveFile *out);
	void loadSaveFile(Common::InSaveFile *in);

	void init();
	bool initScript(Common::SeekableReadStream *stream, const char *scriptName, int32 length);

	void pushInt(int value);
	void pushString(char *string);
	void pushFunction(char *func);
	void call(int args, int returns);
	bool callFunction(const char *name, int returns);
	void invokeLuaFunction(char *luaFunc, int x, int y, int value1, int value2);

	bool executeMPC(Common::SeekableReadStream *stream, const char *name, const char *scriptName, int32 length);
	bool executeFile(const Common::String &filename);
	bool executeChunk(Common::String &chunk, const Common::String &chunkName) const;
	void checkParameters(const char *func, int params);

	const char *getStringOffStack();

	void setLuaGlobalValue(const char *name, int value);
	bool isValid() {
		return _systemInit;
	}

	// Platform-specific Constants
	int _cameraXOff;
	int _cameraYOff;

private:
	lua_State *_state;
	int _pcallErrorhandlerRegistryIndex;

	Common::SeekableReadStream* _globalLuaStream;
	int32 _globalLuaLength;
	bool _systemInit;

	bool registerExtensions();
	void stripComments(char *chunk);
	void addPatches(Common::String &chunk, const char *scriptName);

	Common::Array<Global *> _globals;
};

void lua_printstack(lua_State *L);

}

#endif // !HDB_LUA_SCRIPT_H
