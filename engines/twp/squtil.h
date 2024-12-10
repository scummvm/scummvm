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

#include "common/util.h"
#include "common/str.h"
#include "common/util.h"
#include "twp/twp.h"
#include "twp/audio.h"
#include "twp/vm.h"
#include "twp/squirrel/squirrel.h"

namespace Twp {

HSQOBJECT sqrootTbl(HSQUIRRELVM v);

template<typename T>
HSQOBJECT sqtoobj(HSQUIRRELVM v, T value);

template<typename T>
SQRESULT sqget(HSQUIRRELVM v, int index, T &value);

SQInteger sqpush(HSQUIRRELVM v);

template<typename T>
SQInteger sqpush(HSQUIRRELVM v, T value);

template<typename T, typename... Args>
void sqpush(HSQUIRRELVM v, T first, Args... args) {
	sqpush(v, first);
	sqpush(v, Common::move(args)...);
}

// set field
template<typename T>
void sqsetf(HSQOBJECT o, const Common::String &key, T obj) {
	HSQUIRRELVM v = g_twp->getVm();
	SQInteger top = sq_gettop(v);
	sq_pushobject(v, o);
	sq_pushstring(v, key.c_str(), -1);
	sqpush(v, Common::move(obj));
	sq_rawset(v, -3);
	sq_settop(v, top);
}

template<typename T>
SQRESULT sqgetf(HSQUIRRELVM v, HSQOBJECT o, const Common::String &name, T &value) {
	sq_pushobject(v, o);
	sq_pushstring(v, name.c_str(), -1);
	if (SQ_FAILED(sq_get(v, -2))) {
		sq_pop(v, 1);
		return sq_throwerror(v, "Failed to get field");
	}

	if (SQ_FAILED(sqget(v, -1, value)))
		return sq_throwerror(v, "Failed to get field");

	sq_pop(v, 2);
	return SQ_OK;
}

template<typename T>
SQRESULT sqgetf(HSQOBJECT o, const Common::String &name, T &value) {
	HSQUIRRELVM v = g_twp->getVm();
	return sqgetf(v, o, name, value);
}

template<typename T>
SQRESULT sqgetf(const Common::String &name, T &value) {
	HSQUIRRELVM v = g_twp->getVm();
	return sqgetf(v, sqrootTbl(v), name, value);
}

void setId(HSQOBJECT &o, int id);

void sqgetarray(HSQUIRRELVM v, HSQOBJECT o, Common::Array<Common::String> &arr);
SQRESULT sqgetarray(HSQUIRRELVM v, int i, Common::Array<Common::String> &arr);
SQRESULT sqgetarray(HSQUIRRELVM v, int i, Common::Array<Common::SharedPtr<SoundDefinition> > &arr);

template<typename TFunc>
void sqgetitems(HSQOBJECT o, TFunc func) {
	HSQUIRRELVM v = g_twp->getVm();
	sq_pushobject(v, o);
	sq_pushnull(v);
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		HSQOBJECT obj;
		if (SQ_FAILED(sqget(v, -1, obj)))
			error("Failed to get item");
		func(obj);
		sq_pop(v, 2);
	}
	sq_pop(v, 2);
}

SQRESULT sqgetpairs(HSQOBJECT obj, void func(const Common::String &key, HSQOBJECT &obj, void *data), void *data);

template<typename TFunc>
SQRESULT sqgetpairs(HSQOBJECT obj, TFunc func) {
	HSQUIRRELVM v = g_twp->getVm();
	sq_pushobject(v, obj);
	sq_pushnull(v);
	while (SQ_SUCCEEDED(sq_next(v, -2))) {
		Common::String key;
		HSQOBJECT o;
		if (SQ_FAILED(sqget(v, -1, o)))
			return sq_throwerror(v, "failed to get object");
		if (SQ_FAILED(sqget(v, -2, key)))
			return sq_throwerror(v, "failed to get key");
		func(key, o);
		sq_pop(v, 2);
	}
	sq_pop(v, 2);
	return SQ_OK;
}

template<typename T>
void sqnewf(HSQOBJECT o, const Common::String &key, T obj) {
	HSQUIRRELVM v = g_twp->getVm();
	SQInteger top = sq_gettop(v);
	sq_pushobject(v, o);
	sq_pushstring(v, key.c_str(), -1);
	sqpush(v, obj);
	sq_newslot(v, -3, SQFalse);
	sq_settop(v, top);
}

bool sqrawexists(HSQOBJECT obj, const Common::String &name);
void sqsetdelegate(HSQOBJECT obj, HSQOBJECT del);

void sqpushfunc(HSQUIRRELVM v, HSQOBJECT o, const char *name);
int sqparamCount(HSQUIRRELVM v, HSQOBJECT obj, const Common::String &name);
void sqcall(const char *name, const Common::Array<HSQOBJECT> &args);

template<typename... T>
void sqcall(const char *name, T... args);

template<typename... T>
void sqcall(HSQOBJECT o, const char *name, T... args);

template<typename TResult, typename... T>
static void sqcallfunc(TResult &result, HSQOBJECT o, const char *name, T... args);

template<typename TResult, typename... T>
static void sqcallfunc(TResult &result, const char *name, T... args);

void sqexec(HSQUIRRELVM v, const char *code, const char *filename = nullptr);

class Room;
class Object;

int getId(HSQOBJECT table);
Common::SharedPtr<Room> sqroom(HSQOBJECT table);
Common::SharedPtr<Room> sqroom(HSQUIRRELVM v, int i);
Common::SharedPtr<Room> getRoom(int id);
Common::SharedPtr<Object> sqobj(HSQOBJECT table);
Common::SharedPtr<Object> sqobj(HSQUIRRELVM v, int i);
Common::SharedPtr<Object> sqobj(int i);
Common::SharedPtr<Object> sqactor(HSQOBJECT table);
Common::SharedPtr<Object> sqactor(HSQUIRRELVM v, int i);
Common::SharedPtr<SoundDefinition> sqsounddef(HSQUIRRELVM v, int i);
Common::SharedPtr<SoundDefinition> sqsounddef(int id);
Common::SharedPtr<ThreadBase> sqthread(HSQUIRRELVM v);
Common::SharedPtr<ThreadBase> sqthread(HSQUIRRELVM v, int id);
Common::SharedPtr<ThreadBase> sqthread(int id);
Light *sqlight(int id);
Light *sqlight(HSQUIRRELVM v, int i);

template<typename... T>
void sqcall(HSQUIRRELVM v, HSQOBJECT o, const char *name, T... args) {
	constexpr size_t n = sizeof...(T);
	SQInteger top = sq_gettop(v);
	sqpushfunc(v, o, name);

	sq_pushobject(v, o);
	if (n > 0) {
		sqpush(v, Common::forward<T>(args)...);
	}
	sq_call(v, 1 + n, SQFalse, SQTrue);
	sq_settop(v, top);
}

template<typename... T>
void sqcall(HSQOBJECT o, const char *name, T... args) {
	constexpr size_t n = sizeof...(T);
	HSQUIRRELVM v = g_twp->getVm();
	SQInteger top = sq_gettop(v);
	sqpushfunc(v, o, name);

	sq_pushobject(v, o);
	if (n > 0) {
		sqpush(v, Common::forward<T>(args)...);
	}
	sq_call(v, 1 + n, SQFalse, SQTrue);
	sq_settop(v, top);
}

template<typename... T>
void sqcall(const char *name, T... args) {
	constexpr size_t n = sizeof...(T);
	HSQUIRRELVM v = g_twp->getVm();
	HSQOBJECT o = sqrootTbl(v);
	SQInteger top = sq_gettop(v);
	sqpushfunc(v, o, name);

	sq_pushobject(v, o);
	if (n > 0) {
		sqpush(v, Common::forward<T>(args)...);
	}
	sq_call(v, 1 + n, SQFalse, SQTrue);
	sq_settop(v, top);
}

template<typename TResult, typename... T>
void sqcallfunc(TResult &result, HSQOBJECT o, const char *name, T... args) {
	constexpr size_t n = sizeof...(T);
	HSQUIRRELVM v = g_twp->getVm();
	SQInteger top = sq_gettop(v);
	sqpush(v, o);
	sq_pushstring(v, _SC(name), -1);
	if (SQ_FAILED(sq_get(v, -2))) {
		sq_settop(v, top);
		error("can't find %s function", name);
		return;
	}
	sq_remove(v, -2);

	sqpush(v, o);
	sqpush(v, Common::forward<T>(args)...);
	if (SQ_FAILED(sq_call(v, n + 1, SQTrue, SQTrue))) {
		// sqstd_printcallstack(v);
		sq_settop(v, top);
		error("function %s call failed", name);
		return;
	}
	if (SQ_FAILED(sqget(v, -1, result)))
		error("function %s call failed to get result", name);
	sq_settop(v, top);
}

template<typename TResult, typename... T>
void sqcallfunc(TResult &result, const char *name, T... args) {
	constexpr size_t n = sizeof...(T);
	HSQUIRRELVM v = g_twp->getVm();
	HSQOBJECT o = sqrootTbl(v);
	SQInteger top = sq_gettop(v);
	sqpush(v, o);
	sq_pushstring(v, _SC(name), -1);
	if (SQ_FAILED(sq_get(v, -2))) {
		sq_settop(v, top);
		error("can't find %s function", name);
		return;
	}
	sq_remove(v, -2);

	sqpush(v, o);
	sqpush(v, Common::forward<T>(args)...);
	if (SQ_FAILED(sq_call(v, n + 1, SQTrue, SQTrue))) {
		// sqstd_printcallstack(v);
		sq_settop(v, top);
		error("function %s call failed", name);
		return;
	}
	if (SQ_FAILED(sqget(v, -1, result))) {
		sq_settop(v, top);
		error("function %s failed to get result", name);
		return;
	}
	sq_settop(v, top);
}

} // namespace Twp

#endif
