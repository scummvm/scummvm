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
#include "common/stream.h"
#include "audio/audiostream.h"
#include "audio/decoders/raw.h"

namespace Colony {

Sound::Sound(ColonyEngine *vm) : _vm(vm) {
	_speaker = new Audio::PCSpeaker();
	_speaker->init();

	_resMan = new Common::MacResManager();
	if (!_resMan->open("Zounds")) {
		if (!_resMan->open("CData/Zounds")) {
			debug("Could not open Zounds resource file");
		}
	}
}

Sound::~Sound() {
	delete _speaker;
	delete _resMan;
}

void Sound::stop() {
	if (_speaker->isPlaying())
		_speaker->stop();
	if (_vm->_mixer->isSoundHandleActive(_handle))
		_vm->_mixer->stopHandle(_handle);
}

void Sound::play(int soundID) {
	stop();

	if (_vm->getPlatform() == Common::kPlatformMacintosh)
		playMacSound(soundID);
	else
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

bool Sound::playMacSound(int soundID) {
	int resID = -1;
	switch (soundID) {
	case kKlaxon: resID = 27317; break;
	case kAirlock: resID = 5141; break;
	case kOuch: resID = 9924; break;
	case kChime: resID = 24694; break;
	case kBang: resID = 24433; break;
	case kShoot: resID = 24010; break;
	case kEat: resID = 11783; break;
	case kBonk: resID = 7970; break;
	case kBzzz: resID = 11642; break;
	case kExplode: resID = 1432; break;
	case kElevator: resID = 12019; break;
	case kPShot: resID = 27539; break;
	case kTest: resID = 25795; break;
	case kDit: resID = 1516; break;
	case kSink: resID = 2920; break;
	case kClatter: resID = 11208; break;
	case kStop: resID = 29382; break;
	case kTeleport: resID = 9757; break;
	case kSlug: resID = 8347; break;
	case kTunnel2: resID = 17354; break;
	case kLift: resID = 28521; break;
	case kGlass: resID = 19944; break;
	case kDoor: resID = 26867; break;
	case kToilet: resID = 4955; break;
	case kBath: resID = 11589; break;
	case kMars: resID = 23390; break;
	case kBeamMe: resID = 5342; break;
	default: break;
	}

	if (resID != -1 && playResource(resID))
		return true;

	// Fallback to DOS sounds if Mac resource is missing
	playPCSpeaker(soundID);
	return false;
}

bool Sound::playResource(int resID) {
	if (!_resMan || !_resMan->isMacFile())
		return false;

	Common::SeekableReadStream *snd = _resMan->getResource(MKTAG('s', 'n', 'd', ' '), resID);
	if (!snd)
		return false;

	// The Mac source skips 42 bytes of the 'snd ' resource header to get to wave data.
	if (snd->size() <= 42) {
		delete snd;
		return false;
	}

	snd->seek(42);
	uint32 dataSize = snd->size() - 42;
	byte *data = (byte *)malloc(dataSize);
	snd->read(data, dataSize);
	delete snd;

	Audio::AudioStream *stream = Audio::makeRawStream(data, dataSize, 11127, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_handle, stream);
	return true;
}

} // End of namespace Colony
