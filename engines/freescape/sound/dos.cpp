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

namespace Freescape {

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
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, _speaker, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO);
}

} // namespace Freescape
