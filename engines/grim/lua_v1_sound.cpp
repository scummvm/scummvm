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
#include "engines/grim/imuse/imuse.h"
#include "engines/grim/lua/lauxlib.h"

namespace Grim {

enum ImuseParam {
	IM_SOUND_PLAY_COUNT = 0x100,
	IM_SOUND_PEND_COUNT = 0x200,
	IM_SOUND_GROUP = 0x400,
	IM_SOUND_PRIORITY = 0x500,
	IM_SOUND_VOL = 0x600,
	IM_SOUND_PAN = 0x700
};

static float ImToMixer = 2.f;

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

	// Start the sound with the appropriate settings
	if (g_imuse->startSound(soundName, group, 0, 127, 64, priority, nullptr)) {
		// FIXME actually it's pushnumber from result of startSound
		lua_pushstring(soundName);
	}
}

void Lua_V1::ImStopSound() {
	lua_Object nameObj = lua_getparam(1);
	if (lua_isnumber(nameObj))
		error("ImStopsound: name from value not supported");

	const char *soundName = lua_getstring(nameObj);
	g_imuse->stopSound(soundName);
}

void Lua_V1::ImStopAllSounds() {
	g_imuse->stopAllSounds();
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
	Debug::warning(Debug::Sound, "ImSetVoiceEffect(%s) Voice effects are not yet supported", effectName);
}

void Lua_V1::ImSetMusicVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, (int)(lua_getnumber(volObj) * ImToMixer));
}

void Lua_V1::ImGetMusicVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) / ImToMixer);
}

void Lua_V1::ImSetVoiceVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, (int)(lua_getnumber(volObj) * ImToMixer));
}

void Lua_V1::ImGetVoiceVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / ImToMixer);
}

void Lua_V1::ImSetSfxVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, (int)(lua_getnumber(volObj) * ImToMixer));
}

void Lua_V1::ImGetSfxVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType) / ImToMixer);
}

void Lua_V1::ImSetParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);
	lua_Object valueObj = lua_getparam(3);

	if (lua_isnumber(nameObj)) {
		warning("ImSetParam: getting name from number is not supported");
		return;
	}
	if (!lua_isstring(nameObj)) {
		return;
	}

	const char *soundName = lua_getstring(nameObj);
	int param = (int)lua_getnumber(paramObj);
	int value = (int)lua_getnumber(valueObj);
	if (value < 0)
		value = 0;
	switch (param) {
	case IM_SOUND_VOL:
		g_imuse->setVolume(soundName, value);
		break;
	case IM_SOUND_PAN:
		g_imuse->setPan(soundName, value);
		break;
	default:
		error("ImSetParam() Unimplemented %d", param);
	}
}

void Lua_V1::ImGetParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);

	if (lua_isnumber(nameObj)) {
		warning("ImGetParam: getting name from number is not supported");
		lua_pushnumber(-1.0);
		return;
	}
	if (!lua_isstring(nameObj)) {
		lua_pushnumber(-1.0);
		return;
	}

	const char *soundName = lua_getstring(nameObj);
	int param = (int)lua_getnumber(paramObj);
	switch (param) {
	case IM_SOUND_PLAY_COUNT:
		lua_pushnumber(g_imuse->getCountPlayedTracks(soundName));
		break;
	case IM_SOUND_VOL:
		lua_pushnumber(g_imuse->getVolume(soundName));
		break;
	default:
		error("ImGetParam() Unimplemented %d", param);
	}
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

	if (lua_isnumber(nameObj)) {
		error("ImFadeParam: getting name from number is not supported");
	}
	const char *soundName = lua_getstring(nameObj);
	int opcode = (int)lua_getnumber(opcodeObj);
	int value = (int)lua_getnumber(valueObj);
	if (value < 0)
		value = 0;
	int duration = (int)lua_getnumber(durationObj);
	switch (opcode) {
	case IM_SOUND_PAN:
		g_imuse->setFadePan(soundName, value, duration);
		break;
	default:
		error("ImFadeParam(%s, %x, %d, %d)", soundName, opcode, value, duration);
		break;
	}
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
	lua_pushnumber(g_imuse->setMusicSequence(state));
}

void Lua_V1::SaveIMuse() {
	SaveGame *savedIMuse = SaveGame::openForSaving("grim.tmp");
	if (!savedIMuse)
		return;
	g_imuse->saveState(savedIMuse);
	delete savedIMuse;
}

void Lua_V1::RestoreIMuse() {
	SaveGame *savedIMuse = SaveGame::openForLoading("grim.tmp");
	if (!savedIMuse)
		return;
	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_imuse->restoreState(savedIMuse);
	delete savedIMuse;
	g_system->getSavefileManager()->removeSavefile("grim.tmp");
}

void Lua_V1::SetSoundPosition() {
	Math::Vector3d pos;
	int minVolume = 10;
	int maxVolume = 127;
	float someParam = 0;
	int argId = 1;
	lua_Object paramObj;

	if (g_grim->getCurrSet()) {
		g_grim->getCurrSet()->getSoundParameters(&minVolume, &maxVolume);
	}

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
		someParam = (int)lua_getnumber(paramObj);
		if (someParam < 0.0)
			someParam = 0.0;
	}

	if (g_grim->getCurrSet()) {
		if (lua_isnumber(nameObj))
			error("SetSoundPosition: number is not yet supported");
		else {
			const char *soundName = lua_getstring(nameObj);
			g_grim->getCurrSet()->setSoundPosition(soundName, pos, minVolume, maxVolume);
		}
	}
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
