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

#include "engines/agos/drivers/elvira_atarist.h"

#include "audio/ym2149.h"
#include "common/func.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/debug.h"

namespace AGOS {


struct ElviraPrgTosImage {
	Common::Array<uint8> mem;
	uint32 textSize = 0;
	uint32 dataSize = 0;
	uint32 bssSize = 0;
	uint32 symSize = 0;
	uint32 memSize = 0;


	bool load(const Common::Array<uint8> &prgBytes) {
		if (prgBytes.size() < 28)
			return false;
		if (READ_BE_UINT16(prgBytes.begin()) != 0x601A)
			return false;

		textSize = READ_BE_UINT32(prgBytes.begin() + 2);
		dataSize = READ_BE_UINT32(prgBytes.begin() + 6);
		bssSize = READ_BE_UINT32(prgBytes.begin() + 10);
		symSize = READ_BE_UINT32(prgBytes.begin() + 14);

		const uint32 offText = 28;
		const uint32 offData = offText + textSize;
		const uint32 offSym = offData + dataSize;
		const uint32 offRel = offSym + symSize;
		if (prgBytes.size() < offRel + 4)
			return false;

		memSize = textSize + dataSize + bssSize;
		mem.resize(memSize);
		memset(mem.begin(), 0, memSize);
		if (textSize)
			memcpy(&mem[0], prgBytes.begin() + offText, textSize);
		if (dataSize)
			memcpy(&mem[0] + textSize, prgBytes.begin() + offData, dataSize);

		const uint8 *rel = prgBytes.begin() + offRel;
		const uint32 relSize = (uint32)prgBytes.size() - offRel;

		uint32 ofs = READ_BE_UINT32(rel);
		uint32 pos = 4;

		auto applyAt = [&](uint32 addr) {
			if (addr + 4 > mem.size())
				return;
			uint32 v = READ_BE_UINT32(&mem[addr]);
			WRITE_BE_UINT32(&mem[addr], v);
		};

		if (ofs != 0)
			applyAt(ofs);

		while (pos < relSize) {
			uint8 b = rel[pos++];
			if (b == 0)
				break;
			if (b == 1) {
				ofs += 254;
				continue;
			}
			ofs += b;
			applyAt(ofs);
		}

		return true;
	}
};

struct ElviraPrgLabels {
	uint32 L003A = 0, L003B = 0, L003C = 0, L003D = 0, L003E = 0, L003F = 0, L0040 = 0, L0041 = 0;
	uint32 tunetab = 0, L0051 = 0, L004B = 0, L0067 = 0, L0068 = 0;
	uint32 L0042 = 0, L0043 = 0, L0044 = 0, L0045 = 0, L0046 = 0, L0047 = 0, L0048 = 0, L0049 = 0, L004A = 0;
	uint32 L004C = 0, L004D = 0, L004E = 0, L004F = 0;
};

static uint32 findBytePattern(const Common::Array<uint8> &mem, const uint8 *needle, uint32 needleLen, uint32 start) {
	for (uint32 i = start; i + needleLen <= mem.size(); ++i) {
		if (memcmp(&mem[i], needle, needleLen) == 0)
			return i;
	}
	return 0;
}

static uint32 findTuneTable(const Common::Array<uint8> &mem) {
	for (uint32 off = 0x100; off + 7 * 16 <= mem.size(); off += 2) {
		bool ok = true;
		for (uint32 e = 0; e < 7; ++e) {
			uint32 base = off + e * 16;
			uint32 p0 = READ_BE_UINT32(&mem[base + 0]);
			uint32 p1 = READ_BE_UINT32(&mem[base + 4]);
			uint32 p2 = READ_BE_UINT32(&mem[base + 8]);
			if (mem[base + 12] || mem[base + 13] || mem[base + 14] || mem[base + 15]) {
				ok = false;
				break;
			}
			if (p0 < 0x100 || p1 < 0x100 || p2 < 0x100 || p0 >= mem.size() || p1 >= mem.size() || p2 >= mem.size()) {
				ok = false;
				break;
			}
		}
		if (ok)
			return off;
	}
	return 0;
}


static bool applyKnownElviraDriverLayout(uint32 textSize, ElviraPrgLabels &L) {
	// Full game (DE)
	if (textSize == 0x15920) {
		L.L003C = 0xDF6A;
		L.L003D = 0xDFFE;
		L.L003E = 0xE004;
		L.L003F = 0xE00A;
		L.L0040 = 0xE016;
		L.L0041 = 0xE022;
		L.L0042 = 0xE032;
		L.L0043 = 0xE04A;
		L.L0044 = 0xE050;
		L.L0045 = 0xE056;
		L.L0046 = 0xE05C;
		L.L0047 = 0xE0B0;
		L.L0048 = 0xE0B2;
		L.L0049 = 0xE0B6;
		L.L004A = 0xE0B8;
		L.L004B = 0xE0C0;
		L.L004C = 0xE0C6;
		L.L004D = 0xE0CC;
		L.L004E = 0xE0D3;
		L.L004F = 0xE0DE;
		L.L003A = L.L003C - 64;
		L.L003B = L.L003C - 62;
		L.tunetab = 0xE114;
		L.L0051 = 0xE184;
		L.L0067 = 0xE5E8;
		L.L0068 = 0xE652;
		return true;
	}

	// Full game (RUNENG variants)
	if (textSize == 0x158DE) {
		L.L003C = 0xDF28;
		L.L003D = 0xDFBC;
		L.L003E = 0xDFC2;
		L.L003F = 0xDFC8;
		L.L0040 = 0xDFD4;
		L.L0041 = 0xDFE0;
		L.L0042 = 0xDFF0;
		L.L0043 = 0xE008;
		L.L0044 = 0xE00E;
		L.L0045 = 0xE014;
		L.L0046 = 0xE01A;
		L.L0047 = 0xE06E;
		L.L0048 = 0xE070;
		L.L0049 = 0xE074;
		L.L004A = 0xE076;
		L.L004B = 0xE07E;
		L.L004C = 0xE084;
		L.L004D = 0xE08A;
		L.L004E = 0xE091;
		L.L004F = 0xE09C;
		L.L003A = L.L003C - 64;
		L.L003B = L.L003C - 62;
		L.tunetab = 0xE0D2;
		L.L0051 = 0xE142;
		L.L0067 = 0xE5A6;
		L.L0068 = 0xE610;
		return true;
	}

	// Demo
	if (textSize == 0x0A4A6) {
		L.L003C = 0x686C;
		L.L003D = 0x6900;
		L.L003E = 0x6906;
		L.L003F = 0x690C;
		L.L0040 = 0x6918;
		L.L0041 = 0x6924;
		L.L0042 = 0x6934;
		L.L0043 = 0x694C;
		L.L0044 = 0x6952;
		L.L0045 = 0x6958;
		L.L0046 = 0x695E;
		L.L0047 = 0x69B2;
		L.L0048 = 0x69B4;
		L.L0049 = 0x69B8;
		L.L004A = 0x69BA;
		L.L004B = 0x69C2;
		L.L004C = 0x69C8;
		L.L004D = 0x69CE;
		L.L004E = 0x69D5;
		L.L004F = 0x69E0;
		L.L003A = L.L003C - 64;
		L.L003B = L.L003C - 62;
		L.tunetab = 0x6A16;
		L.L0051 = 0x6A86;
		L.L0067 = 0x6EEA;
		L.L0068 = 0x6F54;
		return true;
	}

	return false;
}


static bool locateEmbeddedDriverLayout(const Common::Array<uint8> &mem, ElviraPrgLabels &L, uint32 textSize) {
	if (applyKnownElviraDriverLayout(textSize, L))
		return true;

	static const uint8 sigL0068[] = {0xFF,0xFF,0x00,0x01,0x00,0xA0,0xFF,0xFF,0x00,0x02,0x00,0x00,0x02,0x00,0x00,0x14};
	static const uint8 sigL004B[] = {0x00,0x00,0x00,0x1A,0x00,0x34};
	static const uint8 sigL003C[] = {0x0D,0x60,0x0C,0xA0,0x0B,0xE8,0x0B,0x40};
	static const uint8 sigL003D[] = {0x00,0x01,0x00,0x02,0x00,0x04};
	static const uint8 sigL003E[] = {0x00,0x01,0x00,0x01,0x00,0x01};
	static const uint8 sigL0067[] = {0x00,0x02,0x00,0x08,0x00,0x12,0x00,0x1C};
	static const uint8 sigL0045[] = {0x00,0x00,0x00,0x1C,0x00,0x38};

	L.L0068 = findBytePattern(mem, sigL0068, sizeof(sigL0068), 0x100);
	L.L004B = findBytePattern(mem, sigL004B, sizeof(sigL004B), 0x100);
	L.L003C = findBytePattern(mem, sigL003C, sizeof(sigL003C), 0x100);
	L.L003D = findBytePattern(mem, sigL003D, sizeof(sigL003D), L.L003C ? L.L003C : 0x100);
	L.L003E = findBytePattern(mem, sigL003E, sizeof(sigL003E), L.L003D ? L.L003D : 0x100);
	L.L0067 = findBytePattern(mem, sigL0067, sizeof(sigL0067), 0x100);
	L.L0045 = findBytePattern(mem, sigL0045, sizeof(sigL0045), L.L003E ? L.L003E : 0x100);
	L.tunetab = findTuneTable(mem);

	if (!L.L0068 || !L.L004B || !L.L003C || !L.L003D || !L.L003E || !L.L0067 || !L.L0045 || !L.tunetab)
		return false;

	if (L.L003C >= 64) {
		L.L003A = L.L003C - 64;
		L.L003B = L.L003C - 62;
	}

	L.L003F = L.L003E + 6;
	L.L0040 = L.L003F + 12;
	L.L0041 = L.L0040 + 12;
	L.L0051 = L.tunetab + 7 * 16;

	L.L0042 = L.L0045 - 0x24;
	L.L0043 = L.L0042 + 0x18;
	L.L0044 = L.L0043 + 0x06;
	L.L0046 = L.L0045 + 0x06;
	L.L0047 = L.L0046 + 0x54;
	L.L0048 = L.L0047 + 0x02;
	L.L0049 = L.L0048 + 0x04;
	L.L004A = L.L0049 + 0x02;
	L.L004C = L.L004B + 0x06;
	L.L004D = L.L004C + 0x06;
	L.L004E = L.L004C + 0x0D;
	L.L004F = L.L004C + 0x18;

	return true;
}


static inline uint16 addByteToLowWord(uint16 w, uint8 b) {
	uint8 lo = (uint8)(w & 0xFF);
	lo = (uint8)(lo + b);
	return (uint16)((w & 0xFF00) | lo);
}

static inline uint16 shiftLeft16(uint16 v, unsigned n) {
	n &= 15;
	return (uint16)(v << n);
}

static inline uint16 rotateLeft16(uint16 v, unsigned n) {
	n &= 15;
	return (uint16)((v << n) | (v >> (16 - n)));
}

static inline uint16 updateMixerWordForVoiceType(uint16 mixerWord, uint16 channelMask, uint8 typeByte) {
	uint16 d3 = mixerWord;
	uint16 d2 = channelMask;

	if (typeByte == 1) {
		d3 = (uint16)(d3 | d2);
		d2 = shiftLeft16(d2, 3);
		d2 = (uint16)~d2;
		d3 = (uint16)(d3 & d2);
		return d3;
	}

	if (typeByte == 2) {
		d2 = (uint16)~d2;
		d3 = (uint16)(d3 & d2);
		d2 = rotateLeft16(d2, 3);
		d3 = (uint16)(d3 & d2);
		return d3;
	}

	d2 = (uint16)~d2;
	d3 = (uint16)(d3 & d2);
	d2 = (uint16)~d2;
	d2 = shiftLeft16(d2, 3);
	d3 = (uint16)(d3 | d2);
	return d3;
}

class ElviraPrgDriver {
public:
	ElviraPrgDriver(ElviraAtariSTPlayer *owner, const Common::Array<uint8> &prgBytes) : _owner(owner) {
		ElviraPrgTosImage prg;
		if (!prg.load(prgBytes)) {
			warning("AGOS: Failed to load ELVIRA.PRG");
			return;
		}
		_mem = prg.mem;
		if (!locateEmbeddedDriverLayout(_mem, _labels, prg.textSize)) {
			warning("AGOS: Failed to locate embedded ELVIRA audio driver");
			return;
		}
		_valid = true;
	}

	bool isValid() const { return _valid; }

	void init(uint16 tune1Based) {
		if (!_valid)
			return;
		_savedD7 = 0;
		WRITE_BE_UINT32(&_mem[_labels.L003E], 0x00010001);
		WRITE_BE_UINT16(&_mem[_labels.L003E + 4], 1);
		WRITE_BE_UINT32(&_mem[_labels.L0040 + 0], _labels.L0068);
		WRITE_BE_UINT32(&_mem[_labels.L003F + 0], _labels.L0068);
		WRITE_BE_UINT32(&_mem[_labels.L0040 + 4], _labels.L0068);
		WRITE_BE_UINT32(&_mem[_labels.L003F + 4], _labels.L0068);
		WRITE_BE_UINT32(&_mem[_labels.L0040 + 8], _labels.L0068);
		WRITE_BE_UINT32(&_mem[_labels.L003F + 8], _labels.L0068);

		uint16 d0w = (uint16)((tune1Based - 1) << 4);
		uint32 a1 = _labels.tunetab;
		WRITE_BE_UINT32(&_mem[_labels.L0041 + 0], READ_BE_UINT32(&_mem[a1 + d0w + 0]));
		WRITE_BE_UINT32(&_mem[_labels.L0041 + 4], READ_BE_UINT32(&_mem[a1 + d0w + 4]));
		WRITE_BE_UINT32(&_mem[_labels.L0041 + 8], READ_BE_UINT32(&_mem[a1 + d0w + 8]));
		WRITE_BE_UINT16(&_mem[_labels.L0049], 0xFFFF);
		WRITE_BE_UINT16(&_mem[_labels.L0048 + 0], 0);
		WRITE_BE_UINT16(&_mem[_labels.L0048 + 2], 0);
		WRITE_BE_UINT16(&_mem[_labels.L0047], 0);
	}

	void vbl() {
		if (!_valid)
			return;
		updateActivePitchSlides_L0017();
		updatePerChannelVibrato_L0025();
		updateSequencePitchLayers_L0020();
		updatePendingMixerRestores_L002B();
		updateVolumeEnvelopeStages_L002E();
		updateAutomaticEnvelopeWrites_L0037();

		int16 d7 = 2;
		uint16 d0 = 0;
		uint32 a0 = _labels.L003E;
		while (true) {
			uint16 v = (uint16)(READ_BE_UINT16(&_mem[a0 + d0]) - 1);
			WRITE_BE_UINT16(&_mem[a0 + d0], v);
			if (v == 0)
				processChannelCommandStream_L0004(d0);
			d0 = (uint16)(d0 + 2);
			if (--d7 < 0)
				break;
		}
	}

private:
	ElviraAtariSTPlayer *_owner;
	bool _valid = false;
	Common::Array<uint8> _mem;
	ElviraPrgLabels _labels;
	uint8 _savedD7 = 0;

	uint8 read8(uint32 a) const {
		return _mem[a];
	}
	void write8(uint32 a, uint8 v) {
		_mem[a] = v;
	}
	void psgWrite(uint8 reg, uint8 val) {
		_owner->writeReg((int)reg, val);
	}

	void processChannelCommandStream_L0004(uint16 d0);
	void applyImmediateMuteCommand_L0012(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3);
	void loadVoiceInstrument_L0013(uint16 d0, uint32 &a3);
	void setupPitchSlideCommand_L0015(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3, uint16 &slideArmed);
	void finalizePitchSlideStep_L0016(uint16 d5x2, uint16 period);
	void emitNoiseModeNote_L000F(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3);
	void setupSequenceTableCommand_L001B(uint16 d0, uint16 d6, uint32 &a3);

	void updateActivePitchSlides_L0017();
	void updateSequencePitchLayers_L0020();
	void updatePerChannelVibrato_L0025();
	void updatePendingMixerRestores_L002B();
	void updateVolumeEnvelopeStages_L002E();
	void updateAutomaticEnvelopeWrites_L0037();
};

void ElviraPrgDriver::processChannelCommandStream_L0004(uint16 d0) {
	uint16 d5 = d0;
	uint16 d6 = (uint16)(d0 >> 1);
	uint16 d5x2 = (uint16)(d5 + d5);

	uint16 slotOffset = READ_BE_UINT16(&_mem[_labels.L004B + d0]);
	uint16 instrumentOffset = READ_BE_UINT16(&_mem[_labels.L004A + d0]);

	uint32 liveVoice = _labels.L004C;
	uint32 instrumentBase = _labels.L0051;
	write8(liveVoice + slotOffset + 1, read8(instrumentBase + instrumentOffset + 1));
	WRITE_BE_UINT32(&_mem[liveVoice + slotOffset + 6], READ_BE_UINT32(&_mem[instrumentBase + instrumentOffset + 6]));
	WRITE_BE_UINT16(&_mem[liveVoice + slotOffset + 10], READ_BE_UINT16(&_mem[instrumentBase + instrumentOffset + 10]));
	write8(liveVoice + slotOffset + 13, read8(instrumentBase + instrumentOffset + 13));
	WRITE_BE_UINT32(&_mem[liveVoice + slotOffset + 14], READ_BE_UINT32(&_mem[instrumentBase + instrumentOffset + 14]));
	WRITE_BE_UINT32(&_mem[liveVoice + slotOffset + 18], READ_BE_UINT32(&_mem[instrumentBase + instrumentOffset + 18]));
	WRITE_BE_UINT16(&_mem[liveVoice + slotOffset + 22], READ_BE_UINT16(&_mem[instrumentBase + instrumentOffset + 22]));
	write8(liveVoice + slotOffset + 24, read8(instrumentBase + instrumentOffset + 24));

	uint16 seqBase = READ_BE_UINT16(&_mem[_labels.L0045 + d0]);
	WRITE_BE_UINT16(&_mem[_labels.L0046 + seqBase + 2], 0);

	uint32 patternBlob = _labels.L0068;
	uint32 patternPtrTable = _labels.L003F;
	uint32 a3 = READ_BE_UINT32(&_mem[patternPtrTable + d5x2]);
	uint16 cmd = READ_BE_UINT16(&_mem[a3]);
	a3 += 2;

	if ((int16)cmd < 0) {
		uint32 seqCursorTable = _labels.L0040;
		uint32 seqCursor = READ_BE_UINT32(&_mem[seqCursorTable + d5x2]);
		uint32 sequenceWord = READ_BE_UINT16(&_mem[seqCursor]);
		seqCursor += 2;

		if ((uint16)sequenceWord == 0xFFFF) {
			seqCursor = READ_BE_UINT32(&_mem[_labels.L0041 + d5x2]);
			sequenceWord = READ_BE_UINT16(&_mem[seqCursor]);
			seqCursor += 2;
		}

		write8(_labels.L0048 + d6, 0);
		if (sequenceWord & 0x8000) {
			write8(_labels.L0048 + d6, read8(seqCursor + 1));
			seqCursor += 2;
			sequenceWord &= 0x7FFF;
		}

		WRITE_BE_UINT32(&_mem[seqCursorTable + d5x2], seqCursor);
		uint16 patternOff = READ_BE_UINT16(&_mem[_labels.L0067 + (uint16)(sequenceWord * 2)]);
		a3 = patternBlob + patternOff;
		cmd = READ_BE_UINT16(&_mem[a3]);
		a3 += 2;
	}

	if (cmd & 0x0001) {
		applyImmediateMuteCommand_L0012(d0, d5x2, d6, a3);
		return;
	}
	if (cmd & 0x0002)
		loadVoiceInstrument_L0013(d0, a3);
	if (cmd & 0x0010)
		setupSequenceTableCommand_L001B(d0, d6, a3);
	if (cmd & 0x0008)
		WRITE_BE_UINT16(&_mem[_labels.L0047], (uint16)(READ_BE_UINT16(&_mem[_labels.L0047]) | 1));

	uint16 slideArmed = 0;
	if (cmd & 0x0004)
		setupPitchSlideCommand_L0015(d0, d5x2, d6, a3, slideArmed);

	uint16 wait = READ_BE_UINT16(&_mem[a3]);
	a3 += 2;
	WRITE_BE_UINT16(&_mem[_labels.L003E + d0], wait);

	uint16 mask = READ_BE_UINT16(&_mem[_labels.L003D + d0]);
	uint16 note = READ_BE_UINT16(&_mem[a3]);
	a3 += 2;
	if (_labels.L003B)
		WRITE_BE_UINT16(&_mem[_labels.L003B], note);
	note = addByteToLowWord(note, read8(_labels.L0048 + d6));
	write8(_labels.L0044 + d6, (uint8)(note & 0xFF));

	uint16 slot = READ_BE_UINT16(&_mem[_labels.L004B + d0]);
	note = addByteToLowWord(note, read8(_labels.L004C + slot + 23));
	uint16 idx = (uint16)((note & 0xFF) * 2);
	uint16 period = READ_BE_UINT16(&_mem[_labels.L003C + idx]);
	WRITE_BE_UINT16(&_mem[_labels.L0043 + d0], period);

	if (slideArmed)
		finalizePitchSlideStep_L0016(d5x2, period);

	if (READ_BE_UINT16(&_mem[_labels.L0047]) != 0) {
		WRITE_BE_UINT16(&_mem[_labels.L0047], 0);
		psgWrite((uint8)d0, (uint8)(period & 0xFF));
		psgWrite((uint8)(d0 + 1), (uint8)(period >> 8));
		WRITE_BE_UINT32(&_mem[_labels.L003F + d5x2], a3);
		return;
	}

	uint8 mode = read8(_labels.L004C + slot + 0);
	if (mode == 1) {
		emitNoiseModeNote_L000F(d0, d5x2, d6, a3);
		return;
	}

	psgWrite((uint8)d0, (uint8)(period & 0xFF));
	psgWrite((uint8)(d0 + 1), (uint8)(period >> 8));
	if (mode == 2)
		psgWrite(6, read8(_labels.L004C + slot + 12));

	uint16 mixer = READ_BE_UINT16(&_mem[_labels.L0049]);
	mixer = updateMixerWordForVoiceType(mixer, mask, mode == 2 ? 2 : 0);
	WRITE_BE_UINT16(&_mem[_labels.L0049], mixer);
	psgWrite(7, (uint8)(mixer & 0xFF));

	psgWrite((uint8)(8 + d6), read8(_labels.L004C + slot + 1));
	uint8 shape = read8(_labels.L004C + slot + 5);
	if (shape != 0) {
		psgWrite(0x0D, shape);
		psgWrite(0x0B, read8(_labels.L004C + slot + 3));
		psgWrite(0x0C, read8(_labels.L004C + slot + 2));
	}

	WRITE_BE_UINT32(&_mem[_labels.L003F + d5x2], a3);
}

void ElviraPrgDriver::applyImmediateMuteCommand_L0012(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3) {
	uint16 wait = READ_BE_UINT16(&_mem[a3]);
	a3 += 2;
	WRITE_BE_UINT16(&_mem[_labels.L003E + d0], wait);

	uint16 mask = READ_BE_UINT16(&_mem[_labels.L003D + d0]);
	uint16 mixer = READ_BE_UINT16(&_mem[_labels.L0049]);
	mixer &= (uint16)~mask;
	WRITE_BE_UINT16(&_mem[_labels.L0049], mixer);
	psgWrite((uint8)(8 + d6), 0);
	WRITE_BE_UINT32(&_mem[_labels.L003F + d5x2], a3);

	uint16 slot = READ_BE_UINT16(&_mem[_labels.L004B + d0]);
	write8(_labels.L004C + slot + 14, 0);
}

void ElviraPrgDriver::loadVoiceInstrument_L0013(uint16 d0, uint32 &a3) {
	uint16 instId = READ_BE_UINT16(&_mem[a3]);
	a3 += 2;

	uint16 d2 = (uint16)(instId * 2);
	uint16 d4 = d2;
	uint16 d2a = (uint16)(d2 << 2);
	uint16 d3 = d2a;
	d2a = (uint16)(d2a + d2a);
	d2a = (uint16)(d2a + d3);
	d2a = (uint16)(d2a + d4);

	WRITE_BE_UINT16(&_mem[_labels.L004A + d0], d2a);

	uint16 slot = READ_BE_UINT16(&_mem[_labels.L004B + d0]);
	uint32 dst = _labels.L004C + slot;
	uint32 src = _labels.L0051 + d2a;
	for (int i = 0; i <= 0x0C; ++i) {
		WRITE_BE_UINT16(&_mem[dst], READ_BE_UINT16(&_mem[src]));
		src += 2;
		dst += 2;
	}

	uint16 seqBase = READ_BE_UINT16(&_mem[_labels.L0045 + d0]);
	WRITE_BE_UINT16(&_mem[_labels.L0046 + seqBase + 0], 0);
}

void ElviraPrgDriver::setupPitchSlideCommand_L0015(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3, uint16 &slideArmed) {
	uint8 channelTranspose = read8(_labels.L0048 + d6);
	uint32 slideState = _labels.L0042;
	uint32 periodTable = _labels.L003C;
	uint16 d3 = (uint16)(d5x2 + d5x2);

	uint16 noteWord = READ_BE_UINT16(&_mem[a3]);
	a3 += 2;
	noteWord = addByteToLowWord(noteWord, channelTranspose);

	WRITE_BE_UINT16(&_mem[slideState + d3 + 2], READ_BE_UINT16(&_mem[a3]));
	a3 += 2;

	uint16 idx = (uint16)((noteWord & 0xFF) * 2);
	uint16 targetPeriod = READ_BE_UINT16(&_mem[periodTable + idx]);
	WRITE_BE_UINT16(&_mem[slideState + d3 + 4], targetPeriod);
	WRITE_BE_UINT16(&_mem[slideState + d3 + 0], targetPeriod);
	WRITE_BE_UINT16(&_mem[slideState + d3 + 6], READ_BE_UINT16(&_mem[a3]));
	a3 += 2;

	slideArmed = 1;
}

void ElviraPrgDriver::finalizePitchSlideStep_L0016(uint16 d5x2, uint16 period) {
	uint32 slideState = _labels.L0042;
	uint16 d4 = (uint16)(d5x2 + d5x2);
	uint32 d1u32 = (uint32)READ_BE_UINT16(&_mem[slideState + d4]);
	int32 dividend = (int32)d1u32 - (int32)((uint32)period & 0xFFFF);
	int16 divisor = (int16)READ_BE_UINT16(&_mem[slideState + d4 + 2]);
	int32 quotient = 0;
	if (divisor != 0)
		quotient = dividend / (int32)divisor;
	WRITE_BE_UINT16(&_mem[slideState + d4], (uint16)(quotient & 0xFFFF));
}

void ElviraPrgDriver::emitNoiseModeNote_L000F(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3) {
	uint16 rawNote = _labels.L003B ? READ_BE_UINT16(&_mem[_labels.L003B]) : 0;
	WRITE_BE_UINT16(&_mem[_labels.L0043 + d0], rawNote);
	psgWrite(6, (uint8)(rawNote & 0xFF));

	uint16 mask = READ_BE_UINT16(&_mem[_labels.L003D + d0]);
	uint16 mixer = READ_BE_UINT16(&_mem[_labels.L0049]);
	mixer |= mask;
	uint16 d2 = shiftLeft16(mask, 3);
	d2 = (uint16)~d2;
	mixer &= d2;
	WRITE_BE_UINT16(&_mem[_labels.L0049], mixer);
	psgWrite(7, (uint8)(mixer & 0xFF));

	uint16 slot = READ_BE_UINT16(&_mem[_labels.L004B + d0]);
	psgWrite((uint8)(8 + d6), read8(_labels.L004C + slot + 1));
	uint8 shape = read8(_labels.L004C + slot + 5);
	if (shape != 0) {
		psgWrite(0x0D, shape);
		psgWrite(0x0B, read8(_labels.L004C + slot + 3));
		psgWrite(0x0C, read8(_labels.L004C + slot + 2));
	}

	WRITE_BE_UINT32(&_mem[_labels.L003F + d5x2], a3);
}

void ElviraPrgDriver::setupSequenceTableCommand_L001B(uint16 d0, uint16 d6, uint32 &a3) {
	uint16 slot = READ_BE_UINT16(&_mem[_labels.L004B + d0]);

	uint8 saveD7 = _savedD7;
	_savedD7 = read8(_labels.L004C + slot + 0) != 0 ? 1 : 0;

	uint32 sequenceTranspose = _labels.L0048;
	uint32 sequenceState = _labels.L0046;
	uint32 sequenceStateIndex = _labels.L0045;
	uint16 stateBase = READ_BE_UINT16(&_mem[sequenceStateIndex + d0]);

	uint32 periodTable = _labels.L003C;
	uint16 count = READ_BE_UINT16(&_mem[a3]);
	a3 += 2;
	WRITE_BE_UINT16(&_mem[sequenceState + stateBase + 0], 0);
	uint16 countTimes2 = (uint16)(count + count);
	WRITE_BE_UINT16(&_mem[sequenceState + stateBase + 2], countTimes2);
	uint16 halfCount = (uint16)(countTimes2 >> 1);
	WRITE_BE_UINT16(&_mem[sequenceState + stateBase + 4], 1);
	WRITE_BE_UINT16(&_mem[sequenceState + stateBase + 6], READ_BE_UINT16(&_mem[a3]));
	a3 += 2;

	for (int16 loop = (int16)(halfCount - 2); loop >= 0; --loop) {
		uint16 d4 = READ_BE_UINT16(&_mem[a3]);
		a3 += 2;
		d4 = addByteToLowWord(d4, read8(sequenceTranspose + d6));
		if (_savedD7 != 0) {
			WRITE_BE_UINT16(&_mem[sequenceState + stateBase + 10], d4);
		} else {
			uint16 idx = (uint16)(d4 + d4);
			WRITE_BE_UINT16(&_mem[sequenceState + stateBase + 10], READ_BE_UINT16(&_mem[periodTable + idx]));
		}
		stateBase = (uint16)(stateBase + 2);
	}

	_savedD7 = saveD7;
}


void ElviraPrgDriver::updateActivePitchSlides_L0017() {
	int16 d7 = 2;
	uint16 d2 = 0;
	uint32 a0 = _labels.L0042;
	uint32 a1 = _labels.L0043;

	while (true) {
		int16 delta = (int16)READ_BE_UINT16(&_mem[a0]);
		if (delta != 0) {
			int16 countdown = (int16)READ_BE_UINT16(&_mem[a0 + 6]);
			bool doStep = false;
			if (countdown < 0) {
				doStep = true;
			} else {
				countdown = (int16)(countdown - 1);
				WRITE_BE_UINT16(&_mem[a0 + 6], (uint16)countdown);
				if (countdown < 0)
					doStep = true;
			}

			if (doStep) {
				uint16 t = (uint16)((int16)READ_BE_UINT16(&_mem[a1]) + (int16)READ_BE_UINT16(&_mem[a0]));
				WRITE_BE_UINT16(&_mem[a1], t);
				psgWrite((uint8)d2, (uint8)(t & 0xFF));
				psgWrite((uint8)(d2 + 1), (uint8)(t >> 8));

				uint16 count = (uint16)(READ_BE_UINT16(&_mem[a0 + 2]) - 1);
				WRITE_BE_UINT16(&_mem[a0 + 2], count);
				if (count == 0) {
					uint16 target = READ_BE_UINT16(&_mem[a0 + 4]);
					psgWrite((uint8)d2, (uint8)(target & 0xFF));
					psgWrite((uint8)(d2 + 1), (uint8)(target >> 8));
					WRITE_BE_UINT16(&_mem[a1], target);
					WRITE_BE_UINT16(&_mem[a0], 0);
				}
			}
		}

		a0 += 8;
		a1 += 2;
		d2 = (uint16)(d2 + 2);
		if (--d7 < 0)
			break;
	}
}

void ElviraPrgDriver::updateSequencePitchLayers_L0020() {
	uint32 voiceState = _labels.L004C;
	uint32 sequenceState = _labels.L0046;
	uint32 currentPeriods = _labels.L0043;
	uint16 channelReg = 0;
	int16 channelCount = 2;

	while (true) {
		uint16 basePeriod = READ_BE_UINT16(&_mem[currentPeriods]);
		WRITE_BE_UINT16(&_mem[sequenceState + 8], basePeriod);
		currentPeriods += 2;

		uint16 seqLen = READ_BE_UINT16(&_mem[sequenceState + 2]);
		if (seqLen != 0) {
			uint16 countdown = (uint16)(READ_BE_UINT16(&_mem[sequenceState + 4]) - 1);
			WRITE_BE_UINT16(&_mem[sequenceState + 4], countdown);
			if (countdown == 0) {
				uint16 reload = READ_BE_UINT16(&_mem[sequenceState + 6]);
				WRITE_BE_UINT16(&_mem[sequenceState + 4], reload);

				uint16 idx = READ_BE_UINT16(&_mem[sequenceState + 0]);
				uint16 tablePeriod = READ_BE_UINT16(&_mem[sequenceState + 8 + idx]);
				tablePeriod = addByteToLowWord(tablePeriod, read8(voiceState + 6));

				if (read8(voiceState + 0) == 1) {
					psgWrite(6, (uint8)(tablePeriod & 0xFF));
				} else {
					psgWrite((uint8)channelReg, (uint8)(tablePeriod & 0xFF));
					psgWrite((uint8)(channelReg + 1), (uint8)((tablePeriod >> 8) & 0xFF));
				}

				uint16 nextIdx = (uint16)(idx + 2);
				if (nextIdx >= seqLen)
					nextIdx = 0;
				WRITE_BE_UINT16(&_mem[sequenceState + 0], nextIdx);
			}
		}

		sequenceState += 28;
		channelReg += 2;
		voiceState += 26;
		if (--channelCount < 0)
			break;
	}
}

void ElviraPrgDriver::updatePerChannelVibrato_L0025() {
	uint32 vibratoState = _labels.L004D;
	uint32 currentPeriods = _labels.L0043;
	uint32 sequenceState = _labels.L0046;
	int16 channelCount = 2;
	uint16 channelReg = 0;

	while (true) {
		bool skipOutput = false;

		if (read8(vibratoState + 1) != 0) {
			if ((int8)read8(vibratoState + 5) >= 0) {
				uint8 v = (uint8)(read8(vibratoState + 5) - 1);
				write8(vibratoState + 5, v);
				if ((int8)v >= 0)
					skipOutput = true;
			}

			if (!skipOutput) {
				uint8 pos = read8(vibratoState + 0);
				if (read8(vibratoState + 2) == 0) {
					pos = (uint8)(pos + read8(vibratoState + 1));
					write8(vibratoState + 0, pos);
					if (pos == read8(vibratoState + 3))
						write8(vibratoState + 2, (uint8)(read8(vibratoState + 2) ^ 1));
				} else {
					pos = (uint8)(pos - read8(vibratoState + 1));
					write8(vibratoState + 0, pos);
					if (pos == read8(vibratoState + 4))
						write8(vibratoState + 2, (uint8)(read8(vibratoState + 2) ^ 1));
				}

				if (READ_BE_UINT16(&_mem[sequenceState + 2]) == 0) {
					uint16 out = (uint16)((int16)READ_BE_UINT16(&_mem[currentPeriods]) + (int16)(int8)pos);
					psgWrite((uint8)channelReg, (uint8)(out & 0xFF));
					psgWrite((uint8)(channelReg + 1), (uint8)(out >> 8));
				}
			}
		}

		vibratoState += 26;
		currentPeriods += 2;
		sequenceState += 28;
		channelReg = (uint16)(channelReg + 2);
		if (--channelCount < 0)
			break;
	}
}

void ElviraPrgDriver::updatePendingMixerRestores_L002B() {
	uint32 delayState = _labels.L004E;
	int16 channelCount = 2;
	uint16 mixerMask = 8;

	while (true) {
		if (read8(delayState) != 0) {
			uint8 v = (uint8)(read8(delayState) - 1);
			write8(delayState, v);
			if (v == 0) {
				uint16 mixer = (uint16)(READ_BE_UINT16(&_mem[_labels.L0049]) | mixerMask);
				WRITE_BE_UINT16(&_mem[_labels.L0049], mixer);
				psgWrite(7, (uint8)(mixer & 0xFF));
			}
		}
		mixerMask = (uint16)(mixerMask + mixerMask);
		delayState += 26;
		if (--channelCount < 0)
			break;
	}
}

void ElviraPrgDriver::updateVolumeEnvelopeStages_L002E() {
	uint32 voiceState = _labels.L004C;
	int16 channelCount = 2;
	uint8 volumeReg = 8;

	while (true) {
		uint8 stage = read8(voiceState + 14);
		if (stage != 0) {
			if (stage == 1) {
				uint8 counter = (uint8)(read8(voiceState + 22) + 1);
				if (counter == read8(voiceState + 15)) {
					write8(voiceState + 1, (uint8)(read8(voiceState + 1) + read8(voiceState + 16)));
					psgWrite(volumeReg, read8(voiceState + 1));
					uint8 c = (uint8)(read8(voiceState + 17) - 1);
					write8(voiceState + 17, c);
					if (c == 0)
						write8(voiceState + 14, (uint8)(read8(voiceState + 14) + 1));
					counter = 0;
				}
				write8(voiceState + 22, counter);
			} else if (stage == 2) {
				uint8 c = (uint8)(read8(voiceState + 18) - 1);
				write8(voiceState + 18, c);
				if (c == 0)
					write8(voiceState + 14, (uint8)(read8(voiceState + 14) + 1));
			} else {
				uint8 counter = (uint8)(read8(voiceState + 22) + 1);
				if (counter == read8(voiceState + 19)) {
					write8(voiceState + 1, (uint8)(read8(voiceState + 1) - read8(voiceState + 20)));
					psgWrite(volumeReg, read8(voiceState + 1));
					uint8 c = (uint8)(read8(voiceState + 21) - 1);
					write8(voiceState + 21, c);
					if (c == 0)
						write8(voiceState + 14, 0);
					counter = 0;
				}
				write8(voiceState + 22, counter);
			}
		}
		voiceState += 26;
		++volumeReg;
		if (--channelCount < 0)
			break;
	}
}

void ElviraPrgDriver::updateAutomaticEnvelopeWrites_L0037() {
	uint32 voiceState = _labels.L004C;
	int16 channelCount = 2;
	uint8 volumeReg = 8;

	while (true) {
		if (read8(voiceState + 5) != 0) {
			psgWrite(0x0D, read8(voiceState + 5));
			psgWrite(0x0B, read8(voiceState + 3));
			psgWrite(0x0C, read8(voiceState + 2));
			psgWrite(volumeReg, read8(voiceState + 1));
		}
		voiceState += 26;
		++volumeReg;
		if (--channelCount < 0)
			break;
	}
}



ElviraAtariSTPlayer::ElviraAtariSTPlayer(Common::SeekableReadStream *stream)
	: _stream(stream) {
	if (!_stream)
		error("Atari ST YM player: null stream");

	_stream->seek(0, SEEK_SET);
	const uint32 sz = (uint32)_stream->size();
	_data.resize(sz);
	if (sz && _stream->read(&_data[0], sz) != sz)
		error("Atari ST YM player: read failed");

	_mode = kModeElvira2PKD;
	_frameHz = kDefaultFrameHz;
	_chip = YM2149::Config::create();
	if (!_chip || !_chip->init())
		error("Atari ST YM player: failed to initialise YM2149 core");

	resetPlayer();
	_chip->start(new Common::Functor0Mem<void, ElviraAtariSTPlayer>(this, &ElviraAtariSTPlayer::onTimer), _frameHz);
}

ElviraAtariSTPlayer::ElviraAtariSTPlayer(Common::SeekableReadStream *stream, uint16 elvira1Tune)
	: _stream(stream), _elvira1Tune(elvira1Tune) {
	if (!_stream)
		error("Atari ST YM player: null stream");

	_stream->seek(0, SEEK_SET);
	const uint32 sz = (uint32)_stream->size();
	_data.resize(sz);
	if (sz && _stream->read(&_data[0], sz) != sz)
		error("Atari ST YM player: read failed");

	_mode = kModeElvira1PRG;
	_frameHz = kElvira1PrgFrameHz;
	_chip = YM2149::Config::create();
	if (!_chip || !_chip->init())
		error("Atari ST YM player: failed to initialise YM2149 core");

	_elviraPrgDriver = new ElviraPrgDriver(this, _data);
	if (!_elviraPrgDriver->isValid()) {
		delete _elviraPrgDriver;
		_elviraPrgDriver = nullptr;
		_isValid = false;
		return;
	}

	resetPlayer();
	_chip->start(new Common::Functor0Mem<void, ElviraAtariSTPlayer>(this, &ElviraAtariSTPlayer::onTimer), _frameHz);
}

ElviraAtariSTPlayer::~ElviraAtariSTPlayer() {
	if (_chip)
		_chip->stop();
	delete _elviraPrgDriver;
	delete _chip;
}

void ElviraAtariSTPlayer::writeReg(int reg, uint8 data) {
	if (_chip)
		_chip->writeReg(reg, data);
}

void ElviraAtariSTPlayer::resetPlayer() {
	_pos = 0;
	_ended = false;
	_framesLeftInWait = 0;
	if (_chip)
		_chip->reset();

	if (_mode == kModeElvira1PRG) {
		if (_elviraPrgDriver)
			_elviraPrgDriver->init(_elvira1Tune);
		else
			_ended = true;
		return;
	}

	parseUntilWait();
}

void ElviraAtariSTPlayer::parseUntilWait() {
	if (_mode == kModeElvira1PRG) {
		if (_elviraPrgDriver)
			_elviraPrgDriver->vbl();
		else
			_ended = true;
		return;
	}

	if (_ended)
		return;

	while (_pos + 1 < _data.size()) {
		const uint8 r = _data[_pos++];
		const uint8 v = _data[_pos++];

		if (r == 0xFF) {
			if (v == 0) {
				resetPlayer();
				return;
			}
			_framesLeftInWait = v;
			return;
		}

		writeReg((int)r, v);
	}

	_ended = true;
}

void ElviraAtariSTPlayer::onTimer() {
	if (_mode == kModeElvira2PKD && _framesLeftInWait) {
		--_framesLeftInWait;
		if (_framesLeftInWait)
			return;
	}

	parseUntilWait();
}

} // namespace AGOS
