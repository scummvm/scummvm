/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "mohawk/myst_sound.h"

#include "mohawk/myst.h"
#include "mohawk/resource.h"
#include "mohawk/sound.h"

#include "common/debug.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

namespace Mohawk {

MystSound::MystSound(MohawkEngine_Myst *vm) :
		_vm(vm),
		_effectId(0),
		_speechSamplesPerSecond(0),
		_backgroundId(0) {
}

MystSound::~MystSound() {
	stopEffect();
	stopBackground();
}

Audio::RewindableAudioStream *MystSound::makeAudioStream(uint16 id, CueList *cueList) {
	if (_vm->isGameVariant(GF_ME))
		return Audio::makeWAVStream(_vm->getResource(ID_MSND, convertMystID(id)), DisposeAfterUse::YES);
	else
		return makeMohawkWaveStream(_vm->getResource(ID_MSND, id), cueList);
}

void MystSound::playEffect(uint16 id, bool loop) {
	debug (0, "Replacing sound %d", id);

	// The original engine also forces looping for those sounds
	switch (id) {
		case 2205:
		case 2207:
		case 5378:
		case 7220:
		case 9119: 	// Elevator engine sound in mechanical age is looping.
		case 9120:
		case 9327:
			loop = true;
			break;
		default:
			break;
	}

	stopEffect();

	Audio::RewindableAudioStream *rewindStream = makeAudioStream(id);
	if (!rewindStream) {
		warning("Unable to open sound '%d'", id);
		return;
	}

	_effectId = id;

	// Set the stream to loop here if it's requested
	Audio::AudioStream *audStream = rewindStream;
	if (loop)
		audStream = Audio::makeLoopingAudioStream(rewindStream, 0);

	_vm->_mixer->playStream(Audio::Mixer::kSFXSoundType, &_effectHandle, audStream);
}

void MystSound::stopEffect() {
	_vm->_mixer->stopHandle(_effectHandle);
	_effectId = 0;
	_effectHandle = Audio::SoundHandle();
}

bool MystSound::isEffectPlaying() {
	return _vm->_mixer->isSoundHandleActive(_effectHandle);
}

uint MystSound::getSpeechNumSamplesPlayed() {
	if (isSpeechPlaying()) {
		return (_vm->_mixer->getSoundElapsedTime(_speechHandle) * _speechSamplesPerSecond) / 1000;
	}

	return 0;
}

uint16 MystSound::convertMystID(uint16 id) {
	// Myst ME is a bit more efficient with sound storage than Myst
	// Myst has lots of sounds repeated. To overcome this, Myst ME
	// has MJMP resources which provide a link to the actual MSND
	// resource we're looking for. This saves a lot of space from
	// repeated data.
	if (_vm->hasResource(ID_MJMP, id)) {
		Common::SeekableReadStream *mjmpStream = _vm->getResource(ID_MJMP, id);
		id = mjmpStream->readUint16LE();
		delete mjmpStream;
	}

	return id;
}

void MystSound::playBackground(uint16 id, uint16 volume) {
	debug(0, "Replacing background sound with %d", id);

	stopEffect();

	Common::String name = _vm->getResourceName(ID_MSND, convertMystID(id));

	// Only the first eight characters need to be the same to have a match
	Common::String prefix;
	if (name.size() >= 8)
		prefix = Common::String(name.c_str(), name.c_str() + 8);
	else
		prefix = name;

	// Check if sound is already playing
	if (_vm->_mixer->isSoundHandleActive(_backgroundHandle)
			&& _vm->getResourceName(ID_MSND, convertMystID(_backgroundId)).hasPrefix(prefix)) {
		// The sound is already playing, just change the volume
		changeBackgroundVolume(volume);
		return;
	}

	// Stop old background sound
	stopBackground();

	// Play new sound
	Audio::RewindableAudioStream *rewindStream = makeAudioStream(id);

	if (rewindStream) {
		_backgroundId = id;

		// Set the stream to loop
		Audio::AudioStream *audStream = Audio::makeLoopingAudioStream(rewindStream, 0);

		_vm->_mixer->playStream(Audio::Mixer::kMusicSoundType, &_backgroundHandle, audStream, -1, volume >> 8);
	}
}

void MystSound::stopBackground() {
	_vm->_mixer->stopHandle(_backgroundHandle);
	_backgroundId = 0;
	_backgroundHandle = Audio::SoundHandle();
}

void MystSound::pauseBackground() {
	_vm->_mixer->pauseHandle(_backgroundHandle, true);
}

void MystSound::resumeBackground() {
	stopEffect();
	_vm->_mixer->pauseHandle(_backgroundHandle, false);
}

void MystSound::changeBackgroundVolume(uint16 volume) {
	_vm->_mixer->setChannelVolume(_backgroundHandle, volume >> 8);
}

void MystSound::playSpeech(uint16 id, CueList *cueList) {
	debug (0, "Playing speech %d", id);

	Audio::RewindableAudioStream *rewindStream = makeAudioStream(id, cueList);
	if (!rewindStream) {
		warning("Unable to open sound '%d'", id);
		return;
	}

	_speechSamplesPerSecond = rewindStream->getRate();
	_vm->_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, rewindStream);
}

bool MystSound::isSpeechPlaying() {
	return _vm->_mixer->isSoundHandleActive(_speechHandle);
}

void MystSound::stopSpeech() {
	_vm->_mixer->stopHandle(_speechHandle);
	_speechHandle = Audio::SoundHandle();
	_speechSamplesPerSecond = 0;
}

} // End of namespace Mohawk
