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
#include "sword25/script/script.h"
#include "sword25/script/luabindhelper.h"

#include "sword25/package/packagemanager.h"

namespace Sword25 {

using namespace Lua;

// -----------------------------------------------------------------------------

static PackageManager *GetPM() {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	PackageManager *pPM = static_cast<PackageManager *>(pKernel->GetService("package"));
	BS_ASSERT(pPM);
	return pPM;
}

// -----------------------------------------------------------------------------

static int LoadPackage(lua_State *L) {
	PackageManager *pPM = GetPM();

	lua_pushbooleancpp(L, pPM->LoadPackage(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));

	return 1;
}

// -----------------------------------------------------------------------------

static int LoadDirectoryAsPackage(lua_State *L) {
	PackageManager *pPM = GetPM();

	lua_pushbooleancpp(L, pPM->LoadDirectoryAsPackage(luaL_checkstring(L, 1), luaL_checkstring(L, 2)));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetCurrentDirectory(lua_State *L) {
	PackageManager *pPM = GetPM();

	lua_pushstring(L, pPM->GetCurrentDirectory().c_str());

	return 1;
}

// -----------------------------------------------------------------------------

static int ChangeDirectory(lua_State *L) {
	PackageManager *pPM = GetPM();

	lua_pushbooleancpp(L, pPM->ChangeDirectory(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetAbsolutePath(lua_State *L) {
	PackageManager *pPM = GetPM();

	lua_pushstring(L, pPM->GetAbsolutePath(luaL_checkstring(L, 1)).c_str());

	return 1;
}

// -----------------------------------------------------------------------------

static int GetFileSize(lua_State *L) {
	PackageManager *pPM = GetPM();

	lua_pushnumber(L, pPM->GetFileSize(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetFileType(lua_State *L) {
	PackageManager *pPM = GetPM();

	lua_pushnumber(L, pPM->GetFileType(luaL_checkstring(L, 1)));

	return 1;
}

// -----------------------------------------------------------------------------

static void SplitSearchPath(const Common::String &Path, Common::String &Directory, Common::String &Filter) {
	// Scan backwards for a trailing slash
	const char *sPath = Path.c_str();
	const char *lastSlash = sPath + strlen(sPath) - 1;
	while ((lastSlash >= sPath) && (*lastSlash != '/')) --lastSlash;

	if (lastSlash >= sPath) {
		Directory = "";
		Filter = Path;
	} else {
		Directory = Common::String(sPath, lastSlash - sPath);
		Filter = Common::String(lastSlash + 1);
	}
}

// -----------------------------------------------------------------------------

static void DoSearch(lua_State *L, const Common::String &path, unsigned int type) {
	PackageManager *pPM = GetPM();

	// Der Packagemanager-Service muss den Suchstring und den Pfad getrennt übergeben bekommen.
	// Um die Benutzbarkeit zu verbessern sollen Skriptprogrammierer dieses als ein Pfad übergeben können.
	// Daher muss der übergebene Pfad am letzten Slash aufgesplittet werden.
	Common::String directory;
	Common::String filter;
	SplitSearchPath(path, directory, filter);

	// Ergebnistable auf dem Lua-Stack erstellen
	lua_newtable(L);

	// Suche durchführen und die Namen aller gefundenen Dateien in die Ergebnistabelle einfügen.
	// Als Indizes werden fortlaufende Nummern verwandt.
	uint resultNr = 1;
	Common::ArchiveMemberList list;
	int numMatches;

	numMatches = pPM->doSearch(list, filter, directory, type);
	if (numMatches) {
		for (Common::ArchiveMemberList::iterator it = list.begin(); it != list.end(); ++it) {
			lua_pushnumber(L, resultNr);
			lua_pushstring(L, (*it)->getName().c_str());
			lua_settable(L, -3);
			resultNr++;
		}
	}
}

// -----------------------------------------------------------------------------

static int FindFiles(lua_State *L) {
	DoSearch(L, luaL_checkstring(L, 1), PackageManager::FT_FILE);
	return 1;
}

// -----------------------------------------------------------------------------

static int FindDirectories(lua_State *L) {
	DoSearch(L, luaL_checkstring(L, 1), PackageManager::FT_DIRECTORY);
	return 1;
}

// -----------------------------------------------------------------------------

static int GetFileAsString(lua_State *L) {
	PackageManager *pPM = GetPM();

	unsigned int FileSize;
	char *FileData = (char *)pPM->GetFile(luaL_checkstring(L, 1), &FileSize);
	if (FileData) {
		lua_pushlstring(L, FileData, FileSize);
		delete FileData;

		return 1;
	} else
		return 0;
}

// -----------------------------------------------------------------------------

static int FileExists(lua_State *L) {
	lua_pushbooleancpp(L, GetPM()->FileExists(luaL_checkstring(L, 1)));
	return 1;
}

// -----------------------------------------------------------------------------

static const char *PACKAGE_LIBRARY_NAME = "Package";

static const luaL_reg PACKAGE_FUNCTIONS[] = {
	{"LoadPackage", LoadPackage},
	{"LoadDirectoryAsPackage", LoadDirectoryAsPackage},
	{"GetCurrentDirectory", GetCurrentDirectory},
	{"ChangeDirectory", ChangeDirectory},
	{"GetAbsolutePath", GetAbsolutePath},
	{"GetFileSize", GetFileSize},
	{"GetFileType", GetFileType},
	{"FindFiles", FindFiles},
	{"FindDirectories", FindDirectories},
	{"GetFileAsString", GetFileAsString},
	{"FileExists", FileExists},
	{0, 0}
};

// -----------------------------------------------------------------------------

bool PackageManager::_RegisterScriptBindings() {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	ScriptEngine *pScript = static_cast<ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->GetScriptObject());
	BS_ASSERT(L);

	if (!LuaBindhelper::AddFunctionsToLib(L, PACKAGE_LIBRARY_NAME, PACKAGE_FUNCTIONS)) return false;

	return true;
}

} // End of namespace Sword25
