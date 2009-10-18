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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"

#include "saga/saga.h"
#include "saga/sound.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/adpcm.h"

namespace Saga {

Sound::Sound(SagaEngine *vm, Audio::Mixer *mixer) :
	_vm(vm), _mixer(mixer), _voxStream(0) {

	for (int i = 0; i < SOUND_HANDLES; i++)
		_handles[i].type = kFreeHandle;

	setVolume();
}

Sound::~Sound() {
	delete _voxStream;
}

SndHandle *Sound::getHandle() {
	for (int i = 0; i < SOUND_HANDLES; i++) {
		if (_handles[i].type == kFreeHandle)
			return &_handles[i];

		if (!_mixer->isSoundHandleActive(_handles[i].handle)) {
			_handles[i].type = kFreeHandle;
			return &_handles[i];
		}
	}

	error("Sound::getHandle(): Too many sound handles");

	return NULL;	// for compilers that don't support NORETURN
}

void Sound::playSoundBuffer(Audio::SoundHandle *handle, SoundBuffer &buffer, int volume,
				sndHandleType handleType, bool loop) {

	buffer.flags |= Audio::Mixer::FLAG_AUTOFREE;

	if (loop)
		buffer.flags |= Audio::Mixer::FLAG_LOOP;

	Audio::Mixer::SoundType soundType = (handleType == kVoiceHandle) ? 
				Audio::Mixer::kSpeechSoundType : Audio::Mixer::kSFXSoundType;

	if (!buffer.isCompressed) {
		_mixer->playRaw(soundType, handle, buffer.buffer,
				buffer.size, buffer.frequency, buffer.flags, -1, volume);
	} else {
		Audio::AudioStream *stream = 0;

		switch (buffer.soundType) {
#ifdef USE_MAD
			case kSoundMP3:
				stream = Audio::makeMP3Stream(new Common::MemoryReadStream(buffer.buffer, buffer.size, Common::DisposeAfterUse::YES), true);
				break;
#endif
#ifdef USE_VORBIS
			case kSoundOGG:
				stream = Audio::makeVorbisStream(new Common::MemoryReadStream(buffer.buffer, buffer.size, Common::DisposeAfterUse::YES), true);
				break;
#endif
#ifdef USE_FLAC
			case kSoundFLAC:
				stream = Audio::makeFlacStream(new Common::MemoryReadStream(buffer.buffer, buffer.size, Common::DisposeAfterUse::YES), true);
				break;
#endif
			default:
				// Unknown compression, ignore sample
				warning("Unknown compression, ignoring sound");
				break;
		}

		if (stream != NULL)
			_mixer->playInputStream(soundType, handle, stream, -1, volume, 0, true, false);
	}
}

void Sound::playSound(SoundBuffer &buffer, int volume, bool loop) {
	SndHandle *handle = getHandle();

	handle->type = kEffectHandle;
	playSoundBuffer(&handle->handle, buffer, 2 * volume, handle->type, loop);
}

void Sound::pauseSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle)
			_mixer->pauseHandle(_handles[i].handle, true);
}

void Sound::resumeSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle)
			_mixer->pauseHandle(_handles[i].handle, false);
}

void Sound::stopSound() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kEffectHandle) {
			_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
		}
}

void Sound::playVoice(SoundBuffer &buffer) {
	SndHandle *handle = getHandle();

	handle->type = kVoiceHandle;
	playSoundBuffer(&handle->handle, buffer, 255, handle->type, false);
}

void Sound::pauseVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle)
			_mixer->pauseHandle(_handles[i].handle, true);
}

void Sound::resumeVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle)
			_mixer->pauseHandle(_handles[i].handle, false);
}

void Sound::stopVoice() {
	for (int i = 0; i < SOUND_HANDLES; i++)
		if (_handles[i].type == kVoiceHandle) {
			_mixer->stopHandle(_handles[i].handle);
			_handles[i].type = kFreeHandle;
		}
}

void Sound::stopAll() {
	stopVoice();
	stopSound();
}

void Sound::setVolume() {
	_vm->_soundVolume = ConfMan.getInt("sfx_volume");
	_vm->_speechVolume = ConfMan.getInt("speech_volume");
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, _vm->_soundVolume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, _vm->_speechVolume);
}

} // End of namespace Saga
