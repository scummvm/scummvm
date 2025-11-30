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
#include "audio/softsynth/ay8912.h"

#include "freescape/freescape.h"
#include "freescape/games/eclipse/eclipse.h"

namespace Freescape {

void FreescapeEngine::loadSpeakerFxZX(Common::SeekableReadStream *file, int sfxTable, int sfxData) {
	debugC(1, kFreescapeDebugParser, "Reading sound table for ZX");
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
				debugC(1, kFreescapeDebugParser, "Raw sound, repetitions: %x", repetitions);
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
					soundUnit.rawFreq = 0.1f;
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

void FreescapeEngine::loadSpeakerFxDOS(Common::SeekableReadStream *file, int offsetFreq, int offsetTable, int numberSounds) {
	debugC(1, kFreescapeDebugParser, "Reading PC speaker sound table for DOS");
	for (int i = 1; i <= numberSounds; i++) {
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

void FreescapeEngine::playSound(int index, bool sync, Audio::SoundHandle &handle) {
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
			playSoundDOS(speakerFxInfo, sync, handle);
		else
			debugC(1, kFreescapeDebugMedia, "WARNING: Sound %d is not available", index);

		return;
	} else if (isSpectrum()) {
		if (isDriller())
			playSoundDrillerZX(index, handle);
		else
			playSoundZX(_soundsSpeakerFxZX[index], handle);
		return;
	} else if (isCPC()) {
		if (isDriller())
			playSoundDrillerCPC(index, handle);
		// else playSoundCPC(...)
		return;
	}

	Common::Path filename;
	filename = Common::String::format("%s-%d.wav", _targetName.c_str(), index);
	debugC(1,  kFreescapeDebugMedia, "Playing sound %s", filename.toString().c_str());
	playWav(filename);
	_syncSound = sync;
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
	if (_usePrerecordedSounds || isAmiga() || isAtariST())
		while (_mixer->isSoundHandleActive(_soundFxHandle))
			waitInLoop(10);
	else {
		while (!_speaker->endOfStream())
			waitInLoop(10);
	}
}

bool FreescapeEngine::isPlayingSound() {
	if (_usePrerecordedSounds || isAmiga() || isAtariST())
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

uint16 FreescapeEngine::playSoundDOSSpeaker(uint16 frequencyStart, soundSpeakerFx *speakerFxInfo) {
	uint8 frequencyStepsNumber = speakerFxInfo->frequencyStepsNumber;
	int16 frequencyStep = speakerFxInfo->frequencyStep;
	uint8 frequencyDuration = speakerFxInfo->frequencyDuration;

	int16 freq = frequencyStart;
	int waveDurationMultipler = 1800;
	int waveDuration = waveDurationMultipler * (frequencyDuration + 1);

	while (true) {
		if (freq > 0) {
			float hzFreq = 1193180.0 / freq;
			debugC(1, kFreescapeDebugMedia, "raw %d, hz: %f, duration: %d", freq, hzFreq, waveDuration);
			_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, hzFreq, waveDuration);
		}
		if (frequencyStepsNumber > 0) {
			// Ascending initial portions of cycle
			freq += frequencyStep;
			frequencyStepsNumber--;
		} else
			break;
	}

	return freq;
}

void FreescapeEngine::playSoundZX(Common::Array<soundUnitZX> *data, Audio::SoundHandle &handle) {
	for (auto &it : *data) {
		soundUnitZX value = it;

		if (value.isRaw) {
			debugC(1, kFreescapeDebugMedia, "raw hz: %f, duration: %d", value.rawFreq, value.rawLengthus);
			if (value.rawFreq == 0) {
				_speaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 1, 5 * value.rawLengthus);
				continue;
			}
			_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, value.rawFreq, 5 * value.rawLengthus);
		} else {
			if (value.freqTimesSeconds == 0 && value.tStates == 0) {
				_speaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 1, 1000 * value.multiplier);
				continue;
			}

			float hzFreq = 1 / ((value.tStates + 30.125) / 437500.0);
			float waveDuration = value.freqTimesSeconds / hzFreq;
			waveDuration = value.multiplier * 1000 * (waveDuration + 1);
			debugC(1, kFreescapeDebugMedia, "non raw hz: %f, duration: %f", hzFreq, waveDuration);
			_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, hzFreq, waveDuration);
		}
	}

	_mixer->stopHandle(_soundFxHandle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, _speaker, -1, kFreescapeDefaultVolume, 0, DisposeAfterUse::NO);
}

void FreescapeEngine::playSoundDOS(soundSpeakerFx *speakerFxInfo, bool sync, Audio::SoundHandle &handle) {
	uint freq = speakerFxInfo->frequencyStart;

	for (int i = 0; i < speakerFxInfo->repetitions; i++) {
		freq = playSoundDOSSpeaker(freq, speakerFxInfo);

		for (auto &it : speakerFxInfo->additionalSteps) {
			assert(it);
			freq = playSoundDOSSpeaker(freq, it);
		}
	}

	_mixer->stopHandle(handle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, _speaker, -1, kFreescapeDefaultVolume / 2, 0, DisposeAfterUse::NO);
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

void FreescapeEngine::playSoundDrillerZX(int index, Audio::SoundHandle &handle) {
	debugC(1, kFreescapeDebugMedia, "Playing Driller ZX sound %d", index);
	Common::Array<soundUnitZX> soundUnits;

	auto addTone = [&](uint16 hl, uint16 de, float multiplier) {
		soundUnitZX s;
		s.isRaw = false;
		s.tStates = hl; // HL determines period
		s.freqTimesSeconds = de; // DE determines duration (number of cycles)
		s.multiplier = multiplier;
		soundUnits.push_back(s);
	};

	// Linear Sweep: Period increases -> Pitch decreases
	auto addSweep = [&](uint16 startHl, uint16 endHl, uint16 step, uint16 duration) {
		for (uint16 hl = startHl; hl < endHl; hl += step) {
			addTone(hl, duration, 10.0f);
		}
	};

	// Zap effect: Decreasing Period (E decrements) -> Pitch increases
	auto addZap = [&](uint16 startE, uint16 endE, uint16 duration) {
		for (uint16 e = startE; e > endE; e--) {
			// Map E (delay loops) to HL (tStates)
			// Small E -> Short Period -> High Freq
			uint16 hl = (24 + e) * 4;
			addTone(hl, duration, 10.0f);
		}
	};

	// Sweep Down: Increasing Period (E increments) -> Pitch decreases
	auto addSweepDown = [&](uint16 startE, uint16 endE, uint16 step, uint16 duration, float multiplier) {
		for (uint16 e = startE; e < endE; e += step) {
			uint16 hl = (24 + e) * 4;
			addTone(hl, duration, multiplier);
		}
	};

	switch (index) {
	case 1: // Shoot (FUN_95A1 -> 95AF)
		// Laser: High Pitch -> Low Pitch
		// Adjusted pitch to be even lower (0x200-0x600 is approx 850Hz-280Hz)
		addSweepDown(0x200, 0x600, 20, 1, 2.0f);
		break;
	case 2: // Collide/Bump (FUN_95DE)
		// Low tone sequence
		addTone(0x93c, 0x40, 10.0f); // 64 cycles ~340ms
		addTone(0x7a6, 0x30, 10.0f); // 48 cycles
		break;
	case 3: // Step (FUN_95E5)
		// Short blip
		// Increased duration significantly again (0xC0 = 192 cycles)
		addTone(0x7a6, 0xC0, 10.0f);
		break;
	case 4: // Silence (FUN_95F7)
		break;
	case 5: // Area Change? (FUN_95F8)
		addTone(0x1f0, 0x60, 10.0f); // High pitch, longer
		break;
	case 6: // Menu (Silence?) (FUN_9601)
		break;
	case 7: // Hit? (Sweep FUN_9605)
		// Sweep down (Period increases)
		addSweep(0x200, 0xC00, 64, 2);
		break;
	case 8: // Zap (FUN_961F)
		// Zap: Low -> High
		addZap(0xFF, 0x10, 2);
		break;
	case 9: // Sweep (FUN_9673)
		addSweep(0x100, 0x600, 16, 4);
		break;
	case 10: // Area Change (FUN_9696)
		addSweep(0x100, 0x500, 16, 4);
		break;
	case 11: // Explosion (FUN_96B9)
		{
			soundUnitZX s;
			s.isRaw = true;
			s.rawFreq = 0.0f; // Noise
			s.rawLengthus = 100000; // 100ms noise
			soundUnits.push_back(s);
		}
		break;
	case 12: // Sweep Down (FUN_96E4)
		addSweepDown(0x01, 0xFF, 1, 2, 10.0f);
		break;
	case 13: // Fall? (FUN_96FD)
		addSweep(300, 800, 16, 2);
		break;
	default:
		debugC(1, kFreescapeDebugMedia, "Unknown Driller ZX sound %d", index);
		break;
	}

	playSoundZX(&soundUnits, handle);
}

class DrillerCPCSfxStream : public Audio::AudioStream {
public:
	DrillerCPCSfxStream(int index, int rate = 44100) : _ay(rate, 1000000), _index(index), _rate(rate) { // 1MHz for CPC AY
		// Initialize sound chip (silence)
		initAY();

		_counter = 0;
		_finished = false;
	}

	void initAY() {
		// Silence all channels
		_ay.setReg(7, 0xFF); // Disable all tones and noise
		_ay.setReg(8, 0);    // Volume A 0
		_ay.setReg(9, 0);    // Volume B 0
		_ay.setReg(10, 0);   // Volume C 0
	}

	int readBuffer(int16 *buffer, const int numSamples) override {
		if (_finished)
			return 0;

		// We need to generate samples from AY
		// And update the AY state periodically

		// Simulate 50Hz updates
		int samplesPerTick = _rate / 50;
		int samplesGenerated = 0;

		while (samplesGenerated < numSamples && !_finished) {
			int samplesTodo = MIN(numSamples - samplesGenerated, samplesPerTick);

			// Update AY state (simulate FUN_4760 tick)
			updateState();

			// Generate audio
			_ay.readBuffer(buffer + samplesGenerated, samplesTodo);
			samplesGenerated += samplesTodo;

			if (_finished) break;
		}

		return samplesGenerated;
	}

	bool isStereo() const override { return true; }
	bool endOfData() const override { return _finished; }
	bool endOfStream() const override { return _finished; }
	int getRate() const override { return _rate; }

private:
	Audio::AY8912Stream _ay;
	int _index;
	int _rate;
	int _counter;
	bool _finished;

	void updateState() {
		// Simulation of Driller CPC sound effects
		// Based on analysis of FUN_4581 and FUN_4760
		// CPC AY-3-8912 Clock is 1MHz

		_counter++;

		switch (_index) {
		case 1: // Shoot
			// Related to FUN_43E2 (0x43E2) and FUN_5A21 (0x5A21)
			// The shoot sound logic involves setting up a channel structure and updating it.
			// FUN_4760 checks 0x3a64 bit 4 and calls FUN_43E2 three times (3 channels?).

			// 0x5A21: LD A,(IX+0x5); ADD A,(IX+0x1); AND 0xf; LD (IX+0x5),A
			// This suggests a volume or parameter update loop.

			// Implementation: Fast frequency sweep down with decay.
			// Replicating the "Laser" effect likely produced by the hardware envelope or software sweep.

			if (_counter > 12) {
				_finished = true;
			} else {
				// Sweep logic
				// Start Frequency High (Period Low) -> End Frequency Low (Period High)
				int period = 20 + _counter * 30;

				// Apply to Channel A (Reg 0, 1)
				_ay.setReg(0, period & 0xff);
				_ay.setReg(1, (period >> 8) & 0xf);

				// Apply to Channel B (Reg 2, 3) - Detuned for thickness
				_ay.setReg(2, (period + 5) & 0xff);
				_ay.setReg(3, ((period + 5) >> 8) & 0xf);

				// Volume Decay (Software Envelope)
				// 0x5A21 suggests updating IX+0x5 (Volume?)
				int vol = 15 - _counter;
				if (vol < 0) vol = 0;

				_ay.setReg(8, vol);
				_ay.setReg(9, vol);

				// Enable Tone A and B (Reg 7)
				// 0x3C = 0011 1100 (Enable Tone A(0), B(1))
				// Noise disabled for cleaner "zap"
				_ay.setReg(7, 0x3C);
			}
			break;
		case 2: // Bump
			// Low pitch noise/tone
			if (_counter > 5) {
				_finished = true;
			} else {
				_ay.setReg(0, 0xA0);
				_ay.setReg(1, 0x05); // Period 0x5A0 (~44Hz)
				_ay.setReg(8, 15);
				_ay.setReg(7, 0x3E); // Tone A
			}
			break;
		case 10: // Area Change / Start (Elevator Bell)
		case 21: // Bell logic from FUN_4581 (0x15)
			// Related to FUN_4581 case 0x15 which swaps registers 0x39b9/a and 0x39bb/c
			// This indicates an alternating two-tone effect.

			if (_counter > 50) {
				_finished = true;
			} else {
				// Alternating tones (Ding-Dong / Trill)
				// Tone 1: High (e.g., C5)
				// Tone 2: Slightly lower (e.g., A4)
				int pitch = ((_counter / 3) % 2 == 0) ? 119 : 142;

				_ay.setReg(0, pitch & 0xff);
				_ay.setReg(1, (pitch >> 8) & 0xf);

				// Volume Decay
				int vol = 15 - (_counter / 3);
				if (vol < 0) vol = 0;

				_ay.setReg(8, vol);
				_ay.setReg(7, 0x3E); // Enable Tone A
			}
			break;
		case 13: // Mission Complete
			// Fanfare?
			if (_counter > 100) _finished = true;
			// TODO: Implement fanfare
			break;
		default:
			// For unknown sounds, play nothing for now
			_finished = true;
			break;
		}
	}
};

void FreescapeEngine::playSoundDrillerCPC(int index, Audio::SoundHandle &handle) {
	debugC(1, kFreescapeDebugMedia, "Playing Driller CPC sound %d", index);
	// Create a new stream for the sound
	DrillerCPCSfxStream *stream = new DrillerCPCSfxStream(index);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, stream, -1, kFreescapeDefaultVolume, 0, DisposeAfterUse::YES);
}

} // namespace Freescape
