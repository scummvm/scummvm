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
#include "twp/thread.h"
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
SQInteger sqpush(HSQUIRRELVM v, int value) {
	sq_pushinteger(v, value);
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, long long value) {
	sq_pushinteger(v, (SQInteger)value);
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, float value) {
	sq_pushfloat(v, value);
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, double value) {
	sq_pushfloat(v, (SQFloat)value);
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, bool value) {
	sq_pushinteger(v, value ? 1 : 0);
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, Common::String value) {
	sq_pushstring(v, value.c_str(), value.size());
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, HSQOBJECT value) {
	sq_pushobject(v, value);
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, Math::Vector2d value) {
	sq_newtable(v);
	sq_pushstring(v, "x", -1);
	sq_pushinteger(v, value.getX());
	sq_newslot(v, -3, SQFalse);
	sq_pushstring(v, "y", -1);
	sq_pushinteger(v, value.getY());
	sq_newslot(v, -3, SQFalse);
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, Rectf value) {
	sq_newtable(v);
	sq_pushstring(v, "x1", -1);
	sq_pushinteger(v, value.left());
	sq_newslot(v, -3, SQFalse);
	sq_pushstring(v, "y1", -1);
	sq_pushinteger(v, value.bottom());
	sq_newslot(v, -3, SQFalse);
	sq_pushstring(v, "x2", -1);
	sq_pushinteger(v, value.right());
	sq_newslot(v, -3, SQFalse);
	sq_pushstring(v, "y2", -1);
	sq_pushinteger(v, value.top());
	sq_newslot(v, -3, SQFalse);
	return 1;
}

template<>
SQInteger sqpush(HSQUIRRELVM v, Common::JSONValue *node) {
	if (node->isIntegerNumber()) {
		return sqpush(v, (int)node->asIntegerNumber());
	} else if (node->isString()) {
		return sqpush(v, node->asString());
	} else if (node->isString()) {
		return sqpush(v, (float)node->asNumber());
	} else if (node->isNull()) {
		sq_pushnull(v);
		return 1;
	} else {
		return sq_throwerror(v, "This kind of node is not supported");
	}
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

void sqcall(const char *name, const Common::Array<HSQOBJECT> &args) {
	HSQUIRRELVM v = g_engine->getVm();
	HSQOBJECT o = sqrootTbl(v);
	SQInteger top = sq_gettop(v);
	sqpushfunc(v, o, name);

	sq_pushobject(v, o);
	for (int i = 0; i < args.size(); i++) {
		sq_pushobject(v, args[i]);
	}
	sq_call(v, 1 + args.size(), SQFalse, SQTrue);
	sq_settop(v, top);
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
	for (int i = 0; i < g_engine->_actors.size(); i++) {
		Object *actor = g_engine->_actors[i];
		if (getId(actor->_table) == id)
			return actor;
	}

	for (int i = 0; i < g_engine->_rooms.size(); i++) {
		Room *room = g_engine->_rooms[i];
		for (int j = 0; j < room->_layers.size(); j++) {
			Layer *layer = room->_layers[j];
			for (int k = 0; k < layer->_objects.size(); k++) {
				Object *obj = layer->_objects[k];
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

Object *sqactor(HSQOBJECT table) {
	int id = getId(table);
	for (int i = 0; i < g_engine->_actors.size(); i++) {
		Object *actor = g_engine->_actors[i];
		if (actor->getId() == id)
			return actor;
	}
	return nullptr;
}

Object *sqactor(HSQUIRRELVM v, int i) {
	HSQOBJECT table;
	if (SQ_SUCCEEDED(sqget(v, i, table)))
		return sqactor(table);
	return nullptr;
}

SoundDefinition *sqsounddef(int id) {
	for (int i = 0; i < g_engine->_audio._soundDefs.size(); i++) {
		SoundDefinition *sound = g_engine->_audio._soundDefs[i];
		if (sound->getId() == id)
			return sound;
	}
	return nullptr;
}

SoundDefinition *sqsounddef(HSQUIRRELVM v, int i) {
	int id;
	if (SQ_SUCCEEDED(sqget(v, i, id)))
		return sqsounddef(id);
	return nullptr;
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

void sqpushfunc(HSQUIRRELVM v, HSQOBJECT o, const char *name) {
	sq_pushobject(v, o);
	sq_pushstring(v, name, -1);
	sq_get(v, -2);
}

void sqexec(HSQUIRRELVM v, const char *code, const char *filename) {
	SQInteger top = sq_gettop(v);
	if (SQ_FAILED(sq_compilebuffer(v, code, strlen(code), filename ? filename : "twp", SQTrue))) {
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

ThreadBase *sqthread(HSQUIRRELVM v, int i) {
	int id;
	if (SQ_SUCCEEDED(sqget(v, i, id)))
		return sqthread(id);
	return nullptr;
}

ThreadBase *sqthread(int id) {
	if (g_engine->_cutscene) {
		if (g_engine->_cutscene->getId() == id) {
			return g_engine->_cutscene;
		}
	}

	for (int i = 0; i < g_engine->_threads.size(); i++) {
		ThreadBase *t = g_engine->_threads[i];
		if (t->getId() == id) {
			return t;
		}
	}
	return nullptr;
}

ThreadBase *sqthread(HSQUIRRELVM v) {
	if (g_engine->_cutscene) {
		if (g_engine->_cutscene->getThread() == v) {
			return g_engine->_cutscene;
		}
	}

	return *Common::find_if(g_engine->_threads.begin(), g_engine->_threads.end(), [&](ThreadBase *t) {
		return t->getThread() == v;
	});
}

static void sqgetarray(HSQUIRRELVM v, HSQOBJECT o, Common::Array<SoundDefinition *> &arr) {
	sq_pushobject(v, o);
	sq_pushnull(v);
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		arr.push_back(sqsounddef(v, -1));
		sq_pop(v, 2);
	}
	sq_pop(v, 1);
}

SQRESULT sqgetarray(HSQUIRRELVM v, int i, Common::Array<SoundDefinition *> &arr) {
	HSQOBJECT obj;
	SQRESULT result = sq_getstackobj(v, i, &obj);
	sqgetarray(v, obj, arr);
	return result;
}

} // namespace Twp
