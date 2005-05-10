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

Sound::Sound(SagaEngine *vm, Audio::Mixer *mixer, int enabled) : 
	_vm(vm), _mixer(mixer), _enabled(enabled), _voxStream(0) {

	_soundInitialized = 1;
	return;
}

Sound::~Sound() {
	if (!_soundInitialized) {
		return;
	}

	delete _voxStream;
	_soundInitialized = 0;
}

int Sound::playSoundBuffer(SoundHandle *handle, SOUNDBUFFER *buf, int volume, bool loop) {
	byte flags;

	if (!_soundInitialized) {
		return FAILURE;
	}

	flags = Audio::Mixer::FLAG_AUTOFREE;

	if (loop)
		flags |= Audio::Mixer::FLAG_LOOP;

	if (buf->s_samplebits == 16) {
		flags |= Audio::Mixer::FLAG_16BITS;
		if (!(_vm->getFeatures() & GF_BIG_ENDIAN_DATA))
			flags |= Audio::Mixer::FLAG_LITTLE_ENDIAN;
	}
	if (buf->s_stereo)
		flags |= Audio::Mixer::FLAG_STEREO;
	if (!buf->s_signed)
		flags |= Audio::Mixer::FLAG_UNSIGNED;

	_mixer->playRaw(handle, buf->s_buf, buf->s_buf_len, buf->s_freq, flags, -1, volume);

	return SUCCESS;
}

int Sound::playSound(SOUNDBUFFER *buf, int volume, bool loop) {
	return playSoundBuffer(&_effectHandle, buf, 2 * volume, loop);
}

int Sound::pauseSound() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->pauseHandle(_effectHandle, true);

	return SUCCESS;
}

int Sound::resumeSound() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->pauseHandle(_effectHandle, false);

	return SUCCESS;
}

int Sound::stopSound() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->stopHandle(_effectHandle);

	return SUCCESS;
}

int Sound::playVoice(SOUNDBUFFER *buf) {
	return playSoundBuffer(&_voiceHandle, buf, 255, false);
}

int Sound::playVoxVoice(SOUNDBUFFER *buf) {
	AudioStream *audioStream;

	if (_voxStream)
		delete _voxStream;

	_voxStream = new Common::MemoryReadStream(buf->s_buf, buf->s_buf_len);

	audioStream = makeADPCMStream(*_voxStream, buf->s_buf_len, kADPCMOki);
	_mixer->playInputStream(Audio::Mixer::kSFXSoundType, &_voiceHandle, audioStream);

	return SUCCESS;
}

int Sound::pauseVoice() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->pauseHandle(_voiceHandle, true);

	return SUCCESS;
}

int Sound::resumeVoice() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->pauseHandle(_voiceHandle, false);

	return SUCCESS;
}

int Sound::stopVoice() {
	if (!_soundInitialized) {
		return FAILURE;
	}

	_mixer->stopHandle(_voiceHandle);

	return SUCCESS;
}

} // End of namespace Saga
