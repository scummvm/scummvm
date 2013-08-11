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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_SOUND_H
#define MORTEVIELLE_SOUND_H

#include "audio/audiostream.h"
#include "audio/mixer.h"
#include "common/mutex.h"
#include "common/queue.h"

namespace Mortevielle {
class MortevielleEngine;

typedef int tablint[256];

/**
 * Structure used to store pending notes to play
 */
struct SpeakerNote {
	int freq;
	uint32 length;

	SpeakerNote(int noteFreq, uint32 noteLength) {
		freq = noteFreq;
		length = noteLength;
	}
};

class SoundManager {
private:
	MortevielleEngine *_vm;
	byte *_ambiantNoiseBuf;
	byte *_noiseBuf;

public:
	Audio::Mixer *_mixer;
	Audio::QueuingAudioStream *_audioStream;
	Audio::SoundHandle _soundHandle;


	SoundManager(Audio::Mixer *mixer);
	~SoundManager();

	void setParent(MortevielleEngine *vm);

	int decodeMusic(const byte *PSrc, byte *PDest, int size);
	void playSong(const byte *buf, uint usize, uint loops);
	void loadAmbiantSounds();
	void loadNoise();

	void regenbruit();
	void litph(tablint &t, int typ, int tempo);
};

} // End of namespace Mortevielle

#endif
