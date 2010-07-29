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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/kernel.h"
#include "luabindhelper.h"
#include "luascript.h"
#include <sstream>

#define BS_LOG_PREFIX "LUABINDHELPER"

// -----------------------------------------------------------------------------

namespace
{
	const char * METATABLES_TABLE_NAME = "__METATABLES";
	const char * PERMANENTS_TABLE_NAME = "Permanents";

	bool RegisterPermanent(lua_State * L, const std::string & Name)
	{
		// Eine C-Funktion muss auf dem Stack liegen.
		if (!lua_iscfunction(L, -1)) return false;

		// Sicherstellen, dass die Permanents-Tabelle oben auf dem Stack liegt.
		lua_getfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);
		if (lua_isnil(L, -1))
		{
			// Permanents-Tabelle existiert noch nicht, sie muss erstellt werden.

			// Nil vom Stack poppen.
			lua_pop(L, 1);

			// Permanents-Tabelle erstellen und eine zweite Referenz darauf auf den Stack legen.
			lua_newtable(L);
			lua_pushvalue(L, -1);

			// Permanents-Tabelle in der Registry speichern. Die zweite Referenz verbleibt auf dem Stack um im Anschluss benutzt zu werden.
			lua_setfield(L, LUA_REGISTRYINDEX, PERMANENTS_TABLE_NAME);
		}

		// C-Funktion mit dem Namen als Index in der Permanents-Tabelle ablegen.
		lua_insert(L, -2);
		lua_setfield(L, -2, Name.c_str());

		// Permanents-Tabelle vom Stack nehmen.
		lua_pop(L, 1);

		return true;
	}
}

// -----------------------------------------------------------------------------

bool BS_LuaBindhelper::AddFunctionsToLib(lua_State * L, const std::string & LibName, const luaL_reg * Functions)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Wenn der Tabellenname leer ist, werden die Funktionen zum globalen Namensraum hinzugefügt.
	if (LibName.size() == 0)
	{
		for (; Functions->name; ++Functions)
		{
			lua_pushstring(L, Functions->name);
			lua_pushcclosure(L, Functions->func, 0);
			lua_settable(L, LUA_GLOBALSINDEX);

			// Funktion als permanent registrieren, damit sie beim Persistieren ignoriert wird.
			lua_pushstring(L, Functions->name);
			lua_gettable(L, LUA_GLOBALSINDEX);
			RegisterPermanent(L, Functions->name);
		}
	}
	// Wenn der Tabellenname nicht leer ist, werden die Funktionen zu dieser Tabelle hinzugefügt.
	else
	{
		// Sicherstellen, dass die Library-Table existiert.
		if (!_CreateTable(L, LibName)) return false;

		// Die einzelnen Funktionen in der Table registrieren.
		for (; Functions->name; ++Functions)
		{
			// Funktion registrieren.
			lua_pushstring(L, Functions->name);
			lua_pushcclosure(L, Functions->func, 0);
			lua_settable(L, -3);

			// Funktion als permanent registrieren, damit sie beim Persistieren ignoriert wird.
			lua_pushstring(L, Functions->name);
			lua_gettable(L, -2);
			RegisterPermanent(L, LibName + "." + Functions->name);
		}

		// Library-Table vom Lua-Stack nehmen.
		lua_pop(L, 1);
	}
	

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

// -----------------------------------------------------------------------------

bool BS_LuaBindhelper::AddConstantsToLib(lua_State * L, const std::string & LibName, const lua_constant_reg * Constants)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Wenn der Tabellenname leer ist, werden die Konstanten zum globalen Namensraum hinzugefügt.
	if (LibName.size() == 0)
	{
		for (; Constants->Name; ++Constants)
		{
			lua_pushstring(L, Constants->Name);
			lua_pushnumber(L, Constants->Value);
			lua_settable(L, LUA_GLOBALSINDEX);
		}
	}
	// Wenn der Tabellenname nicht leer ist, werden die Konstanten zu dieser Tabelle hinzugefügt.
	else
	{
		// Sicherstellen, dass die Library-Table existiert.
		if (!_CreateTable(L, LibName)) return false;
		
		// Die einzelnen Konstanten in der Table registrieren
		for (; Constants->Name; ++Constants)
		{
			lua_pushstring(L, Constants->Name);
			lua_pushnumber(L, Constants->Value);
			lua_settable(L, -3);
		}

		// Library-Tabelle vom Lua-Stack nehmen
		lua_pop(L, 1);
	}

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

// -----------------------------------------------------------------------------

bool BS_LuaBindhelper::AddMethodsToClass(lua_State * L, const std::string & ClassName, const luaL_reg * Methods)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Metatable auf den Lua-Stack laden
	if (!GetMetatable(L, ClassName)) return false;
	
	// Die einzelnen Methoden in der Metatable registrieren
	for (; Methods->name; ++Methods)
	{
		lua_pushstring(L, Methods->name);
		lua_pushcclosure(L, Methods->func, 0);
		lua_settable(L, -3);

		// Funktion als permanent registrieren, damit sie beim Persistieren ignoriert wird.
		lua_pushstring(L, Methods->name);
		lua_gettable(L, -2);
		RegisterPermanent(L, ClassName + "." + Methods->name);
	}

	// Metatable vom Lua-Stack nehmen
	lua_pop(L, 1);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

// -----------------------------------------------------------------------------

bool BS_LuaBindhelper::SetClassGCHandler(lua_State * L, const std::string & ClassName, lua_CFunction GCHandler)
{
#ifdef DEBUG
	int __startStackDepth = lua_gettop(L);
#endif

	// Metatable auf den Lua-Stack laden
	if (!GetMetatable(L, ClassName)) return false;

	// Den GC-Handler in die Metatable schreiben
	lua_pushstring(L, "__gc");
	lua_pushcclosure(L, GCHandler, 0);
	lua_settable(L, -3);

	// Funktion als permanent registrieren, damit sie beim Persistieren ignoriert wird.
	lua_pushstring(L, "__gc");
	lua_gettable(L, -2);
	RegisterPermanent(L, ClassName + ".__gc");

	// Metatable vom Lua-Stack nehmen
	lua_pop(L, 1);

#ifdef DEBUG
	BS_ASSERT(__startStackDepth == lua_gettop(L));
#endif

	return true;
}

// -----------------------------------------------------------------------------

namespace
{
	void PushMetatableTable(lua_State * L)
	{
		// Tabelle mit den Metatabellen auf den Stack legen.
		lua_getglobal(L, METATABLES_TABLE_NAME);

		// Wenn die Tabelle noch nicht existiert, muss sie erstellt werden.
		if (lua_isnil(L, -1))
		{
			// nil vom Stack poppen.
			lua_pop(L, 1);

			// Neue Tabelle erstellen, in die globale Table eintragen und eine Referenz auf dem Stack lassen.
			lua_newtable(L);
			lua_pushvalue(L, -1);
			lua_setglobal(L, METATABLES_TABLE_NAME);
		}
	}
}


bool BS_LuaBindhelper::GetMetatable(lua_State * L, const std::string & TableName)
{
	// Tabelle mit den Metatabellen auf den Stack legen.
	PushMetatableTable(L);

	// Versuchen, die gewünschte Metatabelle auf den Stack zu legen. Wenn sie noch nicht existiert, muss sie erstellt werden.
	lua_getfield(L, -1, TableName.c_str());
	if (lua_isnil(L, -1))
	{
		// nil vom Stack poppen.
		lua_pop(L, 1);

		// Neue Tabelle erstellen.
		lua_newtable(L);

		// Das __index Feld der Metatabele zeigt auf die Metatabelle selbst.
		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");

		// Persistfeld auf true setzen. Dies sorgt dafür, dass Objekte mit dieser Metatabelle direkt gespeichert werden.
		lua_pushbooleancpp(L, true);
		lua_setfield(L, -2, "__persist");

		// Metatabelle in die Tabelle für Metatabellen eintragen und eine Referenz auf dem Stack lassen.
		lua_pushvalue(L, -1);
		lua_setfield(L, -3, TableName.c_str());
	}

	// Tabelle mit den Metatabellen vom Stack nehmen.
	lua_remove(L, -2);

	return true;
}

// -----------------------------------------------------------------------------

bool BS_LuaBindhelper::_CreateTable(lua_State * L, const std::string & TableName)
{
	// Der Tabellenname wird an den Punkten auseinandergetrennt und jeweils die Untertabellen erstellt.
	// Auf diese Weise können auch Tabellen mit Untertabellen erstellt werden (z.B. Foo.Bar).
	std::string::size_type PartBegin = 0;
	while (PartBegin <= TableName.size())
	{
		std::string::size_type PartEnd;
		PartEnd = TableName.find(".", PartBegin);
		if (PartEnd == std::string::npos) PartEnd = TableName.size();
		std::string SubTableName = TableName.substr(PartBegin, PartEnd - PartBegin);

		// Tabellen mit einen leeren String als Namen sind nicht zulässig.
		if (SubTableName.size() == 0) return false;

		// Überprüfen, ob die Tabelle mit dem Namen bereits existiert.
		// Beim ersten Durchgang wird im globalen Namensbereich gesucht, bei späteren Durchgängen in der entsprechenden Elterntabelle auf dem Stack.
		if (PartBegin == 0)
		{
			lua_pushstring(L, SubTableName.c_str());
			lua_gettable(L, LUA_GLOBALSINDEX);
		}
		else
		{
			lua_pushstring(L, SubTableName.c_str());
			lua_gettable(L, -2);
			if (!lua_isnil(L, -1)) lua_remove(L, -2);
		}

		// Wenn nicht, Table erstellen
		if (lua_isnil(L, -1))
		{
			// nil-Wert vom Stack holen
			lua_pop(L, 1);

			// Neue Tabelle erstellen
			lua_newtable(L);
			lua_pushstring(L, SubTableName.c_str());
			lua_pushvalue(L, -2);
			if (PartBegin == 0)
				lua_settable(L, LUA_GLOBALSINDEX);
			else
			{
				lua_settable(L, -4);
				lua_remove(L, -2);
			}
		}

		PartBegin = PartEnd + 1;
	}

	return true;
}

namespace
{
	std::string GetLuaValueInfo(lua_State * L, int StackIndex)
	{
		switch (lua_type(L, StackIndex))
		{
			case LUA_TNUMBER:
				lua_pushstring(L, lua_tostring(L, StackIndex));
				break;

			case LUA_TSTRING:
				lua_pushfstring(L, "\"%s\"", lua_tostring(L, StackIndex));
				break;

			case LUA_TBOOLEAN:
				lua_pushstring(L, (lua_toboolean(L, StackIndex) ? "true" : "false"));
				break;

			case LUA_TNIL:
				lua_pushliteral(L, "nil");
				break;

			default:
				lua_pushfstring(L, "%s: %p", luaL_typename(L, StackIndex), lua_topointer(L, StackIndex));
				break;
		}

		std::string Result(lua_tostring(L, -1));
		lua_pop(L, 1);

		return Result;
	}
}

std::string BS_LuaBindhelper::StackDump(lua_State *L)
{
	std::ostringstream oss;

	int i = lua_gettop(L);
	oss << "------------------- Stack Dump -------------------\n";

	while(i)
	{
		oss << i << ": " << GetLuaValueInfo(L, i) << "\n";
		i--;
	}

	oss << "-------------- Stack Dump Finished ---------------\n";

	return oss.str();
} 

std::string BS_LuaBindhelper::TableDump(lua_State * L)
{
	std::ostringstream oss;

	oss << "------------------- Table Dump -------------------\n";

	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		// Der Wert des aktuellen Elementes liegt oben auf dem Stack, darunter der Index.
		oss << GetLuaValueInfo(L, -2) << " : " << GetLuaValueInfo(L, -1) << "\n";

		// Wert vom Stack poppen. Der Index liegt dann oben für den nächsten Aufruf von lua_next().
		lua_pop(L, 1);
	}

	oss << "-------------- Table Dump Finished ---------------\n";

	return oss.str();
}
