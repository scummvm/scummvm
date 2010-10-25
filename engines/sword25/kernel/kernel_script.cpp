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

#include "sword25/kernel/common.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/filesystemutil.h"
#include "sword25/kernel/resmanager.h"
#include "sword25/kernel/persistenceservice.h"
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

namespace Sword25 {

static int disconnectService(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushboolean(L, pKernel->disconnectService(luaL_checkstring(L, 1)));

	return 1;
}

static int getActiveServiceIdentifier(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushstring(L, pKernel->getActiveServiceIdentifier(luaL_checkstring(L, 1)).c_str());

	return 1;
}

static int getSuperclassCount(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushnumber(L, pKernel->getSuperclassCount());

	return 1;
}

static int getSuperclassIdentifier(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushstring(L, pKernel->getSuperclassIdentifier(
	                   static_cast<uint>(luaL_checknumber(L, 1))).c_str());

	return 1;
}

static int getServiceCount(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushnumber(L, pKernel->getServiceCount(luaL_checkstring(L, 1)));

	return 1;
}

static int getServiceIdentifier(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushstring(L, pKernel->getServiceIdentifier(luaL_checkstring(L, 1),
	               static_cast<uint>(luaL_checknumber(L, 2))).c_str());

	return 1;
}

static int getMilliTicks(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushnumber(L, pKernel->getMilliTicks());

	return 1;
}

static int getTimer(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushnumber(L, static_cast<lua_Number>(pKernel->getMilliTicks()) / 1000.0);

	return 1;
}

static int startService(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);

	lua_pushbooleancpp(L, pKernel->newService(luaL_checkstring(L, 1), luaL_checkstring(L, 2)) != NULL);

	return 1;
}

static int sleep(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	pKernel->sleep(static_cast<uint>(luaL_checknumber(L, 1) * 1000));
	return 0;
}

static int crash(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	pKernel->crash();
	return 0;
}

static int executeFile(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ScriptEngine *pSE = pKernel->getScript();
	BS_ASSERT(pSE);

	lua_pushbooleancpp(L, pSE->executeFile(luaL_checkstring(L, 1)));

	return 0;
}

static int getUserdataDirectory(lua_State *L) {
	lua_pushstring(L, FileSystemUtil::getInstance().getUserdataDirectory().c_str());
	return 1;
}

static int getPathSeparator(lua_State *L) {
	lua_pushstring(L, FileSystemUtil::getInstance().getPathSeparator().c_str());
	return 1;
}

static int fileExists(lua_State *L) {
	lua_pushbooleancpp(L, FileSystemUtil::getInstance().fileExists(luaL_checkstring(L, 1)));
	return 1;
}

static int createDirectory(lua_State *L) {
	lua_pushbooleancpp(L, FileSystemUtil::getInstance().createDirectory(luaL_checkstring(L, 1)));
	return 1;
}

static int getWinCode(lua_State *L) {
	lua_pushstring(L, "ScummVM");
	return 1;
}

static int getSubversionRevision(lua_State *L) {
	// ScummVM is 1337
	lua_pushnumber(L, 1337);
	return 1;
}

static int getUsedMemory(lua_State *L) {
	lua_pushnumber(L, Kernel::getInstance()->getUsedMemory());
	return 1;
}

static const char *KERNEL_LIBRARY_NAME = "Kernel";

static const luaL_reg KERNEL_FUNCTIONS[] = {
	{"DisconnectService", disconnectService},
	{"GetActiveServiceIdentifier", getActiveServiceIdentifier},
	{"GetSuperclassCount", getSuperclassCount},
	{"GetSuperclassIdentifier", getSuperclassIdentifier},
	{"GetServiceCount", getServiceCount},
	{"GetServiceIdentifier", getServiceIdentifier},
	{"GetMilliTicks", getMilliTicks},
	{"GetTimer", getTimer},
	{"StartService", startService},
	{"Sleep", sleep},
	{"Crash", crash},
	{"ExecuteFile", executeFile},
	{"GetUserdataDirectory", getUserdataDirectory},
	{"GetPathSeparator", getPathSeparator},
	{"FileExists", fileExists},
	{"CreateDirectory", createDirectory},
	{"GetWinCode", getWinCode},
	{"GetSubversionRevision", getSubversionRevision},
	{"GetUsedMemory", getUsedMemory},
	{0, 0}
};

static int isVisible(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushbooleancpp(L, true);

	return 1;
}

static int setVisible(lua_State *L) {
	// This function apparently is not used by the game scripts
//	pWindow->setVisible(lua_tobooleancpp(L, 1));

	return 0;
}

static int getX(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushnumber(L, 0);

	return 1;
}

static int getY(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushnumber(L, 0);

	return 1;
}

static int setX(lua_State *L) {
	// This is called by system/boot.lua with -1 as value.
//	pWindow->setX(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

static int setY(lua_State *L) {
	// This is called by system/boot.lua with -1 as value.
//	pWindow->setY(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

static int getClientX(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushnumber(L, 0);

	return 1;
}

static int getClientY(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushnumber(L, 0);

	return 1;
}

static int getWidth(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushnumber(L, 800);

	return 1;
}

static int getHeight(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushnumber(L, 600);

	return 1;
}

static int setWidth(lua_State *L) {
	// This is called by system/boot.lua with 800 as value.
//	pWindow->setWidth(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

static int setHeight(lua_State *L) {
	// This is called by system/boot.lua with 600 as value.
//	pWindow->setHeight(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

static int getTitle(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushstring(L, "");

	return 1;
}

static int setTitle(lua_State *L) {
	// This is called by system/boot.lua and system/menu.lua, to
	// set the window title to the (localized) game name.
	// FIXME: Should we call OSystem::setWindowCaption() here?
//	pWindow->setTitle(luaL_checkstring(L, 1));

	return 0;
}

static int processMessages(lua_State *L) {
	// This is called by the main loop in system/boot.lua,
	// and the game keeps running as true is returned here.
	// It terminates if we return false.

	// TODO: We could do more stuff here if desired...

	// TODO: We could always return true here, and leave quit handling
	// to the closeWanted() opcode; see also the TODO comment in there.

	lua_pushbooleancpp(L, !Engine::shouldQuit());

	return 1;
}

static int closeWanted(lua_State *L) {
	// This is called by system/interface.lua to determine whether the
	// user requested the game to close (e.g. by clicking the 'close' widget
	// of the game window). As a consequence (i.e. this function returns true),
	// a quit confirmation dialog is shown.

	// TODO: ScummVM currently has a bug / misfeature where some engines provide
	// quit confirmation dialogs, some don't; in addition, we have a global confirmation
	// dialog (but the user has to explicitly activate that in the config).
	// Anyway, this can lead to *two* confirmation dialogs being shown.
	// If it wasn't for that, we could simply check for Engine::shouldQuit() here,
	// and then invoke EventMan::resetQuit. But currently this would result in
	// the user seeing two confirmation dialogs. Bad.
	lua_pushbooleancpp(L, false);

	return 1;
}

static int waitForFocus(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushbooleancpp(L, true);

	return 1;
}

static int hasFocus(lua_State *L) {
	// This function apparently is not used by the game scripts
	lua_pushbooleancpp(L, true);

	return 1;
}

static const char *WINDOW_LIBRARY_NAME = "Window";

static const luaL_reg WINDOW_FUNCTIONS[] = {
	{"IsVisible", isVisible},
	{"SetVisible", setVisible},
	{"GetX", getX},
	{"SetX", setX},
	{"GetY", getY},
	{"SetY", setY},
	{"GetClientX", getClientX},
	{"GetClientY", getClientY},
	{"GetWidth", getWidth},
	{"GetHeight", getHeight},
	{"SetWidth", setWidth},
	{"SetHeight", setHeight},
	{"GetTitle", getTitle},
	{"SetTitle", setTitle},
	{"ProcessMessages", processMessages},
	{"CloseWanted", closeWanted},
	{"WaitForFocus", waitForFocus},
	{"HasFocus", hasFocus},
	{0, 0}
};

static int precacheResource(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ResourceManager *pResource = pKernel->getResourceManager();
	BS_ASSERT(pResource);

	lua_pushbooleancpp(L, pResource->precacheResource(luaL_checkstring(L, 1)));

	return 1;
}

static int forcePrecacheResource(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ResourceManager *pResource = pKernel->getResourceManager();
	BS_ASSERT(pResource);

	lua_pushbooleancpp(L, pResource->precacheResource(luaL_checkstring(L, 1), true));

	return 1;
}

static int getMaxMemoryUsage(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ResourceManager *pResource = pKernel->getResourceManager();
	BS_ASSERT(pResource);

	lua_pushnumber(L, pResource->getMaxMemoryUsage());

	return 1;
}

static int setMaxMemoryUsage(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ResourceManager *pResource = pKernel->getResourceManager();
	BS_ASSERT(pResource);

	pResource->setMaxMemoryUsage(static_cast<uint>(lua_tonumber(L, 1)));

	return 0;
}

static int emptyCache(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ResourceManager *pResource = pKernel->getResourceManager();
	BS_ASSERT(pResource);

	pResource->emptyCache();

	return 0;
}

static int isLogCacheMiss(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ResourceManager *pResource = pKernel->getResourceManager();
	BS_ASSERT(pResource);

	lua_pushbooleancpp(L, pResource->isLogCacheMiss());

	return 1;
}

static int setLogCacheMiss(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ResourceManager *pResource = pKernel->getResourceManager();
	BS_ASSERT(pResource);

	pResource->setLogCacheMiss(lua_tobooleancpp(L, 1));

	return 0;
}

static int dumpLockedResources(lua_State *L) {
	Kernel *pKernel = Kernel::getInstance();
	BS_ASSERT(pKernel);
	ResourceManager *pResource = pKernel->getResourceManager();
	BS_ASSERT(pResource);

	pResource->dumpLockedResources();

	return 0;
}

static const char *RESOURCE_LIBRARY_NAME = "Resource";

static const luaL_reg RESOURCE_FUNCTIONS[] = {
	{"PrecacheResource", precacheResource},
	{"ForcePrecacheResource", forcePrecacheResource},
	{"GetMaxMemoryUsage", getMaxMemoryUsage},
	{"SetMaxMemoryUsage", setMaxMemoryUsage},
	{"EmptyCache", emptyCache},
	{"IsLogCacheMiss", isLogCacheMiss},
	{"SetLogCacheMiss", setLogCacheMiss},
	{"DumpLockedResources", dumpLockedResources},
	{0, 0}
};

static int reloadSlots(lua_State *L) {
	PersistenceService::getInstance().reloadSlots();
	lua_pushnil(L);
	return 1;
}

static int getSlotCount(lua_State *L) {
	lua_pushnumber(L, PersistenceService::getInstance().getSlotCount());
	return 1;
}

static int isSlotOccupied(lua_State *L) {
	lua_pushbooleancpp(L, PersistenceService::getInstance().isSlotOccupied(static_cast<uint>(luaL_checknumber(L, 1)) - 1));
	return 1;
}

static int getSavegameDirectory(lua_State *L) {
	lua_pushstring(L, PersistenceService::getInstance().getSavegameDirectory().c_str());
	return 1;
}

static int isSavegameCompatible(lua_State *L) {
	lua_pushbooleancpp(L, PersistenceService::getInstance().isSavegameCompatible(
	                       static_cast<uint>(luaL_checknumber(L, 1)) - 1));
	return 1;
}

static int getSavegameDescription(lua_State *L) {
	lua_pushstring(L, PersistenceService::getInstance().getSavegameDescription(
	                   static_cast<uint>(luaL_checknumber(L, 1)) - 1).c_str());
	return 1;
}

static int getSavegameFilename(lua_State *L) {
	lua_pushstring(L, PersistenceService::getInstance().getSavegameFilename(static_cast<uint>(luaL_checknumber(L, 1)) - 1).c_str());
	return 1;
}

static int loadGame(lua_State *L) {
	lua_pushbooleancpp(L, PersistenceService::getInstance().loadGame(static_cast<uint>(luaL_checknumber(L, 1)) - 1));
	return 1;
}

static int saveGame(lua_State *L) {
	lua_pushbooleancpp(L, PersistenceService::getInstance().saveGame(static_cast<uint>(luaL_checknumber(L, 1)) - 1, luaL_checkstring(L, 2)));
	return 1;
}

static const char *PERSISTENCE_LIBRARY_NAME = "Persistence";

static const luaL_reg PERSISTENCE_FUNCTIONS[] = {
	{"ReloadSlots", reloadSlots},
	{"GetSlotCount", getSlotCount},
	{"IsSlotOccupied", isSlotOccupied},
	{"GetSavegameDirectory", getSavegameDirectory},
	{"IsSavegameCompatible", isSavegameCompatible},
	{"GetSavegameDescription", getSavegameDescription},
	{"GetSavegameFilename", getSavegameFilename},
	{"LoadGame", loadGame},
	{"SaveGame", saveGame},
	{0, 0}
};

bool Kernel::registerScriptBindings() {
	ScriptEngine *pScript = getScript();
	BS_ASSERT(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->getScriptObject());
	BS_ASSERT(L);

	if (!LuaBindhelper::addFunctionsToLib(L, KERNEL_LIBRARY_NAME, KERNEL_FUNCTIONS)) return false;
	if (!LuaBindhelper::addFunctionsToLib(L, WINDOW_LIBRARY_NAME, WINDOW_FUNCTIONS)) return false;
	if (!LuaBindhelper::addFunctionsToLib(L, RESOURCE_LIBRARY_NAME, RESOURCE_FUNCTIONS)) return false;
	if (!LuaBindhelper::addFunctionsToLib(L, PERSISTENCE_LIBRARY_NAME, PERSISTENCE_FUNCTIONS)) return false;

	return true;
}

} // End of namespace Sword25
