/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/debug.h"
#include "engines/grim/grim.h"
#include "engines/grim/lua.h"
#include "engines/grim/actor.h"
#include "engines/grim/lipsync.h"
#include "engines/grim/savegame.h"
#include "engines/grim/colormap.h"
#include "engines/grim/scene.h"

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

void L1_ImStartSound() {
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
	if (g_imuse->startSound(soundName, group, 0, 127, 64, priority, NULL)) {
		// FIXME actually it's pushnumber from result of startSound
		lua_pushstring(soundName);
	}
}

void L1_ImStopSound() {
	lua_Object nameObj = lua_getparam(1);
	if (lua_isnumber(nameObj))
		error("ImStopsound: name from value not supported");

	const char *soundName = lua_getstring(nameObj);
	g_imuse->stopSound(soundName);
}

void L1_ImStopAllSounds() {
	g_imuse->stopAllSounds();
}

void L1_ImPause() {
	g_imuse->pause(true);
}

void L1_ImResume() {
	g_imuse->pause(false);
}

void L1_ImSetVoiceEffect() {
	const char *effectName;

	effectName = luaL_check_string(1);
	if (gDebugLevel == DEBUG_WARN || gDebugLevel == DEBUG_ALL)
		warning("ImSetVoiceEffect(%s) Voice effects are not yet supported", effectName);
}

void L1_ImSetMusicVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, (int)lua_getnumber(volObj));
}

void L1_ImGetMusicVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kMusicSoundType));
}

void L1_ImSetVoiceVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, (int)lua_getnumber(volObj));
}

void L1_ImGetVoiceVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType));
}

void L1_ImSetSfxVol() {
	lua_Object volObj = lua_getparam(1);
	if (!lua_isnumber(volObj))
		return;
	g_system->getMixer()->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, (int)lua_getnumber(volObj));
}

void L1_ImGetSfxVol() {
	lua_pushnumber(g_system->getMixer()->getVolumeForSoundType(Audio::Mixer::kSFXSoundType));
}

void L1_ImSetParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);
	lua_Object valueObj = lua_getparam(3);

	if (lua_isnumber(nameObj))
		error("ImSetParam: getting name from number is not supported");
	if (!lua_isstring(nameObj)) {
		lua_pushnumber(-1.0);
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

void L1_ImGetParam() {
	lua_Object nameObj = lua_getparam(1);
	lua_Object paramObj = lua_getparam(2);

	if (lua_isnumber(nameObj))
		error("ImGetParam: getting name from number is not supported");
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

void L1_ImFadeParam() {
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

void L1_ImSetState() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	g_imuseState = (int)lua_getnumber(stateObj);
}

void L1_ImSetSequence() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	int state = (int)lua_getnumber(stateObj);
	lua_pushnumber(g_imuse->setMusicSequence(state));
}

void L1_SaveIMuse() {
	SaveGame *savedIMuse = SaveGame::openForSaving("grim.tmp");
	if (!savedIMuse)
		return;
	g_imuse->saveState(savedIMuse);
	delete savedIMuse;
}

void L1_RestoreIMuse() {
	SaveGame *savedIMuse = SaveGame::openForLoading("grim.tmp");
	if (!savedIMuse)
		return;
	g_imuse->stopAllSounds();
	g_imuse->resetState();
	g_imuse->restoreState(savedIMuse);
	delete savedIMuse;
	g_system->getSavefileManager()->removeSavefile("grim.tmp");
}

void L1_SetSoundPosition() {
	Math::Vector3d pos;
	int minVolume = 10;
	int maxVolume = 127;
	float someParam = 0;
	int argId = 1;
	lua_Object paramObj;

	if (g_grim->getCurrScene()) {
		g_grim->getCurrScene()->getSoundParameters(&minVolume, &maxVolume);
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

	if (g_grim->getCurrScene()) {
		if (lua_isnumber(nameObj))
			error("SetSoundPosition: number is not yet supported");
		else {
			const char *soundName = lua_getstring(nameObj);
			g_grim->getCurrScene()->setSoundPosition(soundName, pos, minVolume, maxVolume);
		}
	}
}

void L1_IsSoundPlaying() {
	// dummy
}

void L1_PlaySoundAt() {
	// dummy
}

void L1_LoadBundle() {
	// loading grimdemo.mus is allready handled
}

void L1_PlaySound() {
	// dummy
}

} // end of namespace Grim
