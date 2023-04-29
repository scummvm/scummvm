/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "audio/decoders/vorbis.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/util.h"
#include "ags/plugins/ags_waves/ags_waves.h"
#include "ags/ags.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

//const float PI = 3.14159265f;

void AGSWaves::SFX_Play(ScriptMethodParams &params) {
	PARAMS2(int, sfxNum, int, repeat);

	SoundEffect &effect = SFX[sfxNum];
	if (_mixer->isSoundHandleActive(effect._soundHandle)) {
		if (effect._allow == 1) {
			// In this case we should start the sound on a new channel, not stopping
			// the one currently playing.
			warning("TODO: play overlapping sound with SFX_Play");
		}
		return;
	}
	_mixer->stopHandle(effect._soundHandle);

	Common::FSNode fsNode = ::AGS::g_vm->getGameFolder().getChild(
		"sounds").getChild(Common::String::format("sound%d.sfx", sfxNum));

	Audio::AudioStream *sound = loadOGG(fsNode);

	if (sound != nullptr) {
		effect._volume = 255;

		playStream(Audio::Mixer::kSFXSoundType, &effect._soundHandle, sound, repeat);

		if (OGG_Filter && effect._filter && effect._volume > 1) {
			warning("TODO: Mix_RegisterEffect(grabChan, LPEffect, NULL, NULL);");
		}

		effect._repeat = repeat;
	}
}

void AGSWaves::SFX_SetVolume(ScriptMethodParams &params) {
	PARAMS2(int, sfxNum, int, volume);

	SoundEffect &effect = SFX[sfxNum];
	_mixer->setChannelVolume(effect._soundHandle, volume);
	effect._volume = volume;
}

void AGSWaves::SFX_GetVolume(ScriptMethodParams &params) {
	PARAMS1(int, sfxNum);

	SoundEffect &effect = SFX[sfxNum];
	params._result = _mixer->getChannelVolume(effect._soundHandle);
}

void AGSWaves::Music_Play(ScriptMethodParams &params) {
	PARAMS6(int, MFX, int, repeat, int, fadeinMS, int, fadeoutMS, int, Position, bool, fixclick);
	MusicPlay(MFX, repeat, fadeinMS, fadeoutMS, Position, false, fixclick);
}

void AGSWaves::Music_SetVolume(ScriptMethodParams &params) {
	PARAMS1(int, volume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, volume);
}

void AGSWaves::Music_GetVolume(ScriptMethodParams &params) {
	params._result = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
}

void AGSWaves::SFX_Stop(ScriptMethodParams &params) {
	PARAMS1(int, sfxNum); //, int, fademsOUT);
	StopSFX(sfxNum);
}

void AGSWaves::SFX_SetPosition(ScriptMethodParams &params) {
#if 0
	PARAMS4(int, sfxNum, int, xS, int, yS, int, intensity);

	SoundEffect &effect = SFX[sfxNum];

	if (_mixer->isSoundHandleActive(effect._soundHandle)) {
		int angle = 0;
		int dist = 0;

		if (xS != 0 && yS != 0) {
			int pid = _engine->GetPlayerCharacter();
			playerCharacter = _engine->GetCharacter(pid);

			int x1 = Character_GetX((intptr_t)playerCharacter);
			int y1 = Character_GetY((intptr_t)playerCharacter);

			int x2 = xS;
			int y2 = yS;

			int defx = (x1 - x2) * (x1 - x2);
			int defy = (y1 - y2) * (y1 - y2);

			float SquareRoot = sqrt(float(defx + defy));
			dist = int(SquareRoot) - intensity;
			if (dist > 255) dist = 255;
			if (dist < 0)  dist = 0;

			float xDiff = float(x2 - x1);
			float yDiff = float(y2 - y1);
			float at2 = atan2(yDiff, xDiff);

			float angles = (at2 * 360.0 / PI);
			angle = int(angles);//%360;
		}

		// TODO: Change Mix_SetPosition to ScummVM equivalent
		//Mix_SetPosition(id, angle, dist);
		(void)angle;
		(void)dist;
	}
#endif
	debug(0, "TODO: SFX_Setposition positional sound not yet implemented");
}

void AGSWaves::SFX_SetGlobalVolume(ScriptMethodParams &params) {
	PARAMS1(int, volume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, volume);
}

void AGSWaves::Load_SFX(ScriptMethodParams &params) {
//	PARAMS1(int, sfxNum);
//	LoadSFX(sfxNum);
}

void AGSWaves::Audio_Apply_Filter(ScriptMethodParams &params) {
	PARAMS1(int, Frequency);

	GlitchFix();
	ApplyFilter(Frequency);
}

void AGSWaves::Audio_Remove_Filter(ScriptMethodParams &params) {
}

void AGSWaves::SFX_AllowOverlap(ScriptMethodParams &params) {
	PARAMS2(int, sfxNum, int, allow);
	SFX[sfxNum]._allow = allow;
}

void AGSWaves::SFX_Filter(ScriptMethodParams &params) {
	PARAMS2(int, sfxNum, int, enable);

	// THIS ENABLES/DISABLES the SFX LOW PASS FILTER,
	// I think by default all sound effects are affected by low pass, but there are some that i've manually disabled from being affected by it with this command
	SFX[sfxNum]._filter = enable;
}

Audio::AudioStream *AGSWaves::loadOGG(const Common::FSNode &fsNode) {
#ifdef USE_VORBIS
	if (fsNode.exists()) {
		Common::File *soundFile = new Common::File();
		if (!soundFile->open(fsNode))
			error("Failed to open");

		Audio::AudioStream *stream = Audio::makeVorbisStream(soundFile, DisposeAfterUse::YES);
		return (stream) ? stream : nullptr;
	}
#endif

	return nullptr;
}

void AGSWaves::playStream(Audio::Mixer::SoundType type, Audio::SoundHandle *handle, Audio::AudioStream *stream, int repeat) {
	if (!handle || !stream)
		return;

	if (repeat != 0) {
		Audio::SeekableAudioStream *sas =
			dynamic_cast<Audio::SeekableAudioStream *>(stream);
		assert(sas);

		// -1 for infinite, >0 number of successive repeats
		Audio::LoopingAudioStream *las =
			new Audio::LoopingAudioStream(sas, repeat + 1);
		_mixer->playStream(type, handle, las);
	} else {
		_mixer->playStream(type, handle, stream);
	}
}

void AGSWaves::StopSFX(int sfxNum) {
	SoundEffect &effect = SFX[sfxNum];
	_mixer->stopHandle(effect._soundHandle);
	effect._playing = 0;
	effect._repeat = 0;
	effect._channel = -2;
}

void AGSWaves::stopAllSounds() {
	for (int i = 0; i < 500; ++i)
		StopSFX(i);

	_mixer->stopHandle(MFXStream._soundHandle);
}

void AGSWaves::GlitchFix() {
	// TODO: Implementation
}

void AGSWaves::ApplyFilter(int setFrequency) {
	// THIS TURNS ON THE LOW PASS FILTER
	OGG_Filter = true;
	GeneralAudio.FilterFrequency = setFrequency;
	SetFilterFrequency(setFrequency);
}

void AGSWaves::SetFilterFrequency(int setFrequency) {
	// TODO: Implementation
}

void AGSWaves::MusicPlay(int MusicToPlay, int repeat, int fadeinMS, int fadeoutMS, int pos, bool forceplay, bool fixclick) {
	if (GeneralAudio.Disabled) {
		return;
	}

	// Stop any previous music
	_mixer->stopHandle(MFXStream._soundHandle);

	// Load OGG file for music
	Common::FSNode fsNode = ::AGS::g_vm->getGameFolder().getChild(
		"Music").getChild(Common::String::format("music%d.mfx", MusicToPlay));
	Audio::AudioStream *musicStream = loadOGG(fsNode);
	if (!musicStream)
		return;

	bool samefile = currentMusic != MusicToPlay;
	if (forceplay)
		samefile = true;

	if (samefile) {
		currentMusicRepeat = repeat;
		currentMusicFadein = fadeinMS;
		currentMusic = MusicToPlay;

		if (!MFXStream.Switch) {
			MFXStream.Channel = 0;

			playStream(Audio::Mixer::kMusicSoundType,
				&MFXStream._soundHandle, musicStream, repeat);

			MFXStream.ID = MusicToPlay;
			MFXStream.FadeTime = (fadeinMS / 1000) * 40;
			MFXStream.FadeRate = (float)_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType)
				/ (float)MFXStream.FadeTime;
			MFXStream.FadeVolume = 0.0;
			MFXStream.HaltedZero = false;
			//MusicVolCanBeAdjusted=true;
		} else {
			MFXStream.HaltedOne = false;
			MFXStream.Channel = 1;

			playStream(Audio::Mixer::kMusicSoundType,
				&MFXStream._soundHandle, musicStream, repeat);

			MFXStream.ID = MusicToPlay;
			MFXStream.FadeTime = (fadeoutMS / 1000) * 40;
			MFXStream.FadeVolume = 0.0;//float(MusicGetVolume());
			MFXStream.FadeRate = (float)_mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType)
				/ (float)MFXStream.FadeTime;
			//MusicVolCanBeAdjusted=false;
		}

		MFXStream.Switch = !MFXStream.Switch;
	}
}

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
