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

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

#include "common/config-manager.h"
#include "common/error.h"
#include "common/file.h"
#include "common/system.h"

#include "cryomni3d/versailles/engine.h"

namespace CryOmni3D {
namespace Versailles {

const char *CryOmni3DEngine_Versailles::kMusicFiles[8][8] = {
	{ "1amb", }, // Level 1
	{ "2amb", "2amb2", "2amb1" }, // Level 2
	{ "3amb", "3amb1", "3amb2" }, // Level 3
	{ "4amb", "4amb1" }, // Level 4
	{ "5amb1", "5amb2" }, // Level 5
	{ "6amb1", "6amb2", "6amb3", "6amb4" }, // Level 6
	{ "7amb", }, // Level 7
	{ "3amb", "3amb1", "3amb2", "2amb", "2amb1", "2amb2", "4amb" }, // Level 8
};

void CryOmni3DEngine_Versailles::musicUpdate() {
	if (!_isPlaying || _currentLevel <= 0 ||
	        _mixer->isSoundTypeMuted(Audio::Mixer::kMusicSoundType) ||
	        _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType) == 0) {
		// No music in all of these cases
		musicStop();
		return;
	}

	uint musicId = getMusicId(_currentLevel, _currentPlaceId);
	const char *musicBName = kMusicFiles[_currentLevel - 1][musicId];
	assert(musicBName != nullptr);

	// Ensure sound is playing in all cases
	musicResume();

	if (musicBName == _musicCurrentFile) {
		// Same file, nothing more to do
		return;
	}

	// New file, stop the old one first
	musicStop();

	Common::String musicFName(prepareFileName(musicBName, "wav"));

	Common::File *musicFile = new Common::File();
	if (!musicFile->open(musicFName)) {
		warning("Failed to open music file %s/%s", musicBName, musicFName.c_str());
		delete musicFile;
		return;
	}

	Audio::SeekableAudioStream *musicDecoder = Audio::makeWAVStream(musicFile, DisposeAfterUse::YES);
	// We lost ownership of the musicFile just set it to nullptr and don't use it
	musicFile = nullptr;

	if (!musicDecoder) {
		warning("Failed to decode music file %s/%s", musicBName, musicFName.c_str());
		return;
	}

	Audio::AudioStream *loopStream = Audio::makeLoopingAudioStream(musicDecoder, 0);
	// We lost ownership of musicDecoder just set it to nullptr and don't use it
	musicDecoder = nullptr;

	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, loopStream);
	_musicCurrentFile = musicBName;
}

void CryOmni3DEngine_Versailles::musicPause() {
	_mixer->pauseHandle(_musicHandle, true);
}

void CryOmni3DEngine_Versailles::musicResume() {
	_mixer->pauseHandle(_musicHandle, false);
}

void CryOmni3DEngine_Versailles::musicStop() {
	// Fade the music first
	if (_mixer->isSoundHandleActive(_musicHandle)) {
		// We recreate the real channel volume to decrease this one 2 by 2
		int musicVol = _mixer->getVolumeForSoundType(Audio::Mixer::kMusicSoundType);
		byte channelVol = _mixer->getChannelVolume(_musicHandle);
		int realVolume = (musicVol * channelVol) / Audio::Mixer::kMaxChannelVolume;
		bool skip = false;
		while (realVolume > 0 && !skip) {
			realVolume -= 2;
			channelVol = CLIP((realVolume * Audio::Mixer::kMaxChannelVolume) / musicVol, 0, 255);
			_mixer->setChannelVolume(_musicHandle, channelVol);
			if (pollEvents() && checkKeysPressed(1, Common::KEYCODE_SPACE)) {
				skip = true;
			}
			g_system->delayMillis(10);
		}
	}
	_mixer->stopHandle(_musicHandle);
	_musicCurrentFile = nullptr;
}

void CryOmni3DEngine_Versailles::musicSetQuiet(bool quiet) {
	float newFactor = quiet ? 3.5f : 1.f;
	if (newFactor != _musicVolumeFactor) {
		_musicVolumeFactor = newFactor;
		syncSoundSettings();
	}
}

bool CryOmni3DEngine_Versailles::musicWouldChange(uint level, uint placeId) const {
	uint musicId = getMusicId(level, placeId);
	const char *musicFile = kMusicFiles[_currentLevel - 1][musicId];

	return musicFile != _musicCurrentFile;
}

uint CryOmni3DEngine_Versailles::getMusicId(uint level,
        uint placeId) const {
	// No need of place state
	switch (level) {
	case 1:
		// Only one music
		return 0;
	case 2:
		switch (placeId) {
		case 4:
			return 1;
		case 10:
		case 11:
		case 13:
			return 2;
		default:
			return 0;
		}
	case 3:
		switch (placeId) {
		case 1:
		case 2:
		case 3:
		case 4:
			return 2;
		case 6:
		case 7:
		case 8:
		case 12:
		case 24:
			return 1;
		default:
			return 0;
		}
	case 4:
		switch (placeId) {
		case 1:
		case 2:
		case 3:
		case 4:
			return 1;
		default:
			return 0;
		}
	case 5:
		switch (placeId) {
		case 6:
		case 7:
		case 8:
		case 12:
		case 26:
		case 27:
		case 30:
		case 31:
			return 1;
		default:
			return 0;
		}
	case 6:
		switch (placeId) {
		case 1:
			return 3;
		case 3:
		case 4:
		case 5:
		case 6:
		case 8:
		case 9:
		case 10:
		case 11:
			return 0;
		case 14:
		case 16:
		case 17:
		case 19:
		case 20:
		case 22:
		case 24:
		case 26:
		case 27:
		case 32:
		case 34:
		case 38:
		case 44:
			return 2;
		default:
			return 1;
		}
	case 7:
		return 0;
	case 8:
		switch (placeId) {
		case 1:
		case 2:
		case 3:
		case 4:
			return 2;
		case 6:
		case 7:
		case 8:
			return 1;
		case 9:
		case 10:
		case 11:
			return 0;
		case 12:
			return 1;
		case 13:
		case 14:
		case 15:
		case 16:
			return 0;
		case 24:
			return 1;
		case 33:
		case 34:
		case 35:
			return 5;
		case 36:
		case 37:
		case 38:
		case 39:
			return 3;
		case 40:
			return 4;
		case 42:
		case 43:
		case 44:
			return 6;
		default:
			return 0;

		}
	default:
		error("Invalid level %d when choosing music", level);
	}
}

} // End of namespace Versailles
} // End of namespace CryOmni3D
