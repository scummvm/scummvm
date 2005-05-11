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

// Sound class

#ifndef SAGA_SOUND_H_
#define SAGA_SOUND_H_

#include "saga/rscfile_mod.h"
#include "sound/mixer.h"

namespace Saga {

enum SOUND_FLAGS {
	SOUND_LOOP = 1
};

struct SOUNDBUFFER {
	uint16 s_freq;
	int s_samplebits;
	int s_stereo;
	int s_signed;

	byte *s_buf;
	size_t s_buf_len;
};

class Sound {
public:

	Sound(SagaEngine *vm, Audio::Mixer *mixer, int enabled);
	~Sound();

	int playSound(SOUNDBUFFER *buf, int volume, bool loop);
	int pauseSound();
	int resumeSound();
	int stopSound();

	int playVoice(SOUNDBUFFER *buf);
	int playVoxVoice(SOUNDBUFFER *buf);
	int pauseVoice();
	int resumeVoice();
	int stopVoice();

 private:

	int playSoundBuffer(Audio::SoundHandle *handle, SOUNDBUFFER *buf, int volume, bool loop);

	int _soundInitialized;
	int _enabled;

	SagaEngine *_vm;
	Audio::Mixer *_mixer;
	Common::MemoryReadStream *_voxStream;

	Audio::SoundHandle _effectHandle;
	Audio::SoundHandle _voiceHandle;

};

} // End of namespace Saga

#endif
