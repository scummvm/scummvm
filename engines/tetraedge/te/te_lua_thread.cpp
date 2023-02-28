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

#include "tetraedge/te/te_lua_thread.h"
#include "tetraedge/te/te_lua_context.h"
#include "tetraedge/te/te_variant.h"

#include "common/str.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/lua/lualib.h"

//#define TETRAEDGE_LUA_DEBUG 1

namespace Tetraedge {

/*static*/
Common::Array<TeLuaThread *> *TeLuaThread::_threadList = nullptr;

TeLuaThread::TeLuaThread(TeLuaContext *context) : _resumeCount(0), _lastResumeResult(0), _released(false) {
	_luaThread = lua_newthread(context->luaState());
	_bottomRef = luaL_ref(context->luaState(), LUA_REGISTRYINDEX);
	threadList()->push_back(this);
}

TeLuaThread::~TeLuaThread() {
	luaL_unref(_luaThread, LUA_REGISTRYINDEX, _bottomRef);
	uint i;
	Common::Array<TeLuaThread *> *threads = threadList();
	for (i = 0; i < threads->size(); i++)
		if ((*threads)[i] == this)
			break;
	if (i < threads->size())
		threads->remove_at(i);
}

/*static*/ TeLuaThread *TeLuaThread::create(TeLuaContext *context) {
	return new TeLuaThread(context);
}

void TeLuaThread::_resume(int nargs) {
	_resumeCount++;
	_lastResumeResult = lua_resume(_luaThread, nargs);
	if (_lastResumeResult > 1) {
		const char *msg = lua_tolstring(_luaThread, -1, nullptr);
		warning("TeLuaThread::_resume: %s", msg);
	}
	if (_lastResumeResult != 1 && _released) {
		//debug("TeLuaThread:: deleting this?");
		delete this;
	}
}

void TeLuaThread::execute(const Common::String &fname) {
	if (!_luaThread)
		return;

#ifdef TETRAEDGE_LUA_DEBUG
	if (fname != "Update" && fname != "UpdateHelp")
		debug("TeLuaThread::execute: %s()", fname.c_str());
#endif

	lua_getglobal(_luaThread, fname.c_str());
	if (lua_type(_luaThread, -1) == LUA_TFUNCTION) {
		_resume(0);
	} else {
		if (!fname.contains("Update"))
			debug("[TeLuaThread::Execute0] Function: \"%s\" does not exist", fname.c_str());
		lua_settop(_luaThread, -2);
	}
}

void TeLuaThread::execute(const Common::String &fname, const TeVariant &p1) {
	if (!_luaThread)
		return;

#ifdef TETRAEDGE_LUA_DEBUG
	debug("TeLuaThread::execute: %s(%s)", fname.c_str(), p1.dumpStr().c_str());
#endif

	lua_getglobal(_luaThread, fname.c_str());
	if (lua_type(_luaThread, -1) == LUA_TFUNCTION) {
		pushValue(p1);
		_resume(1);
	} else {
		// Don't report Update (as original) or some other functions which are not
		// implemented in both games
		if (!fname.contains("Update") && !fname.equals("OnCellCharacterAnimationPlayerFinished")
				&& !fname.equals("OnCharacterAnimationFinished") && !fname.equals("OnCellDialogFinished")
				&& !fname.equals("OnCellFreeSoundFinished"))
			debug("[TeLuaThread::Execute1] Function: \"%s\" does not exist", fname.c_str());
		lua_settop(_luaThread, -2);
	}
}

void TeLuaThread::execute(const Common::String &fname, const TeVariant &p1, const TeVariant &p2) {
	if (!_luaThread)
		return;

#ifdef TETRAEDGE_LUA_DEBUG
	debug("TeLuaThread::execute: %s(%s, %s)", fname.c_str(), p1.dumpStr().c_str(),
			p2.dumpStr().c_str());
#endif

	lua_getglobal(_luaThread, fname.c_str());
	if (lua_type(_luaThread, -1) == LUA_TFUNCTION) {
		pushValue(p1);
		pushValue(p2);
		_resume(2);
	} else {
		if (!fname.contains("Update"))
			debug("[TeLuaThread::Execute2] Function: \"%s\" does not exist.", fname.c_str());
		lua_settop(_luaThread, -2);
	}
}

void TeLuaThread::execute(const Common::String &fname, const TeVariant &p1, const TeVariant &p2, const TeVariant &p3) {
	if (!_luaThread)
		return;

#ifdef TETRAEDGE_LUA_DEBUG
	debug("TeLuaThread::execute: %s(%s, %s, %s)", fname.c_str(), p1.dumpStr().c_str(),
				p2.dumpStr().c_str(), p3.dumpStr().c_str());
#endif


	lua_getglobal(_luaThread, fname.c_str());
	if (lua_type(_luaThread, -1) == LUA_TFUNCTION) {
		pushValue(p1);
		pushValue(p2);
		pushValue(p3);
		_resume(3);
	} else {
		if (!fname.contains("Update"))
			debug("[TeLuaThread::Execute3] Function: \"%s\" does not exist.", fname.c_str());
		lua_settop(_luaThread, -4);
	}
}

void TeLuaThread::executeFile(const Common::FSNode &node) {
	Common::File scriptFile;
	if (!scriptFile.open(node)) {
		warning("TeLuaThread::executeFile: File %s can't be opened", node.getName().c_str());
		return;
	}

#ifdef TETRAEDGE_LUA_DEBUG
	debug("TeLuaThread::executeFile: %s", node.getName().c_str());
#endif

	int64 fileLen = scriptFile.size();
	char *buf = new char[fileLen + 1];
	scriptFile.read(buf, fileLen);
	buf[fileLen] = 0;
	scriptFile.close();

	//
	// WORKAROUND: Some script files have rogue ";" lines in them with nothing
	// else, and ScummVM common lua version doesn't like them. Clean those up.
	//
	char *fixline = strstr(buf, "\n\t;");
	if (fixline)
		fixline[2] = '\t';

	//
	// WORKAROUND: Syberia 2 constantly re-seeds the random number generator.
	// This fails on ScummVM Lua because os.time() returns a large Number and
	// math.randomseed() clamps the number to an int, so it always seeds on the
	// same value.  It's also kind of pointless, so just patch it out.
	//
	static const char RESEED_PATTERN[] = "math.randomseed( os.time() )";
	fixline = strstr(buf, RESEED_PATTERN);
	while (fixline != nullptr) {
		for (int i = 0; i < ARRAYSIZE(RESEED_PATTERN); i++) {
			fixline[i] = ' ';
		}
		fixline = strstr(fixline, RESEED_PATTERN);
	}

	//
	// WORKAROUND: Syberia 2 A1_Cabaret/11420/Logic11420.lua has a typo on a
	// variable name that causes the game to lock up
	//
	fixline = strstr(buf, "OBJECT_10050_Inventory_obj_coeurmec_Taketoun ");
	if (fixline) {
		// Taketoun -> Taken
		fixline[40] = 'n';
		fixline[41] = ' ';
		fixline[42] = ' ';
		fixline[43] = ' ';
	}

	_lastResumeResult = luaL_loadbuffer(_luaThread, buf, fileLen, node.getPath().c_str());
	if (_lastResumeResult) {
		const char *msg = lua_tostring(_luaThread, -1);
		warning("TeLuaThread::executeFile: %s", msg);
	}
	delete [] buf;

	_resume(0);
}

void TeLuaThread::pushValue(const TeVariant &val) {
	TeVariant::VariantType valType = val.type();
	switch(valType) {
		case TeVariant::TypeBoolean:
			lua_pushboolean(_luaThread, val.toBoolean());
			break;
		case TeVariant::TypeInt32:
			lua_pushinteger(_luaThread, val.toSigned32());
			break;
		case TeVariant::TypeUInt32:
			lua_pushinteger(_luaThread, val.toUnsigned32());
			break;
		case TeVariant::TypeInt64:
			lua_pushinteger(_luaThread, val.toSigned64());
			break;
		case TeVariant::TypeUInt64:
			lua_pushinteger(_luaThread, val.toUnsigned64());
			break;
		case TeVariant::TypeFloat32:
			lua_pushnumber(_luaThread, val.toFloat32());
			break;
		case TeVariant::TypeFloat64:
			lua_pushnumber(_luaThread, val.toFloat64());
			break;
		case TeVariant::TypeString:
			lua_pushstring(_luaThread, val.toString().c_str());
			break;
		default:
			warning("TeLuaThread::pushValue: Unknown type");
			return;
	}
}

void TeLuaThread::release() {
	_released = true;
	if (_lastResumeResult != 1) {
		//debug("TeLuaThread:: deleting this?");
		delete this;
	}
}

void TeLuaThread::resume() {
#ifdef TETRAEDGE_LUA_DEBUG
	debug("TeLuaThread::resume");
#endif

	if (_luaThread)
		_resume(0);
}

void TeLuaThread::resume(const TeVariant &p1) {
#ifdef TETRAEDGE_LUA_DEBUG
	debug("TeLuaThread::resume(%s)", p1.dumpStr().c_str());
#endif

	if (_luaThread) {
		pushValue(p1);
		_resume(1);
	}
}

void TeLuaThread::resume(const TeVariant &p1, const TeVariant &p2) {
#ifdef TETRAEDGE_LUA_DEBUG
	debug("TeLuaThread::resume(%s, %s)", p1.dumpStr().c_str(), p2.dumpStr().c_str());
#endif

	if (_luaThread) {
		pushValue(p1);
		pushValue(p2);
		_resume(2);
	}
}

void TeLuaThread::resume(const TeVariant &p1, const TeVariant &p2, const TeVariant &p3) {
#ifdef TETRAEDGE_LUA_DEBUG
	debug("TeLuaThread::resume(%s, %s, %s)", p1.dumpStr().c_str(), p2.dumpStr().c_str(),
				p3.dumpStr().c_str());
#endif

	if (_luaThread) {
		pushValue(p1);
		pushValue(p2);
		pushValue(p3);
		_resume(3);
	}
}

/*static*/
TeLuaThread *TeLuaThread::threadFromState(lua_State *state) {
	Common::Array<TeLuaThread *> *threads = threadList();
	for (auto &thread : *threads) {
		if (thread->_luaThread == state)
			return thread;
	}
	return nullptr;
}

/*static*/
Common::Array<TeLuaThread *> *TeLuaThread::threadList() {
	if (!_threadList)
		_threadList = new Common::Array<TeLuaThread *>();
	return _threadList;
}

/*static*/
void TeLuaThread::cleanup() {
	delete _threadList;
	_threadList = nullptr;
}

int TeLuaThread::yield() {
	return lua_yield(_luaThread, 0);
}

} // end namespace Tetraedge
