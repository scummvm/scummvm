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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#define BS_LOG_PREFIX "LUA"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/array.h"
#include "common/debug-channels.h"

#include "sword25/sword25.h"
#include "sword25/package/packagemanager.h"
#include "sword25/script/luascript.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/kernel/inputpersistenceblock.h"

namespace Lua {

extern "C" {
#include "sword25/util/lua/lua.h"
#include "sword25/util/lua/lualib.h"
#include "sword25/util/lua/lauxlib.h"
#include "sword25/util/pluto/pluto.h"
}

}

namespace Sword25 {

using namespace Lua;

// -----------------------------------------------------------------------------
// Constructor / Destructor
// -----------------------------------------------------------------------------

LuaScriptEngine::LuaScriptEngine(Kernel *KernelPtr) :
	ScriptEngine(KernelPtr),
	m_State(0),
	m_PcallErrorhandlerRegistryIndex(0) {
}

// -----------------------------------------------------------------------------

LuaScriptEngine::~LuaScriptEngine() {
	// Lua de-initialisation
	if (m_State)
		lua_close(m_State);
}

// -----------------------------------------------------------------------------

Service *LuaScriptEngine_CreateObject(Kernel *KernelPtr) {
	return new LuaScriptEngine(KernelPtr);
}

// -----------------------------------------------------------------------------

namespace {
int PanicCB(lua_State *L) {
	BS_LOG_ERRORLN("Lua panic. Error message: %s", lua_isnil(L, -1) ? "" : lua_tostring(L, -1));
	return 0;
}

void debugHook(lua_State *L, lua_Debug *ar) {
	if (!lua_getinfo(L, "Sn", ar))
		return;

	debug("LUA: %s %s: %s %d", ar->namewhat, ar->name, ar->short_src, ar->currentline);
}
}

// -----------------------------------------------------------------------------

bool LuaScriptEngine::Init() {
	// Lua-State initialisation, as well as standard libaries initialisation
	m_State = luaL_newstate();
	if (!m_State || ! RegisterStandardLibs() || !RegisterStandardLibExtensions()) {
		BS_LOG_ERRORLN("Lua could not be initialized.");
		return false;
	}

	// Register panic callback function
	lua_atpanic(m_State, PanicCB);

	// Error handler for lua_pcall calls
	// The code below contains a local error handler function
	const char ErrorHandlerCode[] =
	    "local function ErrorHandler(message) "
	    "	return message .. '\\n' .. debug.traceback('', 2) "
	    "end "
	    "return ErrorHandler";

	// Compile the code
	if (luaL_loadbuffer(m_State, ErrorHandlerCode, strlen(ErrorHandlerCode), "PCALL ERRORHANDLER") != 0) {
		// An error occurred, so dislay the reason and exit
		BS_LOG_ERRORLN("Couldn't compile luaL_pcall errorhandler:\n%s", lua_tostring(m_State, -1));
		lua_pop(m_State, 1);

		return false;
	}
	// Running the code, the error handler function sets the top of the stack
	if (lua_pcall(m_State, 0, 1, 0) != 0) {
		// An error occurred, so dislay the reason and exit
		BS_LOG_ERRORLN("Couldn't prepare luaL_pcall errorhandler:\n%s", lua_tostring(m_State, -1));
		lua_pop(m_State, 1);

		return false;
	}

	// Place the error handler function in the Lua registry, and remember the index
	m_PcallErrorhandlerRegistryIndex = luaL_ref(m_State, LUA_REGISTRYINDEX);

	// Initialise the Pluto-Persistence library
	luaopen_pluto(m_State);
	lua_pop(m_State, 1);

	// Initialize debugging callback
	if (DebugMan.isDebugChannelEnabled(kDebugScript)) {
		int mask = 0;
		if ((gDebugLevel & 1) != 0)
			mask |= LUA_MASKCALL;
		if ((gDebugLevel & 2) != 0)
			mask |= LUA_MASKRET;
		if ((gDebugLevel & 4) != 0)
			mask |= LUA_MASKLINE;

		if (mask != 0)
			lua_sethook(m_State, debugHook, mask, 0);
	}

	BS_LOGLN("Lua initialized.");

	return true;
}

// -----------------------------------------------------------------------------

bool LuaScriptEngine::ExecuteFile(const Common::String &FileName) {
#ifdef DEBUG
	int __startStackDepth = lua_gettop(m_State);
#endif
	debug(2, "ExecuteFile(%s)", FileName.c_str());

	// Get a pointer to the package manager
	PackageManager *pPackage = static_cast<PackageManager *>(Kernel::GetInstance()->GetService("package"));
	BS_ASSERT(pPackage);

	// File read
	uint FileSize;
	byte *FileData = pPackage->getFile(FileName, &FileSize);
	if (!FileData) {
		BS_LOG_ERRORLN("Couldn't read \"%s\".", FileName.c_str());
#ifdef DEBUG
		BS_ASSERT(__startStackDepth == lua_gettop(m_State));
#endif
		return false;
	}

	// Run the file content
	if (!ExecuteBuffer(FileData, FileSize, "@" + pPackage->getAbsolutePath(FileName))) {
		// Release file buffer
		delete[] FileData;
#ifdef DEBUG
		BS_ASSERT(__startStackDepth == lua_gettop(m_State));
#endif
		return false;
	}

	// Release file buffer
	delete[] FileData;

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(m_State));
#endif

	return true;
}

// -----------------------------------------------------------------------------

bool LuaScriptEngine::ExecuteString(const Common::String &Code) {
	return ExecuteBuffer((byte *)Code.c_str(), Code.size(), "???");
}

// -----------------------------------------------------------------------------

namespace {

void RemoveForbiddenFunctions(lua_State *L) {
	static const char *FORBIDDEN_FUNCTIONS[] = {
		"dofile",
		0
	};

	const char **Iterator = FORBIDDEN_FUNCTIONS;
	while (*Iterator) {
		lua_pushnil(L);
		lua_setfield(L, LUA_GLOBALSINDEX, *Iterator);
		++Iterator;
	}
}
}

bool LuaScriptEngine::RegisterStandardLibs() {
	luaL_openlibs(m_State);
	RemoveForbiddenFunctions(m_State);
	return true;
}

// -----------------------------------------------------------------------------

bool LuaScriptEngine::ExecuteBuffer(const byte *Data, uint Size, const Common::String &Name) const {
	// Compile buffer
	if (luaL_loadbuffer(m_State, (const char *)Data, Size, Name.c_str()) != 0) {
		BS_LOG_ERRORLN("Couldn't compile \"%s\":\n%s", Name.c_str(), lua_tostring(m_State, -1));
		lua_pop(m_State, 1);

		return false;
	}

	// Error handling function to be executed after the function is put on the stack
	lua_rawgeti(m_State, LUA_REGISTRYINDEX, m_PcallErrorhandlerRegistryIndex);
	lua_insert(m_State, -2);

	// Run buffer contents
	if (lua_pcall(m_State, 0, 0, -2) != 0) {
		BS_LOG_ERRORLN("An error occured while executing \"%s\":\n%s.",
		               Name.c_str(),
		               lua_tostring(m_State, -1));
		lua_pop(m_State, 2);

		return false;
	}

	// Remove the error handler function from the stack
	lua_pop(m_State, 1);

	return true;
}

// -----------------------------------------------------------------------------

void LuaScriptEngine::SetCommandLine(const Common::StringArray &CommandLineParameters) {
	debug(0, "SetCommandLine()");

	lua_newtable(m_State);

	for (size_t i = 0; i < CommandLineParameters.size(); ++i) {
		lua_pushnumber(m_State, i + 1);
		lua_pushstring(m_State, CommandLineParameters[i].c_str());
		lua_settable(m_State, -3);
	}

	lua_setglobal(m_State, "CommandLine");
}

// -----------------------------------------------------------------------------

namespace {
const char *PERMANENTS_TABLE_NAME = "Permanents";

// -------------------------------------------------------------------------

// This array contains the name of global Lua objects that should not be persisted
const char *STANDARD_PERMANENTS[] = {
	"string",
	"xpcall",
	"package",
	"tostring",
	"print",
	"os",
	"unpack",
	"require",
	"getfenv",
	"setmetatable",
	"next",
	"assert",
	"tonumber",
	"io",
	"rawequal",
	"collectgarbage",
	"getmetatable",
	"module",
	"rawset",
	"warning",
	"math",
	"debug",
	"pcall",
	"table",
	"newproxy",
	"type",
	"coroutine",
	"select",
	"gcinfo",
	"pairs",
	"rawget",
	"loadstring",
	"ipairs",
	"_VERSION",
	"setfenv",
	"load",
	"error",
	"loadfile",

	"pairs_next",
	"ipairs_next",
	"pluto",
	"Cfg",
	"Translator",
	"Persistence",
	"CommandLine",
	0
};

// -------------------------------------------------------------------------

enum PERMANENT_TABLE_TYPE {
	PTT_PERSIST,
	PTT_UNPERSIST
};

// -------------------------------------------------------------------------

bool PushPermanentsTable(lua_State *L, PERMANENT_TABLE_TYPE TableType) {
	// Permanents-Table
	lua_newtable(L);

	// All standard permanents are inserted into this table
	uint Index = 0;
	while (STANDARD_PERMANENTS[Index]) {
		// Permanents are placed onto the stack; if it does not exist, it is simply ignored
		lua_getglobal(L, STANDARD_PERMANENTS[Index]);
		if (!lua_isnil(L, -1)) {
			// Name of the element as a unique value on the stack
			lua_pushstring(L, STANDARD_PERMANENTS[Index]);

			// If it is loaded, then it can be used
			// In this case, the position of name and object are reversed on the stack
			if (TableType == PTT_UNPERSIST) lua_insert(L, -2);

			// Make an entry in the table
			lua_settable(L, -3);
		} else {
			// Pop nil value from stack
			lua_pop(L, 1);
		}

		++Index;
	}

	// All registered C functions to be inserted into the table
	// BS_LuaBindhelper places in the register a table in which all registered C functions
	// are stored

	// Table is put on the stack
	lua_getfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);

	if (!lua_isnil(L, -1)) {
		// Iterate over all elements of the table
		lua_pushnil(L);
		while (lua_next(L, -2) != 0) {
			// Value and index duplicated on the stack and changed in the sequence
			lua_pushvalue(L, -1);
			lua_pushvalue(L, -3);

			// If it is loaded, then it can be used
			// In this case, the position of name and object are reversed on the stack
			if (TableType == PTT_UNPERSIST) lua_insert(L, -2);

			// Make an entry in the results table
			lua_settable(L, -6);

			// Pop value from the stack. The index is then ready for the next call to lua_next()
			lua_pop(L, 1);
		}
	}

	// Pop the C-Permanents table from the stack
	lua_pop(L, 1);

	// coroutine.yield must be registered in the extra-Permanents table because they
	// are inactive coroutine C functions on the stack

	// Function coroutine.yield placed on the stack
	lua_getglobal(L, "coroutine");
	lua_pushstring(L, "yield");
	lua_gettable(L, -2);

	// Store coroutine.yield with it's own unique value in the Permanents table
	lua_pushstring(L, "coroutine.yield");

	if (TableType == PTT_UNPERSIST) lua_insert(L, -2);

	lua_settable(L, -4);

	// Coroutine table is popped from the stack
	lua_pop(L, 1);

	return true;
}
}

// -----------------------------------------------------------------------------

namespace {
int Chunkwriter(lua_State *L, const void *p, size_t sz, void *ud) {
	Common::Array<byte> & chunkData = *reinterpret_cast<Common::Array<byte> * >(ud);
	const byte *buffer = reinterpret_cast<const byte *>(p);

	while (sz--) chunkData.push_back(*buffer++) ;

	return 1;
}
}

bool LuaScriptEngine::persist(OutputPersistenceBlock &Writer) {
	// Empty the Lua stack. pluto_persist() xepects that the stack is empty except for its parameters
	lua_settop(m_State, 0);

	// Garbage Collection erzwingen.
	lua_gc(m_State, LUA_GCCOLLECT, 0);

	// Permanents-Table is set on the stack
	// pluto_persist expects these two items on the Lua stack
	PushPermanentsTable(m_State, PTT_PERSIST);
	lua_getglobal(m_State, "_G");

	// Lua persists and stores the data in a Common::Array
	Common::Array<byte> chunkData;
	pluto_persist(m_State, Chunkwriter, &chunkData);

	// Persistenzdaten in den Writer schreiben.
	Writer.write(&chunkData[0], chunkData.size());

	// Die beiden Tabellen vom Stack nehmen.
	lua_pop(m_State, 2);

	return true;
}

// -----------------------------------------------------------------------------

namespace {
// -------------------------------------------------------------------------

struct ChunkreaderData {
	void   *BufferPtr;
	size_t  Size;
	bool    BufferReturned;
};

// ------------------------------------------------------------------------

const char *Chunkreader(lua_State *L, void *ud, size_t *sz) {
	ChunkreaderData &cd = *reinterpret_cast<ChunkreaderData *>(ud);

	if (!cd.BufferReturned) {
		cd.BufferReturned = true;
		*sz = cd.Size;
		return reinterpret_cast<const char *>(cd.BufferPtr);
	} else {
		return 0;
	}
}

// -------------------------------------------------------------------------

void ClearGlobalTable(lua_State *L, const char **Exceptions) {
	// Iterate over all elements of the global table
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0) {
		// Now the value and the index of the current element is on the stack
		// This value does not interest us, so it is popped from the stack
		lua_pop(L, 1);

		// Determine whether the item is set to nil, so you want to remove from the global table.
		// For this will determine whether the element name is a string and is present in
		// the list of exceptions
		bool SetElementToNil = true;
		if (lua_isstring(L, -1)) {
			const char *IndexString = lua_tostring(L, -1);
			const char **ExceptionsWalker = Exceptions;
			while (*ExceptionsWalker) {
				if (strcmp(IndexString, *ExceptionsWalker) == 0) SetElementToNil = false;
				++ExceptionsWalker;
			}
		}

		// If the above test showed that the item should be removed, it is removed by setting the value to nil.
		if (SetElementToNil) {
			lua_pushvalue(L, -1);
			lua_pushnil(L);
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}

	// Pop the Global table from the stack
	lua_pop(L, 1);

	// Perform garbage collection, so that all removed elements are deleted
	lua_gc(L, LUA_GCCOLLECT, 0);
}
}

// -----------------------------------------------------------------------------

bool LuaScriptEngine::unpersist(InputPersistenceBlock &Reader) {
	// Empty the Lua stack. pluto_persist() xepects that the stack is empty except for its parameters
	lua_settop(m_State, 0);

	// Permanents table is placed on the stack. This has already happened at this point, because
	// to create the table all permanents must be accessible. This is the case only for the
	// beginning of the function, because the global table is emptied below
	PushPermanentsTable(m_State, PTT_UNPERSIST);

	// All items from global table of _G and __METATABLES are removed.
	// After a garbage collection is performed, and thus all managed objects deleted

	// __METATABLES is not immediately removed becausen the Metatables are needed
	// for the finalisers of objects.
	static const char *ClearExceptionsFirstPass[] = {
		"_G",
		"__METATABLES",
		0
	};
	ClearGlobalTable(m_State, ClearExceptionsFirstPass);

	// In the second pass, the Metatables are removed
	static const char *ClearExceptionsSecondPass[] = {
		"_G",
		0
	};
	ClearGlobalTable(m_State, ClearExceptionsSecondPass);

	// Persisted Lua data
	Common::Array<byte> chunkData;
	Reader.read(chunkData);

	// Chunk-Reader initialisation. It is used with pluto_unpersist to restore read data
	ChunkreaderData cd;
	cd.BufferPtr = &chunkData[0];
	cd.Size = chunkData.size();
	cd.BufferReturned = false;

	pluto_unpersist(m_State, Chunkreader, &cd);

	// Permanents-Table is removed from stack
	lua_remove(m_State, -2);

	// The read elements in the global table about
	lua_pushnil(m_State);
	while (lua_next(m_State, -2) != 0) {
		// The referenec to the global table (_G) must not be overwritten, or ticks from Lua total
		bool IsGlobalReference = lua_isstring(m_State, -2) && strcmp(lua_tostring(m_State, -2), "_G") == 0;
		if (!IsGlobalReference) {
			lua_pushvalue(m_State, -2);
			lua_pushvalue(m_State, -2);

			lua_settable(m_State, LUA_GLOBALSINDEX);
		}

		// Pop value from the stack. The index is then ready for the next call to lua_next()
		lua_pop(m_State, 1);
	}

	// The table with the loaded data is popped from the stack
	lua_pop(m_State, 1);

	// Force garbage collection
	lua_gc(m_State, LUA_GCCOLLECT, 0);

	return true;
}

} // End of namespace Sword25
