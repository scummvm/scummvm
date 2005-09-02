/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "saga/saga.h"

#include "saga/sound.h"

#include "sound/audiostream.h"
#include "sound/mixer.h"
#include "sound/adpcm.h"

namespace Saga {

Sound::Sound(SagaEngine *vm, Audio::Mixer *mixer, int volume) :
	_vm(vm), _mixer(mixer), _voxStream(0) {

	for (int i = 0; i < SOUND_HANDLES; i++)
		_handles[i].type = kFreeHandle;

	setVolume(volume == 10 ? 255 : volume * 25);
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

	return NULL;
}

void Sound::playSoundBuffer(Audio::SoundHandle *handle, SoundBuffer &buffer, int volume, bool loop) {
	byte flags;

	flags = Audio::Mixer::FLAG_AUTOFREE;

	if (loop)
		flags |= Audio::Mixer::FLAG_LOOP;

	if (buffer.sampleBits == 16) {
		flags |= Audio::Mixer::FLAG_16BITS;

		if (!buffer.isBigEndian)
			flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
	}
	if (buffer.stereo)
		flags |= Audio::Mixer::FLAG_STEREO;
	if (!buffer.isSigned)
		flags |= Audio::Mixer::FLAG_UNSIGNED;

	_mixer->playRaw(handle, buffer.buffer, buffer.size, buffer.frequency, flags, -1, volume);
}

void Sound::playSound(SoundBuffer &buffer, int volume, bool loop) {
	SndHandle *handle = getHandle();

	handle->type = kEffectHandle;
	playSoundBuffer(&handle->handle, buffer, 2 * volume, loop);
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
	playSoundBuffer(&handle->handle, buffer, 255, false);
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

void Sound::setVolume(int volume) {
	_mixer->setVolumeForSoundType(Audio::Mixer::kSFXSoundType, volume);
	_mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume);
}

} // End of namespace Saga
