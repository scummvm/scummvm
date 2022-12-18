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

#ifndef TETRAEDGE_TE_TE_LUA_CONTEXT_H
#define TETRAEDGE_TE_TE_LUA_CONTEXT_H

#include "common/error.h"
#include "common/str.h"
#include "common/serializer.h"

#include "tetraedge/te/te_variant.h"

struct lua_State;

namespace Tetraedge {

class TeLuaGUI;

/*
 * The lua state holder.  In the original Te engine this is split into an
 * interface and Impl class, but it just ends up being a 1-line wrapper in
 * each function so there's very little point.
 */
class TeLuaContext {
public:
	TeLuaContext();
	~TeLuaContext();

	void addBindings(void(*fn)(lua_State *));
	void create();
	void destroy();
	TeVariant global(const Common::String &name);
	void global(const Common::String &name, bool &outVal);
	void global(const Common::String &name, Common::String &outVal);
	void global(const Common::String &name, int &outVal);
	void global(const Common::String &name, float &outVal);
	bool isCreated() {
		return _luaState != nullptr;
	}

	//void load(TiXmlNode *node);
	//void save(TiXmlNode *node);

	lua_State *luaState() { return _luaState; }

	void registerCFunction(const Common::String &name, int(*fn)(lua_State *));

	void removeGlobal(const Common::String &name);

	void setGlobal(const Common::String &name, int val);
	void setGlobal(const Common::String &name, bool val);
	void setGlobal(const Common::String &name, const Common::String &val);

	void setInRegistry(const Common::String &name, TeLuaGUI *gui);

	Common::Error syncState(Common::Serializer &s);

private:
	lua_State *_luaState;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_LUA_CONTEXT_H
