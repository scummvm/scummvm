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
#include "freescape/games/eclipse/eclipse.h"

namespace Freescape {

void FreescapeEngine::loadSpeakerFxZX(Common::SeekableReadStream *file, int sfxTable, int sfxData) {
	int numberSounds = 25;

	if (isDark())
		numberSounds = 34;

	if (isEclipse() && (_variant & GF_ZX_DEMO_MICROHOBBY))
		numberSounds = 21;

	for (int i = 1; i < numberSounds; i++) {
		debugC(1, kFreescapeDebugParser, "Reading sound table entry: %d ", i);
		_soundsSpeakerFxZX[i] = new Common::Array<soundUnitZX>();
		int soundIdx = (i - 1) * 4;
		file->seek(sfxTable + soundIdx);

		byte SFXtempStruct[8] = {};

		uint8 dataIndex = file->readByte();
		uint16 soundValue = file->readUint16LE();
		SFXtempStruct[0] = file->readByte();

		file->seek(sfxData + dataIndex * 4);
		uint8 soundType = file->readByte();
		int original_sound_ptr = sfxData + dataIndex * 4 + 1;
		int sound_ptr = original_sound_ptr;
		uint8 soundSize = 0;
		int16 repetitions = 0;
		debugC(1, kFreescapeDebugParser, "dataIndex: %x, value: %x, SFXtempStruct[0]: %x, type: %x", dataIndex, soundValue, SFXtempStruct[0], soundType);
		if (soundType == 0xff)
			break;

		if ((soundType & 0x80) == 0) {
			SFXtempStruct[6] = 0;
			SFXtempStruct[4] = soundType;

			while (true) {
				while (true) {
					file->seek(sound_ptr);
					//debug("start sound ptr: %x", sound_ptr);
					soundSize = file->readByte();
					SFXtempStruct[1] = soundSize;
					SFXtempStruct[2] = file->readByte();
					SFXtempStruct[3] = file->readByte();

					for (int j = 0; j <= 7; j++)
						debugC(1, kFreescapeDebugParser, "SFXtempStruct[%d]: %x", j, SFXtempStruct[j]);

					do {
						uint32 var9 = 0xffffff & (SFXtempStruct[3] * 0xd0);
						uint32 var10 = var9 / soundValue;

						var9 = 0xffffff & (7 * soundValue);
						uint16 var5 = (0xffff & var9) - 0x1e;
						if ((short)var5 < 0)
							var5 = 1;

						soundUnitZX soundUnit;
						soundUnit.isRaw = false;
						soundUnit.freqTimesSeconds = (var10 & 0xffff) + 1;
						soundUnit.tStates = var5;
						soundUnit.multiplier = 10;
						//debug("playSFX(%x, %x)", soundUnit.freqTimesSeconds, soundUnit.tStates);
						_soundsSpeakerFxZX[i]->push_back(soundUnit);
						int16 var4 = 0;

						if ((SFXtempStruct[2] & 0x80) != 0) {
							var4 = 0xff;
						}
						//debug("var4: %d", var4);
						//debug("soundValue delta: %d", int16(((var4 << 8) | SFXtempStruct[2])));
						soundValue = soundValue + int16(((var4 << 8) | SFXtempStruct[2]));
						//debug("soundValue: %x", soundValue);
						soundSize = soundSize - 1;
					} while (soundSize != 0);
					SFXtempStruct[5] = SFXtempStruct[5] + 1;
					if (SFXtempStruct[5] == SFXtempStruct[4])
						break;

					sound_ptr = original_sound_ptr + SFXtempStruct[5] * 3;
					//debug("sound ptr: %x", sound_ptr);
				}

				soundSize = SFXtempStruct[0];
				SFXtempStruct[0] = soundSize - 1;
				sound_ptr = original_sound_ptr;
				if ((soundSize - 1) == 0)
					break;
				SFXtempStruct[5] = 0;
			}
		} else if (soundType & 0x80) {
			file->seek(sound_ptr);
			for (int j = 1; j <= 7; j++) {
				SFXtempStruct[j] = file->readByte();
				debugC(1, kFreescapeDebugParser, "SFXtempStruct[%d]: %x", j, SFXtempStruct[j]);
			}
			soundSize = SFXtempStruct[0];
			repetitions = SFXtempStruct[1] | (SFXtempStruct[2] << 8);
			uint16 var5 = soundValue;
			//debug("Repetitions: %x", repetitions);
			if ((soundType & 0x7f) == 1) {
				do  {
					do {
						soundUnitZX soundUnit;
						soundUnit.isRaw = false;
						soundUnit.tStates = var5;
						soundUnit.freqTimesSeconds = SFXtempStruct[3] | (SFXtempStruct[4] << 8);
						soundUnit.multiplier = 1.8f;
						//debug("playSFX(%x, %x)", soundUnit.freqTimesSeconds, soundUnit.tStates);
						_soundsSpeakerFxZX[i]->push_back(soundUnit);
						repetitions = repetitions - 1;
						var5 = var5 + (SFXtempStruct[5] | (SFXtempStruct[6] << 8));

					} while ((byte)((byte)repetitions | (byte)((uint16)repetitions >> 8)) != 0);
					soundSize = soundSize - 1;
					repetitions = SFXtempStruct[1] | (SFXtempStruct[2] << 8);
					var5 = soundValue;
				} while (soundSize != 0);
			} else if ((soundType & 0x7f) == 2) {
				repetitions = SFXtempStruct[1] | (SFXtempStruct[0] << 8);
				debugC(1, kFreescapeDebugParser, "Repetitions: %x", repetitions);
				uint16 sVar7 = SFXtempStruct[3];
				soundType = 0;
				soundSize = SFXtempStruct[2];
				uint16 silenceSize = SFXtempStruct[4];
				bool cond1 = (SFXtempStruct[4] != 0 && SFXtempStruct[4] != 2);
				bool cond2 = SFXtempStruct[4] == 2;
				bool cond3 = SFXtempStruct[4] == 0;

				assert(cond1 || cond2 || cond3);
				do {
					soundUnitZX soundUnit;
					soundUnit.isRaw = true;
					int totalSize = soundSize + sVar7;
					soundUnit.rawFreq = 1.0;
					soundUnit.rawLengthus = totalSize;
					_soundsSpeakerFxZX[i]->push_back(soundUnit);
					//debugN("%x ", silenceSize);
					soundUnit.rawFreq = 0;
					soundUnit.rawLengthus = silenceSize;
					_soundsSpeakerFxZX[i]->push_back(soundUnit);
					repetitions = repetitions + -1;
					soundSize = SFXtempStruct[5] + soundSize;

					if (cond1)
						silenceSize = (repetitions & 0xff) | (repetitions >> 8);
					else if (cond2)
						silenceSize = (repetitions & 0xff);
					else
						silenceSize = soundSize;

					//debug("soundSize: %x", soundSize);
					//sVar7 = (uint16)bVar9 << 8;
				} while (repetitions != 0);
				//debug("\n");
				//if (i == 15)
				//	assert(0);
			} else {
				debugC(1, kFreescapeDebugParser, "Sound type: %x", soundType);
				bool beep = false;
				do {
					soundType = 0;
					uint16 uVar2 = SFXtempStruct[1] | (SFXtempStruct[2] << 8);
					uint8 cVar3 = 0;
					do {
						//debug("start cycle %d:", cVar3);
						//ULA_PORT = bVar4;
						//bVar4 = bVar4 ^ 0x10;
						beep = !beep;
						repetitions = (((uint16)soundType * 0x100 + (uint16)soundType * -2) -
									(uint16)((uint16)soundType * 0x100 < (uint16)soundType)) + (uVar2 & 0xff);
						uint8 bVar9 = (byte)repetitions;
						uint8 bVar8 = (byte)((uint16)repetitions >> 8);
						uint8 bVar1 = bVar9 - bVar8;
						soundType = bVar1;
						if (bVar8 <= bVar9) {
							bVar1 = bVar1 - 1;
							soundType = bVar1;
						}
						//debug("wait %d", bVar1);
						assert(bVar1 > 0);
						soundUnitZX soundUnit;
						soundUnit.isRaw = false;
						soundUnit.freqTimesSeconds = beep ? 1000 : 0;
						soundUnit.tStates = beep ? 437500 / 1000 - 30.125 : 0;
						soundUnit.multiplier = float(bVar1) / 500;
						_soundsSpeakerFxZX[i]->push_back(soundUnit);

						// No need to wait
						//do {
						//	bVar1 = bVar1 - 1;
						//} while (bVar1 != 0);
						cVar3 = (char)(uVar2 >> 8) + -1;
						uVar2 = (((uint16)cVar3) << 8) | (uint8)uVar2;
					} while (cVar3 != '\0');
					soundSize = soundSize + -1;
				} while (soundSize != '\0');
			}
		}
	}
	//assert(0);
}

void FreescapeEngine::loadSpeakerFxDOS(Common::SeekableReadStream *file, int offsetFreq, int offsetTable) {
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
	if (index < 0) {
		debugC(1, kFreescapeDebugMedia, "Sound not specified");
		return;
	}

	if (_syncSound)
		waitForSounds();

	_syncSound = sync;

	debugC(1, kFreescapeDebugMedia, "Playing sound %d with sync: %d", index, sync);
	if (isAmiga() || isAtariST()) {
		playSoundFx(index, sync);
		return;
	}

	if (isDOS()) {
		soundSpeakerFx *speakerFxInfo = _soundsSpeakerFx[index];
		if (speakerFxInfo)
			playSoundDOS(speakerFxInfo, sync);
		else
			debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d is not available", index);

		return;
	} else if (isSpectrum() && !isDriller()) {
		playSoundZX(_soundsSpeakerFxZX[index]);
		return;
	} else if (isCPC() && !isDriller()) {
		debugC(1, kFreescapeDebugMedia, "Not implemented");
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

	if (index < 0 || index >= int(_soundsFx.size())) {
		debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d not available", index);
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
	if (_usePrerecordedSounds || isAmiga() || isAtariST())
		return _mixer->isSoundIDActive(-1);

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

void FreescapeEngine::playSoundZX(Common::Array<soundUnitZX> *data) {
	for (auto &it : *data) {
		soundUnitZX value = it;

		if (value.isRaw) {
			debugC(1, kFreescapeDebugMedia, "hz: %f, duration: %d", value.rawFreq, value.rawLengthus);
			if (value.rawFreq == 0) {
				_speaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 0, value.rawLengthus);
				continue;
			}
			_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, value.rawFreq, value.rawLengthus);
		} else {
			if (value.freqTimesSeconds == 0 && value.tStates == 0) {
				_speaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 0, 1000 * value.multiplier);
				continue;
			}

			float hzFreq = 1 / ((value.tStates + 30.125) / 437500.0);
			float waveDuration = value.freqTimesSeconds / hzFreq;
			waveDuration = value.multiplier * 1000 * (waveDuration + 1);
			debugC(1, kFreescapeDebugMedia, "hz: %f, duration: %f", hzFreq, waveDuration);
			_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, hzFreq, waveDuration);
		}
	}

	_mixer->stopHandle(_soundFxHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundFxHandle, _speaker, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
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
		debugC(1, kFreescapeDebugParser, "Loading sound: %d (size: %d, sample rate: %d) at %" PRIx64, i, size, sampleRate, file->pos());
		byte *data = (byte *)malloc(size * sizeof(byte));
		file->read(data, size);
		sound->sampleRate = sampleRate;
		sound->size = size;
		sound->data = (byte *)data;
		_soundsFx[i] = sound;
	}
}

} // namespace Freescape
