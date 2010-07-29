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

#include <string>
#include <algorithm>

#include "kernel/common.h"
#include "kernel/kernel.h"
#include "kernel/callbackregistry.h"
#include "script/script.h"
#include "script/luabindhelper.h"
#include "script/luacallback.h"

#include "inputengine.h"

#define BS_LOG_PREFIX "INPUTENGINE"

// -----------------------------------------------------------------------------
// Callback-Objekte
// -----------------------------------------------------------------------------

static void TheCharacterCallback(unsigned char Character);
static void TheCommandCallback(BS_InputEngine::KEY_COMMANDS Command);

namespace
{
	class CharacterCallbackClass : public BS_LuaCallback
	{
	public:
		CharacterCallbackClass(lua_State * L) : BS_LuaCallback(L) {};

		std::string Character;

	protected:
		int PreFunctionInvokation(lua_State * L)
		{
			lua_pushstring(L, Character.c_str());
			return 1;
		}
	};
	std::auto_ptr<CharacterCallbackClass> CharacterCallbackPtr;

	// -----------------------------------------------------------------------------

	class CommandCallbackClass : public BS_LuaCallback
	{
	public:
		CommandCallbackClass(lua_State * L) : BS_LuaCallback(L) { Command = BS_InputEngine::KEY_COMMAND_BACKSPACE; }

		BS_InputEngine::KEY_COMMANDS Command;

	protected:
		int PreFunctionInvokation(lua_State * L)
		{
			lua_pushnumber(L, Command);
			return 1;
		}
	};
	std::auto_ptr<CommandCallbackClass> CommandCallbackPtr;

	// -------------------------------------------------------------------------

	struct CallbackfunctionRegisterer
	{
		CallbackfunctionRegisterer()
		{
			BS_CallbackRegistry::GetInstance().RegisterCallbackFunction("LuaCommandCB", TheCommandCallback);
			BS_CallbackRegistry::GetInstance().RegisterCallbackFunction("LuaCharacterCB", TheCharacterCallback);
		}
	};
	static CallbackfunctionRegisterer Instance;
}

// -----------------------------------------------------------------------------

static BS_InputEngine * GetIE()
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_InputEngine * pIE = static_cast<BS_InputEngine *>(pKernel->GetService("input"));
	BS_ASSERT(pIE);
	return pIE;
}

// -----------------------------------------------------------------------------

static int Init(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushbooleancpp(L, pIE->Init());
	return 1;
}

// -----------------------------------------------------------------------------

static int Update(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	// Beim ersten Aufruf der Update()-Methode werden die beiden Callbacks am Input-Objekt registriert.
	// Dieses kann nicht in _RegisterScriptBindings() passieren, da diese Funktion vom Konstruktor der abstrakten Basisklasse aufgerufen wird und die
	// Register...()-Methoden abstrakt sind, im Konstruktor der Basisklasse also nicht aufgerufen werden können.
	static bool FirstCall = true;
	if (FirstCall)
	{
		FirstCall = false;
		pIE->RegisterCharacterCallback(TheCharacterCallback);
		pIE->RegisterCommandCallback(TheCommandCallback);
	}

	pIE->Update();
	return 0;
}

// -----------------------------------------------------------------------------

static int IsLeftMouseDown(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushbooleancpp(L, pIE->IsLeftMouseDown());
	return 1;
}

// -----------------------------------------------------------------------------

static int IsRightMouseDown(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushbooleancpp(L, pIE->IsRightMouseDown());
	return 1;
}

// -----------------------------------------------------------------------------

static int WasLeftMouseDown(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushbooleancpp(L, pIE->WasLeftMouseDown());
	return 1;
}

// -----------------------------------------------------------------------------

static int WasRightMouseDown(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushbooleancpp(L, pIE->WasRightMouseDown());
	return 1;
}

// -----------------------------------------------------------------------------

static int IsLeftDoubleClick(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushbooleancpp(L, pIE->IsLeftDoubleClick());
	return 1;
}

// -----------------------------------------------------------------------------

static int GetMouseX(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushnumber(L, pIE->GetMouseX());
	return 1;
}

// -----------------------------------------------------------------------------

static int GetMouseY(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushnumber(L, pIE->GetMouseY());
	return 1;
}

// -----------------------------------------------------------------------------

static int IsKeyDown(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushbooleancpp(L, pIE->IsKeyDown((unsigned int) luaL_checknumber(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static int WasKeyDown(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	lua_pushbooleancpp(L, pIE->WasKeyDown((unsigned int) luaL_checknumber(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static int SetMouseX(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	pIE->SetMouseX((int) luaL_checknumber(L, 1));
	return 0;
}

// -----------------------------------------------------------------------------

static int SetMouseY(lua_State * L)
{
	BS_InputEngine * pIE = GetIE();

	pIE->SetMouseY((int) luaL_checknumber(L, 1));
	return 0;
}

// -----------------------------------------------------------------------------

static void TheCharacterCallback(unsigned char Character)
{
	CharacterCallbackPtr->Character = Character;
	lua_State * L = static_cast<lua_State *>(BS_Kernel::GetInstance()->GetScript()->GetScriptObject());
	CharacterCallbackPtr->InvokeCallbackFunctions(L, 1);
}

// -----------------------------------------------------------------------------

static int RegisterCharacterCallback(lua_State * L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);
	CharacterCallbackPtr->RegisterCallbackFunction(L, 1);

	return 0;
}

// -----------------------------------------------------------------------------

static int UnregisterCharacterCallback(lua_State * L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);
	CharacterCallbackPtr->UnregisterCallbackFunction(L, 1);

	return 0;
}

// -----------------------------------------------------------------------------

static void TheCommandCallback(BS_InputEngine::KEY_COMMANDS Command)
{
	CommandCallbackPtr->Command = Command;
	lua_State * L = static_cast<lua_State *>(BS_Kernel::GetInstance()->GetScript()->GetScriptObject());
	CommandCallbackPtr->InvokeCallbackFunctions(L, 1);
}

// -----------------------------------------------------------------------------

static int RegisterCommandCallback(lua_State * L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);
	CommandCallbackPtr->RegisterCallbackFunction(L, 1);

	return 0;
}

// -----------------------------------------------------------------------------

static int UnregisterCommandCallback(lua_State * L)
{
	luaL_checktype(L, 1, LUA_TFUNCTION);
	CommandCallbackPtr->UnregisterCallbackFunction(L, 1);

	return 0;
}

// -----------------------------------------------------------------------------

static const char * PACKAGE_LIBRARY_NAME = "Input";

static const luaL_reg PACKAGE_FUNCTIONS[] =
{
	"Init", Init,
	"Update", Update,
	"IsLeftMouseDown", IsLeftMouseDown,
	"IsRightMouseDown", IsRightMouseDown,
	"WasLeftMouseDown", WasLeftMouseDown,
	"WasRightMouseDown", WasRightMouseDown,
	"IsLeftDoubleClick", IsLeftDoubleClick,
	"GetMouseX", GetMouseX,
	"GetMouseY", GetMouseY,
	"SetMouseX", SetMouseX,
	"SetMouseY", SetMouseY,
	"IsKeyDown", IsKeyDown,
	"WasKeyDown", WasKeyDown,
	"RegisterCharacterCallback", RegisterCharacterCallback,
	"UnregisterCharacterCallback", UnregisterCharacterCallback,
	"RegisterCommandCallback", RegisterCommandCallback,
	"UnregisterCommandCallback", UnregisterCommandCallback,
	0, 0,	
};

#define X(k) "KEY_" #k, BS_InputEngine::KEY_##k
#define Y(k) "KEY_COMMAND_" #k, BS_InputEngine::KEY_COMMAND_##k
static const lua_constant_reg PACKAGE_CONSTANTS[] =
{
	X(BACKSPACE), X(TAB), X(CLEAR), X(RETURN), X(PAUSE), X(CAPSLOCK), X(ESCAPE), X(SPACE), X(PAGEUP), X(PAGEDOWN), X(END), X(HOME), X(LEFT),
	X(UP), X(RIGHT), X(DOWN), X(PRINTSCREEN), X(INSERT), X(DELETE), X(0), X(1), X(2), X(3), X(4), X(5), X(6), X(7), X(8), X(9), X(A), X(B),
	X(C), X(D), X(E), X(F), X(G), X(H), X(I), X(J), X(K), X(L), X(M), X(N), X(O), X(P), X(Q), X(R), X(S), X(T), X(U), X(V), X(W), X(X), X(Y),
	X(Z), X(NUMPAD0), X(NUMPAD1), X(NUMPAD2), X(NUMPAD3), X(NUMPAD4), X(NUMPAD5), X(NUMPAD6), X(NUMPAD7), X(NUMPAD8), X(NUMPAD9), X(MULTIPLY),
	X(ADD), X(SEPARATOR), X(SUBTRACT), X(DECIMAL), X(DIVIDE), X(F1), X(F2), X(F3), X(F4), X(F5), X(F6),  X(F7), X(F8), X(F9), X(F10), X(F11),
	X(F12), X(NUMLOCK), X(SCROLL), X(LSHIFT), X(RSHIFT), X(LCONTROL), X(RCONTROL),
	Y(ENTER), Y(LEFT), Y(RIGHT), Y(HOME), Y(END), Y(BACKSPACE), Y(TAB), Y(INSERT), Y(DELETE),
	0, 0,
};
#undef X
#undef Y

// -----------------------------------------------------------------------------

bool BS_InputEngine::_RegisterScriptBindings()
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ScriptEngine * pScript = static_cast<BS_ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pScript);
	lua_State * L = static_cast<lua_State *>(pScript->GetScriptObject());
	BS_ASSERT(L);

	if (!BS_LuaBindhelper::AddFunctionsToLib(L, PACKAGE_LIBRARY_NAME, PACKAGE_FUNCTIONS)) return false;
	if (!BS_LuaBindhelper::AddConstantsToLib(L, PACKAGE_LIBRARY_NAME, PACKAGE_CONSTANTS)) return false;

	CharacterCallbackPtr.reset(new CharacterCallbackClass(L));
	CommandCallbackPtr.reset(new CommandCallbackClass(L));

	return true;
}
