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

#ifndef TETRAEDGE_TE_TE_LUA_SCRIPT_H
#define TETRAEDGE_TE_TE_LUA_SCRIPT_H

#include "common/str.h"
#include "common/path.h"
#include "tetraedge/te/te_variant.h"
#include "tetraedge/tetraedge.h"

namespace Tetraedge {

class TeLuaContext;

class TeLuaScript {
public:
	TeLuaScript();

	void attachToContext(TeLuaContext *context);

	void execute();
	void execute(const Common::String &fname);
	void execute(const Common::String &fname, const TeVariant &p1);
	void execute(const Common::String &fname, const TeVariant &p1, const TeVariant &p2);
	void execute(const Common::String &fname, const TeVariant &p1, const TeVariant &p2, const TeVariant &p3);

	void load(const TetraedgeFSNode &node);
	void unload();

private:
	TeLuaContext *_luaContext;

	TetraedgeFSNode _scriptNode;
	bool _started;
};

} // end namespace Tetraedge

#endif // TETRAEDGE_TE_TE_LUA_SCRIPT_H
