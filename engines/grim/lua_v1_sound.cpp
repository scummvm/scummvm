/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/savefile.h"

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/lua_v1.h"
#include "engines/grim/actor.h"
#include "engines/grim/savegame.h"
#include "engines/grim/set.h"
#include "engines/grim/sound.h"
#include "engines/grim/imuse/imuse_engine.h"
#include "engines/grim/lua/lauxlib.h"

namespace Grim {

// Like the original, a sound is identified either by its name or by the number ImStartSound returned
static const char *getSoundKey(lua_Object nameObj) {
	if (lua_isnumber(nameObj))
		return (const char *)(intptr)(int)lua_getnumber(nameObj);

	return lua_getstring(nameObj);
}

void Lua_V1::ImStartSound() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object priorityObj = lua_getparam(2);
	lua_Object groupObj = lua_getparam(3);

	if (!lua_isstring(nameObj) && !lua_isnumber(nameObj))
		return;
	if (!lua_isnumber(priorityObj) || !lua_isnumber(groupObj))
		return;

	const char *soundName = lua_getstring(nameObj);
	int priority = (int)lua_getnumber(priorityObj);
	int group = (int)lua_getnumber(groupObj);

	lua_pushnumber(g_imuse->soundsLoadStartSound(soundName, priority, group));
}

void Lua_V1::ImStopSound() {
	lua_Object nameObj = lua_getparam(1);
	g_imuse->soundsStopSound(getSoundKey(nameObj));
}

void Lua_V1::ImStopAllSounds() {
	g_imuse->imStopAllSounds();
}

void Lua_V1::ImPause() {
	g_imuse->pause(true);
}

void Lua_V1::ImResume() {
	g_imuse->pause(false);
}

void Lua_V1::ImSetVoiceEffect() {
	const char *effectName;

	effectName = luaL_check_string(1);
	g_imuse->iMUSESetVoiceEffect(effectName);
}

void Lua_V1::ImSetMusicVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_imuse->setMusicGroupVol((int)lua_getnumber(volObj));
}

void Lua_V1::ImGetMusicVol() {
	lua_pushnumber(g_imuse->getMusicGroupVol());
}

void Lua_V1::ImSetVoiceVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_imuse->setVoiceGroupVol((int)lua_getnumber(volObj));
}

void Lua_V1::ImGetVoiceVol() {
	lua_pushnumber(g_imuse->getVoiceGroupVol());
}

void Lua_V1::ImSetSfxVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_imuse->setSFXGroupVol((int)lua_getnumber(volObj));
}

void Lua_V1::ImGetSfxVol() {
	lua_pushnumber(g_imuse->getSFXGroupVol());
}

void Lua_V1::ImSetParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);
	lua_Object valueObj = lua_getparam(3);

	if (!lua_isnumber(nameObj) && !lua_isstring(nameObj))
		return;
	if (!lua_isnumber(paramObj) || !lua_isnumber(valueObj))
		return;

	int param = (int)lua_getnumber(paramObj);
	int value = (int)lua_getnumber(valueObj);
	if (value < 0)
		value = 0;

	g_imuse->soundsSetParam(getSoundKey(nameObj), param, value);
}

void Lua_V1::ImGetParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);

	if (!lua_isnumber(nameObj) && !lua_isstring(nameObj)) {
		lua_pushnumber(-1.0);
		return;
	}

	int param = (int)lua_getnumber(paramObj);
	lua_pushnumber(g_imuse->soundsGetParam(getSoundKey(nameObj), param));
}

void Lua_V1::ImFadeParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object opcodeObj = lua_getparam(2);
	lua_Object valueObj = lua_getparam(3);
	lua_Object durationObj = lua_getparam(4);

	if (!lua_isstring(nameObj) && !lua_isnumber(nameObj)) {
		lua_pushnumber(0);
		return;
	}
	if (!lua_isnumber(opcodeObj) || !lua_isnumber(valueObj) || !lua_isnumber(durationObj))
		return;

	int opcode = (int)lua_getnumber(opcodeObj);
	int value = (int)lua_getnumber(valueObj);
	if (value < 0)
		value = 0;
	int duration = (int)lua_getnumber(durationObj);

	g_imuse->soundsFadeParam(getSoundKey(nameObj), opcode, value, duration);
}

void Lua_V1::ImSetState() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	g_imuseState = (int)lua_getnumber(stateObj);
}

void Lua_V1::ImSetSequence() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	int state = (int)lua_getnumber(stateObj);
	lua_pushnumber(g_imuse->imSetSequence(state));
}

void Lua_V1::ImSetAttribute() {
	lua_Object attrIndexObj = lua_getparam(1);
	lua_Object attrValObj = lua_getparam(2);
	if (!lua_isnumber(attrIndexObj) || !lua_isnumber(attrValObj))
		return;

	int attrIndex = (int)lua_getnumber(attrIndexObj);
	int attrVal = (int)lua_getnumber(attrValObj);
	lua_pushnumber(g_imuse->imSetAttribute(attrIndex, attrVal));
}

void Lua_V1::ImSetCuePoint() {
	lua_Object cueObj = lua_getparam(1);
	if (!lua_isnumber(cueObj))
		return;

	g_imuse->imSetCuePoint((int)lua_getnumber(cueObj));
}

void Lua_V1::SaveIMuse() {
	SaveGame *savedIMuse = SaveGame::openForSaving("grim.tmp");
	if (!savedIMuse)
		return;
	g_sound->saveState(savedIMuse);
	delete savedIMuse;
}

void Lua_V1::RestoreIMuse() {
	SaveGame *savedIMuse = SaveGame::openForLoading("grim.tmp");
	if (!savedIMuse)
		return;
	g_sound->restoreState(savedIMuse);
	delete savedIMuse;
	g_system->getSavefileManager()->removeSavefile("grim.tmp");
}

void Lua_V1::SetSoundPosition() {
	Math::Vector3d pos;
	int minVolume = 10;
	int maxVolume = 127;
	float maxDist = kSoundDefaultMaxDist;
	float panFactor, maxRadius;
	int argId = 1;
	lua_Object paramObj;

	// Use the room defaults if a room is currently set
	if (g_grim->getCurrSet())
		g_grim->getCurrSet()->getSoundParameters(&maxDist, &minVolume, &maxVolume, &panFactor, &maxRadius);

	lua_Object nameObj = lua_getparam(argId++);
	if (!lua_isnumber(nameObj) && !lua_isstring(nameObj))
		return;

	lua_Object actorObj = lua_getparam(argId++);
	if (lua_isuserdata(actorObj) && lua_tag(actorObj) == MKTAG('A','C','T','R')) {
		Actor *actor = getactor(actorObj);
		if (!actor)
			return;
		pos = actor->getPos();
	} else if (lua_isnumber(actorObj)) {
		float x = lua_getnumber(actorObj);
		float y = lua_getnumber(argId++);
		float z = lua_getnumber(argId++);
		pos.set(x, y, z);
	}

	paramObj = (int)lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		minVolume = (int)lua_getnumber(paramObj);
		if (minVolume > 127)
			minVolume = 127;
	}
	paramObj = lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		maxVolume = (int)lua_getnumber(paramObj);
		if (maxVolume > 127)
			maxVolume = 127;
		else if (maxVolume < minVolume)
			maxVolume = minVolume;
	}

	paramObj = lua_getparam(argId++);
	if (lua_isnumber(paramObj)) {
		maxDist = lua_getnumber(paramObj);
		if (maxDist < 0.0f)
			maxDist = 0.0f;
	}

	if (g_grim->getCurrSet()) {
		if (lua_isnumber(nameObj))
			g_grim->getCurrSet()->setSoundPosition((int)lua_getnumber(nameObj), pos, minVolume, maxVolume, maxDist, kSoundDefaultRadius);
		else
			g_grim->getCurrSet()->setSoundPosition(lua_getstring(nameObj), pos, minVolume, maxVolume, maxDist, kSoundDefaultRadius);
	}
}

void Lua_V1::SetSoundParameters() {
	lua_Object maxDistObj = lua_getparam(1);
	lua_Object minVolObj = lua_getparam(2);
	lua_Object maxVolObj = lua_getparam(3);
	lua_Object panFactorObj = lua_getparam(4);
	lua_Object maxRadiusObj = lua_getparam(5);

	Set *currSet = g_grim->getCurrSet();
	if (!currSet) {
		warning("SetSoundParameters: No current room!");
		return;
	}

	if (lua_isnil(maxDistObj) && lua_isnil(minVolObj) && lua_isnil(maxVolObj) &&
		lua_isnil(panFactorObj) && lua_isnil(maxRadiusObj)) {
		// Reset all parameters
		currSet->setSoundParameters();
		return;
	}

	// The current values are kept for the parameters which aren't specified
	float maxDist, panFactor, maxRadius;
	int minVol, maxVol;
	currSet->getSoundParameters(&maxDist, &minVol, &maxVol, &panFactor, &maxRadius);

	if (!lua_isnil(maxDistObj))
		maxDist = lua_getnumber(maxDistObj);
	// The volumes are bytes in the original
	if (!lua_isnil(minVolObj))
		minVol = (uint8)(int)lua_getnumber(minVolObj);
	if (!lua_isnil(maxVolObj))
		maxVol = (uint8)(int)lua_getnumber(maxVolObj);
	if (!lua_isnil(panFactorObj))
		panFactor = lua_getnumber(panFactorObj);
	if (!lua_isnil(maxRadiusObj))
		maxRadius = lua_getnumber(maxRadiusObj);

	currSet->setSoundParameters(maxDist, minVol, maxVol, panFactor, maxRadius);
}

void Lua_V1::GetSoundParameters() {
	Set *currSet = g_grim->getCurrSet();
	if (!currSet)
		return;

	float maxDist, panFactor, maxRadius;
	int minVol, maxVol;
	currSet->getSoundParameters(&maxDist, &minVol, &maxVol, &panFactor, &maxRadius);

	lua_pushnumber(maxDist);
	lua_pushnumber(minVol);
	lua_pushnumber(maxVol);
	lua_pushnumber(panFactor);
	lua_pushnumber(maxRadius);
}

void Lua_V1::IsSoundPlaying() {
	// dummy
}

void Lua_V1::PlaySoundAt() {
	// dummy
}

void Lua_V1::LoadBundle() {
	// loading grimdemo.mus is already handled
}

void Lua_V1::PlaySound() {
	// dummy
}

} // end of namespace Grim
