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

#include "common/file.h"
#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

#include "freescape/freescape.h"

namespace Freescape {

void FreescapeEngine::playSound(int index, bool sync, Audio::SoundHandle &handle) {
	if (index < 0) {
		debugC(1, kFreescapeDebugMedia, "Sound not specified");
		return;
	}

	if (_syncSound)
		waitForSounds();

	_syncSound = sync;

	debugC(1, kFreescapeDebugMedia, "Playing sound %d with sync: %d", index, sync);
	if (_sound) {
		_sound->playSound(index);
		return;
	}

	if (isC64()) {
		playSoundC64(index);
		return;
	}

	Common::Path filename;
	filename = Common::String::format("%s-%d.wav", _targetName.c_str(), index);
	debugC(1,  kFreescapeDebugMedia, "Playing sound %s", filename.toString().c_str());
	playWav(filename);
	_syncSound = sync;
}
void FreescapeEngine::playSoundC64(int index) {
	debugC(1, kFreescapeDebugMedia, "C64 sound %d not implemented for this engine", index);
}

void FreescapeEngine::playWav(const Common::Path &filename) {

	Common::SeekableReadStream *s = _dataBundle->createReadStreamForMember(filename);
	if (!s) {
		debugC(1, kFreescapeDebugMedia, "WARNING: Sound %s not found", filename.toString().c_str());
		return;
	}
	Audio::AudioStream *stream = Audio::makeWAVStream(s, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream);
}

void FreescapeEngine::playMusic(const Common::Path &filename) {
	Audio::SeekableAudioStream *stream = nullptr;
	stream = Audio::SeekableAudioStream::openStreamFile(filename);
	if (stream) {
		_mixer->stopHandle(_musicHandle);
		Audio::LoopingAudioStream *loop = new Audio::LoopingAudioStream(stream, 0);
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, loop);
	}
}

void FreescapeEngine::stopAllSounds(Audio::SoundHandle &handle) {
	debugC(1, kFreescapeDebugMedia, "Stopping sound");
	if (_sound)
		_sound->stopSound();
	else
		_mixer->stopHandle(handle);
}

void FreescapeEngine::waitForSounds() {
	while (isPlayingSound())
		waitInLoop(10);
}

bool FreescapeEngine::isPlayingSound() {
	if (_sound)
		return _sound->isPlayingSound();

	if (_usePrerecordedSounds)
		return _mixer->isSoundHandleActive(_soundFxHandle);

	return false;
}

} // namespace Freescape
