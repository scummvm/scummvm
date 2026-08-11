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

#include "common/debug.h"
#include "common/textconsole.h"
#include "common/file.h"
#include "tetraedge/te/te_lua_script.h"
#include "tetraedge/te/te_lua_thread.h"
#include "tetraedge/te/te_lua_context.h"

namespace Tetraedge {

TeLuaScript::TeLuaScript() : _started(false), _luaContext(nullptr) {
}

void TeLuaScript::attachToContext(TeLuaContext *context) {
	_luaContext = context;
}

void TeLuaScript::execute() {
	if (_luaContext) {
		//debug("TeLuaScript::execute %s", _scriptNode.toString().c_str());
		lua_State *state = _luaContext->luaState();
		if (state) {
			TeLuaThread *thread = TeLuaThread::create(_luaContext);
			thread->executeFile(_scriptNode);
			thread->release();
			_started = true;
		}
	}
}

void TeLuaScript::execute(const Common::String &fname) {
	if (_luaContext) {
		//debug("TeLuaScript::execute %s %s", _scriptNode.toString().c_str(), fname.c_str());
		TeLuaThread *thread = TeLuaThread::create(_luaContext);
		thread->execute(fname);
		thread->release();
	}
}

void TeLuaScript::execute(const Common::String &fname, const TeVariant &p1) {
	if (_luaContext) {
		//debug("TeLuaScript::execute %s %s(%s)", _scriptNode.toString().c_str(), fname.c_str(), p1.toString().c_str());
		TeLuaThread *thread = TeLuaThread::create(_luaContext);
		thread->execute(fname, p1);
		thread->release();
	}
}

void TeLuaScript::execute(const Common::String &fname, const TeVariant &p1, const TeVariant &p2) {
	if (_luaContext) {
		TeLuaThread *thread = TeLuaThread::create(_luaContext);
		thread->execute(fname, p1, p2);
		thread->release();
	}
}

void TeLuaScript::execute(const Common::String &fname, const TeVariant &p1, const TeVariant &p2, const TeVariant &p3) {
	if (_luaContext) {
		TeLuaThread *thread = TeLuaThread::create(_luaContext);
		thread->execute(fname, p1, p2, p3);
		thread->release();
	}
}

void TeLuaScript::load(const TetraedgeFSNode &node) {
	_started = false;
	_scriptNode = node;
}

void TeLuaScript::unload() {
	_scriptNode = TetraedgeFSNode();
	_started = false;
}

} // end namespace Tetraedge
