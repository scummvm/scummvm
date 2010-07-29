// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdörfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#define BS_LOG_PREFIX "LUA"

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

extern "C"
{
	#include <lua.h>
	#include <lualib.h>
	#include <lauxlib.h>
	#include "util/pluto/pluto.h"
}

#include "package/packagemanager.h"
#include "luascript.h"
#include "luabindhelper.h"

#include "kernel/outputpersistenceblock.h"
#include "kernel/inputpersistenceblock.h"

#include "kernel/memlog_off.h"
#include <vector>
#include "kernel/memlog_on.h"

using namespace std;

// -----------------------------------------------------------------------------
// Konstruktion / Destruktion
// -----------------------------------------------------------------------------

BS_LuaScriptEngine::BS_LuaScriptEngine(BS_Kernel * KernelPtr) :
	BS_ScriptEngine(KernelPtr),
	m_State(0),
	m_PcallErrorhandlerRegistryIndex(0)
{
}

// -----------------------------------------------------------------------------

BS_LuaScriptEngine::~BS_LuaScriptEngine()
{
	// Lua deinitialisieren
	if (m_State) lua_close(m_State);
}

// -----------------------------------------------------------------------------

BS_Service * BS_LuaScriptEngine_CreateObject(BS_Kernel * KernelPtr) { return new BS_LuaScriptEngine(KernelPtr); }

// -----------------------------------------------------------------------------

namespace
{
	int PanicCB(lua_State * L)
	{
		BS_LOG_ERRORLN("Lua panic. Error message: %s", lua_isnil(L, -1) ? "" : lua_tostring(L, -1));
		return 0;
	}
}

// -----------------------------------------------------------------------------

bool BS_LuaScriptEngine::Init()
{
	// Lua-State intialisieren und Standardbibliotheken initialisieren
	m_State = luaL_newstate();
	if (!m_State || ! RegisterStandardLibs() || !RegisterStandardLibExtensions())
	{
		BS_LOG_ERRORLN("Lua could not be initialized.");
		return false;
	}

	// Panic-Callbackfunktion registrieren.
	lua_atpanic(m_State, PanicCB);
	
	// Errorhandlerfunktion für lua_pcall-Aufrufe.
	// Der untenstehende Code enthält eine lokale ErrorHandler-Funktion und gibt diese zurück.
	const char ErrorHandlerCode[] =
		"local function ErrorHandler(message) "
		"	return message .. '\\n' .. debug.traceback('', 2) "
		"end "
		"return ErrorHandler";

	// Den Code compilieren.
	if (luaL_loadbuffer(m_State, ErrorHandlerCode, strlen(ErrorHandlerCode), "PCALL ERRORHANDLER") != 0)
	{
		// Fehlernachricht ausgeben und Methode beenden.
		BS_LOG_ERRORLN("Couldn't compile luaL_pcall errorhandler:\n%s", lua_tostring(m_State, -1));
		lua_pop(m_State, 1);

		return false;
	}
	// Den Code ausführen, dies legt die Errorhandler-Funktion oben auf den Stack.
	if (lua_pcall(m_State, 0, 1, 0) != 0)
	{
		// Fehlernachricht ausgeben und Methode beenden.
		BS_LOG_ERRORLN("Couldn't prepare luaL_pcall errorhandler:\n%s", lua_tostring(m_State, -1));
		lua_pop(m_State, 1);

		return false;
	}
	
	// Die Errorhandler-Funktion in der Lua-Registry ablegen und den Index merken.
	m_PcallErrorhandlerRegistryIndex = luaL_ref(m_State, LUA_REGISTRYINDEX);

	// Die Pluto Persistenz-Bibliothek initialisieren.
	luaopen_pluto(m_State);
	lua_pop(m_State, 1);

	BS_LOGLN("Lua initialized.");

	return true;
}

// -----------------------------------------------------------------------------

bool BS_LuaScriptEngine::ExecuteFile(const std::string & FileName)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(m_State);
#endif

	// Pointer auf den Packagemanager holen
	BS_PackageManager * pPackage = static_cast<BS_PackageManager *>(BS_Kernel::GetInstance()->GetService("package"));
	BS_ASSERT(pPackage);

	// Datei einlesen
	unsigned int FileSize;
	char * FileData = static_cast<char *>(pPackage->GetFile(FileName, &FileSize));
	if (!FileData)
	{
		BS_LOG_ERRORLN("Couldn't read \"%s\".", FileName.c_str());
#ifdef DEBUG
		BS_ASSERT(__startStackDepth == lua_gettop(m_State));
#endif
		return false;
	}

	// Dateiinhalt ausführen
	if (!ExecuteBuffer(FileData, FileSize, "@" + pPackage->GetAbsolutePath(FileName)))
	{
		// Dateipuffer freigeben
		delete[] FileData;
#ifdef DEBUG
		BS_ASSERT(__startStackDepth == lua_gettop(m_State));
#endif
		return false;
	}

	// Dateipuffer freigeben
	delete[] FileData;

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(m_State));
#endif

	return true;
}

// -----------------------------------------------------------------------------

bool BS_LuaScriptEngine::ExecuteString(const std::string & Code)
{
	return ExecuteBuffer(Code.c_str(), Code.length(), "???");
}

// -----------------------------------------------------------------------------

namespace
{
	void RemoveForbiddenFunctions(lua_State * L)
	{
		static const char * FORBIDDEN_FUNCTIONS[] =
		{
			"dofile",
			0
		};

		const char ** Iterator = FORBIDDEN_FUNCTIONS;
		while (*Iterator)
		{
			lua_pushnil(L);
			lua_setfield(L, LUA_GLOBALSINDEX, *Iterator);
			++Iterator;
		}
	}
}

bool BS_LuaScriptEngine::RegisterStandardLibs()
{
	luaL_openlibs(m_State);
	RemoveForbiddenFunctions(m_State);
	return true;
}

// -----------------------------------------------------------------------------

bool BS_LuaScriptEngine::ExecuteBuffer(const char * Data, unsigned int Size, const std::string & Name) const
{
	// Puffer kompilieren
	if (luaL_loadbuffer(m_State, Data, Size, Name.c_str()) != 0)
	{
		BS_LOG_ERRORLN("Couldn't compile \"%s\":\n%s", Name.c_str(), lua_tostring(m_State, -1));
		lua_pop(m_State, 1);

		return false;
	}

	// Error-Handler Funktion hinter der auszuführenden Funktion auf den Stack legen.
	lua_rawgeti(m_State, LUA_REGISTRYINDEX, m_PcallErrorhandlerRegistryIndex);
	lua_insert(m_State, -2);

	// Pufferinhalt ausführen
	if (lua_pcall(m_State, 0, 0, -2) != 0)
	{
		BS_LOG_ERRORLN("An error occured while executing \"%s\":\n%s.",
						Name.c_str(),
						lua_tostring(m_State, -1));
		lua_pop(m_State, 2);

		return false;
	}

	// Error-Handler Funktion vom Stack nehmen.
	lua_pop(m_State, 1);

	return true;
}

// -----------------------------------------------------------------------------

void BS_LuaScriptEngine::SetCommandLine(const vector<string> & CommandLineParameters)
{
	lua_newtable(m_State);

	for (size_t i = 0; i < CommandLineParameters.size(); ++i)
	{
		lua_pushnumber(m_State, i + 1);
		lua_pushstring(m_State, CommandLineParameters[i].c_str());
		lua_settable(m_State, -3);
	}

	lua_setglobal(m_State, "CommandLine");
}

// -----------------------------------------------------------------------------

namespace
{
	const char * PERMANENTS_TABLE_NAME = "Permanents";

	// -------------------------------------------------------------------------

	// Diese Array enthält die Namen der globalen Lua-Objekte, die nicht persistiert werden sollen.
	const char * STANDARD_PERMANENTS[] =
	{
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

	enum PERMANENT_TABLE_TYPE
	{
		PTT_PERSIST,
		PTT_UNPERSIST,
	};

	// -------------------------------------------------------------------------

	bool PushPermanentsTable(lua_State * L, PERMANENT_TABLE_TYPE TableType)
	{
		// Permanents-Tabelle erstellen.
		lua_newtable(L);

		// Alle Standard-Permanents in die Tabelle einfügen.
		unsigned int Index = 0;
		while (STANDARD_PERMANENTS[Index])
		{
			// Permanent auf den Stack legen, falls es nicht existiert, wird es einfach ignoriert.
			lua_getglobal(L, STANDARD_PERMANENTS[Index]);
			if (!lua_isnil(L, -1))
			{
				// Namen des Elementes als einzigartigen Wert auf den Stack legen.
				lua_pushstring(L, STANDARD_PERMANENTS[Index]);

				// Falls geladen wird, ist der Name der Index und das Objekt der Wert.
				// In diesem Fall müssen also die Position von Name und Objekt auf dem Stack vertauscht werden.
				if (TableType == PTT_UNPERSIST) lua_insert(L, -2);

				// Eintrag in der Tabelle vornehmen.
				lua_settable(L, -3);
			}
			else
			{
				// nil von Stack poppen.
				lua_pop(L, 1);
			}

			++Index;
		}
		
		// Alle Registrierten C-Funktionen in die Tabelle einfügen.
		// BS_LuaBindhelper legt in der Registry eine Tabelle an, in der alle registrierten C-Funktionen gespeichert werden.

		// Tabelle mit den C-Permanents auf den Stack legen.
		lua_getfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);

		if (!lua_isnil(L, -1))
		{
			// Über alle Elemente der Tabelle iterieren.
			lua_pushnil(L);
			while (lua_next(L, -2) != 0)
			{
				// Wert und Index auf dem Stack duplizieren und in der Reihenfolge vertauschen.
				lua_pushvalue(L, -1);
				lua_pushvalue(L, -3);

				// Falls geladen wird, ist der Name der Index und das Objekt der Wert.
				// In diesem Fall müssen also die Position von Name und Objekt auf dem Stack vertauscht werden.
				if (TableType == PTT_UNPERSIST) lua_insert(L, -2);

				// Eintrag in der Ergebnistabelle vornehmen.
				lua_settable(L, -6);

				// Wert vom Stack poppen. Der Index liegt dann oben für den nächsten Aufruf von lua_next().
				lua_pop(L, 1);
			}
		}

		// Tabelle mit den C-Permanents vom Stack poppen.
		lua_pop(L, 1);

		// coroutine.yield muss extra in die Permanents-Tabelle eingetragen werden, da inaktive Coroutinen diese C-Funktion auf dem Stack liegen
		// haben.

		// Funktion coroutine.yield auf den Stack legen.
		lua_getglobal(L, "coroutine");
		lua_pushstring(L, "yield");
		lua_gettable(L, -2);

		// coroutine.yield mit eigenem eindeutigen Wert in der Permanents-Tabelle ablegen.
		lua_pushstring(L, "coroutine.yield");

		if (TableType == PTT_UNPERSIST) lua_insert(L, -2);

		lua_settable(L, -4);

		// Tabelle coroutine vom Stack poppen.
		lua_pop(L, 1);

		return true;
	}
}

// -----------------------------------------------------------------------------

namespace
{
	int Chunkwriter(lua_State *L, const void* p, size_t sz, void* ud)
	{
		vector<unsigned char> & chunkData = *reinterpret_cast<vector<unsigned char> * >(ud);
		const unsigned char * buffer = reinterpret_cast<const unsigned char *>(p);

		while (sz--) chunkData.push_back(*buffer++);

		return 1;
	}
}

bool BS_LuaScriptEngine::Persist(BS_OutputPersistenceBlock & Writer)
{
	// Den Lua-Stack leeren. pluto_persist() erwartet, dass der Stack bis aus seine Parameter leer ist.
	lua_settop(m_State, 0);

	// Garbage Collection erzwingen.
	lua_gc(m_State, LUA_GCCOLLECT, 0);

	// Permanents-Tabelle und die zu persistierende Tabelle auf den Stack legen.
	// pluto_persist erwartet diese beiden Objekte auf dem Lua-Stack.
	PushPermanentsTable(m_State, PTT_PERSIST);
	lua_getglobal(m_State, "_G");

	// Lua persistieren und die Daten in einem vector ablegen.
	vector<unsigned char> chunkData;
	pluto_persist(m_State, Chunkwriter, &chunkData);

	// Persistenzdaten in den Writer schreiben.
	Writer.Write(&chunkData[0], chunkData.size());

	// Die beiden Tabellen vom Stack nehmen.
	lua_pop(m_State, 2);

	return true;
}

// -----------------------------------------------------------------------------

namespace
{
	// -------------------------------------------------------------------------

	struct ChunkreaderData
	{
		void *	BufferPtr;
		size_t	Size;
		bool	BufferReturned;
	};

	// -------------------------------------------------------------------------

	const char * Chunkreader(lua_State *L, void *ud, size_t *sz)
	{
		ChunkreaderData & cd = *reinterpret_cast<ChunkreaderData *>(ud);

		if (!cd.BufferReturned)
		{
			cd.BufferReturned = true;
			*sz = cd.Size;
			return reinterpret_cast<const char *>(cd.BufferPtr);
		}
		else
		{
			return 0;
		}
	}

	// -------------------------------------------------------------------------

	void ClearGlobalTable(lua_State * L, const char ** Exceptions)
	{
		// Über alle Elemente der globalen Tabelle iterieren.
		lua_pushvalue(L, LUA_GLOBALSINDEX);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			// Jetzt liegen der Wert und der Index des aktuellen Elementes auf dem Stack.
			// Der Wert interessiert uns nicht, daher wird er vom Stack gepoppt.
			lua_pop(L, 1);

			// Feststellen, ob das Element auf nil gesetzt , also aus der globalen Tabelle entfernt werden soll.
			// Hierfür wird geprüft, ob das Elementname ein String ist und in der Liste der Ausnahmen vorkommt.
			bool SetElementToNil = true;
			if (lua_isstring(L, -1))
			{
				const char * IndexString = lua_tostring(L, -1);
				const char ** ExceptionsWalker = Exceptions;
				while (*ExceptionsWalker)
				{
					if (strcmp(IndexString, *ExceptionsWalker) == 0) SetElementToNil = false;
					++ExceptionsWalker;
				}
			}

			// Wenn der obige Test ergeben hat, dass das Element entfernt werden soll, wird es entfernt indem der Wert auf nil gesetzt wird.
			if (SetElementToNil)
			{
				lua_pushvalue(L, -1);
				lua_pushnil(L);
				lua_settable(L, LUA_GLOBALSINDEX);
			}
		}	

		// Globale Tabelle vom Stack nehmen.
		lua_pop(L, 1);

		// Garbage-Collection vornehmen, damit die entfernten Elemente alle gelöscht werden.
		lua_gc(L, LUA_GCCOLLECT, 0);
	}
}

// -----------------------------------------------------------------------------

bool BS_LuaScriptEngine::Unpersist(BS_InputPersistenceBlock & Reader)
{
	// Den Lua-Stack leeren. pluto_unpersist() erwartet, dass der Stack bis aus seine Parameter leer ist.
	lua_settop(m_State, 0);

	// Permanents Tabelle auf den Stack legen. Dies passiert schon an dieser Stelle, da zum Erstellen der Tabelle alle Permanents zugreifbar sein
	// müssen. Dies ist nur am Anfang dieser Funktion der Fall, im Folgenden wird die globale Tabelle geleert.
	PushPermanentsTable(m_State, PTT_UNPERSIST);

	// Alle Elemente aus der globalen Tabelle mit Ausnhame von _G und __METATABLES entfernen.
	// Danach wird eine Garbage Collection durchgeführt und somit alle von Lua verwalteten Objekte gelöscht.
	// __METATABLES wird zunächst nicht entfernt, da die Metatables für die Finalizer der Objekte benötigt werden.
	static const char * ClearExceptionsFirstPass[] =
	{
		"_G",
		"__METATABLES",
		0
	};
	ClearGlobalTable(m_State, ClearExceptionsFirstPass);

	// Im zweiten Durchgang werden auch die Metatables entfernt.
	static const char * ClearExceptionsSecondPass[] =
	{
		"_G",
		0
	};
	ClearGlobalTable(m_State, ClearExceptionsSecondPass);

	// Persistierte Lua-Daten einlesen.
	vector<unsigned char> chunkData;
	Reader.Read(chunkData);

	// Chunk-Reader initialisieren. Er wird von pluto_unpersist benutzt um die benötigten Daten einzulesen.
	ChunkreaderData cd;
	cd.BufferPtr = &chunkData[0];
	cd.Size = chunkData.size();
	cd.BufferReturned = false;

	pluto_unpersist(m_State, Chunkreader, &cd);

	// Permanents-Tabelle vom Stack nehmen.
	lua_remove(m_State, -2);

	// Die eingelesenen Elemente in die globale Tabelle eintragen.
	lua_pushnil(m_State);
	while (lua_next(m_State, -2) != 0)
	{
		// Die Referenz auf die globale Tabelle (_G) darf nicht überschrieben werden, sonst tickt Lua total aus.
		bool IsGlobalReference = lua_isstring(m_State, -2) && strcmp(lua_tostring(m_State, -2), "_G") == 0;
		if (!IsGlobalReference)
		{
			lua_pushvalue(m_State, -2);
			lua_pushvalue(m_State, -2);

			lua_settable(m_State, LUA_GLOBALSINDEX);
		}

		// Wert vom Stack poppen. Der Index liegt dann oben für den nächsten Aufruf von lua_next().
		lua_pop(m_State, 1);
	}

	// Tabelle mit den geladenen Daten vom Stack poppen.
	lua_pop(m_State, 1);
	
	// Garbage Collection erzwingen.
	lua_gc(m_State, LUA_GCCOLLECT, 0);

	return true;
}
