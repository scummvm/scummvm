/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
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
#include "saga/game_mod.h"

#include "sound/mixer.h"

namespace Saga {

Sound::Sound(SagaEngine *vm, SoundMixer *mixer, int enabled) : 
	_vm(vm), _mixer(mixer), _enabled(enabled) {

	_soundInitialized = 1;
	return;
}

Sound::~Sound() {
	if (!_soundInitialized) {
		return;
	}

	_soundInitialized = 0;
}

int Sound::playSoundBuffer(PlayingSoundHandle *handle, R_SOUNDBUFFER *buf, int volume, bool loop) {
	byte flags;

	if (!_soundInitialized) {
		return R_FAILURE;
	}

	flags = SoundMixer::FLAG_AUTOFREE;

	if (loop)
		flags |= SoundMixer::FLAG_LOOP;

	if (buf->s_samplebits == 16)
		flags |= (SoundMixer::FLAG_16BITS | SoundMixer::FLAG_LITTLE_ENDIAN);
	if (buf->s_stereo)
		flags |= SoundMixer::FLAG_STEREO;
	if (!buf->s_signed)
		flags |= SoundMixer::FLAG_UNSIGNED;

	// FIXME: Remove the code below if the code above works.

#if 0
	int game_id = GAME_GetGame();

	if((game_id == R_GAME_ITE_DISK) || (game_id == R_GAME_ITE_DEMO)) {
		flags = SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	} else {
		flags = SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_16BITS |
			SoundMixer::FLAG_LITTLE_ENDIAN;
	}
#endif

	_mixer->playRaw(handle, buf->s_buf, buf->s_buf_len, buf->s_freq, flags, -1, volume);

	return R_SUCCESS;
}

int Sound::playSound(R_SOUNDBUFFER *buf, int volume) {
	return playSoundBuffer(&_effectHandle, buf, 2 * volume, false);
}

int Sound::pauseSound() {
	if (!_soundInitialized) {
		return R_FAILURE;
	}

	_mixer->pauseHandle(_effectHandle, true);

	return R_SUCCESS;
}

int Sound::resumeSound() {
	if (!_soundInitialized) {
		return R_FAILURE;
	}

	_mixer->pauseHandle(_effectHandle, false);

	return R_SUCCESS;
}

int Sound::stopSound() {
	if (!_soundInitialized) {
		return R_FAILURE;
	}

	_mixer->stopHandle(_effectHandle);

	return R_SUCCESS;
}

int Sound::playVoice(R_SOUNDBUFFER *buf) {
	return playSoundBuffer(&_voiceHandle, buf, 255, false);
}

int Sound::pauseVoice() {
	if (!_soundInitialized) {
		return R_FAILURE;
	}

	_mixer->pauseHandle(_voiceHandle, true);

	return R_SUCCESS;
}

int Sound::resumeVoice() {
	if (!_soundInitialized) {
		return R_FAILURE;
	}

	_mixer->pauseHandle(_voiceHandle, false);

	return R_SUCCESS;
}

int Sound::stopVoice() {
	if (!_soundInitialized) {
		return R_FAILURE;
	}

	_mixer->stopHandle(_voiceHandle);

	return R_SUCCESS;
}

} // End of namespace Saga
