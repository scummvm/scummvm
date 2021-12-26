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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "audio/decoders/wave.h"
#include "audio/decoders/raw.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "trecision/sound.h"
#include "trecision/trecision.h"
#include "trecision/defines.h"

namespace Trecision {

SoundManager::SoundManager(TrecisionEngine *vm) : _vm(vm) {
	if (!_speechFile.open(_vm, "nlspeech.cd0"))
		warning("SoundManager - nlspeech.cd0 is missing - skipping");

	_stepLeftStream = nullptr;
	_stepRightStream = nullptr;
}

SoundManager::~SoundManager() {
	g_system->getMixer()->stopAll();
	_speechFile.close();
	stopAll();
}

Audio::SeekableAudioStream *SoundManager::loadWAV(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	if (_vm->isAmiga()) {
		return Audio::makeRawStream(stream, 11025, 0, disposeAfterUse);
	} else {
		return Audio::makeWAVStream(stream, disposeAfterUse);
	}
}

void SoundManager::play(int soundId) {
	SRoom *curRoom = &_vm->_room[_vm->_curRoom];

	for (uint16 soundSlot = 0; soundSlot < MAXSOUNDSINROOM; soundSlot++) {
		if (curRoom->_sounds[soundSlot] == 0)
			return;

		if (curRoom->_sounds[soundSlot] == soundId) {
			const SoundType soundType = (_gSample[soundId]._flag & kSoundFlagBgMusic) ? kSoundTypeMusic : kSoundTypeSfx;
			Common::SeekableReadStream *soundFileStream = _vm->_dataFile.createReadStreamForMember(_gSample[soundId]._name);
			if (!soundFileStream)
				continue;

			// We need to copy this WAV to memory since it will be streamed
			Common::SeekableReadStream *memStream = soundFileStream->readStream(soundFileStream->size());
			delete soundFileStream;

			stopSoundType(soundType);

			Audio::Mixer::SoundType type =
				(_gSample[soundId]._flag & kSoundFlagBgMusic) ?
				Audio::Mixer::kMusicSoundType :
				Audio::Mixer::kSFXSoundType;

			int volume = VOLUME(_gSample[soundId]._volume);

			// FIXME: This looks wrong - it makes the room music silent
			/*if (_gSample[soundId]._flag & kSoundFlagSoundOn) {
				volume = 0;
			}*/

			Audio::AudioStream *stream = nullptr;

			if (_gSample[soundId]._flag & kSoundFlagSoundLoop)
				stream = Audio::makeLoopingAudioStream(loadWAV(memStream), 0);
			else
				stream = loadWAV(memStream);

			g_system->getMixer()->playStream(
				type,
				&_soundHandles[soundType],
				stream,
				-1,
				volume,
				0,
				DisposeAfterUse::YES
			);
		}
	}
}

void SoundManager::stopAll() {
	for (int i = 0; i < MAXSOUNDS; i++) {
		g_system->getMixer()->stopHandle(_soundHandles[i]);
	}

	delete _stepLeftStream;
	_stepLeftStream = nullptr;
	delete _stepRightStream;
	_stepRightStream = nullptr;
}

void SoundManager::stopAllExceptMusic() {
	for (int i = 0; i < MAXSOUNDS; i++) {
		if (i != kSoundTypeMusic) {
			g_system->getMixer()->stopHandle(_soundHandles[i]);
		}
	}

	delete _stepLeftStream;
	_stepLeftStream = nullptr;
	delete _stepRightStream;
	_stepRightStream = nullptr;
}

void SoundManager::stopSoundType(SoundType type) {
	if (g_system->getMixer()->isSoundHandleActive(_soundHandles[type])) {
		g_system->getMixer()->stopHandle(_soundHandles[type]);
	}
}

void SoundManager::soundStep(int midx, int midz, int act, int frame) {
	SRoom *curRoom = &_vm->_room[_vm->_curRoom];
	bool stepRight = false;
	bool stepLeft = false;

	switch (act) {
	case hWALK:
		if (frame == 3)
			stepLeft = true;
		else if (frame == 8)
			stepRight = true;
		break;

	case hWALKIN:
		if (frame == 3)
			stepLeft = true;
		else if (frame == 9)
			stepRight = true;
		break;

	case hWALKOUT:
		if (frame == 5)
			stepLeft = true;
		else if (frame == 10)
			stepRight = true;
		break;

	case hSTOP0:
	case hSTOP1:
	case hSTOP2:
	case hSTOP3:
	case hSTOP9:
		if (frame >= defActionLen[act] - 1)
			stepLeft = true;
		break;
	case hSTOP4:
	case hSTOP5:
	case hSTOP6:
	case hSTOP7:
	case hSTOP8:
		if (frame >= defActionLen[act] - 1)
			stepRight = true;
		break;
	default:
		break;
	}

	if (!stepRight && !stepLeft)
		return;

	int soundId;
	for (int soundSlot = 0; soundSlot < MAXSOUNDSINROOM; soundSlot++) {
		soundId = curRoom->_sounds[soundSlot];

		if (stepRight && (_gSample[soundId]._flag & kSoundFlagStepRight)) {
			if (!_stepRightStream) {
				Common::SeekableReadStream *soundFileStream = _vm->_dataFile.createReadStreamForMember(_gSample[soundId]._name);
				_stepRightStream = loadWAV(soundFileStream);
			}
			break;
		}

		if (stepLeft && (_gSample[soundId]._flag & kSoundFlagStepLeft)) {
			if (!_stepLeftStream) {
				Common::SeekableReadStream *soundFileStream = _vm->_dataFile.createReadStreamForMember(_gSample[soundId]._name);
				_stepLeftStream = loadWAV(soundFileStream);
			}
			break;
		}

		if (soundId == 0)
			return;
	}

	midz = ((int)(_gSample[soundId]._volume) * 1000) / ABS(midz);

	if (midz > 255)
		midz = 255;

	g_system->getMixer()->stopHandle(_soundHandles[kSoundTypeStep]);

	Audio::SeekableAudioStream *stream = stepLeft ? _stepLeftStream : _stepRightStream;
	stream->rewind();

	const int panpos = ((midx - 320) * 127 / 320) / 2;

	g_system->getMixer()->playStream(
		Audio::Mixer::kSFXSoundType,
		&_soundHandles[kSoundTypeStep],
		stream,
		-1,
		VOLUME(midz),
		panpos,
		DisposeAfterUse::NO
	);
}

int32 SoundManager::talkStart(const Common::String &name) {
	if (!_speechFile.isOpen())
		return 0;

	stopSoundType(kSoundTypeSpeech);

	Common::SeekableReadStream *stream = _speechFile.createReadStreamForMember(name);
	if (!stream)
		return 0;

	Audio::SeekableAudioStream *audioStream = loadWAV(stream);

	g_system->getMixer()->playStream(
		Audio::Mixer::kSpeechSoundType,
		&_soundHandles[kSoundTypeSpeech],
		audioStream,
		-1,
		Audio::Mixer::kMaxChannelVolume,	// TODO
		0,
		DisposeAfterUse::YES
	);
	_vm->_characterSpeakTime = _vm->readTime();

	return TIME(audioStream->getLength().msecs());
}

void SoundManager::loadRoomSounds() {
	SRoom *curRoom = &_vm->_room[_vm->_curRoom];

	stopAll();

	for (uint16 soundSlot = 0; soundSlot < MAXSOUNDSINROOM; soundSlot++) {
		const uint16 soundId = curRoom->_sounds[soundSlot];

		if (soundId == 0)
			break;

		if (_gSample[soundId]._name.equalsIgnoreCase("RUOTE2C.WAV"))
			break;

		if ((_gSample[soundId]._flag & kSoundFlagBgMusic) || (_gSample[soundId]._flag & kSoundFlagSoundOn))
			play(soundId);
	}
}

void SoundManager::loadSamples(Common::SeekableReadStreamEndian *stream) {
	for (int i = 0; i < NUMSAMPLES; ++i) {
		_gSample[i]._name = stream->readString(0, 14);
		_gSample[i]._volume = stream->readByte();
		_gSample[i]._flag = stream->readByte();
		_gSample[i]._panning = stream->readSByte();
	}
}

} // End of namespace Trecision
