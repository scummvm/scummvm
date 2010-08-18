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

#include "sword25/sfx/soundengine.h"

namespace Sword25 {
// -----------------------------------------------------------------------------

static int Init(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	if (lua_gettop(L) == 0)
		lua_pushbooleancpp(L, pSfx->Init(44100, 32));
	else if (lua_gettop(L) == 1)
		lua_pushbooleancpp(L, pSfx->Init(static_cast<unsigned int>(luaL_checknumber(L, 1)), 32));
	else
		lua_pushbooleancpp(L, pSfx->Init(static_cast<unsigned int>(luaL_checknumber(L, 1)), static_cast<unsigned int>(luaL_checknumber(L, 2))));

	return 1;
}

// -----------------------------------------------------------------------------

static int Update(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->Update();

	return 0;
}

// -----------------------------------------------------------------------------

static int SetVolume(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->SetVolume(static_cast<float>(luaL_checknumber(L, 1)),
	                static_cast<SoundEngine::SOUND_TYPES>(static_cast<unsigned int>(luaL_checknumber(L, 2))));

	return 0;
}

// -----------------------------------------------------------------------------

static int GetVolume(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	lua_pushnumber(L, pSfx->GetVolume(static_cast<SoundEngine::SOUND_TYPES>(static_cast<unsigned int>(luaL_checknumber(L, 1)))));

	return 1;
}

// -----------------------------------------------------------------------------

static int PauseAll(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->PauseAll();

	return 0;
}

// -----------------------------------------------------------------------------

static int ResumeAll(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->ResumeAll();

	return 0;
}

// -----------------------------------------------------------------------------

static int PauseLayer(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->PauseLayer(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int ResumeLayer(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->ResumeLayer(static_cast<int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static void ProcessPlayParams(lua_State *L, Common::String &FileName, SoundEngine::SOUND_TYPES &Type, float &Volume, float &Pan, bool &Loop, int &LoopStart, int &LoopEnd, unsigned int &Layer) {
	FileName = luaL_checkstring(L, 1);

	Type = static_cast<SoundEngine::SOUND_TYPES>(static_cast<unsigned int>(luaL_checknumber(L, 2)));

	if (lua_gettop(L) < 3 || lua_isnil(L, 3)) Volume = 1.0f;
	else Volume = static_cast<float>(luaL_checknumber(L, 3));

	if (lua_gettop(L) < 4 || lua_isnil(L, 4)) Pan = 0.0f;
	else Pan = static_cast<float>(luaL_checknumber(L, 4));

	if (lua_gettop(L) < 5 || lua_isnil(L, 5)) Loop = false;
	else Loop = lua_tobooleancpp(L, 5);

	if (lua_gettop(L) < 6 || lua_isnil(L, 6)) LoopStart = -1;
	else LoopStart = static_cast<int>(luaL_checknumber(L, 6));

	if (lua_gettop(L) < 7 || lua_isnil(L, 7)) LoopEnd = -1;
	else LoopEnd = static_cast<int>(luaL_checknumber(L, 7));

	if (lua_gettop(L) < 8 || lua_isnil(L, 8)) Layer = 0;
	else Layer = static_cast<unsigned int>(luaL_checknumber(L, 8));
}

static int PlaySound(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	Common::String FileName;
	SoundEngine::SOUND_TYPES Type;
	float Volume;
	float Pan;
	bool Loop;
	int LoopStart;
	int LoopEnd;
	unsigned int Layer;
	ProcessPlayParams(L, FileName, Type, Volume, Pan, Loop, LoopStart, LoopEnd, Layer);

	lua_pushbooleancpp(L, pSfx->PlaySound(FileName, Type, Volume, Pan, Loop, LoopStart, LoopEnd, Layer));

	return 1;
}

static int PlaySoundEx(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	Common::String FileName;
	SoundEngine::SOUND_TYPES Type;
	float Volume;
	float Pan;
	bool Loop;
	int LoopStart;
	int LoopEnd;
	unsigned int Layer;
	ProcessPlayParams(L, FileName, Type, Volume, Pan, Loop, LoopStart, LoopEnd, Layer);

	lua_pushnumber(L, pSfx->PlaySoundEx(FileName, Type, Volume, Pan, Loop, LoopStart, LoopEnd, Layer));

	return 1;
}

// -----------------------------------------------------------------------------

static int SetSoundVolume(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->SetSoundVolume(static_cast<unsigned int>(luaL_checknumber(L, 1)), static_cast<float>(luaL_checknumber(L, 2)));

	return 0;
}

// -----------------------------------------------------------------------------

static int SetSoundPanning(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->SetSoundPanning(static_cast<unsigned int>(luaL_checknumber(L, 1)), static_cast<float>(luaL_checknumber(L, 2)));

	return 0;
}

// -----------------------------------------------------------------------------

static int PauseSound(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->PauseSound(static_cast<unsigned int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int ResumeSound(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->ResumeSound(static_cast<unsigned int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int StopSound(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	pSfx->StopSound(static_cast<unsigned int>(luaL_checknumber(L, 1)));

	return 0;
}

// -----------------------------------------------------------------------------

static int IsSoundPaused(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	lua_pushbooleancpp(L, pSfx->IsSoundPaused(static_cast<unsigned int>(luaL_checknumber(L, 1))));

	return 1;
}

// -----------------------------------------------------------------------------

static int IsSoundPlaying(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	lua_pushbooleancpp(L, pSfx->IsSoundPlaying(static_cast<unsigned int>(luaL_checknumber(L, 1))));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetSoundVolume(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	lua_pushnumber(L, pSfx->GetSoundVolume(static_cast<unsigned int>(luaL_checknumber(L, 1))));

	return 1;
}

// -----------------------------------------------------------------------------

static int GetSoundPanning(lua_State *L) {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	SoundEngine *pSfx = static_cast<SoundEngine *>(BS_Kernel::GetInstance()->GetService("sfx"));
	BS_ASSERT(pSfx);

	lua_pushnumber(L, pSfx->GetSoundPanning(static_cast<unsigned int>(luaL_checknumber(L, 1))));

	return 1;
}

// -----------------------------------------------------------------------------

static const char *SFX_LIBRARY_NAME = "Sfx";

static const luaL_reg SFX_FUNCTIONS[] = {
	{"Init", Init},
	{"Update", Update},
	{"__SetVolume", SetVolume},
	{"__GetVolume", GetVolume},
	{"PauseAll", PauseAll},
	{"ResumeAll", ResumeAll},
	{"PauseLayer", PauseLayer},
	{"ResumeLayer", ResumeLayer},
	{"__PlaySound", PlaySound},
	{"__PlaySoundEx", PlaySoundEx},
	{"__SetSoundVolume", SetSoundVolume},
	{"__SetSoundPanning", SetSoundPanning},
	{"__PauseSound", PauseSound},
	{"__ResumeSound", ResumeSound},
	{"__StopSound", StopSound},
	{"__IsSoundPaused", IsSoundPaused},
	{"__IsSoundPlaying", IsSoundPlaying},
	{"__GetSoundVolume", GetSoundVolume},
	{"__GetSoundPanning", GetSoundPanning},
	{0, 0}
};

static const lua_constant_reg SFX_CONSTANTS[] = {
	{"MUSIC", SoundEngine::MUSIC},
	{"SPEECH", SoundEngine::SPEECH},
	{"SFX", SoundEngine::SFX},
	{0, 0}
};

// -----------------------------------------------------------------------------

bool SoundEngine::_RegisterScriptBindings() {
	BS_Kernel *pKernel = BS_Kernel::GetInstance();
	BS_ASSERT(pKernel);
	ScriptEngine *pScript = static_cast<ScriptEngine *>(pKernel->GetService("script"));
	BS_ASSERT(pScript);
	lua_State *L = static_cast<lua_State *>(pScript->GetScriptObject());
	BS_ASSERT(L);

	if (!LuaBindhelper::AddFunctionsToLib(L, SFX_LIBRARY_NAME, SFX_FUNCTIONS)) return false;
	if (!LuaBindhelper::AddConstantsToLib(L, SFX_LIBRARY_NAME, SFX_CONSTANTS)) return false;

	return true;
}

} // End of namespace Sword25
