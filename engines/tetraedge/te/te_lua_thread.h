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

#ifndef TETRAEDGE_TE_TE_LUA_THREAD_H
#define TETRAEDGE_TE_TE_LUA_THREAD_H

#include "common/array.h"
#include "common/str.h"
#include "common/fs.h"

struct lua_State;

namespace Tetraedge {

class TeLuaContext;
class TeVariant;

class TeLuaThread {
public:
	TeLuaThread(TeLuaContext *context);

	~TeLuaThread();

	static TeLuaThread *create(TeLuaContext *context);

	void execute(const Common::String &str);
	void execute(const Common::String &str, const TeVariant &p1);
	void execute(const Common::String &str, const TeVariant &p1, const TeVariant &p2);
	void execute(const Common::String &str, const TeVariant &p1, const TeVariant &p2, const TeVariant &p3);

	void executeFile(const TetraedgeFSNode &node);
	void pushValue(const TeVariant &val);

	void release();

	void resume();
	void resume(const TeVariant &p1);
	void resume(const TeVariant &p1, const TeVariant &p2);
	void resume(const TeVariant &p1, const TeVariant &p2, const TeVariant &p3);

	static TeLuaThread *threadFromState(lua_State *state);
	int yield();

	static void cleanup();

private:
	void _resume(int nargs);

	void applyScriptWorkarounds(char *buf, const Common::String &fileName);

	lua_State *_luaThread;
	uint64 _resumeCount;
	int _bottomRef;
	int _lastResumeResult;
	bool _released;

	static Common::Array<TeLuaThread *> *threadList();
	static Common::Array<TeLuaThread *> *_threadList;

};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_LUA_THREAD_H
