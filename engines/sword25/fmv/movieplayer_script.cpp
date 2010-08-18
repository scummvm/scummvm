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

#include "sword25/fmv/movieplayer.h"

namespace Sword25 {

int LoadMovie(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->LoadMovie(luaL_checkstring(L, 1), lua_gettop(L) == 2 ? static_cast<unsigned int>(luaL_checknumber(L, 2)) : 10));

	return 1;
}

// -------------------------------------------------------------------------

int UnloadMovie(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->UnloadMovie());

	return 1;
}

// -------------------------------------------------------------------------

int Play(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->Play());

	return 1;
}

// -------------------------------------------------------------------------

int Pause(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->Pause());

	return 1;
}

// -------------------------------------------------------------------------

int Update(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	FMVPtr->Update();

	return 0;
}

// -------------------------------------------------------------------------

int IsMovieLoaded(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->IsMovieLoaded());

	return 1;
}

// -------------------------------------------------------------------------

int IsPaused(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	lua_pushbooleancpp(L, FMVPtr->IsPaused());

	return 1;
}

// -------------------------------------------------------------------------

int GetScaleFactor(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	lua_pushnumber(L, FMVPtr->GetScaleFactor());

	return 1;
}

// -------------------------------------------------------------------------

int SetScaleFactor(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	FMVPtr->SetScaleFactor(static_cast<float>(luaL_checknumber(L, 1)));

	return 0;
}

// -------------------------------------------------------------------------

int GetTime(lua_State *L) {
	MoviePlayer *FMVPtr = BS_Kernel::GetInstance()->GetFMV();
	BS_ASSERT(FMVPtr);

	lua_pushnumber(L, FMVPtr->GetTime());

	return 1;
}

// -------------------------------------------------------------------------

const char *LIBRARY_NAME = "Movieplayer";

const luaL_reg LIBRARY_FUNCTIONS[] = {
	{ "LoadMovie", LoadMovie },
	{ "UnloadMovie", UnloadMovie },
	{ "Play", Play },
	{ "Pause", Pause },
	{ "Update", Update },
	{ "IsMovieLoaded", IsMovieLoaded },
	{ "IsPaused", IsPaused },
	{ "GetScaleFactor", GetScaleFactor },
	{ "SetScaleFactor", SetScaleFactor },
	{ "GetTime", GetTime },
	{ 0, 0 }
};

bool MoviePlayer::_RegisterScriptBindings() {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	ScriptEngine *pScript = static_cast<ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->GetScriptObject());
	BS_ASSERT(L);

	if (!LuaBindhelper::AddFunctionsToLib(L, LIBRARY_NAME, LIBRARY_FUNCTIONS)) return false;

	return true;
}

} // End of namespace Sword25
