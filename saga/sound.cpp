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
#include "saga.h"
#include "reinherit.h"

#include "yslib.h"

#include "sound.h"
#include "game_mod.h"

#include "sound/mixer.h"

namespace Saga {

Sound::Sound(SagaEngine *vm, SoundMixer *mixer, int enabled) : 
	_vm(vm), _mixer(mixer), _enabled(enabled) {
	int result;

	// Load sound module resource file contexts
	result = GAME_GetFileContext(&_soundContext, R_GAME_SOUNDFILE, 0);
	if (result != R_SUCCESS) {
		return;
	}

	result = GAME_GetFileContext(&_voiceContext, R_GAME_VOICEFILE, 0);
	if (result != R_SUCCESS) {
		return;
	}

	// Grab sound resource information for the current game
	GAME_GetSoundInfo(&_snd_info);

	_soundInitialized = 1;
	return;
}

Sound::~Sound() {
	if (!_soundInitialized) {
		return;
	}

	_soundInitialized = 0;
}

int Sound::play(int sound_rn, int channel) {
	if (!_soundInitialized) {
		return R_FAILURE;
	}

	if (channel > 3) {
		return R_FAILURE;
	}
	
	return R_SUCCESS;
}

int Sound::pause(int channel) {
	(void)channel;

	if (!_soundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int Sound::resume(int channel) {
	(void)channel;

	if (!_soundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int Sound::stop(int channel) {
	(void)channel;

	if (!_soundInitialized) {
		return R_FAILURE;
	}

	return R_SUCCESS;
}

int Sound::playVoice(R_SOUNDBUFFER *buf) {
	byte flags;
	int game_id = GAME_GetGame();

	if (!_soundInitialized) {
		return R_FAILURE;
	}

	if((game_id == R_GAME_ITE_DISK) || (game_id == R_GAME_ITE_DEMO)) {
		flags = SoundMixer::FLAG_UNSIGNED | SoundMixer::FLAG_AUTOFREE;
	} else {
		flags = SoundMixer::FLAG_AUTOFREE | SoundMixer::FLAG_16BITS |
			SoundMixer::FLAG_LITTLE_ENDIAN;
	}
	_mixer->playRaw(&_voiceHandle, buf->res_data, buf->res_len, buf->s_freq, flags);

	return R_SUCCESS;
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
