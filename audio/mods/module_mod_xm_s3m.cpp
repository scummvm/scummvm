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

#include "common/debug.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "module_mod_xm_s3m.h"

namespace Modules {

const int ModuleModXmS3m::FP_SHIFT = 0xF;
const int ModuleModXmS3m::FP_ONE = 0x8000;
const int ModuleModXmS3m::FP_MASK = 0x7FFF;

const int ModuleModXmS3m::exp2table[] = {
		32768, 32946, 33125, 33305, 33486, 33667, 33850, 34034,
		34219, 34405, 34591, 34779, 34968, 35158, 35349, 35541,
		35734, 35928, 36123, 36319, 36516, 36715, 36914, 37114,
		37316, 37518, 37722, 37927, 38133, 38340, 38548, 38757,
		38968, 39180, 39392, 39606, 39821, 40037, 40255, 40473,
		40693, 40914, 41136, 41360, 41584, 41810, 42037, 42265,
		42495, 42726, 42958, 43191, 43425, 43661, 43898, 44137,
		44376, 44617, 44859, 45103, 45348, 45594, 45842, 46091,
		46341, 46593, 46846, 47100, 47356, 47613, 47871, 48131,
		48393, 48655, 48920, 49185, 49452, 49721, 49991, 50262,
		50535, 50810, 51085, 51363, 51642, 51922, 52204, 52488,
		52773, 53059, 53347, 53637, 53928, 54221, 54515, 54811,
		55109, 55408, 55709, 56012, 56316, 56622, 56929, 57238,
		57549, 57861, 58176, 58491, 58809, 59128, 59449, 59772,
		60097, 60423, 60751, 61081, 61413, 61746, 62081, 62419,
		62757, 63098, 63441, 63785, 64132, 64480, 64830, 65182,
		65536
};

int ModuleModXmS3m::moduleExp2(int x) {
	int c, m, y;
	int x0 = (x & FP_MASK) >> (FP_SHIFT - 7);
	c = exp2table[x0];
	m = exp2table[x0 + 1] - c;
	y = (m * (x & (FP_MASK >> 7)) >> 8) + c;
	return (y << FP_SHIFT) >> (FP_SHIFT - (x >> FP_SHIFT));
}

int ModuleModXmS3m::moduleLog2(int x) {
	int y = 16 << FP_SHIFT;
	for (int step = y; step > 0; step >>= 1) {
		if (moduleExp2(y - step) >= x) {
			y -= step;
		}
	}
	return y;
}

bool ModuleModXmS3m::load(Common::SeekableReadStream &st) {
	int32 setPos = st.pos();

	// xm file
	char sigXm[18] = { 0 };
	st.read(sigXm, 17);
	if (!memcmp(sigXm, "Extended Module: ", 17)) {
		return loadXm(st);
	}
	st.seek(setPos);

	// s3m file
	char sigS3m[4];
	st.skip(44);
	st.read(sigS3m, 4);
	if (!memcmp(sigS3m, "SCRM", 4)) {
		st.seek(setPos);
		return loadS3m(st);
	}
	st.seek(setPos);

	// amf file
	char sigAmf[25] = {};
	st.read(sigAmf, 24);
	if (!memcmp(sigAmf, "ASYLUM Music Format V1.0", 24)) {
		return loadAmf(st);
	}
	st.seek(setPos);

	// mod file
	return loadMod(st);
}

ModuleModXmS3m::ModuleModXmS3m() {
	sequenceLen = 1;
	sequence = nullptr;
	restartPos = 0;

	// patterns
	numChannels = 4;
	numPatterns = 1;
	patterns = nullptr;

	// instruments
	numInstruments = 1;
	instruments = nullptr;

	// others
	defaultGvol = 64;
	defaultSpeed = 6;
	defaultTempo = 125;
	c2Rate = 8287;
	gain = 64;
	linearPeriods = false;
	fastVolSlides = false;
	defaultPanning = nullptr; //{ 51, 204, 204, 51 };
}

ModuleModXmS3m::~ModuleModXmS3m() {
	// free song position
	if (sequence) {
		delete[] sequence;
		sequence = nullptr;
	}

	// free instruments
	if (instruments) {
		for (int i = 0; i <= numInstruments; ++i) {
			// free samples
			for (int j = 0; j < instruments[i].numSamples; ++j) {
				if (instruments[i].samples[j].data) {
					delete[] instruments[i].samples[j].data;
					instruments[i].samples[j].data = nullptr;
				}
			}
			delete[] instruments[i].samples;
			instruments[i].samples = nullptr;
		}
		delete[] instruments;
		instruments = nullptr;
	}

	// free patterns
	if (patterns) {
		for (int i = 0; i < numPatterns; ++i) {
			delete []patterns[i].notes;
		}
		delete[] patterns;
		patterns = nullptr;
	}

	// free default values
	if (defaultPanning) {
		delete[] defaultPanning;
		defaultPanning = nullptr;
	}
}

bool ModuleModXmS3m::loadMod(Common::SeekableReadStream &st) {
	// load song name
	st.read(name, 20);
	name[20] = '\0';

	// load instruments
	numInstruments = 31;
	instruments = new Instrument[numInstruments + 1];
	memset(instruments, 0, sizeof(Instrument) * (numInstruments + 1));
	instruments[0].numSamples = 1;
	instruments[0].samples = new Sample[1];
	memset(&instruments[0].samples[0], 0, sizeof(Sample));

	for (int i = 1; i <= numInstruments; ++i) {
		instruments[i].numSamples = 1;
		instruments[i].samples = new Sample[1];
		memset(&instruments[i].samples[0], 0, sizeof(Sample));

		// load sample
		Sample &sample = instruments[i].samples[0];
		st.read((byte *)sample.name, 22);
		sample.name[22] = '\0';
		sample.length = 2 * st.readUint16BE();

		sample.finetune = st.readByte();
		assert(sample.finetune < 0x10);

		sample.volume = st.readByte();
		sample.loopStart = 2 * st.readUint16BE();
		sample.loopLength = 2 * st.readUint16BE();

		if (sample.loopStart + sample.loopLength > sample.length) {
			sample.loopLength = sample.length - sample.loopStart;
		}
		if (sample.loopLength < 4) {
			sample.loopStart = sample.length;
			sample.loopLength = 0;
		}
	}

	sequenceLen = st.readByte();
	if (sequenceLen > 128)
		sequenceLen = 128;

	restartPos = 0;
	st.readByte(); // undefined byte, should be 127

	sequence = new byte[128];
	st.read(sequence, 128);

	// check signature
	byte xx[2];
	st.read(xx, 2); // first 2 bytes of the signature
	switch (st.readUint16BE()) {
		case MKTAG16('K', '.'): /* M.K. */
			// Fall Through intended
		case MKTAG16('K', '!'): /* M!K! */
			// Fall Through intended
		case MKTAG16('T', '4'): /* FLT4 */
			// Fall Through intended
			numChannels = 4;
			c2Rate = 8287;
			gain = 64;
			break;

		case MKTAG16('H', 'N'): /* xCHN */
			numChannels = xx[0] - '0';
			c2Rate = 8363;
			gain = 32;
			break;

		case MKTAG16('C', 'H'): /* xxCH */
			numChannels = (xx[0] - '0') * 10 + xx[1] - '0';
			c2Rate = 8363;
			gain = 32;
			break;

		default:
			warning("No known signature found in micromod module");
			return false;

	}

	// default values
	defaultGvol = 64;
	defaultSpeed = 6;
	defaultTempo = 125;
	defaultPanning = new byte[numChannels];
	for (int i = 0; i < numChannels; ++i) {
		defaultPanning[i] = 51;
		if ((i & 3) == 1 || (i & 3) == 2) {
			defaultPanning[i] = 204;
		}
	}

	// load patterns
	numPatterns = 0;
	for (int i = 0; i < 128; ++i)
		if (numPatterns < sequence[i])
			numPatterns = sequence[i];
	++numPatterns;

	// load patterns
	patterns = new Pattern[numPatterns];
	for (int i = 0; i < numPatterns; ++i) {
		patterns[i].numChannels = numChannels;
		patterns[i].numRows = 64;

		// load notes
		/*
		 Old (amiga) noteinfo:

		 _____byte 1_____   byte2_    _____byte 3_____   byte4_
		 /                \ /      \  /                \ /      \
		0000          0000-00000000  0000          0000-00000000

		 Upper four    12 bits for    Lower four    Effect command.
		 bits of sam-  note period.   bits of sam-
		 ple number.                  ple number.
		 */

		int numNotes = patterns[i].numChannels * patterns[i].numRows;
		patterns[i].notes = new Note[numNotes];
		memset(patterns[i].notes, 0, numNotes * sizeof(Note));
		for (int idx = 0; idx < numNotes; ++idx) {
			byte first = st.readByte();
			byte second = st.readByte();
			byte third = st.readByte();
			byte fourth = st.readByte();

			// period, key
			uint period = (first & 0xF) << 8;
			period = (period | second) * 4;
			if (period >= 112 && period <= 6848) {
				int key = -12 * moduleLog2((period << FP_SHIFT) / 29021);
				key = (key + (key & (FP_ONE >> 1))) >> FP_SHIFT;
				patterns[i].notes[idx].key = key;
			}

			// instrument
			uint ins = (third & 0xF0) >> 4;
			ins = ins | (first & 0x10);
			patterns[i].notes[idx].instrument = ins;

			// effect, param
			byte effect = third & 0x0F;
			byte param = fourth & 0xff;
			if (param == 0 && (effect < 3 || effect == 0xA)) {
				effect = 0;
			}
			if (param == 0 && (effect == 5 || effect == 6)) {
				effect -= 2;
			}
			if (effect == 8) {
				if (numChannels == 4) {
					effect = param = 0;
				} else if (param > 128) {
					param = 128;
				} else {
					param = (param * 255) >> 7;
				}
			}
			patterns[i].notes[idx].effect = effect;
			patterns[i].notes[idx].param = param;
		}
	}

	// load data for the sample of instruments
	for (int i = 1; i <= numInstruments; ++i) {
		Sample &sample = instruments[i].samples[0];
		if (!sample.length) {
			sample.data = 0;
		} else {
			sample.data = new int16[sample.length + 1];
			readSampleSint8(st, sample.length, sample.data);
			sample.data[sample.loopStart + sample.loopLength] = sample.data[sample.loopStart];
		}
	}

	return true;
}

bool ModuleModXmS3m::loadXm(Common::SeekableReadStream &st) {
	st.read(name, 20);
	name[20] = '\0';
	st.readByte(); // reserved byte

	byte trackername[20];
	st.read(trackername, 20);
	bool deltaEnv = !memcmp(trackername, "DigiBooster Pro", 15);

	uint16 version = st.readUint16LE();
	if (version != 0x0104) {
		warning("XM format version must be 0x0104!");
		return false;
	}

	uint offset = st.pos() + st.readUint32LE();

	sequenceLen = st.readUint16LE();
	restartPos = st.readUint16LE();

	numChannels = st.readUint16LE();
	numPatterns = st.readUint16LE();
	numInstruments = st.readUint16LE();
	linearPeriods = st.readUint16LE() & 0x1;
	defaultGvol = 64;
	defaultSpeed = st.readUint16LE();
	defaultTempo = st.readUint16LE();
	c2Rate = 8363;
	gain = 64;

	defaultPanning = new byte[numChannels];
	for (int i = 0; i < numChannels; ++i) {
		defaultPanning[i] = 128;
	}

	sequence = new byte[sequenceLen];
	for (int i = 0; i < sequenceLen; ++i) {
		int entry = st.readByte();
		sequence[i] = entry < numPatterns ? entry : 0;
	}

	// load patterns
	patterns = new Pattern[numPatterns];
	for (int i = 0; i < numPatterns; ++i) {
		st.seek(offset, SEEK_SET);
		offset += st.readUint32LE();
		if (st.readByte()) {
			warning("Unknown pattern packing type!");
			return false;
		}
		patterns[i].numRows = st.readUint16LE();
		if (patterns[i].numRows < 1)
			patterns[i].numRows = 1;
		uint16 patDataLength = st.readUint16LE();
		offset += patDataLength;

		// load notes
		patterns[i].numChannels = numChannels;
		int numNotes = patterns[i].numRows * numChannels;
		patterns[i].notes = new Note[numNotes];
		memset(patterns[i].notes, 0, numNotes * sizeof(Note));

		if (patDataLength > 0) {
			for (int j = 0; j < numNotes; ++j) {
				Note &note = patterns[i].notes[j];
				byte cmp = st.readByte();
				if (cmp & 0x80) {
					if (cmp & 1)
						note.key = st.readByte();
					if (cmp & 2)
						note.instrument = st.readByte();
					if (cmp & 4)
						note.volume = st.readByte();
					if (cmp & 8)
						note.effect = st.readByte();
					if (cmp & 16)
						note.param = st.readByte();
				} else {
					note.key = cmp;
					note.instrument = st.readByte();
					note.volume = st.readByte();
					note.effect = st.readByte();
					note.param = st.readByte();
				}
				if( note.effect >= 0x40 ) {
					note.effect = note.param = 0;
				}
			}
		}
	}

	// load instruments
	instruments = new Instrument[numInstruments + 1];
	memset(instruments, 0, (numInstruments + 1) * sizeof(Instrument));
	instruments[0].samples = new Sample[1];
	memset(instruments[0].samples, 0, sizeof(Sample));
	for (int i = 1; i <= numInstruments; ++i) {
		st.seek(offset, SEEK_SET);
		offset += st.readUint32LE();

		Instrument &ins = instruments[i];
		st.read(ins.name, 22);
		ins.name[22] = '\0';

		st.readByte(); // Instrument type (always 0)

		// load sample number
		int nSamples = st.readUint16LE();
		ins.numSamples = nSamples > 0 ? nSamples : 1;
		ins.samples = new Sample[ins.numSamples];
		memset(ins.samples, 0, ins.numSamples * sizeof(Sample));
		st.readUint32LE(); // skip 4 byte

		// load instrument informations
		if (nSamples > 0) {
			for (int k = 0; k < 96; ++k) {
				ins.keyToSample[k + 1] = st.readByte();
			}
			int pointTick = 0;
			for (int p = 0; p < 12; ++p) {
				pointTick = (deltaEnv ? pointTick : 0) + st.readUint16LE();
				ins.volEnv.pointsTick[p] = pointTick;
				ins.volEnv.pointsAmpl[p] = st.readUint16LE();
			}
			pointTick = 0;
			for (int p = 0; p < 12; ++p) {
				pointTick = (deltaEnv ? pointTick : 0) + st.readUint16LE();
				ins.panEnv.pointsTick[p] = pointTick;
				ins.panEnv.pointsAmpl[p] = st.readUint16LE();
			}
			ins.volEnv.numPoints = st.readByte();
			if (ins.volEnv.numPoints > 12)
				ins.volEnv.numPoints = 0;
			ins.panEnv.numPoints = st.readByte();
			if (ins.panEnv.numPoints > 12)
				ins.panEnv.numPoints = 0;
			ins.volEnv.sustainTick = ins.volEnv.pointsTick[st.readByte() & 0xF];
			ins.volEnv.loopStartTick = ins.volEnv.pointsTick[st.readByte() & 0xF];
			ins.volEnv.loopEndTick = ins.volEnv.pointsTick[st.readByte() & 0xF];
			ins.panEnv.sustainTick = ins.panEnv.pointsTick[st.readByte() & 0xF];
			ins.panEnv.loopStartTick = ins.panEnv.pointsTick[st.readByte() & 0xF];
			ins.panEnv.loopEndTick = ins.panEnv.pointsTick[st.readByte() & 0xF];
			byte volParam = st.readByte();
			ins.volEnv.enabled = ins.volEnv.numPoints > 0 && (volParam & 0x1);
			ins.volEnv.sustain = (volParam & 0x2) > 0;
			ins.volEnv.looped = (volParam & 0x4) > 0;
			byte panParam = st.readByte();
			ins.panEnv.enabled = ins.panEnv.numPoints > 0 && (panParam & 0x1);
			ins.panEnv.sustain = (panParam & 0x2) > 0;
			ins.panEnv.looped = (panParam & 0x4) > 0;
			ins.vibType = st.readByte();
			ins.vibSweep = st.readByte();
			ins.vibDepth = st.readByte();
			ins.vibRate = st.readByte();
			ins.volFadeout = st.readUint16LE();
		}

		// load samples
		uint samHeadOffset = offset;
		offset += nSamples * 40; // offset for sample data
		for (int j = 0; j < nSamples; ++j) {
			// load sample head
			st.seek(samHeadOffset, SEEK_SET);
			samHeadOffset += 40; // increment
			Sample &sample = ins.samples[j];
			uint samDataBytes = st.readUint32LE();
			uint samLoopStart = st.readUint32LE();
			uint samLoopLength = st.readUint32LE();
			sample.volume = st.readByte();
			sample.finetune = st.readSByte();
			byte loopType = st.readByte();
			bool looped = (loopType & 0x3) > 0;
			bool pingPong = (loopType & 0x2) > 0;
			bool sixteenBit = (loopType & 0x10) > 0;
			sample.panning = st.readByte() + 1;
			sample.relNote = st.readSByte();
			st.readByte(); // reserved byte
			st.read(sample.name, 22);
			sample.name[22] = '\0';

			uint samDataSamples = samDataBytes;
			if (sixteenBit) {
				samDataSamples = samDataSamples >> 1;
				samLoopStart = samLoopStart >> 1;
				samLoopLength = samLoopLength >> 1;
			}
			if (!looped || (samLoopStart + samLoopLength) > samDataSamples) {
				samLoopStart = samDataSamples;
				samLoopLength = 0;
			}
			sample.loopStart = samLoopStart;
			sample.loopLength = samLoopLength;

			// load sample data
			st.seek(offset, SEEK_SET);
			offset += samDataBytes; // increment
			sample.data = new int16[samDataSamples + 1];
			if (sixteenBit) {
				readSampleSint16LE(st, samDataSamples, sample.data);
			} else {
				readSampleSint8(st, samDataSamples, sample.data);
			}
			int amp = 0;
			for (uint idx = 0; idx < samDataSamples; idx++) {
				amp = amp + sample.data[idx];
				amp = (amp & 0x7FFF) - (amp & 0x8000);
				sample.data[idx] = amp;
			}
			sample.data[samLoopStart + samLoopLength] = sample.data[samLoopStart];
			if (pingPong) {
				SamplePingPong(sample);
			}
		}
	}
	return true;
}

bool ModuleModXmS3m::loadS3m(Common::SeekableReadStream &st) {
	st.read(name, 28);
	name[28] = '\0';
	st.skip(4); // skip 4 bytes

	sequenceLen = st.readUint16LE();
	numInstruments = st.readUint16LE();
	numPatterns = st.readUint16LE();
	uint16 flags = st.readUint16LE();
	uint16 version = st.readUint16LE();
	fastVolSlides = ((flags & 0x40) == 0x40) || version == 0x1300;
	bool signedSamples = st.readUint16LE() == 1;

	// check signature
	if (st.readUint32BE() != MKTAG('S', 'C', 'R', 'M')) {
		warning("Not an S3M file!");
		return false;
	}

	defaultGvol = st.readByte();
	defaultSpeed = st.readByte();
	defaultTempo = st.readByte();
	c2Rate = 8363;
	byte mastermult = st.readByte();
	gain = mastermult & 0x7F;
	bool stereoMode = (mastermult & 0x80) == 0x80;
	st.readByte(); // skip ultra-click
	bool defaultPan = st.readByte() == 0xFC;
	st.skip(10); // skip 10 bytes

	// load channel map
	numChannels = 0;
	int channelMap[32];
	for (int i = 0; i < 32; ++i) {
		channelMap[i] = -1;
		if (st.readByte() < 16) {
			channelMap[i] = numChannels++;
		}
	}

	// load sequence
	sequence = new byte[sequenceLen];
	st.read(sequence, sequenceLen);

	int moduleDataIndex = st.pos();

	// load instruments
	instruments = new Instrument[numInstruments + 1];
	memset(instruments, 0, sizeof(Instrument) * (numInstruments + 1));
	instruments[0].numSamples = 1;
	instruments[0].samples = new Sample[1];
	memset(instruments[0].samples, 0, sizeof(Sample));
	for (int i = 1; i <= numInstruments; ++i) {
		Instrument &instrum = instruments[i];
		instrum.numSamples = 1;
		instrum.samples = new Sample[1];
		memset(instrum.samples, 0, sizeof(Sample));
		Sample &sample = instrum.samples[0];

		// get instrument offset
		st.seek(moduleDataIndex, SEEK_SET);
		int instOffset = st.readUint16LE() << 4;
		moduleDataIndex += 2;
		st.seek(instOffset, SEEK_SET);

		// load instrument, sample
		if (st.readByte() == 1) { // type
			st.skip(12); // skip file name
			int sampleOffset = (st.readByte() << 20) + (st.readUint16LE() << 4);
			uint sampleLength = st.readUint32LE();
			uint loopStart = st.readUint32LE();
			uint loopLength = st.readUint32LE() - loopStart;
			sample.volume = st.readByte();
			st.skip(1); // skip dsk
			if (st.readByte() != 0) {
				warning("Packed samples not supported for S3M files");
				return false;
			}
			byte samParam = st.readByte();

			if (loopStart + loopLength > sampleLength) {
				loopLength = sampleLength - loopStart;
			}
			if (loopLength < 1 || !(samParam & 0x1)) {
				loopStart = sampleLength;
				loopLength = 0;
			}

			sample.loopStart = loopStart;
			sample.loopLength = loopLength;

			bool sixteenBit = samParam & 0x4;
			int tune = (moduleLog2(st.readUint32LE()) - moduleLog2(c2Rate)) * 12;
			sample.relNote = tune >> FP_SHIFT;
			sample.finetune = (tune & FP_MASK) >> (FP_SHIFT - 7);
			st.skip(12); // skip unused bytes
			st.read(instrum.name, 28);

			// load sample data
			sample.data = new int16[sampleLength + 1];
			st.seek(sampleOffset, SEEK_SET);
			if (sixteenBit) {
				readSampleSint16LE(st, sampleLength, sample.data);
			} else {
				readSampleSint8(st, sampleLength, sample.data);
			}
			if (!signedSamples) {
				for (uint idx = 0; idx < sampleLength; ++idx) {
					sample.data[idx] = (sample.data[idx] & 0xFFFF) - 32768;
				}
			}
			sample.data[loopStart + loopLength] = sample.data[loopStart];
		}
	}

	// load patterns
	patterns = new Pattern[numPatterns];
	memset(patterns, 0, numPatterns * sizeof(Pattern));
	for (int i = 0; i < numPatterns; ++i) {
		patterns[i].numChannels = numChannels;
		patterns[i].numRows = 64;

		// get pattern data offset
		st.seek(moduleDataIndex, SEEK_SET);
		int patOffset = (st.readUint16LE() << 4) + 2;
		st.seek(patOffset, SEEK_SET);

		// load notes
		patterns[i].notes = new Note[numChannels * 64];
		memset(patterns[i].notes, 0, numChannels * 64 * sizeof(Note));
		int row = 0;
		while (row < 64) {
			byte token = st.readByte();
			if (token) {
				byte key = 0;
				byte ins = 0;
				if ((token & 0x20) == 0x20) {
					/* Key + Instrument.*/
					key = st.readByte();
					ins = st.readByte();
					if (key < 0xFE) {
						key = (key >> 4) * 12 + (key & 0xF) + 1;
					} else if (key == 0xFF) {
						key = 0;
					}
				}
				byte volume = 0;
				if ((token & 0x40) == 0x40) {
					/* Volume Column.*/
					volume = (st.readByte() & 0x7F) + 0x10;
					if (volume > 0x50) {
						volume = 0;
					}
				}
				byte effect = 0;
				byte param = 0;
				if ((token & 0x80) == 0x80) {
					/* Effect + Param.*/
					effect = st.readByte();
					param = st.readByte();
					if (effect < 1 || effect >= 0x40) {
						effect = param = 0;
					} else if (effect > 0) {
						effect += 0x80;
					}
				}
				int chan = channelMap[token & 0x1F];
				if (chan >= 0) {
					int noteIndex = row * numChannels + chan;
					patterns[i].notes[noteIndex].key = key;
					patterns[i].notes[noteIndex].instrument = ins;
					patterns[i].notes[noteIndex].volume = volume;
					patterns[i].notes[noteIndex].effect = effect;
					patterns[i].notes[noteIndex].param = param;
				}
			} else {
				row++;
			}
		}

		// increment index
		moduleDataIndex += 2;
	}

	// load default panning
	defaultPanning = new byte[numChannels];
	memset(defaultPanning, 0, numChannels);
	for (int chan = 0; chan < 32; ++chan) {
		if (channelMap[chan] >= 0) {
			byte panning = 7;
			if (stereoMode) {
				panning = 12;
				st.seek(64 + chan, SEEK_SET);
				if (st.readByte() < 8) {
					panning = 3;
				}
			}
			if (defaultPan) {
				st.seek(moduleDataIndex + chan, SEEK_SET);
				flags = st.readByte();
				if ((flags & 0x20) == 0x20) {
					panning = flags & 0xF;
				}
			}
			defaultPanning[channelMap[chan]] = panning * 17;
		}
	}
	return true;
}

bool ModuleModXmS3m::loadAmf(Common::SeekableReadStream &st) {
	// already skipped the signature ("ASYLUM Music Format V1.0")
	// total signature length is 32 bytes (the rest are null)
	st.skip(8);
	memcpy(name, "Asylum Module", 14);

	numChannels = 8;
	defaultSpeed = st.readByte();
	defaultTempo = st.readByte();
	numInstruments = st.readByte(); // actually number of samples, but we'll do 1:1 mapping
	numPatterns = st.readByte();
	sequenceLen = st.readByte();
	restartPos = st.readByte();

	sequence = new byte[256];
	st.read(sequence, 256); // Always 256 bytes in the file.

	// Read sample headers..
	instruments = new Instrument[numInstruments + 1];
	memset(instruments, 0, sizeof(Instrument) * (numInstruments + 1));
	instruments[0].numSamples = 1;
	instruments[0].samples = new Sample[1];
	memset(&instruments[0].samples[0], 0, sizeof(Sample));

	for (int i = 1; i <= numInstruments; ++i) {
		instruments[i].numSamples = 1;
		instruments[i].samples = new Sample[1];
		memset(&instruments[i].samples[0], 0, sizeof(Sample));

		// load sample
		Sample &sample = instruments[i].samples[0];
		st.read((byte *)sample.name, 22);
		sample.name[22] = '\0';

		sample.finetune = st.readSByte();
		sample.volume = st.readByte();
		sample.relNote = st.readSByte(); // aka "transpose"
		sample.length = st.readUint32LE();
		sample.loopStart = st.readUint32LE();
		sample.loopLength = st.readUint32LE();

		if (sample.loopStart + sample.loopLength > sample.length) {
			sample.loopLength = sample.length - sample.loopStart;
		}
		if (sample.loopLength < 4) {
			sample.loopStart = sample.length;
			sample.loopLength = 0;
		}

		// Sample data comes later.
	}

	st.skip((64 - numInstruments) * 37); // 37 == sample header len

	// load patterns
	patterns = new Pattern[numPatterns];
	memset(patterns, 0, numPatterns * sizeof(Pattern));
	for (int i = 0; i < numPatterns; ++i) {
		// Always 8 channels, 64 rows.
		patterns[i].numChannels = 8;
		patterns[i].numRows = 64;

		// load notes
		patterns[i].notes = new Note[8 * 64];
		memset(patterns[i].notes, 0, 8 * 64 * sizeof(Note));
		for (int row = 0; row < 64; row++) {
			for (int channel = 0; channel < 8; channel++) {
				Note &n = patterns[i].notes[row * 8 + channel];
				uint8 note = st.readByte();
				if (note != 0) {
					note = note + 1;
				}
				n.key = note;
				n.instrument = st.readByte();
				n.effect = st.readByte();
				n.param = st.readByte();
				// TODO: copied from libmodplug .. is this needed?
				if (n.effect < 1 || n.effect > 0x0f) {
					n.effect = n.param = 0;
				}
				// TODO: copied from mod loader.. is this needed?
				if (n.param == 0 && (n.effect < 3 || n.effect == 0xA))
					n.effect = 0;
				if (n.param == 0 && (n.effect == 5 || n.effect == 6))
					n.effect -= 2;
				if (n.effect == 8) {
					if (n.param > 128) {
						n.param = 128;
					} else {
						n.param = (n.param * 255) >> 7;
					}
				}
			}
		}
	}

	// Load sample data
	for (int i = 1; i <= numInstruments; ++i) {
		Sample &sample = instruments[i].samples[0];
		sample.data = new int16[sample.length];
		readSampleSint8(st, sample.length, sample.data);
	}

	// default to panning to middle?
	defaultPanning = new byte[numChannels];
	for (int i = 0; i < numChannels; ++i) {
		defaultPanning[i] = 128;
	}

	return true;
}


void ModuleModXmS3m::readSampleSint8(Common::SeekableReadStream &stream, int length, int16 *dest) {
	for (int i = 0; i < length; ++i) {
		dest[i] = static_cast<int16>(stream.readSByte() * 256);
		dest[i] = (dest[i] & 0x7FFF) - (dest[i] & 0x8000);
	}
}

void ModuleModXmS3m::readSampleSint16LE(Common::SeekableReadStream &stream, int length, int16 *dest) {
	for (int i = 0; i < length; ++i) {
		dest[i] = stream.readSint16LE();
		dest[i] = (dest[i] & 0x7FFF) - (dest[i] & 0x8000);
	}
}

void ModuleModXmS3m::SamplePingPong(Sample &sample) {
	int loopStart = sample.loopStart;
	int loopLength = sample.loopLength;
	int loopEnd = loopStart + loopLength;
	int16 *sampleData = sample.data;
	int16 *newData = new int16[loopEnd + loopLength + 1];
	if (newData) {
		memcpy(newData, sampleData, loopEnd * sizeof(int16));
		for (int idx = 0; idx < loopLength; idx++) {
			newData[loopEnd + idx] = sampleData[loopEnd - idx - 1];
		}
		delete []sample.data;
		sample.data = newData;
		sample.loopLength *= 2;
		sample.data[loopStart + sample.loopLength] = sample.data[loopStart];
	}
}

} // End of namespace Modules
