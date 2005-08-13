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
}

Sound::~Sound() {
	delete _voxStream;
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
	playSoundBuffer(&_effectHandle, buffer, 2 * volume, loop);
}

void Sound::pauseSound() {
	_mixer->pauseHandle(_effectHandle, true);
}

void Sound::resumeSound() {
	_mixer->pauseHandle(_effectHandle, false);
}

void Sound::stopSound() {
	_mixer->stopHandle(_effectHandle);
}

void Sound::playVoice(SoundBuffer &buffer) {
	playSoundBuffer(&_voiceHandle, buffer, 255, false);
}

void Sound::pauseVoice() {
	_mixer->pauseHandle(_voiceHandle, true);
}

void Sound::resumeVoice() {
	_mixer->pauseHandle(_voiceHandle, false);
}

void Sound::stopVoice() {
	_mixer->stopHandle(_voiceHandle);
}

void Sound::stopAll() {
	_mixer->stopAll();
}

} // End of namespace Saga
