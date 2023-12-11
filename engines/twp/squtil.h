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

#ifndef TWP_SQUTIL_H
#define TWP_SQUTIL_H

#include "squirrel/squirrel.h"
#include "common/str.h"
#include "twp/twp.h"
#include "twp/vm.h"

namespace Twp {

template<typename T>
HSQOBJECT sqobj(HSQUIRRELVM v, T value);

template<typename T>
void push(HSQUIRRELVM v, T value);

template<typename T>
SQRESULT get(HSQUIRRELVM v, int index, T &value);

// set field
template<typename T>
void setf(HSQOBJECT &o, const Common::String &key, T obj) {
	HSQUIRRELVM v = g_engine->getVm();
	SQInteger top = sq_gettop(v);
	sq_pushobject(v, o);
	sq_pushstring(v, key.c_str(), -1);
	push(v, obj);
	sq_rawset(v, -3);
	sq_settop(v, top);
}

template<typename T>
void getf(HSQUIRRELVM v, HSQOBJECT o, const Common::String &name, T &value) {
	sq_pushobject(v, o);
	sq_pushstring(v, name.c_str(), -1);
	if (SQ_FAILED(sq_get(v, -2)))
		sq_pop(v, 1);
	else {
		get(v, -1, value);
		sq_pop(v, 2);
	}
}

template<typename T>
void getf(HSQOBJECT o, const Common::String &name, T &value) {
	HSQUIRRELVM v = g_engine->getVm();
	getf(v, o, name, value);
}

void setId(HSQOBJECT &o, int id);

void sqgetarray(HSQUIRRELVM v, HSQOBJECT o, Common::Array<Common::String> &arr);
SQRESULT sqgetarray(HSQUIRRELVM v, int i, Common::Array<Common::String> &arr);

template <typename TFunc>
void sqgetitems(HSQOBJECT o, TFunc func) {
	HSQUIRRELVM v = g_engine->getVm();
	sq_pushobject(v, o);
	sq_pushnull(v);
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		HSQOBJECT obj;
		get(v, -1, obj);
		func(obj);
		sq_pop(v, 2);
	}
	sq_pop(v, 2);
}

} // namespace Twp

#endif
