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

#ifndef SWORD25_LUABINDHELPER_H
#define SWORD25_LUABINDHELPER_H

#include "sword25/kernel/common.h"

extern "C"
{
	#include "sword25/util/lua/lua.h"
	#include "sword25/util/lua/lauxlib.h"
}

#define lua_pushbooleancpp(L, b) (lua_pushboolean(L, b ? 1 : 0))
#define lua_tobooleancpp(L, i) (lua_toboolean(L, i) == 0 ? false : true)

struct lua_constant_reg
{
	const char *	Name;
	lua_Number		Value;
};

class BS_LuaBindhelper
{
public:
	/**
		@brief Registriert eine Menge von Funktionen und fügt dieser einer Lua-Library hinzu.
		@param L ein Pointer auf die Lua-VM in der die Funktionen registriert werden sollen
		@param LibName der Name der Library.<br>
					   Wenn dies ein Leerer String ist, werden die Funktionen zum globalen Namensraum hinzugefügt.
		@param Functions ein Array das die Funktionspointer mit ihren Namen enthält.<br>
						 Das Array muss mit dem Eintrag {0, 0} terminiert sein.
		@return Gibt true bei Erfolg zurück, ansonsten false.
	*/
	static bool AddFunctionsToLib(lua_State * L, const std::string & LibName, const luaL_reg * Functions);

	/**
		@brief Fügt eine Menge von Konstanten einer Lua-Library hinzu.
		@param L ein Pointer auf die Lua-VM in der die Konstanten registriert werden sollen
		@param LibName der Name der Library.<br>
					   Wenn dies ein Leerer String ist, werden die Konstanten zum globalen Namensraum hinzugefügt.
		@param Constants ein Array das die Werte der Konstanten mit ihren Namen enthält.<br
						 Das Array muss mit dem Eintrag {0, 0} terminiert sein.
		@return Gibt true bei Erfolg zurück, ansonsten false.
	*/
	static bool AddConstantsToLib(lua_State * L, const std::string & LibName, const lua_constant_reg * Constants);

	/**
	    @brief Fügt eine Menge von Methoden zu einer Lua-Klasse hinzu.
		@param L ein Pointer auf die Lua-VM in der die Methoden registriert werden sollen
		@param ClassName der Name der Metatable der Klasse.<br>
						 Wenn die Metatable noch nicht existiert, wird sie erstellt.
		@param Methods ein Array das die Funktionspointer der Methoden mit ihren Namen enthält.<br>
					   Das Array muss mit dem Eintrag {0, 0} terminiert sein.
		@return Gibt true bei Erfolg zurück, ansonsten false.
	*/
	static bool AddMethodsToClass(lua_State * L, const std::string & ClassName, const luaL_reg * Methods);

	/**
	    @brief Legt eine Funktion fest, die aufgerufen wird, wenn Exemplare einer bestimmten Lua-Klasse vom Garbage-Collecter gelöscht werden.
		@param L ein Pointer auf die Lua-VM
		@param ClassName der Name der Metatable der Klasse.<br>
						 Wenn die Metatable noch nicht existiert, wird sie erstellt.
		@param GCHandler ein Funktionspointer auf die Funktion.
		@return Gibt true bei Erfolg zurück, ansonsten false.
	*/
	static bool SetClassGCHandler(lua_State * L, const std::string & ClassName, lua_CFunction GCHandler);

	/**
		@brief Gibt einen String zurück, der einen Stackdump des Lua-Stacks enthält.

		@param L ein Pointer auf die Lua-VM.
	*/
	static std::string StackDump(lua_State * L);

	/**
		@brief Gibt einen String zurück, den den Inhalt einer Tabelle beschreibt.

        @param L ein Pointer auf die Lua-VM.
		@remark Auf dem Lua-Stack muss die Tabelle liegen, die ausgelesen werden soll.
	*/
	static std::string TableDump(lua_State * L);

	static bool GetMetatable(lua_State * L, const std::string & TableName);

private:
	static bool _CreateTable(lua_State * L, const std::string & TableName);
};

#endif
