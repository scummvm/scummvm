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

#include "luacallback.h"
#include "luabindhelper.h"

extern "C"
{
	#include <lua.h>
	#include <lauxlib.h>
}

#define BS_LOG_PREFIX "LUA"

// -----------------------------------------------------------------------------

namespace
{
	const char * CALLBACKTABLE_NAME = "__CALLBACKS";
}

// -----------------------------------------------------------------------------

BS_LuaCallback::BS_LuaCallback(lua_State * L)
{
	// Callbacktabelle erstellen.
	lua_newtable(L);
	lua_setglobal(L, CALLBACKTABLE_NAME);
}

// -----------------------------------------------------------------------------

BS_LuaCallback::~BS_LuaCallback()
{
}

// -----------------------------------------------------------------------------

void BS_LuaCallback::RegisterCallbackFunction(lua_State * L, unsigned int ObjectHandle)
{
	BS_ASSERT(lua_isfunction(L, -1));
	EnsureObjectCallbackTableExists(L, ObjectHandle);

	// Funktion in der Objekt-Callbacktabelle speichern.
	lua_pushvalue(L, -2);
	luaL_ref(L, -2);

	// Funktion und Objekt-Callbacktabelle vom Stack poppen.
	lua_pop(L, 2);
}

// -----------------------------------------------------------------------------

void BS_LuaCallback::UnregisterCallbackFunction(lua_State * L, unsigned int ObjectHandle)
{
	BS_ASSERT(lua_isfunction(L, -1));
	EnsureObjectCallbackTableExists(L,ObjectHandle);

	// Über alle Elemente der Objekt-Callbacktabelle iterieren und die Funktion daraus entfernen.
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		// Der Wert des aktuellen Elementes liegt oben auf dem Stack, darunter der Index.

		// Falls der Wert identisch mit dem Funktionsparameter ist, wird sie aus der Tabelle entfernt.
		if (lua_equal(L, -1, -4))
		{
			lua_pushvalue(L, -2);
			lua_pushnil(L);
			lua_settable(L, -5);

			// Die Funktion wurde gefunden, die Iteration kann abgebrochen werden.
			lua_pop(L, 2);
			break;
		}
		else
		{
			// Wert vom Stack poppen. Der Index liegt dann oben für den nächsten Aufruf von lua_next().
			lua_pop(L, 1);
		}
	}

	// Funktion und Objekt-Callbacktabelle vom Stack poppen.
	lua_pop(L, 2);
}

// -----------------------------------------------------------------------------

void BS_LuaCallback::RemoveAllObjectCallbacks(lua_State * L, unsigned int ObjectHandle)
{
	PushCallbackTable(L);

	// Objekt-Callbacktabelle aus der Callbacktabelle entfernen.
	lua_pushnumber(L, ObjectHandle);
	lua_pushnil(L);
	lua_settable(L, -3);

	lua_pop(L, 1);
}

// -----------------------------------------------------------------------------

void BS_LuaCallback::InvokeCallbackFunctions(lua_State * L, unsigned int ObjectHandle)
{
	EnsureObjectCallbackTableExists(L, ObjectHandle);

	// Über die Tabelle iterieren und alle Callbacks ausführen.
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		// Der Wert des aktuellen Elementes liegt oben auf dem Stack, darunter der Index.

		// Falls der Wert eine Funktion ist, wird sie ausgeführt.
		if (lua_type(L, -1) == LUA_TFUNCTION)
		{
			// Pre-Funktion aufrufen.
			// Abgeleitete Klassen könnten in dieser Funktion Parameter auf den Stack schieben.
			// Der Rückgabewert gibt die Anzahl der Parameter zurück.
			int ArgumentCount = PreFunctionInvokation(L);

			// lua_pcall poppt die Funktion und die Parameter selber vom Stack.
			if (lua_pcall(L, ArgumentCount, 0, 0) != 0)
			{
				// Ein Fehler ist aufgetreten.
				BS_LOG_ERRORLN("An error occured executing a callback function: %s", lua_tostring(L, -1));

				// Fehlernachricht vom Stack poppen.
				lua_pop(L, 1);
			}
		}
		else
		{
			// Wert vom Stack poppen. Der Index liegt dann oben für den nächsten Aufruf von lua_next().
			lua_pop(L, 1);
		}
	}
}

// -----------------------------------------------------------------------------

void BS_LuaCallback::EnsureObjectCallbackTableExists(lua_State * L, unsigned int ObjectHandle)
{
	PushObjectCallbackTable(L, ObjectHandle);

	// Falls die Tabelle nil ist, muss sie zunächst erstellt werden.
	if (lua_isnil(L, -1))
	{
		// Nil vom Stack poppen.
		lua_pop(L, 1);

		PushCallbackTable(L);

		// Neue Tabelle unter dem Index ObjectHandle in der Callbacktabelle ablegen.
		lua_newtable(L);
		lua_pushnumber(L, ObjectHandle);
		lua_pushvalue(L, -2);
		lua_settable(L, -4);

		// Callbacktabelle vom Stack entfernen, Objekt-Callbacktabelle aber dort lassen.
		lua_remove(L, -2);
	}
}

// -----------------------------------------------------------------------------

void BS_LuaCallback::PushCallbackTable(lua_State * L)
{
	lua_getglobal(L, CALLBACKTABLE_NAME);
}

// -----------------------------------------------------------------------------

void BS_LuaCallback::PushObjectCallbackTable(lua_State * L, unsigned int ObjectHandle)
{
	PushCallbackTable(L);

	// Objekt-Callbacktabelle auf den Stack legen.
	lua_pushnumber(L, ObjectHandle);
	lua_gettable(L, -2);

	// Callbacktabelle vom Stack entfernen, Objekt-Callbacktabelle aber dort lassen.
	lua_remove(L, -2);
}
