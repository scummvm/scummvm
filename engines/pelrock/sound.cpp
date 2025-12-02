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

#include "audio/audiostream.h"
#include "audio/decoders/mp3.h"
#include "audio/mixer.h"
#include "common/file.h"
#include "common/debug.h"
#include "common/scummsys.h"

#include "pelrock/sound.h"

namespace Pelrock {

SoundManager::SoundManager(Audio::Mixer *mixer)
    : _mixer(mixer), _currentVolume(255), _musicFile(nullptr) {
	// TODO: Initialize sound manager
}

SoundManager::~SoundManager() {
	stopSound();
    stopMusic();
}

void SoundManager::playSound(const Common::String &filename, int volume) {
	// TODO: Play sound file
}

void SoundManager::stopSound() {
	// _mixer->stopHandle(_soundHandle);
	// TODO: Stop currently playing sound
}

void SoundManager::setVolume(int volume) {
	// TODO: Set sound volume
}

bool SoundManager::isPlaying() const {
	// TODO: Return whether a sound is playing
	return false;
}

void SoundManager::stopMusic() {
    if(_isMusicPlaying) {
        debug("Stopping music");
        _mixer->stopHandle(_musicHandle);
        _isMusicPlaying = false;
    }
}

void SoundManager::playMusicTrack(int trackNumber) {
    if(_currentMusicTrack == trackNumber && _isMusicPlaying) {
        // Already playing this track
        return;
    }
    _currentMusicTrack = trackNumber;
	stopMusic();
	// Open the file
	_musicFile = new Common::File();
	Common::String filename = Common::String::format("music/track%d.mp3", trackNumber);

	if (!_musicFile->open(Common::Path(filename))) {
		delete _musicFile;
        _musicFile = nullptr;
        return;
	}
#ifdef USE_MAD
	Audio::SeekableAudioStream *stream = Audio::makeMP3Stream(_musicFile, DisposeAfterUse::YES);
	if (!stream) {
		_musicFile->close();
        delete _musicFile;
        _musicFile = nullptr;
		return;
	}
	Audio::AudioStream *loopStream = Audio::makeLoopingAudioStream(stream, 0);
	_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, loopStream, -1, _currentVolume);
    _isMusicPlaying = true;
    _musicFile = nullptr;
#endif
}
} // End of namespace Pelrock
