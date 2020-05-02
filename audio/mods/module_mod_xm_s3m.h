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
 * This code is based on IBXM mod player
 *
 * Copyright (c) 2015, Martin Cameron
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the
 * following conditions are met:
 *
 * * Redistributions of source code must retain the above
 * copyright notice, this list of conditions and the
 * following disclaimer.
 *
 * * Redistributions in binary form must reproduce the
 * above copyright notice, this list of conditions and the
 * following disclaimer in the documentation and/or other
 * materials provided with the distribution.
 *
 * * Neither the name of the organization nor the names of
 *  its contributors may be used to endorse or promote
 *  products derived from this software without specific
 *  prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef AUDIO_MODS_MODULE_MOD_XM_S3M_H
#define AUDIO_MODS_MODULE_MOD_XM_S3M_H

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
}

namespace Modules {

struct Note {
	byte key;
	byte instrument;
	byte volume;
	byte effect;	// effect type
	byte param;		// parameter of effect
};

struct Pattern {
	int numChannels, numRows;
	Note *notes;

	Note getNote(int row, int chan) {
		Note res;
		if (row >= 0 && chan >= 0 && row < numRows && chan < numChannels)
			res = notes[row * numChannels + chan];
		else
			memset(&res, 0, sizeof(struct Note));
		return res;
	}
};

struct Sample {
	char name[32];		// sample name
	int16 finetune;		// fine tune
	int16 volume;		// volume
	int length;		// loop start
	int loopStart;		// loop start
	int loopLength;	// loop length
	int16 panning;
	int16 relNote;
	int16 *data;
};

struct Envelope {
	byte enabled, sustain, looped, numPoints;
	uint16 sustainTick, loopStartTick, loopEndTick;
	uint16 pointsTick[16], pointsAmpl[16];
};

struct Instrument {
	int numSamples, volFadeout;
	char name[32], keyToSample[97];
	int8 vibType, vibSweep, vibDepth, vibRate;
	Envelope volEnv, panEnv;
	Sample *samples;
};

struct ModuleModXmS3m {

private:
	static const int FP_SHIFT;
	static const int FP_ONE;
	static const int FP_MASK;
	static const int exp2table[];

public:
	// sound properties
	byte name[32];
	int sequenceLen;
	int restartPos;
	byte *sequence;

	// patterns
	int numChannels;
	int numPatterns;
	Pattern *patterns;

	// instruments
	int numInstruments;
	Instrument *instruments;

	// others
	int defaultGvol, defaultSpeed, defaultTempo, c2Rate, gain;
	bool linearPeriods, fastVolSlides;
	byte *defaultPanning;

	ModuleModXmS3m();
	~ModuleModXmS3m();

	bool load(Common::SeekableReadStream &stream);

	// math functions
	static int moduleLog2(int x);
	static int moduleExp2(int x);

private:
	bool loadMod(Common::SeekableReadStream &stream);
	bool loadXm(Common::SeekableReadStream &stream);
	bool loadS3m(Common::SeekableReadStream &stream);
	bool loadAmf(Common::SeekableReadStream &st);

	void readSampleSint8(Common::SeekableReadStream &stream, int length, int16 *dest);
	void readSampleSint16LE(Common::SeekableReadStream &stream, int length, int16 *dest);

	void SamplePingPong(Sample &sample);
};

} // End of namespace Modules

#endif
