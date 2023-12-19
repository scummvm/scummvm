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
HSQOBJECT sqtoobj(HSQUIRRELVM v, T value);

template<typename T>
SQInteger sqpush(HSQUIRRELVM v, T value);

template<typename T>
SQRESULT sqget(HSQUIRRELVM v, int index, T &value);

// set field
template<typename T>
void sqsetf(HSQOBJECT o, const Common::String &key, T obj) {
	HSQUIRRELVM v = g_engine->getVm();
	SQInteger top = sq_gettop(v);
	sq_pushobject(v, o);
	sq_pushstring(v, key.c_str(), -1);
	sqpush(v, obj);
	sq_rawset(v, -3);
	sq_settop(v, top);
}

template<typename T>
void sqgetf(HSQUIRRELVM v, HSQOBJECT o, const Common::String &name, T &value) {
	sq_pushobject(v, o);
	sq_pushstring(v, name.c_str(), -1);
	if (SQ_FAILED(sq_get(v, -2)))
		sq_pop(v, 1);
	else {
		sqget(v, -1, value);
		sq_pop(v, 2);
	}
}

template<typename T>
void sqgetf(HSQOBJECT o, const Common::String &name, T &value) {
	HSQUIRRELVM v = g_engine->getVm();
	sqgetf(v, o, name, value);
}

void setId(HSQOBJECT &o, int id);

void sqgetarray(HSQUIRRELVM v, HSQOBJECT o, Common::Array<Common::String> &arr);
SQRESULT sqgetarray(HSQUIRRELVM v, int i, Common::Array<Common::String> &arr);

template<typename TFunc>
void sqgetitems(HSQOBJECT o, TFunc func) {
	HSQUIRRELVM v = g_engine->getVm();
	sq_pushobject(v, o);
	sq_pushnull(v);
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		HSQOBJECT obj;
		sqget(v, -1, obj);
		func(obj);
		sq_pop(v, 2);
	}
	sq_pop(v, 2);
}

template<typename TFunc>
void sqgetpairs(HSQOBJECT obj, TFunc func) {
	HSQUIRRELVM v = g_engine->getVm();
	sq_pushobject(v, obj);
	sq_pushnull(v);
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		Common::String key;
		HSQOBJECT o;
		sqget(v, -1, o);
		sqget(v, -2, key);
		func(key, o);
		sq_pop(v, 2);
	}
	sq_pop(v, 2);
}

template<typename T>
void sqnewf(HSQOBJECT o, const Common::String &key, T obj) {
	HSQUIRRELVM v = g_engine->getVm();
	SQInteger top = sq_gettop(v);
	sq_pushobject(v, o);
	sq_pushstring(v, key.c_str(), -1);
	sqpush(v, obj);
	sq_newslot(v, -3, SQFalse);
	sq_settop(v, top);
}

bool sqrawexists(HSQOBJECT obj, const Common::String &name);
void sqsetdelegate(HSQOBJECT obj, HSQOBJECT del);
HSQOBJECT sqrootTbl(HSQUIRRELVM v);
int sqparamCount(HSQUIRRELVM v, HSQOBJECT obj, const Common::String& name);
void sqcall(const Common::String &name, int numArgs = 0, HSQOBJECT *args = nullptr);
void sqcall(HSQOBJECT o, const Common::String& name, int numArgs = 0, HSQOBJECT* args = nullptr);
void sqexec(HSQUIRRELVM v, const char *code);

class Room;
class Object;

Room *sqroom(HSQOBJECT table);
Room *sqroom(HSQUIRRELVM v, int i);
Object *sqobj(HSQOBJECT table);
Object *sqobj(HSQUIRRELVM v, int i);
Object* sqactor(HSQOBJECT table);
Object* sqactor(HSQUIRRELVM v, int i);

} // namespace Twp

#endif
