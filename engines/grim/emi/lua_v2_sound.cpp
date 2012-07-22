/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
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

#include "audio/mixer.h"
#include "audio/audiostream.h"
#include "common/system.h"

#include "engines/grim/emi/sound/aifftrack.h"
#include "engines/grim/emi/lua_v2.h"
#include "engines/grim/lua/lua.h"

#include "engines/grim/sound.h"
#include "engines/grim/grim.h"
#include "engines/grim/resource.h"
#include "audio/decoders/aiff.h"

namespace Grim {

void Lua_V2::ImGetMillisecondPosition() {
	lua_Object soundObj = lua_getparam(1);

	if (lua_isnumber(soundObj)) {
		int sound = (int)lua_getnumber(soundObj);
		// FIXME int ms = func(sound);
		// lua_pushnumber(ms);
		// push -1 for now
		// Currently a bit of guesswork, and probably wrong, as the stateId
		// is ignored by emisound (which only has one music-track now).
		uint32 msPos = g_sound->getMsPos(sound);
		warning("Lua_V2::ImGetMillisecondPosition: sound: %d ms: %d", sound, msPos);
		lua_pushnumber(msPos);
	}
}

void Lua_V2::SetReverb() {
	lua_Object eaxObj = lua_getparam(1);
	lua_Object decayObj = lua_getparam(2);
	lua_Object mixObj = lua_getparam(3);
	lua_Object predelayObj = lua_getparam(4);
	lua_Object dampingObj = lua_getparam(5);

	if (!lua_isnumber(eaxObj))
		return;

	int eax = (int)lua_getnumber(eaxObj);
	int param = 0;
	float decay = -1;
	float mix = -1;
	float predelay = -1;
	float damping = -1;

	if (eax == 60) {
		param = 26;
	} else if (eax == 70) {
		param = 27;
	} else if (eax >= 0 && eax <= 25) {
		param = eax;
		// there is some table, initialy is like eax
	} else {
		return;
	}

	if (lua_isnumber(decayObj))
		decay = lua_getnumber(decayObj);
	if (lua_isnumber(mixObj))
		mix = lua_getnumber(mixObj);
	if (lua_isnumber(predelayObj))
		predelay = lua_getnumber(predelayObj);
	if (lua_isnumber(dampingObj))
		damping = lua_getnumber(dampingObj);

	warning("Lua_V2::SetReverb, eax: %d, decay: %f, mix: %f, predelay: %f, damping: %f", param, decay, mix, predelay, damping);
	// FIXME: func(param, decay, mix, predelay, damping);
}

void Lua_V2::ImSetState() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	int state = (int)lua_getnumber(stateObj);
	g_imuseState = state;
	warning("Lua_V2::ImSetState: stub, state: %d", state);
}

void Lua_V2::ImStateHasEnded() {
	lua_Object stateObj = lua_getparam(1);
	if (!lua_isnumber(stateObj))
		return;

	int state = (int)lua_getnumber(stateObj);

	// FIXME: Make sure this logic is correct.
	pushbool(g_imuseState != state);

	warning("Lua_V2::ImStateHasEnded: state %d.", state);
}

void Lua_V2::EnableVoiceFX() {
	lua_Object stateObj = lua_getparam(1);

	bool state = false;
	if (!lua_isnil(stateObj))
		state = true;

	// FIXME: func(state);
	warning("Lua_V2::EnableVoiceFX: implement opcode, state: %d", (int)state);
}

void Lua_V2::SetGroupVolume() {
	lua_Object groupObj = lua_getparam(1);
	lua_Object volumeObj = lua_getparam(2);

	if (!lua_isnumber(groupObj))
		return;
	int group = (int)lua_getnumber(groupObj);

	int volume = 100;
	if (lua_isnumber(volumeObj))
		volume = (int)lua_getnumber(volumeObj);

	// FIXME: func(group, volume);
	warning("Lua_V2::SetGroupVolume: implement opcode, group: %d, volume %d", group, volume);
}

void Lua_V2::EnableAudioGroup() {
	lua_Object groupObj = lua_getparam(1);
	lua_Object stateObj = lua_getparam(2);

	if (!lua_isnumber(groupObj))
		return;
	int group = (int)lua_getnumber(groupObj);

	bool state = false;
	if (!lua_isnil(stateObj))
		state = true;

	// FIXME: func(group, state);
	warning("Lua_V2::EnableAudioGroup: implement opcode, group: %d, state %d", group, (int)state);
}

void Lua_V2::ImSelectSet() {
	lua_Object qualityObj = lua_getparam(1);

	if (lua_isnumber(qualityObj)) {
		int quality = (int)lua_getnumber(qualityObj);
		// FIXME: func(quality);
		warning("Lua_V2::ImSelectSet: implement opcode, quality mode: %d", quality);
	}
}

void Lua_V2::ImFlushStack() {
	// FIXME
	warning("Lua_V2::ImFlushStack: implement opcode");
}

class PoolSound : public PoolObject<PoolSound>{
public:
	PoolSound(const Common::String &filename);
	static int32 getStaticTag() { return MKTAG('A', 'I', 'F', 'F'); }
	AIFFTrack *track;
};

PoolSound::PoolSound(const Common::String &filename) {
	track = new AIFFTrack(Audio::Mixer::kSFXSoundType);
	Common::SeekableReadStream *stream = g_resourceloader->openNewStreamFile(filename);
	if (!stream)
		return;
	track->openSound(filename, stream);
}

void Lua_V2::LoadSound() {
	lua_Object strObj = lua_getparam(1);

	if (!lua_isstring(strObj))
		return;

	const char *str = lua_getstring(strObj);

	Common::String filename = str;
	filename += ".aif";

	PoolSound *sound = new PoolSound(filename);
	lua_pushusertag(sound->getId(), MKTAG('A', 'I', 'F', 'F'));
}

void Lua_V2::FreeSound() {
	lua_Object idObj = lua_getparam(1);
	if (!lua_isuserdata(idObj) || lua_tag(idObj) != MKTAG('A', 'I', 'F', 'F'))
		return;
	PoolSound *sound = PoolSound::getPool().getObject(lua_getuserdata(idObj));
	if (sound) {
		sound->track->stop();
		delete sound;
	}
}

void Lua_V2::PlayLoadedSound() {
	lua_Object idObj = lua_getparam(1);
	lua_Object bool1Obj = lua_getparam(2);
	/*lua_Object volumeObj =*/ lua_getparam(3);
	/*lua_Object bool2Obj =*/ lua_getparam(4);


	if (!lua_isuserdata(idObj) || lua_tag(idObj) != MKTAG('A', 'I', 'F', 'F'))
		return;

	bool looping = !lua_isnil(bool1Obj);

	PoolSound *sound = PoolSound::getPool().getObject(lua_getuserdata(idObj));
	sound->track->setLooping(looping);
	sound->track->play();
}

void Lua_V2::PlayLoadedSoundFrom() {
	warning("Lua_V2::PlayLoadedSoundFrom: implement opcode");
}

void Lua_V2::StopSound() {
	lua_Object idObj = lua_getparam(1);

	if (!lua_isuserdata(idObj) || lua_tag(idObj) != MKTAG('A', 'I', 'F', 'F'))
		return;

	PoolSound *sound = PoolSound::getPool().getObject(lua_getuserdata(idObj));
	if (sound) {
		sound->track->stop();
	}
}

void Lua_V2::PlaySound() {
	lua_Object strObj = lua_getparam(1);
	//FIXME: get the second param
	lua_getparam(2);

	if (!lua_isstring(strObj))
		return;

	const char *str = lua_getstring(strObj);
	Common::String filename = str;

	if (g_grim->getGameFlags() != ADGF_DEMO) {
		filename += ".aif";
	}

	AIFFTrack *track = new AIFFTrack(Audio::Mixer::kSFXSoundType);
	Common::SeekableReadStream *stream = g_resourceloader->openNewStreamFile(filename);
	track->openSound(filename, stream);
	track->play();
}

// FIXME: implement sound positioning
void Lua_V2::PlaySoundFrom() {
//  lua_Object strObj = lua_getparam(1);
//  lua_Object volObj = lua_getparam(2);
//  lua_Object posxObj = lua_getparam(3);
//  lua_Object posyObj = lua_getparam(4);
//  lua_Object posZObj = lua_getparam(5);

	return PlaySound();
}

void Lua_V2::ImSetMusicVol() {
	// This only seems to be used in the demo.
	lua_Object volumeObj = lua_getparam(1);

	if (!lua_isnumber(volumeObj))
		return;
	int volume = (int)lua_getnumber(volumeObj);
	warning("Lua_V2::ImSetMusicVol: implement opcode, wants volume %d", volume);
}

void Lua_V2::ImSetSfxVol() {
	// This only seems to be used in the demo.
	lua_Object volumeObj = lua_getparam(1);

	if (!lua_isnumber(volumeObj))
		return;
	int volume = (int)lua_getnumber(volumeObj);
	warning("Lua_V2::ImSetSfxVol: implement opcode, wants volume %d", volume);
}

void Lua_V2::ImSetVoiceVol() {
	// This only seems to be used in the demo.
	lua_Object volumeObj = lua_getparam(1);

	if (!lua_isnumber(volumeObj))
		return;
	int volume = (int)lua_getnumber(volumeObj);
	warning("Lua_V2::ImSetVoiceVol: implement opcode, wants volume %d", volume);
}

void Lua_V2::ImSetVoiceEffect() {
	// This only seems to be used in the demo.
	lua_Object strObj = lua_getparam(1);

	if (!lua_isstring(strObj))
		return;

	const char *str = lua_getstring(strObj);
	warning("Lua_V2::ImSetVoiceEffect: implement opcode, wants effect %s", str);
}

void Lua_V2::StopAllSounds() {
	warning("Lua_V2::StopAllSounds: implement opcode");
}

void Lua_V2::ImPushState() {
	warning("Lua_V2::ImPushState: implement opcode");
}
void Lua_V2::ImPopState() {
	warning("Lua_V2::ImPopState: implement opcode");
}

} // end of namespace Grim
