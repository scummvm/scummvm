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

/**
 * This code is heavily based on the Pluto code base. Copyright below
 */

/* Tamed Pluto - Heavy-duty persistence for Lua
 * Copyright (C) 2004 by Ben Sunshine-Hill, and released into the public
 * domain. People making use of this software as part of an application
 * are politely requested to email the author at sneftel@gmail.com
 * with a brief description of the application, primarily to satisfy his
 * curiosity.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Instrumented by Stefan Reich (info@luaos.net)
 * for Mobile Lua (http://luaos.net/pages/mobile-lua.php)
 */


#include "lua_persistence.h"

#include "double_serialization.h"
#include "lua_persistence_util.h"

#include "common/stream.h"

#include "lobject.h"
#include "lstate.h"
#include "lgc.h"
#include "lopcodes.h"


namespace Lua {

struct UnSerializationInfo {
	lua_State *luaState;
	Common::ReadStream *readStream;
};

static void unpersist(UnSerializationInfo *info);

static void unpersistBoolean(UnSerializationInfo *info);
static void unpersistNumber(UnSerializationInfo *info);
static void unpersistString(UnSerializationInfo *info);
static void unpersistTable(UnSerializationInfo *info, int index);
static void unpersistFunction(UnSerializationInfo *info, int index);
static void unpersistThread(UnSerializationInfo *info, int index);
static void unpersistProto(UnSerializationInfo *info, int index);
static void unpersistUpValue(UnSerializationInfo *info, int index);
static void unpersistUserData(UnSerializationInfo *info, int index);
static void unpersistPermanent(UnSerializationInfo *info, int index);


void unpersistLua(lua_State *luaState, Common::ReadStream *readStream) {
	UnSerializationInfo info;
	info.luaState = luaState;
	info.readStream = readStream;

	// The process starts with the lua stack as follows:
	// >>>>> permTbl
	// That's the table of permanents

	// Make sure there is enough room on the stack
	lua_checkstack(luaState, 3);

	// Create a table to hold indexes of everything that's already been read
	lua_newtable(luaState);
	// >>>>> permTbl indexTbl

	// Prevent garbage collection while we unserialize
	lua_gc(luaState, LUA_GCSTOP, 0);

	// Unserialize the root object
	unpersist(&info);
	// >>>>> permTbl indexTbl rootObj

	// Re-start garbage collection
	lua_gc(luaState, LUA_GCRESTART, 0);

	// Remove the indexTbl
	lua_replace(luaState, 2);
	// >>>>> permTbl rootObj
}

/* The object is left on the stack. This is primarily used by unserialize, but
 * may be used by GCed objects that may incur cycles in order to preregister
 * the object. */
static void registerObjectInIndexTable(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ...... obj

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 1);

	lua_pushvalue(info->luaState, -1);
	// >>>>> permTbl indexTbl ...... obj obj

	// Push the k/v pair into the indexTbl
	lua_rawseti(info->luaState, 2, index);
	// >>>>> permTbl indexTbl ...... obj
}

static void unpersist(UnSerializationInfo *info) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	byte isARealValue = info->readStream->readByte();
	if (isARealValue) {
		int index = info->readStream->readSint32LE();
		int type = info->readStream->readSint32LE();

		switch (type) {
		case LUA_TBOOLEAN:
			unpersistBoolean(info);
			break;
		case LUA_TLIGHTUSERDATA:
			// You can't serialize a pointer
			// It would be meaningless on the next run
			assert(0);
			break;
		case LUA_TNUMBER:
			unpersistNumber(info);
			break;
		case LUA_TSTRING:
			unpersistString(info);
			break;
		case LUA_TTABLE:
			unpersistTable(info, index);
			break;
		case LUA_TFUNCTION:
			unpersistFunction(info, index);
			break;
		case LUA_TTHREAD:
			unpersistThread(info, index);
			break;
		case LUA_TPROTO:
			unpersistProto(info, index);
			break;
		case LUA_TUPVAL:
			unpersistUpValue(info, index);
			break;
		case LUA_TUSERDATA:
			unpersistUserData(info, index);
			break;
		case PERMANENT_TYPE:
			unpersistPermanent(info, index);
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

			lua_rawgeti(info->luaState, 2, index);
			// >>>>> permTbl indexTbl ...... ?obj?

			assert(!lua_isnil(info->luaState, -1));
		}
		// >>>>> permTbl indexTbl ...... obj/nil
	}

	// >>>>> permTbl indexTbl ...... obj/nil
}

static void unpersistBoolean(UnSerializationInfo *info) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 1);

	int value = info->readStream->readSint32LE();

	lua_pushboolean(info->luaState, value);
	// >>>>> permTbl indexTbl ...... bool
}

static void unpersistNumber(UnSerializationInfo *info) {
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

static void unpersistString(UnSerializationInfo *info) {
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

	unpersist(info);

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
	unpersist(info);
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
		unpersist(info);
		// >>>>> permTbl indexTbl ...... tbl key/nil

		// The table serialization is nil terminated
		if (lua_isnil(info->luaState, -1)) {
			// >>>>> permTbl indexTbl ...... tbl nil
			lua_pop(info->luaState, 1);
			// >>>>> permTbl indexTbl ...... tbl

			break;
		}

		// >>>>> permTbl indexTbl ...... tbl key
		unpersist(info);
		// >>>>> permTbl indexTbl ...... tbl value

		lua_rawset(info->luaState, -3);
		// >>>>> permTbl indexTbl ...... tbl
	}
}

void unpersistTable(UnSerializationInfo *info, int index) {
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

void unpersistFunction(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	byte numUpValues = info->readStream->readByte();

	LClosure *lclosure = (LClosure *)lua_newLclosure(info->luaState, numUpValues, hvalue(&info->luaState->l_gt));
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
	unpersist(info);
	// >>>>> permTbl indexTbl ...... func proto

	lclosure->p = gco2p(getObject(info->luaState, -1)->value.gc);

	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... func

	for (byte i = 0; i < numUpValues; ++i) {
		// >>>>> permTbl indexTbl ...... func
		unpersist(info);
		// >>>>> permTbl indexTbl ...... func func2

		unboxUpValue(info->luaState);
		// >>>>> permTbl indexTbl ...... func upValue
		lclosure->upvals[i] = gco2uv(getObject(info->luaState, -1)->value.gc);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... func
	}

	// Finally, the fenv
	unpersist(info);

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

void unpersistThread(UnSerializationInfo *info, int index) {
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
	lua_checkstack(info->luaState, (int)stackSize);

	// Make sure that the first stack element (a nil, representing
	// the imaginary top-level C function) is written to the very,
	// very bottom of the stack
	L2->top--;
	for (uint32 i = 0; i < stackSize; ++i) {
		unpersist(info);
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
		unpersist(info);
		// >>>>> permTbl indexTbl ...... thread upVal/nil

		// The list is terminated by a nil
		if (lua_isnil(info->luaState, -1)) {
			// >>>>> permTbl indexTbl ...... thread nil
			lua_pop(info->luaState, 1);
			// >>>>> permTbl indexTbl ...... thread
			break;
		}

		// >>>>> permTbl indexTbl ...... thread boxedUpVal
		unboxUpValue(info->luaState);
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

void unpersistProto(UnSerializationInfo *info, int index) {
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
		unpersist(info);
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
		unpersist(info);
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
			unpersist(info);
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
			unpersist(info);
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
	unpersist(info);
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

void unpersistUpValue(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......
	lua_checkstack(info->luaState, 2);

	boxUpValue_start(info->luaState);
	// >>>>> permTbl indexTbl ...... func
	registerObjectInIndexTable(info, index);

	unpersist(info);
	// >>>>> permTbl indexTbl ...... func obj

	boxUpValue_finish(info->luaState);
	// >>>>> permTbl indexTbl ...... func
}

void unpersistUserData(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	int isspecial = info->readStream->readSint32LE();
	if (isspecial) {
		unpersist(info);
		// >>>>> permTbl indexTbl ...... specialFunc

		lua_call(info->luaState, 0, 1);
		// >>>>> permTbl indexTbl ...... udata
	} else {
		uint32 length = info->readStream->readUint32LE();
		lua_newuserdata(info->luaState, length);
		// >>>>> permTbl indexTbl ...... udata
		registerObjectInIndexTable(info, index);

		info->readStream->read(lua_touserdata(info->luaState, -1), length);

		unpersist(info);
		// >>>>> permTbl indexTbl ...... udata metaTable/nil

		lua_setmetatable(info->luaState, -2);
		// >>>>> permTbl indexTbl ...... udata
	}
	// >>>>> permTbl indexTbl ...... udata
}

void unpersistPermanent(UnSerializationInfo *info, int index) {
	// >>>>> permTbl indexTbl ......

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	unpersist(info);
	// >>>>> permTbl indexTbl ...... permKey

	lua_gettable(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... perm
}

} // End of namespace Lua
