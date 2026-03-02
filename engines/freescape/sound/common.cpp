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
#include "audio/decoders/raw.h"
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
	if (isC64()) {
		playSoundC64(index);
		return;
	}

	if (isAmiga() || isAtariST()) {
		playSoundFx(index, sync);
		return;
	}

	if (isDOS()) {
		soundSpeakerFx *speakerFxInfo = _soundsSpeakerFx[index];
		if (speakerFxInfo)
			playSoundDOS(speakerFxInfo, sync, handle);
		else
			debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d is not available", index);

		return;
	} else if (isSpectrum()) {
		playSoundZX(index, handle);
		return;
	} else if (isCPC()) {
		playSoundCPC(index, handle);
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

void FreescapeEngine::playSoundFx(int index, bool sync) {
	if (!_amigaSfxTable.empty()) {
		playSoundAmiga(index, _soundFxHandle);
		return;
	}

	if (_soundsFx.size() == 0) {
		debugC(1, kFreescapeDebugMedia, "WARNING: Sounds are not loaded");
		return;
	}

	if (index < 0 || index >= int(_soundsFx.size())) {
		debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d not available", index);
		return;
	}

	int size = _soundsFx[index]->size;
	int sampleRate = _soundsFx[index]->sampleRate;
	int repetitions = _soundsFx[index]->repetitions;
	byte *data = _soundsFx[index]->data;

	if (size > 4) {
		Audio::SeekableAudioStream *s = Audio::makeRawStream(data, size, sampleRate, Audio::FLAG_16BITS, DisposeAfterUse::NO);
		Audio::AudioStream *stream = new Audio::LoopingAudioStream(s, repetitions);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream);
	} else
		debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d is empty", index);
}

void FreescapeEngine::stopAllSounds(Audio::SoundHandle &handle) {
	debugC(1, kFreescapeDebugMedia, "Stopping sound");
	_mixer->stopHandle(handle);
}

void FreescapeEngine::waitForSounds() {
	if (_usePrerecordedSounds || isAmiga() || isAtariST() || isCPC())
		while (_mixer->isSoundHandleActive(_soundFxHandle))
			waitInLoop(10);
	else {
		while (!_speaker->endOfStream())
			waitInLoop(10);
	}
}

bool FreescapeEngine::isPlayingSound() {
	if (_usePrerecordedSounds || isAmiga() || isAtariST() || isCPC())
		return _mixer->isSoundHandleActive(_soundFxHandle);

	return (!_speaker->endOfStream());
}

void FreescapeEngine::playSilence(int duration, bool sync) {
	_speaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 0, 1000 * 10 * duration);
	_mixer->stopHandle(_soundFxHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, _speaker, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

void FreescapeEngine::queueSoundConst(double hzFreq, int duration) {
	_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, hzFreq, 1000 * 10 * duration);
}

void FreescapeEngine::loadSoundsFx(Common::SeekableReadStream *file, int offset, int number) {
	file->seek(offset);
	soundFx *sound = nullptr;
	_soundsFx[0] = sound;
	for (int i = 1; i < number + 1; i++) {
		sound = (soundFx *)malloc(sizeof(soundFx));
		int zero = file->readUint16BE();
		assert(zero == 0);
		int size = file->readUint16BE();
		float sampleRate = float(file->readUint16BE()) / 2;
		debugC(1, kFreescapeDebugParser, "Loading sound: %d (size: %d, sample rate: %f) at %" PRIx64, i, size, sampleRate, file->pos());
		byte *data = (byte *)malloc(size * sizeof(byte));
		file->read(data, size);
		sound->sampleRate = sampleRate;
		sound->size = size;
		sound->data = (byte *)data;
		sound->repetitions = 1;
		_soundsFx[i] = sound;
	}
}

} // namespace Freescape
