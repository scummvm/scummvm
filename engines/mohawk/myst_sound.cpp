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

#include "common/debug.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"

namespace Mohawk {

MystSound::MystSound(MohawkEngine_Myst *vm) :
		_vm(vm) {
	_mystBackgroundSound.type = kFreeHandle;
}

MystSound::~MystSound() {
	stopSound();
	stopBackgroundMyst();
}

Audio::RewindableAudioStream *MystSound::makeAudioStream(uint16 id, CueList *cueList) {
	if (_vm->getFeatures() & GF_ME)
		return Audio::makeWAVStream(_vm->getResource(ID_MSND, convertMystID(id)), DisposeAfterUse::YES);
	else
		return makeMohawkWaveStream(_vm->getResource(ID_MSND, id), cueList);
}

Audio::SoundHandle *MystSound::playSound(uint16 id, byte volume, bool loop, CueList *cueList) {
	debug (0, "Playing sound %d", id);

	Audio::RewindableAudioStream *rewindStream = makeAudioStream(id, cueList);

	if (rewindStream) {
		SndHandle *handle = getHandle();
		handle->type = kUsedHandle;
		handle->id = id;
		handle->samplesPerSecond = rewindStream->getRate();

		// Set the stream to loop here if it's requested
		Audio::AudioStream *audStream = rewindStream;
		if (loop)
			audStream = Audio::makeLoopingAudioStream(rewindStream, 0);

		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &handle->handle, audStream, -1, volume);
		return &handle->handle;
	}

	return nullptr;
}

Audio::SoundHandle *MystSound::replaceSoundMyst(uint16 id, byte volume, bool loop) {
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
	}

	stopSound();
	return playSound(id, volume, loop);
}

SndHandle *MystSound::getHandle() {
	for (uint32 i = 0; i < _handles.size(); i++) {
		if (_handles[i].type == kFreeHandle)
			return &_handles[i];

		if (!_vm->_mixer->isSoundHandleActive(_handles[i].handle)) {
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
			return &_handles[i];
		}
	}

	// Let's add a new sound handle!
	SndHandle handle;
	handle.handle = Audio::SoundHandle();
	handle.type = kFreeHandle;
	handle.id = 0;
	_handles.push_back(handle);

	return &_handles[_handles.size() - 1];
}

void MystSound::stopSound() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle) {
			_vm->_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
		}
}

void MystSound::stopSound(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id) {
			_vm->_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
			_handles[i].id = 0;
		}
}

bool MystSound::isPlaying(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id)
			return _vm->_mixer->isSoundHandleActive(_handles[i].handle);

	return false;
}

bool MystSound::isPlaying() {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle)
			if (_vm->_mixer->isSoundHandleActive(_handles[i].handle))
				return true;

	return false;
}

uint MystSound::getNumSamplesPlayed(uint16 id) {
	for (uint32 i = 0; i < _handles.size(); i++)
		if (_handles[i].type == kUsedHandle && _handles[i].id == id) {
			return (_vm->_mixer->getSoundElapsedTime(_handles[i].handle) * _handles[i].samplesPerSecond) / 1000;
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

void MystSound::replaceBackgroundMyst(uint16 id, uint16 volume) {
	debug(0, "Replacing background sound with %d", id);

	// TODO: The original engine does fading

	Common::String name = _vm->getResourceName(ID_MSND, convertMystID(id));

	// Only the first eight characters need to be the same to have a match
	Common::String prefix;
	if (name.size() >= 8)
		prefix = Common::String(name.c_str(), name.c_str() + 8);
	else
		prefix = name;

	// Check if sound is already playing
	if (_mystBackgroundSound.type == kUsedHandle && _vm->_mixer->isSoundHandleActive(_mystBackgroundSound.handle)
			&& _vm->getResourceName(ID_MSND, convertMystID(_mystBackgroundSound.id)).hasPrefix(prefix)) {
		// The sound is already playing, just change the volume
		changeBackgroundVolumeMyst(volume);
		return;
	}

	// Stop old background sound
	stopBackgroundMyst();

	// Play new sound
	Audio::RewindableAudioStream *rewindStream = makeAudioStream(id);

	if (rewindStream) {
		_mystBackgroundSound.type = kUsedHandle;
		_mystBackgroundSound.id = id;
		_mystBackgroundSound.samplesPerSecond = rewindStream->getRate();

		// Set the stream to loop
		Audio::AudioStream *audStream = Audio::makeLoopingAudioStream(rewindStream, 0);

		_vm->_mixer->playStream(Audio::Mixer::kPlainSoundType, &_mystBackgroundSound.handle, audStream, -1, volume >> 8);
	}
}

void MystSound::stopBackgroundMyst() {
	if (_mystBackgroundSound.type == kUsedHandle) {
		_vm->_mixer->stopHandle(_mystBackgroundSound.handle);
		_mystBackgroundSound.type = kFreeHandle;
		_mystBackgroundSound.id = 0;
	}
}

void MystSound::pauseBackgroundMyst() {
	if (_mystBackgroundSound.type == kUsedHandle)
		_vm->_mixer->pauseHandle(_mystBackgroundSound.handle, true);
}

void MystSound::resumeBackgroundMyst() {
	if (_mystBackgroundSound.type == kUsedHandle)
		_vm->_mixer->pauseHandle(_mystBackgroundSound.handle, false);
}

void MystSound::changeBackgroundVolumeMyst(uint16 vol) {
	if (_mystBackgroundSound.type == kUsedHandle)
		_vm->_mixer->setChannelVolume(_mystBackgroundSound.handle, vol >> 8);
}

} // End of namespace Mohawk
