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
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, _speaker, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

void FreescapeEngine::playSoundZX(int index, Audio::SoundHandle &handle) {
	playSoundZX(_soundsSpeakerFxZX[index], handle);
}

} // namespace Freescape
