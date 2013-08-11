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

#ifndef MORTEVIELLE_SPEECH_H
#define MORTEVIELLE_SPEECH_H

#include "mortevielle/sound.h"

#include "common/scummsys.h"

namespace Mortevielle {

const int kAdrTroct  = 0x406b;
const int kAdrWord   = 0x4000;

const float kfreq0 = 1.19318e6;
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

class SpeechManager {
private:
	MortevielleEngine *_vm;

	int _phonemeNumb;
	SpeechQueue _queue[3];
public:
	int _soundType;
	bool _buildingSentence;

	int _ptr_oct;
	uint16 *_cfiphBuffer;
	int _tbi[256];

	SpeechManager();
	~SpeechManager();
	void setParent(MortevielleEngine *vm);
	void spfrac(int wor);
	void charg_car(int &currWordNumb);
	void entroct(byte o);
	void veracf(byte b);
	void cctable(tablint &t);
	void loadPhonemeSounds();
	void trait_car();

	void moveQueue();
	void initQueue();
	void handlePhoneme();
	void startSpeech(int rep, int ht, int typ);
};

} // End of namespace Mortevielle

#endif
