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

#include "colony/sound.h"
#include "colony/colony.h"

namespace Colony {

Sound::Sound(ColonyEngine *vm) : _vm(vm) {
	_speaker = new Audio::PCSpeaker();
	_speaker->init();
}

Sound::~Sound() {
	delete _speaker;
}

void Sound::stop() {
	if (_speaker->isPlaying())
		_speaker->stop();
}

void Sound::play(int soundID) {
	if (_speaker->isPlaying())
		_speaker->stop();
	playPCSpeaker(soundID);
}

void Sound::playPCSpeaker(int soundID) {
	const uint32 tickUs = 54945; // ~18.2 Hz (PIT 1193181 / 65536)

	auto queueTick = [&](uint32 divider, uint32 ticks) {
		if (divider == 0)
			_speaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 0, ticks * tickUs);
		else
			_speaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, 1193180.0f / divider, ticks * tickUs);
	};

	switch (soundID) {
	case kShoot: // "Them"
		queueTick(8000, 10);
		for (int i = 0; i < 10; i++) {
			queueTick(8000, 1);
			queueTick(0, 1);
		}
		break;
	case kDoor: // "Door1"
		queueTick(1000, 7);
		queueTick(2000, 3);
		break;
	case kBang: // "Ugh"
	{
		uint32 div = 400;
		for (int i = 0; i < 30; i++) {
			queueTick(div, 1);
			div += 134;
		}
		break;
	}
	case kOuch:
	case kBonk:
		queueTick(4649, 3);
		break;
	case kKlaxon:
		for (int j = 0; j < 4; j++) {
			uint32 div = 4649;
			while (div > 4000) {
				queueTick(div, 1);
				div -= 64;
			}
		}
		break;
	case kChime:
		queueTick(4649, 7);
		queueTick(3690, 7);
		queueTick(3103, 7);
		break;
	case kEat:
		queueTick(0x8888, 2);
		queueTick(0x7777, 2);
		queueTick(0x6666, 2);
		queueTick(0x5555, 2);
		queueTick(0x4444, 2);
		queueTick(0x3333, 2);
		queueTick(0x2222, 2);
		queueTick(0x1111, 2);
		queueTick(1536, 2); // 0600h
		break;
	case kExplode: // "Explode2"
	{
		uint32 div = 1280;
		queueTick(div, 1);
		for (int i = 0; i < 48; i++) {
			div -= 2;
			queueTick(div, 1);
		}
		div = 36864;
		queueTick(div, 1);
		for (int i = 0; i < 80; i++) {
			queueTick(div, 1);
		}
		break;
	}
	case kAirlock:
		queueTick(57344, 150);
		queueTick(512, 2);
		queueTick(1024, 1);
		break;
	case kElevator:
		for (int i = 0; i < 8; i++) {
			queueTick(512, 2);
			queueTick(1024, 75);
		}
		break;
	case kTeleport:
	{
		uint32 div = 400;
		queueTick(div, 1);
		for (int i = 0; i < 100; i++) {
			div += 134;
			queueTick(div, 1);
		}
		break;
	}
	case kLift:
	{
		uint32 div = 4649;
		queueTick(div, 1);
		while (div > 3103) {
			div -= 8;
			queueTick(div, 1);
		}
		break;
	}
	case kDrop:
	{
		uint32 div = 3103;
		queueTick(div, 1);
		while (div < 4649) {
			div += 8;
			queueTick(div, 1);
		}
		break;
	}
	case kGlass:
		for (int j = 0; j < 20; j++) {
			uint32 div = 60000;
			queueTick(div, 1);
			while (div > 50000) {
				div -= 1000;
				queueTick(div, 1);
			}
			queueTick(0, 10);
			queueTick(div, 1);
		}
		break;
	case kSink:
		for (int j = 0; j < 1; j++) {
			uint32 div = 200;
			queueTick(div, 1);
			while (div < 1600) {
				div += 200;
				queueTick(div, 1);
			}
		}
		break;
	case kDit:
		queueTick(210, 3);
		queueTick(9210, 2);
		queueTick(3000, 3);
		queueTick(1000, 2);
		queueTick(40000, 3);
		break;
	case kStars1:
	case kStars2:
	case kStars3:
	case kStars4:
		queueTick(4000, 2);
		queueTick(8000, 2);
		queueTick(2000, 2);
		queueTick(6000, 2);
		break;
	case kToilet: // "Sailor's Hornpipe"
		queueTick(2651, 4); // G
		queueTick(1985, 4); // C
		queueTick(1985, 2); // C
		queueTick(1985, 4); // C
		queueTick(1768, 2); // D
		queueTick(1575, 4); // E
		queueTick(1768, 4); // D
		queueTick(1985, 4); // C
		break;
	case kBath: // "Rubber Ducky"
		queueTick(1985, 4); // C
		queueTick(1575, 4); // E
		queueTick(1324, 4); // G
		queueTick(1182, 8); // A
		break;
	default:
		break;
	}
}

} // End of namespace Colony
