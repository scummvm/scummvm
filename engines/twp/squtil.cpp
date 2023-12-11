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
void push(HSQUIRRELVM v, int value) {
	sq_pushinteger(v, value);
}

template<>
void push(HSQUIRRELVM v, bool value) {
	sq_pushinteger(v, value ? 1 : 0);
}

template<>
void push(HSQUIRRELVM v, Common::String value) {
	sq_pushstring(v, value.c_str(), value.size());
}

template<>
void push(HSQUIRRELVM v, HSQOBJECT value) {
	sq_pushobject(v, value);
}

template<>
HSQOBJECT sqobj(HSQUIRRELVM v, int value) {
	SQObject o;
	o._type = OT_INTEGER;
	o._unVal.nInteger = value;
	return o;
}

template<>
HSQOBJECT sqobj(HSQUIRRELVM v, const SQChar *value) {
	SQObject o;
	o._type = OT_STRING;
	o._unVal.pString = SQString::Create(_ss(v), value);
	return o;
}

template<>
SQRESULT get(HSQUIRRELVM v, int i, SQInteger &value) {
	return sq_getinteger(v, i, &value);
}

template<>
SQRESULT get(HSQUIRRELVM v, int i, int &value) {
	SQInteger itg;
	SQRESULT result = sq_getinteger(v, i, &itg);
	value = static_cast<int>(itg);
	return result;
}

template<>
SQRESULT get(HSQUIRRELVM v, int i, float &value) {
	SQFloat f;
	SQRESULT result = sq_getfloat(v, i, &f);
	value = static_cast<float>(f);
	return result;
}

template<>
SQRESULT get(HSQUIRRELVM v, int i, Common::String &value) {
	const SQChar *s;
	SQRESULT result = sq_getstring(v, i, &s);
	value = s;
	return result;
}

template<>
SQRESULT get(HSQUIRRELVM v, int i, HSQOBJECT &value) {
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
	setf(o, "_id", id);
}

} // namespace Twp
