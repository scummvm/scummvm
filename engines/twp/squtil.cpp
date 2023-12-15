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

#include "twp/squtil.h"
#include "twp/room.h"
#include "twp/object.h"
#include "squirrel/squirrel.h"
#include "squirrel/sqvm.h"
#include "squirrel/sqobject.h"
#include "squirrel/sqstring.h"
#include "squirrel/sqstate.h"
#include "squirrel/sqtable.h"
#include "squirrel/sqstdstring.h"
#include "squirrel/sqstdmath.h"
#include "squirrel/sqstdio.h"
#include "squirrel/sqstdaux.h"
#include "squirrel/sqfuncproto.h"
#include "squirrel/sqclosure.h"

namespace Twp {

template<>
void sqpush(HSQUIRRELVM v, int value) {
	sq_pushinteger(v, value);
}

template<>
void sqpush(HSQUIRRELVM v, bool value) {
	sq_pushinteger(v, value ? 1 : 0);
}

template<>
void sqpush(HSQUIRRELVM v, Common::String value) {
	sq_pushstring(v, value.c_str(), value.size());
}

template<>
void sqpush(HSQUIRRELVM v, HSQOBJECT value) {
	sq_pushobject(v, value);
}

template<>
HSQOBJECT sqtoobj(HSQUIRRELVM v, int value) {
	SQObject o;
	o._type = OT_INTEGER;
	o._unVal.nInteger = value;
	return o;
}

template<>
HSQOBJECT sqtoobj(HSQUIRRELVM v, const SQChar *value) {
	SQObject o;
	o._type = OT_STRING;
	o._unVal.pString = SQString::Create(_ss(v), value);
	return o;
}

template<>
SQRESULT sqget(HSQUIRRELVM v, int i, SQInteger &value) {
	return sq_getinteger(v, i, &value);
}

template<>
SQRESULT sqget(HSQUIRRELVM v, int i, int &value) {
	SQInteger itg;
	SQRESULT result = sq_getinteger(v, i, &itg);
	value = static_cast<int>(itg);
	return result;
}

template<>
SQRESULT sqget(HSQUIRRELVM v, int i, bool &value) {
	SQInteger itg;
	SQRESULT result = sq_getinteger(v, i, &itg);
	value = itg != 0;
	return result;
}

template<>
SQRESULT sqget(HSQUIRRELVM v, int i, float &value) {
	SQFloat f;
	SQRESULT result = sq_getfloat(v, i, &f);
	value = static_cast<float>(f);
	return result;
}

template<>
SQRESULT sqget(HSQUIRRELVM v, int i, Common::String &value) {
	const SQChar *s;
	SQRESULT result = sq_getstring(v, i, &s);
	value = s;
	return result;
}

template<>
SQRESULT sqget(HSQUIRRELVM v, int i, const SQChar *&value) {
	return sq_getstring(v, i, &value);
}

template<>
SQRESULT sqget(HSQUIRRELVM v, int i, HSQOBJECT &value) {
	return sq_getstackobj(v, i, &value);
}

void sqgetarray(HSQUIRRELVM v, HSQOBJECT o, Common::Array<Common::String> &arr) {
	sq_pushobject(v, o);
	sq_pushnull(v);
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		const SQChar *str;
		sq_getstring(v, -1, &str);
		arr.push_back(str);
		sq_pop(v, 2);
	}
	sq_pop(v, 1);
}

SQRESULT sqgetarray(HSQUIRRELVM v, int i, Common::Array<Common::String> &arr) {
	HSQOBJECT obj;
	SQRESULT result = sq_getstackobj(v, i, &obj);
	sqgetarray(v, obj, arr);
	return result;
}

void setId(HSQOBJECT &o, int id) {
	sqsetf(o, "_id", id);
}

bool sqrawexists(HSQOBJECT obj, const Common::String &name) {
	HSQUIRRELVM v = g_engine->getVm();
	SQInteger top = sq_gettop(v);
	sqpush(v, obj);
	sq_pushstring(v, name.c_str(), -1);
	if (SQ_SUCCEEDED(sq_rawget(v, -2))) {
		SQObjectType oType = sq_gettype(v, -1);
		sq_settop(v, top);
		return oType != OT_NULL;
	}
	sq_settop(v, top);
	return false;
}

void sqsetdelegate(HSQOBJECT obj, HSQOBJECT del) {
	HSQUIRRELVM v = g_engine->getVm();
	sqpush(v, obj);
	sqpush(v, del);
	sq_setdelegate(v, -2);
	sq_pop(v, 1);
}

HSQOBJECT sqrootTbl(HSQUIRRELVM v) {
	HSQOBJECT result;
	sq_resetobject(&result);
	sq_pushroottable(v);
	sq_getstackobj(v, -1, &result);
	sq_pop(v, 1);
	return result;
}

static int getId(HSQOBJECT table) {
	SQInteger result = 0;
	sqgetf(table, "_id", result);
	return (int)result;
}

Room *sqroom(HSQOBJECT table) {
	int id = getId(table);
	for (int i = 0; i < g_engine->_rooms.size(); i++) {
		Room *room = g_engine->_rooms[i];
		if (getId(room->_table) == id)
			return room;
	}
	return nullptr;
}

Room *sqroom(HSQUIRRELVM v, int i) {
	HSQOBJECT table;
	if (SQ_SUCCEEDED(sqget(v, i, table))) {
		return sqroom(table);
	}
	return nullptr;
}

Object *sqobj(HSQOBJECT table) {
	int id = getId(table);
	for (int i = 0; i < g_engine->_rooms.size(); i++) {
		Room *room = g_engine->_rooms[i];
		for (int j = 0; j < room->_layers.size(); i++) {
			Layer *layer = room->_layers[i];
			for (int k = 0; k < layer->_objects.size(); k++) {
				Object *obj = layer->_objects[i];
				if (getId(obj->_table) == id)
					return obj;
			}
		}
	}
	return nullptr;
}

Object *sqobj(HSQUIRRELVM v, int i) {
	HSQOBJECT table;
	sq_getstackobj(v, i, &table);
	return sqobj(table);
}

int sqparamCount(HSQUIRRELVM v, HSQOBJECT obj, const Common::String &name) {
	SQInteger top = sq_gettop(v);
	sqpush(v, obj);
	sq_pushstring(v, name.c_str(), -1);
	if (SQ_FAILED(sq_get(v, -2))) {
		sq_settop(v, top);
		debug("can't find %s function", name.c_str());
		return 0;
	}
	SQInteger nparams, nfreevars;
	sq_getclosureinfo(v, -1, &nparams, &nfreevars);
	debug("%s function found with %lld parameters", name.c_str(), nparams);
	sq_settop(v, top);
	return nparams;
}

static void sqpushfunc(HSQUIRRELVM v, HSQOBJECT o, const Common::String &name) {
	sq_pushobject(v, o);
	sq_pushstring(v, name.c_str(), -1);
	sq_get(v, -2);
}

static void sqcall(HSQUIRRELVM v, HSQOBJECT o, const Common::String &name, int numArgs, HSQOBJECT *args) {
	SQInteger top = sq_gettop(v);
	sqpushfunc(v, o, name);

	sq_pushobject(v, o);
	for (int i = 0; i < numArgs; i++) {
		sq_pushobject(v, args[i]);
	}
	sq_call(v, 1 + numArgs, SQFalse, SQTrue);
	sq_settop(v, top);
}

void sqcall(HSQOBJECT o, const Common::String &name, int numArgs, HSQOBJECT *args) {
	sqcall(g_engine->getVm(), o, name, numArgs, args);
}

void sqcall(const Common::String &name, int numArgs, HSQOBJECT *args) {
	HSQUIRRELVM v = g_engine->getVm();
	sqcall(v, sqrootTbl(v), name, numArgs, args);
}

void sqexec(HSQUIRRELVM v, const char *code) {
	SQInteger top = sq_gettop(v);
	if (SQ_FAILED(sq_compilebuffer(v, code, strlen(code), "twp", SQTrue))) {
		sqstd_printcallstack(v);
		return;
	}
	sq_pushroottable(v);
	if (SQ_FAILED(sq_call(v, 1, SQFalse, SQTrue))) {
		sqstd_printcallstack(v);
		sq_pop(v, 1); // removes the closure
		return;
	}
	sq_settop(v, top);
}

} // namespace Twp
