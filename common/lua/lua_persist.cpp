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


namespace Lua {

#define PERMANENT_TYPE 101

struct SerializationInfo {
	lua_State *luaState;
	Common::WriteStream *writeStream;
	uint counter;
};

static void persist(SerializationInfo *info);

static void persistBoolean(SerializationInfo *info);
static void persistNumber(SerializationInfo *info);
static void persistString(SerializationInfo *info);
static void persistTable(SerializationInfo *info);
static void persistFunction(SerializationInfo *info);
static void persistThread(SerializationInfo *info);
static void persistProto(SerializationInfo *info);
static void persistUpValue(SerializationInfo *info);
static void persistUserData(SerializationInfo *info);


void persistLua(lua_State *luaState, Common::WriteStream *writeStream) {
	SerializationInfo info;
	info.luaState = luaState;
	info.writeStream = writeStream;
	info.counter = 1u;

	// The process starts with the lua stack as follows:
	// >>>>> permTbl rootObj
	// That's the table of permanents and the root object to be serialized

	// Make sure there is enough room on the stack
	lua_checkstack(luaState, 4);
	assert(lua_gettop(luaState) == 2);
	// And that the root isn't nil
	assert(!lua_isnil(luaState, 2));

	// Create a table to hold indexes of everything that's serialized
	// This allows us to only serialize an object once
	// Every other time, just reference the index
	lua_newtable(luaState);
	// >>>>> permTbl rootObj indexTbl

	// Now we're going to make the table weakly keyed. This prevents the
	// GC from visiting it and trying to mark things it doesn't want to
	// mark in tables, e.g. upvalues. All objects in the table are
	// a priori reachable, so it doesn't matter that we do this.

	// Create the metatable
	lua_newtable(luaState);
	// >>>>> permTbl rootObj indexTbl metaTbl

	lua_pushstring(luaState, "__mode");
	// >>>>> permTbl rootObj indexTbl metaTbl "__mode"

	lua_pushstring(luaState, "k");
	// >>>>> permTbl rootObj indexTbl metaTbl "__mode" "k"

	lua_settable(luaState, 4);
	// >>>>> permTbl rootObj indexTbl metaTbl

	lua_setmetatable(luaState, 3);
	// >>>>> permTbl rootObj indexTbl

	// Swap the indexTable and the rootObj
	lua_insert(luaState, 2);
	// >>>>> permTbl indexTbl rootObj

	// Serialize the root recursively
	persist(&info);

	// Return the stack back to the original state
	lua_remove(luaState, 2);
	// >>>>> permTbl rootObj
}

static void persist(SerializationInfo *info) {
	// The stack can potentially have many things on it
	// The object we want to serialize is the item on the top of the stack
	// >>>>> permTbl indexTbl rootObj ...... obj

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	// If the object has already been written, don't write it again
	// Instead write the index of the object from the indexTbl

	// Check the indexTbl
	lua_pushvalue(info->luaState, -1);
	// >>>>> permTbl indexTbl rootObj ...... obj obj

	lua_rawget(info->luaState, 2);
	// >>>>> permTbl indexTbl rootObj ...... obj ?index?

	// If the index isn't nil, the object has already been written
	if (!lua_isnil(info->luaState, -1)) {
		// Write out a flag that indicates that it's an index
		info->writeStream->writeByte(0);

		// Retrieve the index from the stack
		uint *index = (uint *)lua_touserdata(info->luaState, -1);

		// Write out the index
		info->writeStream->writeUint32LE(*index);

		// Pop the index off the stack
		lua_pop(info->luaState, 1);

		return;
	}

	// Pop the index/nil off the stack
	lua_pop(info->luaState, 1);

	// If the obj itself is nil, we represent it as an index of 0
	if (lua_isnil(info->luaState, -1)) {
		// Write out a flag that indicates that it's an index
		info->writeStream->writeByte(0);
		// Write out the index
		info->writeStream->writeUint32LE(0);

		return;
	}

	// Write out a flag that indicates that this is a real object
	info->writeStream->writeByte(1);

	// Add the object to the indexTbl

	lua_pushvalue(info->luaState, -1);
	// >>>>> permTbl indexTbl rootObj ...... obj obj

	uint *ref = (uint *)lua_newuserdata(info->luaState, sizeof(uint));
	*ref = ++(info->counter);
	// >>>>> permTbl indexTbl rootObj ...... obj obj index

	lua_rawset(info->luaState, 2);
	// >>>>> permTbl indexTbl rootObj ...... obj


	// Write out the index
	info->writeStream->writeUint32LE(info->counter);


	// Objects that are in the permanents table are serialized in a special way

	lua_pushvalue(info->luaState, -1);
	// >>>>> permTbl indexTbl rootObj ...... obj obj

	lua_gettable(info->luaState, 1);
	// >>>>> permTbl indexTbl rootObj ...... obj obj ?permKey?

	if (!lua_isnil(info->luaState, -1)) {
		// Write out the type
		info->writeStream->writeSint32LE(PERMANENT_TYPE);

		// Serialize the key
		persist(info);

		// Pop the key off the stack
		lua_pop(info->luaState, 1);

		return;
	}

	// Pop the nil off the stack
	lua_pop(info->luaState, 1);

	// Query the type of the object
	int objType = lua_type(info->luaState, -1);

	// Write it out
	info->writeStream->writeSint32LE(objType);

	// Serialize the object by its type

	switch (objType) {
	case LUA_TBOOLEAN:
		persistBoolean(info);
		break;
	case LUA_TLIGHTUSERDATA:
		// You can't serialize a pointer
		// It would be meaningless on the next run
		assert(0);
		break;
	case LUA_TNUMBER:
		persistNumber(info);
		break;
	case LUA_TSTRING:
		persistString(info);
		break;
	case LUA_TTABLE:
		persistTable(info);
		break;
	case LUA_TFUNCTION:
		persistFunction(info);
		break;
	case LUA_TTHREAD:
		persistThread(info);
		break;
	case LUA_TPROTO:
		persistProto(info);
		break;
	case LUA_TUPVAL:
		persistUpValue(info);
		break;
	case LUA_TUSERDATA:
		persistUserData(info);
		break;
	default:
		assert(0);
	}
}

static void persistBoolean(SerializationInfo *info) {
	int value = lua_toboolean(info->luaState, -1);

	info->writeStream->writeSint32LE(value);
}

static void persistNumber(SerializationInfo *info) {
	lua_Number value = lua_tonumber(info->luaState, -1);

	Util::SerializedDouble serializedValue(Util::encodeDouble(value));

	info->writeStream->writeUint32LE(serializedValue.significandOne);
	info->writeStream->writeUint32LE(serializedValue.signAndSignificandTwo);
	info->writeStream->writeSint16LE(serializedValue.exponent);
}

static void persistString(SerializationInfo *info) {
	// Hard cast to a uint32 to force size_t to an explicit size
	// *Theoretically* this could truncate, but if we have a 4gb string, we have bigger problems
	uint32 length = static_cast<uint32>(lua_strlen(info->luaState, -1));
	info->writeStream->writeUint32LE(length);

	const char *str = lua_tostring(info->luaState, -1);
	info->writeStream->write(str, length);
}

/* Choose whether to do a regular or special persistence based on an object's
 * metatable. "default" is whether the object, if it doesn't have a __persist
 * entry, is literally persistable or not.
 * Pushes the unpersist closure and returns true if special persistence is
 * used. */
static bool serializeSpecialObject(SerializationInfo *info, bool defaction) {
	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 4);

	// Check whether we should persist literally, or via the __persist metafunction
	if (!lua_getmetatable(info->luaState, -1)) {
		if (defaction) {
			// Write out a flag declaring that the object isn't special and should be persisted normally
			info->writeStream->writeSint32LE(0);

			return false;
		} else {
			lua_pushstring(info->luaState, "Type not literally persistable by default");
			lua_error(info->luaState);

			return false; // Not reached
		}
	}

	// >>>>> permTbl indexTbl ...... obj metaTbl
	lua_pushstring(info->luaState, "__persist");
	// >>>>> permTbl indexTbl rootObj ...... obj metaTbl "__persist"

	lua_rawget(info->luaState, -2);
	// >>>>> permTbl indexTbl ...... obj metaTbl ?__persist?

	if (lua_isnil(info->luaState, -1)) {
		// >>>>> permTbl indexTbl ...... obj metaTbl nil
		lua_pop(info->luaState, 2);
		// >>>>> permTbl indexTbl ...... obj

		if (defaction) {
			// Write out a flag declaring that the object isn't special and should be persisted normally
			info->writeStream->writeSint32LE(0);

			return false;
		} else {
			lua_pushstring(info->luaState, "Type not literally persistable by default");
			lua_error(info->luaState);

			return false; // Return false
		}

	} else if (lua_isboolean(info->luaState, -1)) {
		// >>>>> permTbl indexTbl ...... obj metaTbl bool
		if (lua_toboolean(info->luaState, -1)) {
			// Write out a flag declaring that the object isn't special and should be persisted normally
			info->writeStream->writeSint32LE(0);

			// >>>>> permTbl indexTbl ...... obj metaTbl true */
			lua_pop(info->luaState, 2);
			// >>>>> permTbl indexTbl ...... obj

			return false;
		} else {
			lua_pushstring(info->luaState, "Metatable forbade persistence");
			lua_error(info->luaState);

			return false; // Not reached
		}
	} else if (!lua_isfunction(info->luaState, -1)) {
		lua_pushstring(info->luaState, "__persist not nil, boolean, or function");
		lua_error(info->luaState);
	}

	// >>>>> permTbl indexTbl ...... obj metaTbl __persist
	lua_pushvalue(info->luaState, -3);
	// >>>>> permTbl indexTbl ...... obj metaTbl __persist obj

	// >>>>> permTbl indexTbl ...... obj metaTbl ?func?

	if (!lua_isfunction(info->luaState, -1)) {
		lua_pushstring(info->luaState, "__persist function did not return a function");
		lua_error(info->luaState);
	}

	// >>>>> permTbl indexTbl ...... obj metaTbl func

	// Write out a flag that the function exists
	info->writeStream->writeSint32LE(1);

	// Serialize the function
	persist(info);

	lua_pop(info->luaState, 2);
	// >>>>> permTbl indexTbl ...... obj

	return true;
}

static void persistTable(SerializationInfo *info) {
	// >>>>> permTbl indexTbl ...... tbl

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 3);

	// Test if the object needs special serialization
	if (serializeSpecialObject(info, 1)) {
		return;
	}

	// >>>>> permTbl indexTbl ...... tbl

	// First, serialize the metatable (if any)
	if (!lua_getmetatable(info->luaState, -1)) {
		lua_pushnil(info->luaState);
	}

	// >>>>> permTbl indexTbl ...... tbl metaTbl/nil */
	persist(info);

	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... tbl


	lua_pushnil(info->luaState);
	// >>>>> permTbl indexTbl ...... tbl nil

	// Now, persist all k/v pairs
	while (lua_next(info->luaState, -2)) {
		// >>>>> permTbl indexTbl ...... tbl k v */

		lua_pushvalue(info->luaState, -2);
		// >>>>> permTbl indexTbl ...... tbl k v k */

		// Serialize the key
		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... tbl k v */

		// Serialize the value
		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... tbl k */
	}

	// >>>>> permTbl indexTbl ...... tbl

	// Terminate the list with a nil
	lua_pushnil(info->luaState);
	// >>>>> permTbl indexTbl ...... tbl

	persist(info);

	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... tbl
}

static void persistFunction(SerializationInfo *info) {
	// >>>>> permTbl indexTbl ...... func
	Closure *cl = clvalue(getObject(info->luaState, -1));
	lua_checkstack(info->luaState, 2);

	if (cl->c.isC) {
		/* It's a C function. For now, we aren't going to allow
		 * persistence of C closures, even if the "C proto" is
		 * already in the permanents table. */
		lua_pushstring(info->luaState, "Attempt to persist a C function");
		lua_error(info->luaState);
	} else {
		// It's a Lua closure

		// We don't really _NEED_ the number of upvals, but it'll simplify things a bit
		info->writeStream->writeByte(cl->l.p->nups);

		// Serialize the prototype
		pushProto(info->luaState, cl->l.p);
		// >>>>> permTbl indexTbl ...... func proto */

		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... func

		// Serialize upvalue values (not the upvalue objects themselves)
		for (byte i = 0; i < cl->l.p->nups; i++) {
			// >>>>> permTbl indexTbl ...... func
			pushUpValue(info->luaState, cl->l.upvals[i]);
			// >>>>> permTbl indexTbl ...... func upval

			persist(info);

			lua_pop(info->luaState, 1);
			// >>>>> permTbl indexTbl ...... func
		}

		// >>>>> permTbl indexTbl ...... func

		// Serialize function environment
		lua_getfenv(info->luaState, -1);
		// >>>>> permTbl indexTbl ...... func fenv

		if (lua_equal(info->luaState, -1, LUA_GLOBALSINDEX)) {
			// Function has the default fenv

			// >>>>> permTbl indexTbl ...... func _G
			lua_pop(info->luaState, 1);
			// >>>>> permTbl indexTbl ...... func

			lua_pushnil(info->luaState);
			// >>>>> permTbl indexTbl ...... func nil
		}

		// >>>>> permTbl indexTbl ...... func fenv/nil
		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... func
	}
}

static void persistThread(SerializationInfo *info) {
	// >>>>> permTbl indexTbl ...... thread
	lua_State *threadState = lua_tothread(info->luaState, -1);

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, threadState->top - threadState->stack + 1);

	if (info->luaState == threadState) {
		lua_pushstring(info->luaState, "Can't persist currently running thread");
		lua_error(info->luaState);
		return; /* not reached */
	}

	// Persist the stack

	// We *could* have truncation here, but if we have more than 4 billion items on a stack, we have bigger problems
	uint32 stackSize = static_cast<uint32>(appendStackToStack_reverse(threadState, info->luaState));
	info->writeStream->writeUint32LE(stackSize);

	// >>>>> permTbl indexTbl ...... thread (reversed contents of thread stack) */
	for (; stackSize > 0; --stackSize) {
		persist(info);

		lua_pop(info->luaState, 1);
	}

	// >>>>> permTbl indexTbl ...... thread

	// Now, serialize the CallInfo stack

	// Again, we *could* have truncation here, but if we have more than 4 billion items on a stack, we have bigger problems
	uint32 numFrames = static_cast<uint32>((threadState->ci - threadState->base_ci) + 1);
	info->writeStream->writeUint32LE(numFrames);

	for (uint32 i = 0; i < numFrames; i++) {
		CallInfo *ci = threadState->base_ci + i;

		// Same argument as above about truncation
		uint32 stackBase = static_cast<uint32>(ci->base - threadState->stack);
		uint32 stackFunc = static_cast<uint32>(ci->func - threadState->stack);
		uint32 stackTop = static_cast<uint32>(ci->top - threadState->stack);

		info->writeStream->writeUint32LE(stackBase);
		info->writeStream->writeUint32LE(stackFunc);
		info->writeStream->writeUint32LE(stackTop);

		info->writeStream->writeSint32LE(ci->nresults);

		uint32 savedpc = (ci != threadState->base_ci) ? static_cast<uint32>(ci->savedpc - ci_func(ci)->l.p->code) : 0u;
		info->writeStream->writeUint32LE(savedpc);
	}


	// Serialize the state's other parameters, with the exception of upval stuff

	assert(threadState->nCcalls <= 1);
	info->writeStream->writeByte(threadState->status);

	// Same argument as above about truncation
	uint32 stackBase = static_cast<uint32>(threadState->base - threadState->stack);
	uint32 stackFunc = static_cast<uint32>(threadState->top - threadState->stack);
	info->writeStream->writeUint32LE(stackBase);
	info->writeStream->writeUint32LE(stackFunc);

	// Same argument as above about truncation
	uint32 stackOffset = static_cast<uint32>(threadState->errfunc);
	info->writeStream->writeUint32LE(stackOffset);

	// Finally, record upvalues which need to be reopened
	// See the comment above serializeUpVal() for why we do this

	UpVal *upVal;

	// >>>>> permTbl indexTbl ...... thread
	for (GCObject *gcObject = threadState->openupval; gcObject != NULL; gcObject = upVal->next) {
		upVal = gco2uv(gcObject);

		/* Make sure upvalue is really open */
		assert(upVal->v != &upVal->u.value);

		pushUpValue(info->luaState, upVal);
		// >>>>> permTbl indexTbl ...... thread upVal

		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... thread

		// Same argument as above about truncation
		uint32 stackpos = static_cast<uint32>(upVal->v - threadState->stack);
		info->writeStream->writeUint32LE(stackpos);
	}

	// >>>>> permTbl indexTbl ...... thread
	lua_pushnil(info->luaState);
	// >>>>> permTbl indexTbl ...... thread nil

	// Use nil as a terminator
	persist(info);

	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... thread
}

static void persistProto(SerializationInfo *info) {
	// >>>>> permTbl indexTbl ...... proto
	Proto *proto = gco2p(getObject(info->luaState, -1)->value.gc);

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	// Serialize constant refs */
	info->writeStream->writeSint32LE(proto->sizek);

	for (int i = 0; i < proto->sizek; ++i) {
		pushObject(info->luaState, &proto->k[i]);
		// >>>>> permTbl indexTbl ...... proto const

		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... proto
	}

	// >>>>> permTbl indexTbl ...... proto

	// Serialize inner Proto refs
	info->writeStream->writeSint32LE(proto->sizep);

	for (int i = 0; i < proto->sizep; ++i) {
		pushProto(info->luaState, proto->p[i]);
		// >>>>> permTbl indexTbl ...... proto subProto */

		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... proto
	}

	// >>>>> permTbl indexTbl ...... proto

	// Serialize the code
	info->writeStream->writeSint32LE(proto->sizecode);

	uint32 len = static_cast<uint32>(sizeof(Instruction) * proto->sizecode);
	info->writeStream->write(proto->code, len);


	// Serialize upvalue names
	info->writeStream->writeSint32LE(proto->sizeupvalues);

	for (int i = 0; i < proto->sizeupvalues; ++i) {
		pushString(info->luaState, proto->upvalues[i]);
		// >>>>> permTbl indexTbl ...... proto str

		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... proto
	}


	// Serialize local variable infos
	info->writeStream->writeSint32LE(proto->sizelocvars);

	for (int i = 0; i < proto->sizelocvars; ++i) {
		pushString(info->luaState, proto->locvars[i].varname);
		// >>>>> permTbl indexTbl ...... proto str

		persist(info);

		lua_pop(info->luaState, 1);
		// >>>>> permTbl indexTbl ...... proto

		info->writeStream->writeSint32LE(proto->locvars[i].startpc);
		info->writeStream->writeSint32LE(proto->locvars[i].endpc);
	}


	// Serialize source string
	pushString(info->luaState, proto->source);
	// >>>>> permTbl indexTbl ...... proto sourceStr

	persist(info);

	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ...... proto

	// Serialize line numbers
	info->writeStream->writeSint32LE(proto->sizelineinfo);

	if (proto->sizelineinfo) {
		len = static_cast<uint32>(sizeof(int) * proto->sizelineinfo);
		info->writeStream->write(proto->lineinfo, len);
	}

	// Serialize linedefined and lastlinedefined
	info->writeStream->writeSint32LE(proto->linedefined);
	info->writeStream->writeSint32LE(proto->lastlinedefined);


	// Serialize misc values
	info->writeStream->writeByte(proto->nups);
	info->writeStream->writeByte(proto->numparams);
	info->writeStream->writeByte(proto->is_vararg);
	info->writeStream->writeByte(proto->maxstacksize);
}

/* Upvalues are tricky. Here's why.
 *
 * A particular upvalue may be either "open", in which case its member v
 * points into a thread's stack, or "closed" in which case it points to the
 * upvalue itself. An upvalue is closed under any of the following conditions:
 * -- The function that initially declared the variable "local" returns
 * -- The thread in which the closure was created is garbage collected
 *
 * To make things wackier, just because a thread is reachable by Lua doesn't
 * mean it's in our root set. We need to be able to treat an open upvalue
 * from an unreachable thread as a closed upvalue.
 *
 * The solution:
 * (a) For the purposes of serializing, don't indicate whether an upvalue is
 *     closed or not.
 * (b) When unserializing, pretend that all upvalues are closed.
 * (c) When serializing, persist all open upvalues referenced by a thread
 *     that is persisted, and tag each one with the corresponding stack position
 * (d) When unserializing, "reopen" each of these upvalues as the thread is
 *     unserialized
 */
static void persistUpValue(SerializationInfo *info) {
	// >>>>> permTbl indexTbl ...... upval
	assert(ttype(getObject(info->luaState, -1)) == LUA_TUPVAL);
	UpVal *upValue = gco2uv(getObject(info->luaState, -1)->value.gc);

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 1);

	// We can't permit the upValue to linger around on the stack, as Lua
	// will bail if its GC finds it.

	lua_pop(info->luaState, 1);
	// >>>>> permTbl indexTbl ......

	pushObject(info->luaState, upValue->v);
	// >>>>> permTbl indexTbl ...... obj

	persist(info);
	// >>>>> permTbl indexTbl ...... obj
}

static void persistUserData(SerializationInfo *info) {
	// >>>>> permTbl rootObj ...... udata

	// Make sure there is enough room on the stack
	lua_checkstack(info->luaState, 2);

	// Test if the object needs special serialization
	if (serializeSpecialObject(info, 0)) {
		return;
	}

	// Use literal persistence

	// Hard cast to a uint32 length
	// This could lead to truncation, but if we have a 4gb block of data, we have bigger problems
	uint32 length = static_cast<uint32>(uvalue(getObject(info->luaState, -1))->len);
	info->writeStream->writeUint32LE(length);

	info->writeStream->write(lua_touserdata(info->luaState, -1), length);

	// Serialize the metatable (if any)
	if (!lua_getmetatable(info->luaState, -1)) {
		lua_pushnil(info->luaState);
	}

	// >>>>> permTbl rootObj ...... udata metaTbl/nil
	persist(info);

	lua_pop(info->luaState, 1);
	/* perms reftbl ... udata */
}


} // End of namespace Lua
