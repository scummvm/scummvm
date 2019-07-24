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

/*
 * This code is based on original Mortville Manor DOS source code
 * Copyright (c) 1987-1989 Lankhor
 */

#ifndef MORTEVIELLE_SOUND_H
#define MORTEVIELLE_SOUND_H

#include "audio/mixer.h"
#include "common/mutex.h"
#include "common/queue.h"
#ifdef USE_TTS
#include "common/text-to-speech.h"
#endif

namespace Audio {
class QueuingAudioStream;
}

namespace Mortevielle {
class MortevielleEngine;

const int kNullValue = 255;
const int kTempoMusic = 71;
const int kTempoNoise = 78;
const int kTempoF = 80;
const int kTempoM = 89;

struct SpeechQueue {
	int _val;
	int _code;
	int _acc;
	int _freq;
	int _rep;
};

typedef int tablint[256];

class SoundManager {
private:
	MortevielleEngine *_vm;

	byte *_ambiantNoiseBuf;
	byte *_noiseBuf;
	int _phonemeNumb;
	int _soundType;
	SpeechQueue _queue[3];
	byte _wordBuf[1712];
	byte _troctBuf[10576];
	bool _buildingSentence;
	int _ptr_oct;
	int _tbi[256];

	Audio::QueuingAudioStream *_audioStream;

	void loadPhonemeSounds();
	void moveQueue();
	void initQueue();
	void handlePhoneme();

	void spfrac(int wor);
	void charg_car(int &currWordNumb);
	void entroct(byte o);
	void cctable(tablint &t);
	void trait_car();

	void regenbruit();
	void litph(tablint &t, int typ, int tempo);

public:
	SoundManager(MortevielleEngine *vm, Audio::Mixer *mixer);
	~SoundManager();

	Audio::Mixer *_mixer;
#ifdef USE_TTS
	Common::TextToSpeechManager *_ttsMan;
#endif //USE_TTS
	Audio::SoundHandle _soundHandle;
	uint16 *_cfiphBuffer;

	int decodeMusic(const byte *PSrc, byte *PDest, int size);
	void playSong(const byte *buf, uint usize, uint loops);
	void loadAmbiantSounds();
	void loadNoise();
	void startSpeech(int rep, int character, int typ);
	void waitSpeech();
};

} // End of namespace Mortevielle

#endif
