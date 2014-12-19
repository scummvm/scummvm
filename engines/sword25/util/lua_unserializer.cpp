/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sword25/util/lua_serialization.h"

#include "sword25/util/double_serializer.h"
#include "sword25/util/lua_serialization_util.h"

#include "common/stream.h"

#include "lua/lobject.h"
#include "lua/lstate.h"
#include "lua/lgc.h"
#include "lua/lopcodes.h"


namespace Lua {

struct UnSerializationInfo {
	lua_State *luaState;
	Common::ReadStream *readStream;
};

static void unserializeObject(UnSerializationInfo *info);

static void unserializeBoolean(UnSerializationInfo *info);
static void unserializeNumber(UnSerializationInfo *info);
static void unserializeString(UnSerializationInfo *info);
static void unserializeTable(UnSerializationInfo *info, int index);
static void unserializeFunction(UnSerializationInfo *info, int index);
static void unserializeThread(UnSerializationInfo *info, int index);
static void unserializeProto(UnSerializationInfo *info, int index);
static void unserializeUpValue(UnSerializationInfo *info, int index);
static void unserializeUserData(UnSerializationInfo *info, int index);
static void unserializePermanent(UnSerializationInfo *info, int index);


void unserializeLua(lua_State *luaState, Common::ReadStream *readStream) {
	UnSerializationInfo info;
	info.luaState = luaState;
	info.readStream = readStream;

	// The process starts with the lua stack as follows:
	// >>>>> permTbl
	// That's the table of permanents

	// Make sure there is enough room on the stack
	lua_checkstack(luaState, 3);

	// Create a table to hold indexes of everything thats already been read
	lua_newtable(luaState);
	// >>>>> permTbl indexTbl

	// Prevent garbage collection while we unserialize
	lua_gc(luaState, LUA_GCSTOP, 0);

	// Unserialize the root object
	unserializeObject(&info);
	// >>>>> permTbl indexTbl rootObj

	// Re-start garbage collection
	lua_gc(luaState, LUA_GCRESTART, 0);

	// Remove the indexTbl
	lua_replace(luaState, 2);
	// >>>>> permTbl rootObj
}

/* The object is left on the stack. This is primarily used by unpersist, but
 * may be used by GCed objects that may incur cycles in order to preregister
 * the object. */
static void registerObjectInIndexTable(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ...... obj

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	lua_pushlightuserdata(info->luaState, (void *)index);
	// >>>>> permTbl indexTbl ...... obj index

	lua_pushvalue(info->luaState, -2);
	// >>>>> permTbl indexTbl ...... obj index obj

	// Push the k/v pair into the indexTbl
	lua_settable(info->luaState, 2);
	// >>>>> permTbl indexTbl ...... obj
}

static void unserializeObject(UnSerializationInfo *info) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	byte isARealValue = info->readStream->readByte();
	if (isARealValue) {
		int index = info->readStream->readSint32LE();
		int type = info->readStream->readSint32LE();

		switch (type) {
		case LUA_TBOOLEAN:
			unserializeBoolean(info);
			break;
		case LUA_TLIGHTUSERDATA:
			// You can't serialize a pointer
			// It would be meaningless on the next run
			assert(0);
			break;
		case LUA_TNUMBER:
			unserializeNumber(info);
			break;
		case LUA_TSTRING:
			unserializeString(info);
			break;
		case LUA_TTABLE:
			unserializeTable(info, index);
			break;
		case LUA_TFUNCTION:
			unserializeFunction(info, index);
			break;
		case LUA_TTHREAD:
			unserializeThread(info, index);
			break;
		case LUA_TPROTO:
			unserializeProto(info, index);
			break;
		case LUA_TUPVAL:
			unserializeUpValue(info, index);
			break;
		case LUA_TUSERDATA:
			unserializeUserData(info, index);
			break;
		case PERMANENT_TYPE:
			unserializePermanent(info, index);
			break;
		default:
			assert(0);
		}


		// >>>>> permTbl indexTbl ...... obj
		assert(lua_type(info->luaState, -1) == type ||
		       type == PERMANENT_TYPE ||
		       // Remember, upvalues get a special dispensation, as described in boxUpValue
		       (lua_type(info->luaState, -1) == LUA_TFUNCTION && type == LUA_TUPVAL));

		registerObjectInIndexTable(info, index);
		// >>>>> permTbl indexTbl ...... obj
	} else {
		int index = info->readStream->readSint32LE();

		if (index == 0) {
			lua_pushnil(info->luaState);
			// >>>>> permTbl indexTbl ...... nil
		} else {
			// Fetch the object from the indexTbl

			lua_pushlightuserdata(info->luaState, (void *)index);
			// >>>>> permTbl indexTbl ...... index

			lua_gettable(info->luaState, 2);
			// >>>>> permTbl indexTbl ...... ?obj?

			assert(!lua_isnil(info->luaState, -1));
		}
		// >>>>> permTbl indexTbl ...... obj/nil
	}

	// >>>>> permTbl indexTbl ...... obj/nil
}

static void unserializeBoolean(UnSerializationInfo *info) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 1);

	int value = info->readStream->readSint32LE();

	lua_pushboolean(info->luaState, value);
	// >>>>> permTbl indexTbl ...... bool
}

static void unserializeNumber(UnSerializationInfo *info) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 1);

	// Read the serialized double
	Util::SerializedDouble serializedValue;
	serializedValue.significandOne = info->readStream->readUint32LE();
	serializedValue.signAndSignificandTwo = info->readStream->readUint32LE();
	serializedValue.exponent = info->readStream->readSint16LE();

	lua_Number value = Util::decodeDouble(serializedValue);

	lua_pushnumber(info->luaState, value);
	// >>>>> permTbl indexTbl ...... num
}

static void unserializeString(UnSerializationInfo *info) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 1);

	uint32 length = info->readStream->readUint32LE();
	char *string = new char[length];

	info->readStream->read(string, length);
	lua_pushlstring(info->luaState, string, length);

	// >>>>> permTbl indexTbl ...... string

	delete[] string;
}

static void unserializeSpecialTable(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 1);

	unserializeObject(info);

	// >>>>> permTbl indexTbl ...... spfunc
	lua_call(info->luaState, 0, 1);
	// >>>>> permTbl indexTbl ...... tbl
}

static void unserializeLiteralTable(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 3);

	// Preregister table for handling of cycles
	lua_newtable(info->luaState);

	// >>>>> permTbl indexTbl ...... tbl
	registerObjectInIndexTable(info, index);
	// >>>>> permTbl indexTbl ...... tbl

	// Unserialize metatable
	unserializeObject(info);
	// >>>>> permTbl indexTbl ...... tbl ?metaTbl/nil?

	if (lua_istable(info->luaState, -1)) {
		// >>>>> permTbl indexTbl ...... tbl metaTbl
		lua_setmetatable(info->luaState, -2);
		// >>>>> permTbl indexTbl ...... tbl
	} else {
		// >>>>> permTbl indexTbl ...... tbl nil
		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... tbl
	}
	// >>>>> permTbl indexTbl ...... tbl


	while (1) {
		// >>>>> permTbl indexTbl ...... tbl
		unserializeObject(info);
		// >>>>> permTbl indexTbl ...... tbl key/nil

		// The table serialization is nil terminated
		if (lua_isnil(info->luaState, -1)) {
			// >>>>> permTbl indexTbl ...... tbl nil
			lua_pop(info->luaState, 1);
			// >>>>> permTbl indexTbl ...... tbl

			break;
		}

		// >>>>> permTbl indexTbl ...... tbl key
		unserializeObject(info);
		// >>>>> permTbl indexTbl ...... tbl value

		lua_rawset(info->luaState, -3);
		// >>>>> permTbl indexTbl ...... tbl
	}
}

void unserializeTable(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 1);

	int isSpecial = info->readStream->readSint32LE();

	if (isSpecial) {
		unserializeSpecialTable(info, index);
		// >>>>> permTbl indexTbl ...... tbl
	} else {
		unserializeLiteralTable(info, index);
		// >>>>> permTbl indexTbl ...... tbl
	}
}



void *lua_realloc(lua_State *luaState, void *block, size_t osize, size_t nsize) {
	global_State *globalState = G(luaState);

	block = (*globalState->frealloc)(globalState->ud, block, osize, nsize);
	globalState->totalbytes = (globalState->totalbytes - osize) + nsize;

	return block;
}

#define lua_malloc(luaState, nsize) lua_realloc(luaState, nullptr, 0, nsize)
#define lua_reallocv(luaState, block, on, n, e) lua_realloc(luaState, block, (on) * (e), (n) * (e))
#define lua_reallocvector(luaState, vec, oldn, n, T) ((vec) = (T *)(lua_reallocv(luaState, vec, oldn, n, sizeof(T))))
#define lua_newVector(luaState, num, T) ((T *)lua_reallocv(luaState, nullptr, 0, num, sizeof(T)))
#define lua_new(luaState,T) (T *)lua_malloc(luaState, sizeof(T))

void lua_linkObjToGC(lua_State *luaState, GCObject *obj, lu_byte type) {
	global_State *globalState = G(luaState);

	obj->gch.next = globalState->rootgc;
	globalState->rootgc = obj;
	obj->gch.marked = luaC_white(globalState);
	obj->gch.tt = type;
}

#define sizeLclosure(n) ((sizeof(LClosure)) + sizeof(TValue *) * ((n) - 1))

Closure *newLClosure(lua_State *luaState, byte numUpValues, Table *env) {
	Closure *newClosure = (Closure *)lua_malloc(luaState, sizeLclosure(numUpValues));

	lua_linkObjToGC(luaState, obj2gco(newClosure), LUA_TFUNCTION);

	newClosure->l.isC = 0;
	newClosure->l.env = env;
	newClosure->l.nupvalues = numUpValues;

	while (numUpValues--) {
		newClosure->l.upvals[numUpValues] = NULL;
	}

	return newClosure;
}

static void pushClosure(lua_State *luaState, Closure *closure) {
	TValue obj;
	setclvalue(luaState, &obj, closure);
	pushObject(luaState, &obj);
}

Proto *createProto(lua_State *luaState) {
	Proto *newProto = (Proto *)lua_malloc(luaState, sizeof(Proto));
	lua_linkObjToGC(luaState, obj2gco(newProto), LUA_TPROTO);

	newProto->k = NULL;
	newProto->sizek = 0;
	newProto->p = NULL;
	newProto->sizep = 0;
	newProto->code = NULL;
	newProto->sizecode = 0;
	newProto->sizelineinfo = 0;
	newProto->sizeupvalues = 0;
	newProto->nups = 0;
	newProto->upvalues = NULL;
	newProto->numparams = 0;
	newProto->is_vararg = 0;
	newProto->maxstacksize = 0;
	newProto->lineinfo = NULL;
	newProto->sizelocvars = 0;
	newProto->locvars = NULL;
	newProto->linedefined = 0;
	newProto->lastlinedefined = 0;
	newProto->source = NULL;

	return newProto;
}

TString *createString(lua_State *luaState, const char *str, size_t len) {
	TString *res;
	lua_pushlstring(luaState, str, len);

	res = rawtsvalue(luaState->top - 1);
	lua_pop(luaState, 1);

	return res;
}

static Proto *makeFakeProto(lua_State *L, lu_byte nups) {
	Proto *p = createProto(L);

	p->sizelineinfo = 1;
	p->lineinfo = lua_newVector(L, 1, int);
	p->lineinfo[0] = 1;
	p->sizecode = 1;
	p->code = lua_newVector(L, 1, Instruction);
	p->code[0] = CREATE_ABC(OP_RETURN, 0, 1, 0);
	p->source = createString(L, "", 0);
	p->maxstacksize = 2;
	p->nups = nups;
	p->sizek = 0;
	p->sizep = 0;

	return p;
}

static UpVal *createUpValue(lua_State *luaState, int stackpos) {
	UpVal *upValue = (UpVal *)lua_malloc(luaState, sizeof(UpVal));
	lua_linkObjToGC(luaState, (GCObject *)upValue, LUA_TUPVAL);
	upValue->tt = LUA_TUPVAL;
	upValue->v = &upValue->u.value;
	upValue->u.l.prev = NULL;
	upValue->u.l.next = NULL;

	const TValue *o2 = (TValue *)getObject(luaState, stackpos);
	upValue->v->value = o2->value;
	upValue->v->tt = o2->tt;
	checkliveness(G(L), upValue->v);

	return upValue;
}

static void unboxUpValue(lua_State *luaState) {
	// >>>>> ...... func
	LClosure *lcl;
	UpVal *uv;

	lcl = (LClosure *)clvalue(getObject(luaState, -1));
	uv = lcl->upvals[0];

	lua_pop(luaState, 1);
	// >>>>> ......

	pushUpValue(luaState, uv);
	// >>>>> ...... upValue
}

void unserializeFunction(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	byte numUpValues = info->readStream->readByte();

	LClosure *lclosure = (LClosure *)newLClosure(info->luaState, numUpValues, hvalue(&info->luaState->l_gt));
	pushClosure(info->luaState, (Closure *)lclosure);
	// >>>>> permTbl indexTbl ...... func

	// Put *some* proto in the closure, before the GC can find it
	lclosure->p = makeFakeProto(info->luaState, numUpValues);

	//Also, we need to temporarily fill the upvalues
	lua_pushnil(info->luaState);
	// >>>>> permTbl indexTbl ...... func nil

	for (byte i = 0; i < numUpValues; ++i) {
		lclosure->upvals[i] = createUpValue(info->luaState, -1);
	}

	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... func

	// I can't see offhand how a function would ever get to be self-
	// referential, but just in case let's register it early
	registerObjectInIndexTable(info, index);

	// Now that it's safe, we can get the real proto
	unserializeObject(info);
	// >>>>> permTbl indexTbl ...... func proto

	lclosure->p = gco2p(getObject(info->luaState, -1)->value.gc);

	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... func

	for (byte i = 0; i < numUpValues; ++i) {
		// >>>>> permTbl indexTbl ...... func
		unserializeObject(info);
		// >>>>> permTbl indexTbl ...... func func2

		unboxUpValue(info->luaState);
		// >>>>> permTbl indexTbl ...... func upValue
		lclosure->upvals[i] = gco2uv(getObject(info->luaState, -1)->value.gc);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... func
	}

	// Finally, the fenv
	unserializeObject(info);

	// >>>>> permTbl indexTbl ...... func ?fenv/nil?
	if (!lua_isnil(info->luaState, -1)) {
		// >>>>> permTbl indexTbl ...... func fenv
		lua_setfenv(info->luaState, -2);
		// >>>>> permTbl indexTbl ...... func
	} else {
		// >>>>> permTbl indexTbl ...... func nil
		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... func
	}

	// >>>>> permTbl indexTbl ...... func
}

static void correctStack(lua_State *L, TValue *oldstack) {
	CallInfo *ci;
	GCObject *up;
	L->top = (L->top - oldstack) + L->stack;
	for (up = L->openupval; up != NULL; up = up->gch.next)
		gco2uv(up)->v = (gco2uv(up)->v - oldstack) + L->stack;
	for (ci = L->base_ci; ci <= L->ci; ci++) {
		ci->top = (ci->top - oldstack) + L->stack;
		ci->base = (ci->base - oldstack) + L->stack;
		ci->func = (ci->func - oldstack) + L->stack;
	}
	L->base = (L->base - oldstack) + L->stack;
}

void lua_reallocstack(lua_State *L, int newsize) {
	TValue *oldstack = L->stack;
	int realsize = newsize + 1 + EXTRA_STACK;

	lua_reallocvector(L, L->stack, L->stacksize, realsize, TValue);
	L->stacksize = realsize;
	L->stack_last = L->stack + newsize;
	correctStack(L, oldstack);
}

void lua_growstack(lua_State *L, int n) {
	// Double size is enough?
	if (n <= L->stacksize) {
		lua_reallocstack(L, 2 * L->stacksize);
	} else {
		lua_reallocstack(L, L->stacksize + n);
	}
}

void lua_reallocCallInfo(lua_State *lauState, int newsize) {
	CallInfo *oldci = lauState->base_ci;
	lua_reallocvector(lauState, lauState->base_ci, lauState->size_ci, newsize, CallInfo);

	lauState->size_ci = newsize;
	lauState->ci = (lauState->ci - oldci) + lauState->base_ci;
	lauState->end_ci = lauState->base_ci + lauState->size_ci - 1;
}

void unboxUpVal(lua_State *luaState) {
	// >>>>> ... func
	LClosure *lcl;
	UpVal *uv;

	lcl = (LClosure *)(&getObject(luaState, -1)->value.gc->cl);
	uv = lcl->upvals[0];
	lua_pop(luaState, 1);
	// >>>>> ...
	pushUpValue(luaState, uv);
	// >>>>> ... upVal
}

/* Does basically the opposite of luaC_link().
 * Right now this function is rather inefficient; it requires traversing the
 * entire root GC set in order to find one object. If the GC list were doubly
 * linked this would be much easier, but there's no reason for Lua to have
 * that. */
static void GCUnlink(lua_State *luaState, GCObject *gco) {
	GCObject *prevslot;
	if (G(luaState)->rootgc == gco) {
		G(luaState)->rootgc = G(luaState)->rootgc->gch.next;
		return;
	}

	prevslot = G(luaState)->rootgc;
	while (prevslot->gch.next != gco) {
		prevslot = prevslot->gch.next;
	}

	prevslot->gch.next = prevslot->gch.next->gch.next;
}

void unserializeThread(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	lua_State *L2;
	uint32 stacklimit = 0;

	L2 = lua_newthread(info->luaState);
	lua_checkstack(info->luaState, 3);

	// L1: permTbl indexTbl ...... thread
	// L2: (empty)
	registerObjectInIndexTable(info, index);

	// First, deserialize the object stack
	uint32 stackSize = info->readStream->readUint32LE();
	lua_growstack(info->luaState, (int)stackSize);

	// Make sure that the first stack element (a nil, representing
	// the imaginary top-level C function) is written to the very,
	// very bottom of the stack
	L2->top--;
	for (uint32 i = 0; i < stackSize; ++i) {
		unserializeObject(info);
		// L1: permTbl indexTbl ...... thread obj*
	}

	lua_xmove(info->luaState, L2, stackSize);
	// L1: permTbl indexTbl ...... thread
	// L2: obj*

	// Hereafter, stacks refer to L1


	// Now, deserialize the CallInfo stack

	uint32 numFrames = info->readStream->readUint32LE();

	lua_reallocCallInfo(L2, numFrames * 2);
	for (uint32 i = 0; i < numFrames; ++i) {
		CallInfo *ci = L2->base_ci + i;
		uint32 stackbase = info->readStream->readUint32LE();
		uint32 stackfunc = info->readStream->readUint32LE();
		uint32 stacktop = info->readStream->readUint32LE();

		ci->nresults = info->readStream->readSint32LE();

		uint32 savedpc = info->readStream->readUint32LE();

		if (stacklimit < stacktop) {
			stacklimit = stacktop;
		}

		ci->base = L2->stack + stackbase;
		ci->func = L2->stack + stackfunc;
		ci->top = L2->stack + stacktop;
		ci->savedpc = (ci != L2->base_ci) ? ci_func(ci)->l.p->code + savedpc : 0;
		ci->tailcalls = 0;

		// Update the pointer each time, to keep the GC happy
		L2->ci = ci;
	}

	// >>>>> permTbl indexTbl ...... thread
	// Deserialize the state's other parameters, with the exception of upval stuff

	L2->savedpc = L2->ci->savedpc;
	L2->status = info->readStream->readByte();
	uint32 stackbase = info->readStream->readUint32LE();
	uint32 stacktop = info->readStream->readUint32LE();


	L2->errfunc = info->readStream->readUint32LE();

	L2->base = L2->stack + stackbase;
	L2->top = L2->stack + stacktop;

	// Finally, "reopen" upvalues. See serializeUpVal() for why we do this
	UpVal *uv;
	GCObject **nextslot = &L2->openupval;
	global_State *g = G(L2);

	while (true) {
		unserializeObject(info);
		// >>>>> permTbl indexTbl ...... thread upVal/nil

		// The list is terminated by a nil
		if (lua_isnil(info->luaState, -1)) {
			// >>>>> permTbl indexTbl ...... thread nil
			lua_pop(info->luaState, 1);
			// >>>>> permTbl indexTbl ...... thread
			break;
		}

		// >>>>> permTbl indexTbl ...... thread boxedUpVal
		unboxUpVal(info->luaState);
		// >>>>> permTbl indexTbl ...... thread boxedUpVal

		uv = &(getObject(info->luaState, -1)->value.gc->uv);
		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... thread

		uint32 stackpos = info->readStream->readUint32LE();
		uv->v = L2->stack + stackpos;

		GCUnlink(info->luaState, (GCObject *)uv);

		uv->marked = luaC_white(g);
		*nextslot = (GCObject *)uv;
		nextslot = &uv->next;
		uv->u.l.prev = &G(L2)->uvhead;
		uv->u.l.next = G(L2)->uvhead.u.l.next;
		uv->u.l.next->u.l.prev = uv;
		G(L2)->uvhead.u.l.next = uv;
		lua_assert(uv->u.l.next->u.l.prev == uv && uv->u.l.prev->u.l.next == uv);
	}
	*nextslot = NULL;

	// The stack must be valid at least to the highest value among the CallInfos
	// 'top' and the values up to there must be filled with 'nil'
	lua_checkstack(L2, (int)stacklimit);
	for (StkId o = L2->top; o <= L2->top + stacklimit; ++o) {
		setnilvalue(o);
	}
}

TString *lua_newlstr(lua_State *luaState, const char *str, size_t len) {
	lua_pushlstring(luaState, str, len);
	TString *luaStr = &(luaState->top - 1)->value.gc->ts;

	lua_pop(luaState, 1);

	return luaStr;
}

void lua_link(lua_State *luaState, GCObject *o, lu_byte tt) {
	global_State *g = G(luaState);
	o->gch.next = g->rootgc;
	g->rootgc = o;
	o->gch.marked = luaC_white(g);
	o->gch.tt = tt;
}

Proto *lua_newproto(lua_State *luaState) {
	Proto *f = (Proto *)lua_malloc(luaState, sizeof(Proto));
	lua_link(luaState, obj2gco(f), LUA_TPROTO);
	f->k = NULL;
	f->sizek = 0;
	f->p = NULL;
	f->sizep = 0;
	f->code = NULL;
	f->sizecode = 0;
	f->sizelineinfo = 0;
	f->sizeupvalues = 0;
	f->nups = 0;
	f->upvalues = NULL;
	f->numparams = 0;
	f->is_vararg = 0;
	f->maxstacksize = 0;
	f->lineinfo = NULL;
	f->sizelocvars = 0;
	f->locvars = NULL;
	f->linedefined = 0;
	f->lastlinedefined = 0;
	f->source = NULL;
	return f;
}

void unserializeProto(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// We have to be careful. The GC expects a lot out of protos. In particular, we need
	// to give the function a valid string for its source, and valid code, even before we
	// actually read in the real code.
	TString *source = lua_newlstr(info->luaState, "", 0);
	Proto *p = lua_newproto(info->luaState);
	p->source = source;
	p->sizecode = 1;
	p->code = (Instruction *)lua_reallocv(info->luaState, NULL, 0, 1, sizeof(Instruction));
	p->code[0] = CREATE_ABC(OP_RETURN, 0, 1, 0);
	p->maxstacksize = 2;
	p->sizek = 0;
	p->sizep = 0;

	lua_checkstack(info->luaState, 2);

	pushProto(info->luaState, p);
	// >>>>> permTbl indexTbl ...... proto

	// We don't need to register early, since protos can never ever be
	// involved in cyclic references

	// Read in constant references
	int sizek = info->readStream->readSint32LE();
	lua_reallocvector(info->luaState, p->k, 0, sizek, TValue);
	for (int i = 0; i < sizek; ++i) {
		// >>>>> permTbl indexTbl ...... proto
		unserializeObject(info);
		// >>>>> permTbl indexTbl ...... proto  k

		setobj2s(info->luaState, &p->k[i], getObject(info->luaState, -1));
		p->sizek++;

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... proto
	}
	// >>>>> permTbl indexTbl ...... proto

	// Read in sub-proto references

	int sizep = info->readStream->readSint32LE();
	lua_reallocvector(info->luaState, p->p, 0, sizep, Proto *);
	for (int i = 0; i < sizep; ++i) {
		// >>>>> permTbl indexTbl ...... proto
		unserializeObject(info);
		// >>>>> permTbl indexTbl ...... proto  subproto

		p->p[i] = (Proto *)getObject(info->luaState, -1)->value.gc;
		p->sizep++;

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... proto
	}
	// >>>>> permTbl indexTbl ...... proto


	// Read in code
	p->sizecode = info->readStream->readSint32LE();
	lua_reallocvector(info->luaState, p->code, 1, p->sizecode, Instruction);
	info->readStream->read(p->code, sizeof(Instruction) * p->sizecode);


	/* Read in upvalue names */
	p->sizeupvalues = info->readStream->readSint32LE();
	if (p->sizeupvalues) {
		lua_reallocvector(info->luaState, p->upvalues, 0, p->sizeupvalues, TString *);
		for (int i = 0; i < p->sizeupvalues; ++i) {
			// >>>>> permTbl indexTbl ...... proto
			unserializeObject(info);
			// >>>>> permTbl indexTbl ...... proto str

			p->upvalues[i] = lua_newlstr(info->luaState, lua_tostring(info->luaState, -1), strlen(lua_tostring(info->luaState, -1)));
			lua_pop(info->luaState, 1);
			// >>>>> permTbl indexTbl ...... proto
		}
	}
	// >>>>> permTbl indexTbl ...... proto

	// Read in local variable infos
	p->sizelocvars = info->readStream->readSint32LE();
	if (p->sizelocvars) {
		lua_reallocvector(info->luaState, p->locvars, 0, p->sizelocvars, LocVar);
		for (int i = 0; i < p->sizelocvars; ++i) {
			// >>>>> permTbl indexTbl ...... proto
			unserializeObject(info);
			// >>>>> permTbl indexTbl ...... proto str

			p->locvars[i].varname = lua_newlstr(info->luaState, lua_tostring(info->luaState, -1), strlen(lua_tostring(info->luaState, -1)));
			lua_pop(info->luaState, 1);
			// >>>>> permTbl indexTbl ...... proto

			p->locvars[i].startpc = info->readStream->readSint32LE();
			p->locvars[i].endpc = info->readStream->readSint32LE();
		}
	}
	// >>>>> permTbl indexTbl ...... proto

	// Read in source string
	unserializeObject(info);
	// >>>>> permTbl indexTbl ...... proto sourceStr

	p->source = lua_newlstr(info->luaState, lua_tostring(info->luaState, -1), strlen(lua_tostring(info->luaState, -1)));
	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... proto

	// Read in line numbers
	p->sizelineinfo = info->readStream->readSint32LE();
	if (p->sizelineinfo) {
		lua_reallocvector(info->luaState, p->lineinfo, 0, p->sizelineinfo, int);
		info->readStream->read(p->lineinfo, sizeof(int) * p->sizelineinfo);
	}


	/* Read in linedefined and lastlinedefined */
	p->linedefined = info->readStream->readSint32LE();
	p->lastlinedefined = info->readStream->readSint32LE();

	// Read in misc values
	p->nups = info->readStream->readByte();
	p->numparams = info->readStream->readByte();
	p->is_vararg = info->readStream->readByte();
	p->maxstacksize = info->readStream->readByte();
}

Closure *lua_newLclosure(lua_State *luaState, int numElements, Table *elementTable) {
	Closure *c = (Closure *)lua_malloc(luaState, sizeLclosure(numElements));
	lua_link(luaState, obj2gco(c), LUA_TFUNCTION);
	c->l.isC = 0;
	c->l.env = elementTable;
	c->l.nupvalues = cast_byte(numElements);

	while (numElements--) {
		c->l.upvals[numElements] = NULL;
	}

	return c;
}

static UpVal *makeUpVal(lua_State *luaState, int stackPos) {
	UpVal *uv = lua_new(luaState, UpVal);
	lua_link(luaState, (GCObject *)uv, LUA_TUPVAL);
	uv->tt = LUA_TUPVAL;
	uv->v = &uv->u.value;
	uv->u.l.prev = NULL;
	uv->u.l.next = NULL;

	setobj(luaState, uv->v, getObject(luaState, stackPos));

	return uv;
}

/**
 * The GC is not fond of finding upvalues in tables. We get around this
 * during persistence using a weakly keyed table, so that the GC doesn't
 * bother to mark them. This won't work in unpersisting, however, since
 * if we make the values weak they'll be collected (since nothing else
 * references them). Our solution, during unpersisting, is to represent
 * upvalues as dummy functions, each with one upvalue.
 */
static void boxupval_start(lua_State *luaState) {
	LClosure *closure;
	closure = (LClosure *)lua_newLclosure(luaState, 1, hvalue(&luaState->l_gt));
	pushClosure(luaState, (Closure *)closure);
	// >>>>> ...... func
	closure->p = makeFakeProto(luaState, 1);

	// Temporarily initialize the upvalue to nil
	lua_pushnil(luaState);
	closure->upvals[0] = makeUpVal(luaState, -1);
	lua_pop(luaState, 1);
}

static void boxupval_finish(lua_State *luaState) {
	// >>>>> ...... func obj
	LClosure *lcl = (LClosure *)clvalue(getObject(luaState, -2));

	lcl->upvals[0]->u.value = *getObject(luaState, -1);
	lua_pop(luaState, 1);
	// >>>>> ...... func
}

void unserializeUpValue(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......
	lua_checkstack(upi->L, 2);

	boxupval_start(upi->L);
	// >>>>> permTbl indexTbl ...... func
	registerObjectInIndexTable(info, index);

	unserializeObject(info);
	// >>>>> permTbl indexTbl ...... func obj

	boxupval_finish(upi->L);
	// >>>>> permTbl indexTbl ...... func
}

void unserializeUserData(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	int isspecial = info->readStream->readSint32LE();
	if (isspecial) {
		unserializeObject(info);
		// >>>>> permTbl indexTbl ...... specialFunc

		lua_call(info->luaState, 0, 1);
		// >>>>> permTbl indexTbl ...... udata
	} else {
		uint32 length = info->readStream->readUint32LE();
		lua_newuserdata(info->luaState, length);
		// >>>>> permTbl indexTbl ...... udata
		registerObjectInIndexTable(info, index);

		info->readStream->read(lua_touserdata(upi->L, -1), length);

		unserializeObject(info);
		// >>>>> permTbl indexTbl ...... udata metaTable/nil

		lua_setmetatable(upi->L, -2);
		// >>>>> permTbl indexTbl ...... udata
	}
	// >>>>> permTbl indexTbl ...... udata
}

void unserializePermanent(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	unserializeObject(info);
	// >>>>> permTbl indexTbl ...... permKey

	lua_gettable(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... perm
}

} // End of namespace Lua
