/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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

const float PI = 3.14159265f;

void AGSWaves::SFX_Play(ScriptMethodParams &params) {
	PARAMS2(int, sfxNum, int, repeat);

	SoundEffect &effect = SFX[sfxNum];
	if (effect._stream == nullptr) {
		LoadSFX(sfxNum);
	}

	Audio::AudioStream *sound = effect._stream;
	if (sound != nullptr) {
		effect._volume = 255;

		if (repeat != 0) {
			Audio::SeekableAudioStream *sas =
				dynamic_cast<Audio::SeekableAudioStream *>(sound);
			assert(sas);

			// -1 for infinite, >0 number of successive repeats
			Audio::LoopingAudioStream *las =
				new Audio::LoopingAudioStream(sas, repeat + 1, DisposeAfterUse::NO);
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &effect._soundHandle, las,
				-1, 255, 0, DisposeAfterUse::YES);
		} else {
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &effect._soundHandle, sound,
				-1, effect._volume, 0, DisposeAfterUse::NO);
		}

		if (OGG_Filter && effect._filter && effect._volume > 1) {
			warning("TODO: Mix_RegisterEffect(grabChan, LPEffect, NULL, NULL);");
		}

		effect._repeat = repeat;
		effect._playing = true;
	}
}

void AGSWaves::SFX_SetVolume(ScriptMethodParams &params) {
	PARAMS2(int, sfxNum, int, volume);

	SoundEffect &effect = SFX[sfxNum];

	if (effect._stream != nullptr) {
		_mixer->setChannelVolume(effect._soundHandle, volume);
		effect._volume = volume;
	}
}

void AGSWaves::SFX_GetVolume(ScriptMethodParams &params) {
	PARAMS1(int, sfxNum);

	SoundEffect &effect = SFX[sfxNum];
	params._result = effect._stream ? effect._volume : 0;
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

	SoundEffect &effect = SFX[sfxNum];

	if (effect._stream != nullptr) {
		_mixer->stopHandle(effect._soundHandle);
		UnloadSFX(sfxNum);

		effect._playing = false;
		effect._repeat = 0;
		effect._channel = -2;
	}
}

void AGSWaves::SFX_SetPosition(ScriptMethodParams &params) {
	PARAMS4(int, sfxNum, int, xS, int, yS, int, intensity);

	SoundEffect &effect = SFX[sfxNum];

	if (effect._stream != nullptr) {
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
	}
}

void AGSWaves::SFX_SetGlobalVolume(ScriptMethodParams &params) {
	PARAMS1(int, volume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kPlainSoundType, volume);
}

void AGSWaves::Load_SFX(ScriptMethodParams &params) {
	PARAMS1(int, sfxNum);
	LoadSFX(sfxNum);
}

void AGSWaves::Audio_Apply_Filter(ScriptMethodParams &params) {
	PARAMS1(int, Frequency);

	GlitchFix();
	ApplyFilter(Frequency);
}

void AGSWaves::Audio_Remove_Filter(ScriptMethodParams &params) {
}

void AGSWaves::SFX_AllowOverlap(ScriptMethodParams &params) {
	//PARAMS2(int, SFX, int, allow);
}

void AGSWaves::SFX_Filter(ScriptMethodParams &params) {
	PARAMS2(int, sfxNum, int, enable);

	// THIS ENABLES/DISABLES the SFX LOW PASS FILTER,
	// I think by default all sound effects are affected by low pass, but there are some that i've manually disabled from being affected by it with this command
	SFX[sfxNum]._filter = enable;
}


void AGSWaves::LoadSFX(int i) {
	Common::FSNode fsNode = ::AGS::g_vm->getGameFolder().getChild(
		"sounds").getChild(Common::String::format("sound%d.sfx", i));

#ifdef USE_VORBIS
	if (fsNode.exists()) {
		Common::File *soundFile = new Common::File();
		if (!soundFile->open(fsNode))
			error("Failed to open");

		SFX[i]._stream = Audio::makeVorbisStream(soundFile, DisposeAfterUse::YES);
		assert(SFX[i]._stream);
	}
#endif
}

void AGSWaves::UnloadSFX(int i) {
	if (SFX[i]._stream != nullptr) {
		_mixer->stopHandle(SFX[i]._soundHandle);
		delete SFX[i]._stream;
		SFX[i]._stream = nullptr;
	}
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

	bool samefile = currentMusic != MusicToPlay;
	if (forceplay) samefile = true;

	if (samefile) {
		currentMusicRepeat = repeat;
		currentMusicFadein = fadeinMS;
		currentMusic = MusicToPlay;

		if (!MFXStream.Switch) {
			MFXStream.Channel = 0;

			warning("TODO: OGGplayMusic(MusicLoads[MusicToPlay].musicPath, 0, repeat, 0, fixclick);");
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

			warning("TODO: OGGplayMusic(MusicLoads[MusicToPlay].musicPath, 0, repeat, 1, fixclick);");

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
