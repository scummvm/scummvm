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

#include "efh/efh.h"

namespace Efh {

void EfhEngine::songDelay(int delay) {
	debugC(3, kDebugEngine, "songDelay %d", delay);

	int remainingDelay = delay / 2;
	while (remainingDelay > 0 && !shouldQuit()) {
		remainingDelay -= 3;
		_system->delayMillis(3);
	}
}

void EfhEngine::playNote(int frequencyIndex, int totalDelay) {
	debugC(3, kDebugEngine, "playNote %d %d", frequencyIndex, totalDelay);
	_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, 0x1234DD / kSoundFrequency[frequencyIndex], -1);
	songDelay(totalDelay);
	_speakerStream->stop();
}

Common::KeyCode EfhEngine::playSong(uint8 *buffer) {
	debugC(3, kDebugEngine, "playSong");

	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
					   _speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	Common::KeyCode inputChar = Common::KEYCODE_INVALID;
	int totalDelay = 0;

	int8 stopFl;
	uint8 varC = *buffer++;
	Common::Event event;
	do {
		stopFl = *buffer & 0x3F;
		if (stopFl != 0) {
			int delay = stopFl * varC * 0x2200 / 1000;

			if (*buffer > 0x7F)
				delay /= 2;

			if (*buffer & 0x40)
				delay = (delay * 2) / 3;

			++buffer;
			uint8 frequencyIndex = *buffer;
			++buffer;

			if (frequencyIndex > 0x7F)
				totalDelay += delay;
			else if (frequencyIndex == 0)
				songDelay(delay);
			else {
				playNote(frequencyIndex, totalDelay + delay);
				totalDelay = 0;
			}
		}

		songDelay(10);
		_system->getEventManager()->pollEvent(event);
		if (event.type == Common::EVENT_KEYUP) {
			inputChar = event.kbd.keycode;
			// Hack, sometimes there's a ghost event after the 2nd note
			if (inputChar == Common::KEYCODE_ESCAPE || inputChar == Common::KEYCODE_RETURN)
				stopFl = 0;
		}
	} while (stopFl != 0);

	_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
	_speakerStream = nullptr;

	return inputChar;
}

void EfhEngine::generateSound1(int lowFreq, int highFreq, int duration) {
	debugC(3, kDebugEngine, "generateSound1 %d %d %d - suspicious code", lowFreq, highFreq, duration);

	if (lowFreq < 19)
		lowFreq = 19;

	if (highFreq < 19)
		highFreq = 19;

	uint16 var2 = 0;
	duration /= 20;

	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
					   _speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, highFreq, -1);
	songDelay(10);
	_speakerStream->stop();		


	for (int i = 0; i < duration; ++i) {
		var2 = ROR(var2 + 0x9248, 3);
		int val = (var2 * (highFreq - lowFreq)) >> 16;
		
		_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, lowFreq + val, -1);
		songDelay(10);
		_speakerStream->stop();		
	}
	

	_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
	_speakerStream = nullptr;
}

void EfhEngine::generateSound2(int startFreq, int endFreq, int speed) {
	debugC(3, kDebugEngine, "generateSound2 %d %d %d", startFreq, endFreq, speed);

	if (startFreq < 19)
		startFreq = 19;

	if (endFreq < 19)
		endFreq = 19;

	int delta;
	// The original is using -/+1 but it takes ages even with speed / 10, so I switched to -/+5
	if (startFreq > endFreq)
		delta = -5;
	else
		delta = 5;

	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
					   _speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	int curFreq = startFreq;

	do {
		_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, curFreq, -1);
		// The original is just looping, making the sound improperly timed as the length of a loop is directly related to the speed of the CPU
		// Dividing by 10 is just a guess based on how it sounds. I suspect it may be still too much
		songDelay(speed / 10);
		_speakerStream->stop();
		curFreq += delta;
	} while (curFreq < endFreq && !shouldQuit());
	

	_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
	_speakerStream = nullptr;

}

void EfhEngine::generateSound3() {
	debugC(3, kDebugEngine, "generateSound3");
	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
					   _speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, 88, -1);
	// The original makes me think the delay is so short it's not possible to hear. So that delay is guessed (and short)
	songDelay(30);
	_speakerStream->stop();

	_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
	_speakerStream = nullptr;
}

void EfhEngine::generateSound4(int repeat) {
	debugC(3, kDebugEngine, "generateSound4 %d", repeat);
	for (int i = 0; i < repeat; ++i)
		//It looks identical, so I'm reusing generateSound1
		generateSound1(256, 4096, 10);
}

void EfhEngine::generateSound5(int repeat) {
	debugC(3, kDebugEngine, "generateSound5 %d", repeat);
	for (int i = 0; i < repeat; ++i)
		//It looks identical, so I'm reusing generateSound2
		generateSound2(256, 4096, 10);
}

void EfhEngine::generateSound(int16 soundType) {
	debugC(3, kDebugEngine, "generateSound %d", soundType);

	switch (soundType) {
	case 5:
		generateSound3();
		break;
	case 9:
		generateSound1(20, 888, 3000);
		generateSound1(20, 888, 3000);
		break;
	case 10:
		generateSound5(1);
		break;
	case 13:
		generateSound2(256, 4096, 18);
		break;
	case 14:
		generateSound2(20, 400, 100);
		break;
	case 15:
		generateSound2(100, 888, 88);
		break;
	case 16:
		generateSound1(2000, 6096, 1500);
		break;
	case 17:
		generateSound4(1);
		break;
	default:
		debug("generateSound %d - Not implemented because not used by the engine", soundType);
		break;
	}
}

void EfhEngine::genericGenerateSound(int16 soundType, int16 repeatCount) {
	debugC(3, kDebugEngine, "genericGenerateSound %d %d", soundType, repeatCount);

	if (repeatCount <= 0)
		return;

	switch (soundType) {
	case 0:
	case 1:
	case 2:
		generateSound(5);
		break;
	case 3:
	case 4:
	case 6:
		generateSound(9);
		break;
	case 5:
	case 7:
		generateSound(13);
		break;
	case 8:
	case 9:
	case 10:
		generateSound(10);
		generateSound(9);
		break;
	case 14:
		generateSound(14);
		break;
	case 11:
	case 12:
	case 13:
		for (int counter = 0; counter < repeatCount; ++counter) {
			generateSound(17);
		}
		break;
	case 15:
		generateSound(16);
	default:
		break;
	}
}

} // End of namespace Efh

