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

#include "sound/mixer.h"

namespace Saga {

enum SOUND_FLAGS {
	SOUND_LOOP = 1
};

struct SoundBuffer {
	uint16 frequency;
	int sampleBits;
	bool stereo;
	bool isSigned;
	
	byte *buffer;
	size_t size;
	bool isBigEndian;
};

class Sound {
public:

	Sound(SagaEngine *vm, Audio::Mixer *mixer, int enabled);
	~Sound();

	void playSound(SoundBuffer &buffer, int volume, bool loop);
	void pauseSound();
	void resumeSound();
	void stopSound();

	void playVoice(SoundBuffer &buffer);
	void pauseVoice();
	void resumeVoice();
	void stopVoice();

 private:

	void playSoundBuffer(Audio::SoundHandle *handle, SoundBuffer &buffer, int volume, bool loop);
	int _enabled;

	SagaEngine *_vm;
	Audio::Mixer *_mixer;
	Common::MemoryReadStream *_voxStream;

	Audio::SoundHandle _effectHandle;
	Audio::SoundHandle _voiceHandle;

};

} // End of namespace Saga

#endif
