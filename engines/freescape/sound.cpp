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

#include "freescape/freescape.h"

namespace Freescape {

void FreescapeEngine::loadSpeakerFx(Common::SeekableReadStream *file, int offsetFreq, int offsetTable) {
	for (int i = 1; i < 20; i++) {
		debugC(1, kFreescapeDebugParser, "Reading sound table entry: %d ", i);
		int soundIdx = (i - 1) * 4;
		file->seek(offsetFreq + soundIdx);
		uint16 index = file->readByte();
		if (index == 0xff)
			continue;
		uint iVar = index * 5;

		uint16 frequencyStart = file->readUint16LE();
		uint8 repetitions = file->readByte();
		debugC(1, kFreescapeDebugParser, "Frequency start: %d ", frequencyStart);
		debugC(1, kFreescapeDebugParser, "Repetitions: %d ", repetitions);

		uint8 frequencyStepsNumber = 0;
		uint16 frequencyStep = 0;

		file->seek(offsetTable + iVar);
		uint8 lastIndex = file->readByte();
		debugC(1, kFreescapeDebugParser, "0x%x %d (lastIndex)", offsetTable - 0x200, lastIndex);

		frequencyStepsNumber = file->readByte();
		debugC(1, kFreescapeDebugParser, "0x%x %d (frequency steps)", offsetTable + 1 - 0x200, frequencyStepsNumber);

		int basePtr = offsetTable + iVar + 1;
		debugC(1, kFreescapeDebugParser, "0x%x (basePtr)", basePtr - 0x200);

		frequencyStep = file->readUint16LE();
		debugC(1, kFreescapeDebugParser, "0x%x %d (steps number)", offsetTable + 2 - 0x200, (int16)frequencyStep);

		uint8 frequencyDuration = file->readByte();
		debugC(1, kFreescapeDebugParser, "0x%x %d (frequency duration)", offsetTable + 4 - 0x200, frequencyDuration);

		soundSpeakerFx *speakerFxInfo = new soundSpeakerFx();
		_soundsSpeakerFx[i] = speakerFxInfo;

		speakerFxInfo->frequencyStart = frequencyStart;
		speakerFxInfo->repetitions = repetitions;
		speakerFxInfo->frequencyStepsNumber = frequencyStepsNumber;
		speakerFxInfo->frequencyStep = frequencyStep;
		speakerFxInfo->frequencyDuration = frequencyDuration;

		for (int j = 1; j < lastIndex; j++) {

			soundSpeakerFx *speakerFxInfoAdditionalStep = new soundSpeakerFx();
			speakerFxInfoAdditionalStep->frequencyStart = 0;
			speakerFxInfoAdditionalStep->repetitions = 0;

			file->seek(basePtr + 4 * j);
			debugC(1, kFreescapeDebugParser, "Reading at %x", basePtr + 4 * j - 0x200);
			frequencyStepsNumber = file->readByte();
			debugC(1, kFreescapeDebugParser, "%d (steps number)", frequencyStepsNumber);
			frequencyStep = file->readUint16LE();
			debugC(1, kFreescapeDebugParser, "%d (frequency step)", (int16)frequencyStep);
			frequencyDuration = file->readByte();
			debugC(1, kFreescapeDebugParser, "%d (frequency duration)", frequencyDuration);

			speakerFxInfoAdditionalStep->frequencyStepsNumber = frequencyStepsNumber;
			speakerFxInfoAdditionalStep->frequencyStep = frequencyStep;
			speakerFxInfoAdditionalStep->frequencyDuration = frequencyDuration;
			speakerFxInfo->additionalSteps.push_back(speakerFxInfoAdditionalStep);
		}
		debugC(1, kFreescapeDebugParser, "\n");
	}
}

void FreescapeEngine::playSound(int index, bool sync) {
	debugC(1, kFreescapeDebugMedia, "Playing sound %d with sync: %d", index, sync);
	if (isAmiga() || isAtariST()) {
		playSoundFx(index, sync);
		_syncSound = sync;
		return;
	}
	if (_syncSound)
		waitForSounds();

	if (isDOS()) {
		soundSpeakerFx *speakerFxInfo = _soundsSpeakerFx[index];
		if (speakerFxInfo)
			playSoundDOS(speakerFxInfo, sync);
		else
			debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d is not available", index);

		return;
	}

	switch (index) {
	case 1:
		playWav("fsDOS_laserFire.wav");
		break;
	case 2: // Done
		playWav("fsDOS_WallBump.wav");
		break;
	case 3:
		playWav("fsDOS_stairDown.wav");
		break;
	case 4:
		playWav("fsDOS_stairUp.wav");
		break;
	case 5:
		playWav("fsDOS_roomChange.wav");
		break;
	case 6:
		playWav("fsDOS_configMenu.wav");
		break;
	case 7:
		playWav("fsDOS_bigHit.wav");
		break;
	case 8:
		playWav("fsDOS_teleporterActivated.wav");
		break;
	case 9:
		playWav("fsDOS_powerUp.wav");
		break;
	case 10:
		playWav("fsDOS_energyDrain.wav");
		break;
	case 11: // ???
		debugC(1, kFreescapeDebugMedia, "Playing unknown sound");
		break;
	case 12:
		playWav("fsDOS_switchOff.wav");
		break;
	case 13: // Seems to be repeated?
		playWav("fsDOS_laserHit.wav");
		break;
	case 14:
		playWav("fsDOS_tankFall.wav");
		break;
	case 15:
		playWav("fsDOS_successJingle.wav");
		break;
	case 16: // Silence?
		break;
	case 17:
		playWav("fsDOS_badJingle.wav");
		break;
	case 18: // Silence?
		break;
	case 19:
		debugC(1, kFreescapeDebugMedia, "Playing unknown sound");
		break;
	case 20:
		playWav("fsDOS_bigHit.wav");
		break;
	default:
		debugC(1, kFreescapeDebugMedia, "Unexpected sound %d", index);
		break;
	}
	_syncSound = sync;
}
void FreescapeEngine::playWav(const Common::Path &filename) {

	Common::SeekableReadStream *s = _dataBundle->createReadStreamForMember(filename);
	assert(s);
	Audio::AudioStream *stream = Audio::makeWAVStream(s, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream);
}

void FreescapeEngine::playMusic(const Common::Path &filename) {
	Audio::SeekableAudioStream *stream = nullptr;
	stream = Audio::SeekableAudioStream::openStreamFile(filename);
	if (stream) {
		Audio::LoopingAudioStream *loop = new Audio::LoopingAudioStream(stream, 0);
		_mixer->playStream(Audio::Mixer::kMusicSoundType, &_musicHandle, loop);
		_mixer->setVolumeForSoundType(Audio::Mixer::kMusicSoundType, Audio::Mixer::kMaxChannelVolume / 10);
	}
}

void FreescapeEngine::playSoundFx(int index, bool sync) {
	if (_soundsFx.size() == 0) {
		debugC(1, kFreescapeDebugMedia, "WARNING: Sounds are not loaded");
		return;
	}

	int size = _soundsFx[index]->size;
	int sampleRate = _soundsFx[index]->sampleRate;
	byte *data = _soundsFx[index]->data;
	int loops = 1;

	if (index == 10)
		loops = 5;
	else if (index == 15)
		loops = 50;

	if (size > 4) {
		Audio::SeekableAudioStream *s = Audio::makeRawStream(data, size, sampleRate, Audio::FLAG_16BITS, DisposeAfterUse::NO);
		Audio::AudioStream *stream = new Audio::LoopingAudioStream(s, loops);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, stream);
	}
}

void FreescapeEngine::stopAllSounds() {
	_speaker->stop();
	_mixer->stopHandle(_soundFxHandle);
}

void FreescapeEngine::waitForSounds() {
	if (_usePrerecordedSounds || isAmiga() || isAtariST())
		while (_mixer->isSoundIDActive(-1))
			g_system->delayMillis(10);
	else {
		while (!_speaker->endOfStream())
			g_system->delayMillis(10);
	}
}

bool FreescapeEngine::isPlayingSound() {
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

uint16 FreescapeEngine::playSoundDOSSpeaker(uint16 frequencyStart, soundSpeakerFx *speakerFxInfo) {
	uint8 frequencyStepsNumber = speakerFxInfo->frequencyStepsNumber;
	int16 frequencyStep = speakerFxInfo->frequencyStep;
	uint8 frequencyDuration = speakerFxInfo->frequencyDuration;

	int16 freq = frequencyStart;
	int waveDurationMultipler = 1800;
	int waveDuration = waveDurationMultipler * (frequencyDuration + 1);

	while (true) {
		float hzFreq = 1193180.0 / freq;
		debugC(1, kFreescapeDebugMedia, "raw %d, hz: %f, duration: %d", freq, hzFreq, waveDuration);
		_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, hzFreq, waveDuration);
		if (frequencyStepsNumber > 0) {
			// Ascending initial portions of cycle
			freq += frequencyStep;
			frequencyStepsNumber--;
		} else
			break;
	}

	return freq;
}

void FreescapeEngine::playSoundDOS(soundSpeakerFx *speakerFxInfo, bool sync) {
	uint freq = speakerFxInfo->frequencyStart;

	for (int i = 0; i < speakerFxInfo->repetitions; i++) {
		freq = playSoundDOSSpeaker(freq, speakerFxInfo);

		for (auto &it : speakerFxInfo->additionalSteps) {
			assert(it);
			freq = playSoundDOSSpeaker(freq, it);
		}
	}

	_mixer->stopHandle(_soundFxHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, _speaker, -1, Audio::Mixer::kMaxChannelVolume / 8, 0, DisposeAfterUse::NO);
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
		int sampleRate = file->readUint16BE();
		debugC(1, kFreescapeDebugParser, "Loading sound: %d (size: %d, sample rate: %d)", i, size, sampleRate);
		byte *data = (byte *)malloc(size * sizeof(byte));
		file->read(data, size);
		sound->sampleRate = sampleRate;
		sound->size = size;
		sound->data = (byte *)data;
		_soundsFx[i] = sound;
	}
}

} // namespace Freescape
