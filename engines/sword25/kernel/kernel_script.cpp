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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/filesystemutil.h"
#include "sword25/kernel/window.h"
#include "sword25/kernel/resmanager.h"
#include "sword25/kernel/persistenceservice.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

// -----------------------------------------------------------------------------

static int DisconnectService(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushboolean(L, pKernel->DisconnectService(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetActiveServiceIdentifier(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushstring(L, pKernel->GetActiveServiceIdentifier(luaL_checkstring(L,1)).c_str());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetSuperclassCount(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushnumber(L, pKernel->GetSuperclassCount());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetSuperclassIdentifier(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushstring(L, pKernel->GetSuperclassIdentifier(static_cast<unsigned int>(luaL_checknumber(L,1))).c_str());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetServiceCount(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushnumber(L, pKernel->GetServiceCount(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetServiceIdentifier(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushstring(L, pKernel->GetServiceIdentifier(luaL_checkstring(L, 1), static_cast<unsigned int>(luaL_checknumber(L, 2))).c_str());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetMilliTicks(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushnumber(L, pKernel->GetMilliTicks());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetTimer(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushnumber(L, static_cast<lua_Number>(pKernel->GetMicroTicks()) / 1000000.0);

	return 1;
}

// -----------------------------------------------------------------------------

static int StartService(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);

	lua_pushbooleancpp(L, pKernel->NewService(luaL_checkstring(L, 1), luaL_checkstring(L, 2)) != NULL);

	return 1;
}

// -----------------------------------------------------------------------------

static int Sleep(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	pKernel->Sleep(static_cast<unsigned int>(luaL_checknumber(L, 1) * 1000));
	return 0;
}

// -----------------------------------------------------------------------------

static int Crash(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	pKernel->Crash();
	return 0;
}

// -----------------------------------------------------------------------------

static int ExecuteFile(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ScriptEngine * pSE = static_cast<BS_ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pSE);

	lua_pushbooleancpp(L, pSE->ExecuteFile(luaL_checkstring(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int GetUserdataDirectory(lua_State * L)
{
	lua_pushstring(L, BS_FileSystemUtil::GetInstance().GetUserdataDirectory().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int GetPathSeparator(lua_State * L)
{
	lua_pushstring(L, BS_FileSystemUtil::GetInstance().GetPathSeparator().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int FileExists(lua_State * L)
{
	lua_pushbooleancpp(L, BS_FileSystemUtil::GetInstance().FileExists(luaL_checkstring(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static int CreateDirectory(lua_State * L)
{
	lua_pushbooleancpp(L, BS_FileSystemUtil::GetInstance().CreateDirectory(luaL_checkstring(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static int GetWinCode(lua_State * L)
{
	lua_pushstring(L, "ScummVM");
	return 1;
}

// -----------------------------------------------------------------------------

static int GetSubversionRevision(lua_State * L)
{
	// ScummVM is 1337
	lua_pushnumber(L, 1337);
	return 1;
}

// -----------------------------------------------------------------------------

static int GetUsedMemory(lua_State * L)
{
	lua_pushnumber(L, BS_Kernel::GetInstance()->GetUsedMemory());
	return 1;
}

// -----------------------------------------------------------------------------

static const char * KERNEL_LIBRARY_NAME = "Kernel";

static const luaL_reg KERNEL_FUNCTIONS[] =
{
	"DisconnectService", DisconnectService,
	"GetActiveServiceIdentifier", GetActiveServiceIdentifier,
	"GetSuperclassCount", GetSuperclassCount,
	"GetSuperclassIdentifier", GetSuperclassIdentifier,
	"GetServiceCount", GetServiceCount,
	"GetServiceIdentifier", GetServiceIdentifier,
	"GetMilliTicks", GetMilliTicks,
	"GetTimer", GetTimer,
	"StartService", StartService,
	"Sleep", Sleep,
	"Crash", Crash,
	"ExecuteFile", ExecuteFile,
	"GetUserdataDirectory", GetUserdataDirectory,
	"GetPathSeparator", GetPathSeparator,
	"FileExists", FileExists,
	"CreateDirectory", CreateDirectory,
	"GetWinCode", GetWinCode,
	"GetSubversionRevision", GetSubversionRevision,
	"GetUsedMemory", GetUsedMemory,
	0, 0,
};

// -----------------------------------------------------------------------------

static int IsVisible(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushbooleancpp(L, pWindow->IsVisible());

	return 1;
}

// -----------------------------------------------------------------------------

static int SetVisible(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	pWindow->SetVisible(lua_tobooleancpp(L, 1));

	return 0;
}

// -----------------------------------------------------------------------------

static int GetX(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushnumber(L, pWindow->GetX());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetY(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushnumber(L, pWindow->GetY());

	return 1;
}

// -----------------------------------------------------------------------------

static int SetX(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	pWindow->SetX(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int SetY(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	pWindow->SetY(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int GetClientX(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushnumber(L, pWindow->GetClientX());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetClientY(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushnumber(L, pWindow->GetClientY());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetWidth(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushnumber(L, pWindow->GetWidth());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetHeight(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushnumber(L, pWindow->GetHeight());

	return 1;
}

// -----------------------------------------------------------------------------

static int SetWidth(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	pWindow->SetWidth(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int SetHeight(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	pWindow->SetHeight(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int GetTitle(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushstring(L, pWindow->GetTitle().c_str());

	return 1;
}

// -----------------------------------------------------------------------------

static int SetTitle(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	pWindow->SetTitle(luaL_checkstring(L, 1));

	return 0;
}

// -----------------------------------------------------------------------------

static int ProcessMessages(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushbooleancpp(L, pWindow->ProcessMessages());

	return 1;
}

// -----------------------------------------------------------------------------

static int CloseWanted(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushbooleancpp(L, pWindow->CloseWanted());

	return 1;
}

// -----------------------------------------------------------------------------

static int WaitForFocus(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushbooleancpp(L, pWindow->WaitForFocus());

	return 1;
}

// -----------------------------------------------------------------------------

static int HasFocus(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_Window * pWindow = pKernel->GetWindow();
	BS_ASSERT(pWindow);

	lua_pushbooleancpp(L, pWindow->HasFocus());

	return 1;
}

// -----------------------------------------------------------------------------

static const char * WINDOW_LIBRARY_NAME = "Window";

static const luaL_reg WINDOW_FUNCTIONS[] =
{
	"IsVisible", IsVisible,
	"SetVisible", SetVisible,
	"GetX", GetX,
	"SetX", SetX,
	"GetY", GetY,
	"SetY", SetY,
	"GetClientX", GetClientX,
	"GetClientY", GetClientY,
	"GetWidth", GetWidth,
	"GetHeight", GetHeight,
	"SetWidth", SetWidth,
	"SetHeight", SetHeight,
	"GetTitle", GetTitle,
	"SetTitle", SetTitle,
	"ProcessMessages", ProcessMessages,
	"CloseWanted", CloseWanted,
	"WaitForFocus", WaitForFocus,
	"HasFocus", HasFocus,
	0, 0,
};

// -----------------------------------------------------------------------------

static int PrecacheResource(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ResourceManager * pResource = pKernel->GetResourceManager();
	BS_ASSERT(pResource);

	lua_pushbooleancpp(L, pResource->PrecacheResource(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int ForcePrecacheResource(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ResourceManager * pResource = pKernel->GetResourceManager();
	BS_ASSERT(pResource);

	lua_pushbooleancpp(L, pResource->PrecacheResource(luaL_checkstring(L, 1), true));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetMaxMemoryUsage(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ResourceManager * pResource = pKernel->GetResourceManager();
	BS_ASSERT(pResource);

	lua_pushnumber(L, pResource->GetMaxMemoryUsage());

	return 1;
}

// -----------------------------------------------------------------------------

static int SetMaxMemoryUsage(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ResourceManager * pResource = pKernel->GetResourceManager();
	BS_ASSERT(pResource);

	pResource->SetMaxMemoryUsage(static_cast<unsigned int>(lua_tonumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int EmptyCache(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ResourceManager * pResource = pKernel->GetResourceManager();
	BS_ASSERT(pResource);

	pResource->EmptyCache();

	return 0;
}

// -----------------------------------------------------------------------------

static int IsLogCacheMiss(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ResourceManager * pResource = pKernel->GetResourceManager();
	BS_ASSERT(pResource);

	lua_pushbooleancpp(L, pResource->IsLogCacheMiss());

	return 1;
}

// -----------------------------------------------------------------------------

static int SetLogCacheMiss(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ResourceManager * pResource = pKernel->GetResourceManager();
	BS_ASSERT(pResource);

	pResource->SetLogCacheMiss(lua_tobooleancpp(L, 1));

	return 0;
}

// -----------------------------------------------------------------------------

static int DumpLockedResources(lua_State * L)
{
	BS_Kernel * pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	BS_ResourceManager * pResource = pKernel->GetResourceManager();
	BS_ASSERT(pResource);

	pResource->DumpLockedResources();

	return 0;
}

// -----------------------------------------------------------------------------

static const char * RESOURCE_LIBRARY_NAME = "Resource";

static const luaL_reg RESOURCE_FUNCTIONS[] =
{
	"PrecacheResource", PrecacheResource,
	"ForcePrecacheResource", ForcePrecacheResource,
	"GetMaxMemoryUsage", GetMaxMemoryUsage,
	"SetMaxMemoryUsage", SetMaxMemoryUsage,
	"EmptyCache", EmptyCache,
	"IsLogCacheMiss", IsLogCacheMiss,
	"SetLogCacheMiss", SetLogCacheMiss,
	"DumpLockedResources", DumpLockedResources,
	0, 0,
};

// -----------------------------------------------------------------------------

static int ReloadSlots(lua_State * L)
{
	BS_PersistenceService::GetInstance().ReloadSlots();
	lua_pushnil(L);
	return 1;
}

// -----------------------------------------------------------------------------

static int GetSlotCount(lua_State * L)
{
	lua_pushnumber(L, BS_PersistenceService::GetInstance().GetSlotCount());
	return 1;
}

// -----------------------------------------------------------------------------

static int IsSlotOccupied(lua_State * L)
{
	lua_pushbooleancpp(L, BS_PersistenceService::GetInstance().IsSlotOccupied(static_cast<unsigned int>(luaL_checknumber(L, 1)) - 1));
	return 1;
}

// -----------------------------------------------------------------------------

static int GetSavegameDirectory(lua_State * L)
{
	lua_pushstring(L, BS_PersistenceService::GetInstance().GetSavegameDirectory().c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int IsSavegameCompatible(lua_State * L)
{
	lua_pushbooleancpp(L, BS_PersistenceService::GetInstance().IsSavegameCompatible(static_cast<unsigned int>(luaL_checknumber(L, 1)) - 1));
	return 1;
}

// -----------------------------------------------------------------------------

static int GetSavegameDescription(lua_State * L)
{
	lua_pushstring(L, BS_PersistenceService::GetInstance().GetSavegameDescription(static_cast<unsigned int>(luaL_checknumber(L, 1)) - 1).c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int GetSavegameFilename(lua_State * L)
{
	lua_pushstring(L, BS_PersistenceService::GetInstance().GetSavegameFilename(static_cast<unsigned int>(luaL_checknumber(L, 1)) - 1).c_str());
	return 1;
}

// -----------------------------------------------------------------------------

static int LoadGame(lua_State * L)
{
	lua_pushbooleancpp(L, BS_PersistenceService::GetInstance().LoadGame(static_cast<unsigned int>(luaL_checknumber(L, 1)) - 1));
	return 1;
}

// -----------------------------------------------------------------------------

static int SaveGame(lua_State * L)
{
	lua_pushbooleancpp(L, BS_PersistenceService::GetInstance().SaveGame(static_cast<unsigned int>(luaL_checknumber(L, 1)) - 1, luaL_checkstring(L, 2)));
	return 1;
}

// -----------------------------------------------------------------------------

static const char * PERSISTENCE_LIBRARY_NAME = "Persistence";

static const luaL_reg PERSISTENCE_FUNCTIONS[] =
{
	"ReloadSlots", ReloadSlots,
	"GetSlotCount", GetSlotCount,
	"IsSlotOccupied", IsSlotOccupied,
	"GetSavegameDirectory", GetSavegameDirectory,
	"IsSavegameCompatible", IsSavegameCompatible,
	"GetSavegameDescription", GetSavegameDescription,
	"GetSavegameFilename", GetSavegameFilename,
	"LoadGame", LoadGame,
	"SaveGame", SaveGame,
	0, 0,
};

// -----------------------------------------------------------------------------

bool BS_Kernel::_RegisterScriptBindings()
{
	BS_ScriptEngine * pScript = static_cast<BS_ScriptEngine *>(GetService("script"));
	BS_ASSERT(pScript);
	lua_State * L = static_cast<lua_State *>(pScript->GetScriptObject());
	BS_ASSERT(L);

	if (!BS_LuaBindhelper::AddFunctionsToLib(L, KERNEL_LIBRARY_NAME, KERNEL_FUNCTIONS)) return false;
	if (!BS_LuaBindhelper::AddFunctionsToLib(L, WINDOW_LIBRARY_NAME, WINDOW_FUNCTIONS)) return false;
	if (!BS_LuaBindhelper::AddFunctionsToLib(L, RESOURCE_LIBRARY_NAME, RESOURCE_FUNCTIONS)) return false;
	if (!BS_LuaBindhelper::AddFunctionsToLib(L, PERSISTENCE_LIBRARY_NAME, PERSISTENCE_FUNCTIONS)) return false;

	return true;
}
