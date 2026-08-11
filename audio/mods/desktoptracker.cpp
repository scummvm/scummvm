/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* Adapted from ArcTracker https://github.com/richardjwild/arctracker/ */

#include "common/array.h"
#include "common/memstream.h"
#include "common/stream.h"
#include "common/util.h"

#include "audio/audiostream.h"

namespace Audio {

namespace Modules {

static uint32 alignToWord(uint32 x) {
	return (x + 1) & ~1U;
}

static uint32 alignToDword(uint32 x) {
	return (x + 3) & ~3U;
}

static uint32 readLE32FromBuf(const byte *p) {
	return (uint32)p[0] | ((uint32)p[1] << 8) | ((uint32)p[2] << 16) | ((uint32)p[3] << 24);
}

static int8 vidcToSigned8(byte enc) {

	static bool sInit = false;
	static int8 sTable[256];

	if (!sInit) {
		const int BIAS = 0x84;
		const int SIGN_BIT = 0x80;
		const int QUANT_MASK = 0x0F;
		const int SEG_MASK = 0x70;
		const double EXPANDED_MAX = 32124.0;

		for (int i = 0; i <= 127; ++i) {
			const int mu = 127 - i;
			const int normal = ~mu;
			const int biased = ((normal & QUANT_MASK) << 3) + BIAS;
			const unsigned int seg = ((unsigned int)normal & (unsigned int)SEG_MASK) >> 4;
			const int linearPos = (normal & SIGN_BIT) ? (BIAS - (biased << (int)seg)) : ((biased << (int)seg) - BIAS);
			const double f = (double)linearPos / EXPANDED_MAX;

			const double p = f * 127.0;
			const double n = -f * 127.0;
			const int vpos = CLIP<int>((int)(p >= 0.0 ? (p + 0.5) : (p - 0.5)), -127, 127);
			const int vneg = CLIP<int>((int)(n >= 0.0 ? (n + 0.5) : (n - 0.5)), -127, 127);

			sTable[i * 2] = (int8)vpos;
			sTable[(i * 2) + 1] = (int8)vneg;
		}

		sInit = true;
	}

	return sTable[enc];
}

static uint16 periodForNote(uint32 n) {

	static const uint16 kPeriods[62] = {
		0x06B0, 0x0650, 0x05F5, 0x05A0, 0x054D, 0x0501, 0x04B9, 0x0475, 0x0435, 0x03F9, 0x03C1, 0x038B,
		0x0358, 0x0328, 0x02FA, 0x02D0, 0x02A6, 0x0280, 0x025C, 0x023A, 0x021A, 0x01FC, 0x01E0, 0x01C5,
		0x01AC, 0x0194, 0x017D, 0x0168, 0x0153, 0x0140, 0x012E, 0x011D, 0x010D, 0x00FE, 0x00F0, 0x00E2,
		0x00D6, 0x00CA, 0x00BE, 0x00B4, 0x00AA, 0x00A0, 0x0097, 0x008F, 0x0087, 0x007F, 0x0078, 0x0071,
		0x006B, 0x0065, 0x005F, 0x005A, 0x0055, 0x0050, 0x004C, 0x0047, 0x0043, 0x0040, 0x003C, 0x0039,
		0x0035, 0x0032
	};
	n = CLIP<uint32>(n, 0, 61);
	return kPeriods[n];
}

struct DttHeader {
	char name[64];
	char author[64];
	uint32 flags;
	uint32 numChannels;
	uint32 tuneLength;
	byte initialStereo[8];
	uint32 initialSpeed;
	uint32 restart;
	uint32 numPatterns;
	uint32 numSamples;
};

struct DttSample {
	char name[32];
	int transpose;
	uint8 defaultGain;
	uint32 repeatOffset;
	uint32 repeatLength;
	uint32 sampleLength;
	uint32 sampleDataOffset;
	Common::Array<int8> pcm;
};

struct DttEffect {
	uint8 cmd;
	uint8 param;
};

struct DttEvent {
	uint8 sample;
	uint8 note;
	DttEffect effects[4];
	uint8 numEffects;
};

struct VoiceState {
	int sampleIdx;
	uint8 vol;
	uint16 period;
	uint32 pos16;
	uint32 step16;
};

static uint8 mask6(uint32 x, int shift) { return (uint8)((x >> shift) & 0x3F); }
static uint8 mask5(uint32 x, int shift) { return (uint8)((x >> shift) & 0x1F); }
static uint8 mask8(uint32 x, int shift) { return (uint8)((x >> shift) & 0xFF); }

static bool isMultipleEffect(uint32 raw0) {
	return (raw0 & (0x1FU << 17)) != 0;
}

static void decodeEvent(const byte *p, DttEvent &out, uint32 &bytesUsed) {
	const uint32 raw0 = readLE32FromBuf(p);
	out.sample = mask6(raw0, 0);
	out.note = mask6(raw0, 6);

	if (isMultipleEffect(raw0)) {
		const uint32 raw1 = readLE32FromBuf(p + 4);
		out.numEffects = 4;
		out.effects[0] = DttEffect{ mask5(raw0, 12), mask8(raw1, 0) };
		out.effects[1] = DttEffect{ mask5(raw0, 17), mask8(raw1, 8) };
		out.effects[2] = DttEffect{ mask5(raw0, 22), mask8(raw1, 16) };
		out.effects[3] = DttEffect{ mask5(raw0, 27), mask8(raw1, 24) };
		bytesUsed = 8;
	} else {
		out.numEffects = 1;
		out.effects[0] = DttEffect{ mask5(raw0, 12), mask8(raw0, 24) };
		out.effects[1] = DttEffect{ 0, 0 };
		out.effects[2] = DttEffect{ 0, 0 };
		out.effects[3] = DttEffect{ 0, 0 };
		bytesUsed = 4;
	}
}

class DesktopTrackerStream final : public AudioStream {
public:
	DesktopTrackerStream(Common::SeekableReadStream *stream, int offs, int rate, bool stereo)
		: _rate(rate), _stereo(false), _songPos(0), _row(0), _speed50ths(6), _samplesUntilNextRow(0), _ended(false), _sawAnyNote(false) {

		if (!stream)
			error("DesktopTrackerStream: null input stream");

		stream->seek(0, SEEK_END);
		const int32 sz = (int32)stream->pos();
		stream->seek(0, SEEK_SET);
		if (sz <= 0)
			error("DesktopTrackerStream: empty input stream");

		_module.resize((uint32)sz);
		if (stream->read(_module.begin(), (uint32)sz) != (uint32)sz)
			error("DesktopTrackerStream: short read");

		delete stream;
		stream = nullptr;

		parseDskT(offs);

		for (uint32 c = 0; c < 8; ++c) {
			_voices[c].sampleIdx = -1;
			_voices[c].vol = 0;
			_voices[c].period = 0;
			_voices[c].pos16 = 0;
			_voices[c].step16 = 0;
		}

		const double secondsPerRow = (double)MAX<uint32>(_speed50ths, 1) / 50.0;
		_samplesUntilNextRow = (int32)MAX<int>(1, (int)(secondsPerRow * (double)_rate + 0.5));

	}

	bool isStereo() const override { return _stereo; }
	int getRate() const override { return _rate; }
	bool endOfData() const override { return _ended; }

	int readBuffer(int16 *buffer, const int numSamples) override {
		if (!buffer || numSamples <= 0)
			return 0;

		for (int i = 0; i < numSamples; ++i)
			buffer[i] = 0;

		int framesLeft = numSamples;
		while (framesLeft > 0 && !_ended) {
			const int chunk = MIN<int>(framesLeft, MAX<int>(1, _samplesUntilNextRow));
			mixMono(buffer, chunk);

			buffer += chunk;
			framesLeft -= chunk;

			_samplesUntilNextRow -= chunk;
			while (_samplesUntilNextRow <= 0 && !_ended) {
				advanceRow();
				const double secondsPerRow = (double)MAX<uint32>(_speed50ths, 1) / 50.0;
				_samplesUntilNextRow += (int32)MAX<int>(1, (int)(secondsPerRow * (double)_rate + 0.5));
			}
		}

		return numSamples;
	}

private:
	void parseDskT(int offs) {
		if ((uint32)offs >= _module.size())
			error("DesktopTrackerStream: offs out of range");

		const byte *fileBase = _module.begin();
		const uint32 fileSize = (uint32)_module.size();
		const byte *base = fileBase + offs;
		const uint32 size = fileSize - (uint32)offs;

		if (size < 4 + 64 + 64)
			error("DesktopTrackerStream: module too small");

		if (base[0] != 'D' || base[1] != 's' || base[2] != 'k' || base[3] != 'T')
			error("DesktopTrackerStream: not a DskT module");

		memset(&_hdr, 0, sizeof(_hdr));

		const byte *p = base + 4;
		memcpy(_hdr.name, p, 64);
		_hdr.name[63] = 0;
		p += 64;

		memcpy(_hdr.author, p, 64);
		_hdr.author[63] = 0;
		p += 64;

		_hdr.flags = readLE32FromBuf(p); p += 4;
		_hdr.numChannels = readLE32FromBuf(p); p += 4;
		_hdr.tuneLength = readLE32FromBuf(p); p += 4;

		memcpy(_hdr.initialStereo, p, 8); p += 8;

		_hdr.initialSpeed = readLE32FromBuf(p); p += 4;
		_hdr.restart = readLE32FromBuf(p); p += 4;
		_hdr.numPatterns = readLE32FromBuf(p); p += 4;
		_hdr.numSamples = readLE32FromBuf(p); p += 4;

		if (_hdr.numChannels == 0 || _hdr.numChannels > 8)
			error("DesktopTrackerStream: unsupported channels=%u", (uint32)_hdr.numChannels);

		if (_hdr.tuneLength == 0 || _hdr.tuneLength > 256)
			error("DesktopTrackerStream: bad tune length=%u", (uint32)_hdr.tuneLength);

		if (_hdr.numPatterns == 0 || _hdr.numPatterns > 1024)
			error("DesktopTrackerStream: bad numPatterns=%u", (uint32)_hdr.numPatterns);

		if (_hdr.numSamples == 0 || _hdr.numSamples > 256)
			error("DesktopTrackerStream: bad numSamples=%u", (uint32)_hdr.numSamples);

		_speed50ths = (uint32)_hdr.initialSpeed != 0 ? (uint8)_hdr.initialSpeed : (uint8)6;

		const uint32 hdrSize = 4 + 64 + 64 + 4 + 4 + 4 + 8 + 4 + 4 + 4 + 4;
		const uint32 positionsOff = hdrSize;
		const uint32 positionsSize = (uint32)_hdr.tuneLength;
		if (positionsOff + positionsSize > size)
			error("DesktopTrackerStream: positions out of range");

		_sequence.resize((uint32)_hdr.tuneLength);
		for (uint32 i = 0; i < (uint32)_hdr.tuneLength; ++i)
			_sequence[i] = base[positionsOff + i];

		const uint32 patternOffsetsOffWord = positionsOff + alignToWord(positionsSize);
		const uint32 patternOffsetsOffDword = positionsOff + alignToDword(positionsSize);

		auto scorePatternOffsets = [&](uint32 candOff) -> uint32 {
			const uint32 need = (uint32)_hdr.numPatterns * 4U + (uint32)_hdr.numPatterns;
			if (candOff + need > size)
				return 0;

			uint32 validOffs = 0;
			for (uint32 i = 0; i < (uint32)_hdr.numPatterns; ++i) {
				const uint32 po = readLE32FromBuf(base + candOff + (i * 4));
				if (po != 0 && po < fileSize)
					validOffs++;
			}

			uint32 validLens = 0;
			const uint32 lensOff = candOff + (uint32)_hdr.numPatterns * 4U;
			for (uint32 i = 0; i < (uint32)_hdr.numPatterns; ++i) {
				const uint8 rows = base[lensOff + i];
				if (rows != 0 && rows <= 128)
					validLens++;
			}

			if (validOffs == 0)
				return 0;

			return (validOffs * 2U) + validLens;
		};

		const uint32 scoreWord = scorePatternOffsets(patternOffsetsOffWord);
		const uint32 scoreDword = scorePatternOffsets(patternOffsetsOffDword);

		const uint32 patternOffsetsOff = (scoreDword > scoreWord) ? patternOffsetsOffDword : patternOffsetsOffWord;
		const uint32 patternOffsetsSize = (uint32)_hdr.numPatterns * 4U;
		if (patternOffsetsOff + patternOffsetsSize > size)
			error("DesktopTrackerStream: patternOffsets out of range");

		_patternOffsets.resize((uint32)_hdr.numPatterns);
		for (uint32 i = 0; i < (uint32)_hdr.numPatterns; ++i)
			_patternOffsets[i] = readLE32FromBuf(base + patternOffsetsOff + (i * 4));

		const uint32 patternLengthsOff = patternOffsetsOff + patternOffsetsSize;
		const uint32 patternLengthsSize = (uint32)_hdr.numPatterns;
		if (patternLengthsOff + patternLengthsSize > size)
			error("DesktopTrackerStream: patternLengths out of range");

		_patternLengths.resize((uint32)_hdr.numPatterns);
		for (uint32 i = 0; i < (uint32)_hdr.numPatterns; ++i)
			_patternLengths[i] = base[patternLengthsOff + i];

		const uint32 samplesOff = patternLengthsOff + alignToDword(patternLengthsSize);
		const uint32 sampleStructSize = 64;
		const uint32 samplesSize = (uint32)_hdr.numSamples * sampleStructSize;
		if (samplesOff + samplesSize > size)
			error("DesktopTrackerStream: samples out of range");

		_samples.resize((uint32)_hdr.numSamples);
		for (uint32 i = 0; i < (uint32)_hdr.numSamples; ++i) {
			const byte *sp = base + samplesOff + i * sampleStructSize;
			DttSample &smp = _samples[i];
			memset(smp.name, 0, sizeof(smp.name));

			const uint8 note = sp[0];
			const uint8 vol = sp[1];

			const uint32 repeatOffset = readLE32FromBuf(sp + 16);
			const uint32 repeatLength = readLE32FromBuf(sp + 20);
			const uint32 sampleLength = readLE32FromBuf(sp + 24);
			memcpy(smp.name, sp + 28, 32);
			smp.name[31] = 0;
			const uint32 sampleDataOffset = readLE32FromBuf(sp + 60);

			smp.transpose = 26 - (int)note;
			smp.defaultGain = (uint8)(vol & 0x7F);
			smp.repeatOffset = repeatOffset;
			smp.repeatLength = repeatLength;
			smp.sampleLength = sampleLength;
			smp.sampleDataOffset = sampleDataOffset;

			smp.pcm.clear();
			if (sampleDataOffset != 0 && sampleLength != 0) {
				if (sampleDataOffset + sampleLength > fileSize) {
					smp.sampleLength = 0;
					smp.pcm.clear();
				} else {
					smp.pcm.resize(sampleLength + 1);
					for (uint32 k = 0; k < sampleLength; ++k)
						smp.pcm[k] = vidcToSigned8(fileBase[sampleDataOffset + k]);
					smp.pcm[sampleLength] = (sampleLength > 0) ? smp.pcm[sampleLength - 1] : 0;
				}
			}
		}
	}

	void advanceRow() {
		if (_ended)
			return;

		if (_songPos >= (uint32)_sequence.size()) {
			_ended = true;
			return;
		}

		const uint32 patIdx = (uint32)_sequence[_songPos];
		if (patIdx >= (uint32)_patternOffsets.size()) {
			_ended = true;
			return;
		}

		applyRow(patIdx, _row);

		const uint32 rows = (uint32)_patternLengths[patIdx];
		_row++;
		if (_row >= rows) {
			_row = 0;
			_songPos++;
			if (_songPos >= (uint32)_sequence.size())
				_songPos = 0;
		}
	}

	void applyRow(uint32 patIdx, uint32 row) {
		const uint32 poff = _patternOffsets[patIdx];
		if (poff == 0 || poff >= _module.size())
			return;

		const byte *p = _module.begin() + poff;
		for (uint32 r = 0; r < row; ++r) {
			for (uint32 c = 0; c < (uint32)_hdr.numChannels; ++c) {
				DttEvent ev;
				uint32 used = 0;
				decodeEvent(p, ev, used);
				p += used;
			}
		}

		for (uint32 c = 0; c < (uint32)_hdr.numChannels; ++c) {
			DttEvent ev;
			uint32 used = 0;
			decodeEvent(p, ev, used);
			p += used;

			if (ev.sample != 0 && ev.sample <= _samples.size()) {
				const int si = (int)ev.sample - 1;
				_voices[c].sampleIdx = si;
				_voices[c].vol = _samples[si].defaultGain;
				_voices[c].pos16 = 0;
			}

			for (uint32 i = 0; i < (uint32)ev.numEffects; ++i) {
				const uint8 cmd = ev.effects[i].cmd;
				const uint8 param = ev.effects[i].param;
				if (cmd == 0x0C) {
					_voices[c].vol = (uint8)MIN<uint32>(param & 0x7F, 127);
				} else if (cmd == 0x0F) {
					if (param != 0)
						_speed50ths = param;
				}
			}

			if (ev.note != 0 && _voices[c].sampleIdx >= 0) {

				const DttSample &smp = _samples[(uint32)_voices[c].sampleIdx];
				if (!_sawAnyNote) {
					_sawAnyNote = true;
				}
				if (!smp.pcm.empty()) {
					const int note = (int)ev.note + smp.transpose;
					const uint32 nn = (uint32)CLIP<int>(note, 0, 61);
					_voices[c].period = periodForNote(nn);

					const double conv = 3273808.59375;
					const double step = conv / ((double)_voices[c].period * (double)_rate);
					const uint32 step16 = (uint32)MAX<uint32>(1, (uint32)(step * 65536.0 + 0.5));
					_voices[c].step16 = step16;
					_voices[c].pos16 = 0;
				}
			}
		}
	}

	void mixMono(int16 *dst, int frames) {
		for (int i = 0; i < frames; ++i) {
			int mix = 0;

			for (uint32 c = 0; c < (uint32)_hdr.numChannels; ++c) {
				VoiceState &vs = _voices[c];
				if (vs.sampleIdx < 0 || vs.step16 == 0)
					continue;

				const DttSample &smp = _samples[(uint32)vs.sampleIdx];
				if (smp.pcm.empty() || smp.sampleLength < 2)
					continue;

				uint32 pos = vs.pos16;
				uint32 idx = pos >> 16;

				if (idx >= smp.sampleLength) {
					if (smp.repeatLength > 2) {
						const uint32 loopEnd = smp.repeatOffset + smp.repeatLength;
						if (loopEnd > smp.repeatOffset) {
							if (idx >= loopEnd)
								idx = smp.repeatOffset + ((idx - smp.repeatOffset) % smp.repeatLength);
						} else {
							idx = smp.repeatOffset;
						}
						pos = (idx << 16) | (pos & 0xFFFF);
					} else {
						continue;
					}
				}

				const uint32 i0 = (pos >> 16);
				const uint32 frac = pos & 0xFFFF;

				const int s0 = (int)smp.pcm[MIN<uint32>(i0, (uint32)smp.pcm.size() - 1)];
				const int s1 = (int)smp.pcm[MIN<uint32>(i0 + 1, (uint32)smp.pcm.size() - 1)];
				const int s8 = (int)((((int64)s0 * (int64)(65536 - (int)frac)) + ((int64)s1 * (int64)frac)) >> 16);

				const int sample16 = s8 << 8;
				mix += (sample16 * (int)vs.vol);

				vs.pos16 = pos + vs.step16;
			}

			const int denom = MAX<int>(1, (int)_hdr.numChannels * 128);
			mix = mix / denom;
			mix = CLIP<int>(mix, -32768, 32767);
			dst[i] = (int16)mix;
		}
	}

private:
	int _rate;
	bool _stereo;

	Common::Array<byte> _module;

	DttHeader _hdr;

	Common::Array<byte> _sequence;
	Common::Array<uint32> _patternOffsets;
	Common::Array<byte> _patternLengths;
	Common::Array<DttSample> _samples;

	uint32 _songPos;
	uint32 _row;

	uint8 _speed50ths;
	int32 _samplesUntilNextRow;

	VoiceState _voices[8];

	bool _ended;
	bool _sawAnyNote;
};

} // namespace Modules

AudioStream *makeDesktopTrackerStream(Common::SeekableReadStream *stream, int offs, int rate, bool stereo) {
	if (!stream)
		return nullptr;
	return new Modules::DesktopTrackerStream(stream, offs, rate, stereo);
}

AudioStream *makeDesktopTrackerStream(Common::SeekableReadStream *stream, DisposeAfterUse::Flag disposeAfterUse) {
	if (!stream)
		return nullptr;

	const int offs = 0;
	const int rate = 44100;
	const bool stereo = false;

	Modules::DesktopTrackerStream *dt = new Modules::DesktopTrackerStream(stream, offs, rate, stereo);

	(void)disposeAfterUse;

	return dt;
}

} // namespace Audio
