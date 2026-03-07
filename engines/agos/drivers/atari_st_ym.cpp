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

/* Adapted from Hatari - https://framagit.org/hatari/hatari/
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Soft-
 * ware Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <https://www.gnu.org/licenses/>.
 *
 * Linking Hatari statically or dynamically with other modules is making a
 * combined work based on Hatari. Thus, the terms and conditions of the GNU
 * General Public License cover the whole combination.\n
 *
 * In addition, as a special exception, the copyright holders of Hatari give you
 * permission to combine Hatari with free software programs or libraries that are
 * released under the GNU LGPL and with code included in the standard release
 * of the IPF support library (a.k.a. libcapsimage) under the Software Preservation
 * Society Licence Agreement.
 */

#include "atari_st_ym.h"

#include "common/endian.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "common/debug.h"

#include <cmath>
#include <cstring>

namespace Audio {


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
	// Full game
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
		return applyKnownElviraDriverLayout(textSize, L);

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
	ElviraPrgDriver(AtariSTYMStream *owner, const Common::Array<uint8> &prgBytes) : _owner(owner) {
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
		write32(_labels.L003E, 0x00010001);
		write16(_labels.L003E + 4, 1);
		write32(_labels.L0040 + 0, _labels.L0068);
		write32(_labels.L003F + 0, _labels.L0068);
		write32(_labels.L0040 + 4, _labels.L0068);
		write32(_labels.L003F + 4, _labels.L0068);
		write32(_labels.L0040 + 8, _labels.L0068);
		write32(_labels.L003F + 8, _labels.L0068);

		uint16 d0w = (uint16)((tune1Based - 1) << 4);
		uint32 a1 = _labels.tunetab;
		write32(_labels.L0041 + 0, read32(a1 + d0w + 0));
		write32(_labels.L0041 + 4, read32(a1 + d0w + 4));
		write32(_labels.L0041 + 8, read32(a1 + d0w + 8));
		write16(_labels.L0049, 0xFFFF);
		write16(_labels.L0048 + 0, 0);
		write16(_labels.L0048 + 2, 0);
		write16(_labels.L0047, 0);
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
			uint16 v = (uint16)(read16(a0 + d0) - 1);
			write16(a0 + d0, v);
			if (v == 0)
				processChannelCommandStream_L0004(d0);
			d0 = (uint16)(d0 + 2);
			if (--d7 < 0)
				break;
		}
	}

private:
	AtariSTYMStream *_owner;
	bool _valid = false;
	Common::Array<uint8> _mem;
	ElviraPrgLabels _labels;
	uint8 _savedD7 = 0;

	uint8 read8(uint32 a) const {
		return _mem[a];
	}
	uint16 read16(uint32 a) const {
		return READ_BE_UINT16(&_mem[a]);
	}
	uint32 read32(uint32 a) const {
		return READ_BE_UINT32(&_mem[a]);
	}
	void write8(uint32 a, uint8 v) {
		_mem[a] = v;
	}
	void write16(uint32 a, uint16 v) {
		_mem[a] = (uint8)(v >> 8);
		_mem[a + 1] = (uint8)v;
	}
	void write32(uint32 a, uint32 v) {
		WRITE_BE_UINT32(&_mem[a], v);
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

	uint16 slotOffset = read16(_labels.L004B + d0);
	uint16 instrumentOffset = read16(_labels.L004A + d0);

	uint32 liveVoice = _labels.L004C;
	uint32 instrumentBase = _labels.L0051;
	write8(liveVoice + slotOffset + 1, read8(instrumentBase + instrumentOffset + 1));
	write32(liveVoice + slotOffset + 6, read32(instrumentBase + instrumentOffset + 6));
	write16(liveVoice + slotOffset + 10, read16(instrumentBase + instrumentOffset + 10));
	write8(liveVoice + slotOffset + 13, read8(instrumentBase + instrumentOffset + 13));
	write32(liveVoice + slotOffset + 14, read32(instrumentBase + instrumentOffset + 14));
	write32(liveVoice + slotOffset + 18, read32(instrumentBase + instrumentOffset + 18));
	write16(liveVoice + slotOffset + 22, read16(instrumentBase + instrumentOffset + 22));
	write8(liveVoice + slotOffset + 24, read8(instrumentBase + instrumentOffset + 24));

	uint16 seqBase = read16(_labels.L0045 + d0);
	write16(_labels.L0046 + seqBase + 2, 0);

	uint32 patternBlob = _labels.L0068;
	uint32 patternPtrTable = _labels.L003F;
	uint32 a3 = read32(patternPtrTable + d5x2);
	uint16 cmd = read16(a3);
	a3 += 2;

	if ((int16)cmd < 0) {
		uint32 seqCursorTable = _labels.L0040;
		uint32 seqCursor = read32(seqCursorTable + d5x2);
		uint32 sequenceWord = read16(seqCursor);
		seqCursor += 2;

		if ((uint16)sequenceWord == 0xFFFF) {
			seqCursor = read32(_labels.L0041 + d5x2);
			sequenceWord = read16(seqCursor);
			seqCursor += 2;
		}

		write8(_labels.L0048 + d6, 0);
		if (sequenceWord & 0x8000) {
			write8(_labels.L0048 + d6, read8(seqCursor + 1));
			seqCursor += 2;
			sequenceWord &= 0x7FFF;
		}

		write32(seqCursorTable + d5x2, seqCursor);
		uint16 patternOff = read16(_labels.L0067 + (uint16)(sequenceWord * 2));
		a3 = patternBlob + patternOff;
		cmd = read16(a3);
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
		write16(_labels.L0047, (uint16)(read16(_labels.L0047) | 1));

	uint16 slideArmed = 0;
	if (cmd & 0x0004)
		setupPitchSlideCommand_L0015(d0, d5x2, d6, a3, slideArmed);

	uint16 wait = read16(a3);
	a3 += 2;
	write16(_labels.L003E + d0, wait);

	uint16 mask = read16(_labels.L003D + d0);
	uint16 note = read16(a3);
	a3 += 2;
	if (_labels.L003B)
		write16(_labels.L003B, note);
	note = addByteToLowWord(note, read8(_labels.L0048 + d6));
	write8(_labels.L0044 + d6, (uint8)(note & 0xFF));

	uint16 slot = read16(_labels.L004B + d0);
	note = addByteToLowWord(note, read8(_labels.L004C + slot + 23));
	uint16 idx = (uint16)((note & 0xFF) * 2);
	uint16 period = read16(_labels.L003C + idx);
	write16(_labels.L0043 + d0, period);

	if (slideArmed)
		finalizePitchSlideStep_L0016(d5x2, period);

	if (read16(_labels.L0047) != 0) {
		write16(_labels.L0047, 0);
		psgWrite((uint8)d0, (uint8)(period & 0xFF));
		psgWrite((uint8)(d0 + 1), (uint8)(period >> 8));
		write32(_labels.L003F + d5x2, a3);
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

	uint16 mixer = read16(_labels.L0049);
	mixer = updateMixerWordForVoiceType(mixer, mask, mode == 2 ? 2 : 0);
	write16(_labels.L0049, mixer);
	psgWrite(7, (uint8)(mixer & 0xFF));

	psgWrite((uint8)(8 + d6), read8(_labels.L004C + slot + 1));
	uint8 shape = read8(_labels.L004C + slot + 5);
	if (shape != 0) {
		psgWrite(0x0D, shape);
		psgWrite(0x0B, read8(_labels.L004C + slot + 3));
		psgWrite(0x0C, read8(_labels.L004C + slot + 2));
	}

	write32(_labels.L003F + d5x2, a3);
}

void ElviraPrgDriver::applyImmediateMuteCommand_L0012(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3) {
	uint16 wait = read16(a3);
	a3 += 2;
	write16(_labels.L003E + d0, wait);

	uint16 mask = read16(_labels.L003D + d0);
	uint16 mixer = read16(_labels.L0049);
	mixer &= (uint16)~mask;
	write16(_labels.L0049, mixer);
	psgWrite((uint8)(8 + d6), 0);
	write32(_labels.L003F + d5x2, a3);

	uint16 slot = read16(_labels.L004B + d0);
	write8(_labels.L004C + slot + 14, 0);
}

void ElviraPrgDriver::loadVoiceInstrument_L0013(uint16 d0, uint32 &a3) {
	uint16 instId = read16(a3);
	a3 += 2;

	uint16 d2 = (uint16)(instId * 2);
	uint16 d4 = d2;
	uint16 d2a = (uint16)(d2 << 2);
	uint16 d3 = d2a;
	d2a = (uint16)(d2a + d2a);
	d2a = (uint16)(d2a + d3);
	d2a = (uint16)(d2a + d4);

	write16(_labels.L004A + d0, d2a);

	uint16 slot = read16(_labels.L004B + d0);
	uint32 dst = _labels.L004C + slot;
	uint32 src = _labels.L0051 + d2a;
	for (int i = 0; i <= 0x0C; ++i) {
		write16(dst, read16(src));
		src += 2;
		dst += 2;
	}

	uint16 seqBase = read16(_labels.L0045 + d0);
	write16(_labels.L0046 + seqBase + 0, 0);
}

void ElviraPrgDriver::setupPitchSlideCommand_L0015(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3, uint16 &slideArmed) {
	uint8 channelTranspose = read8(_labels.L0048 + d6);
	uint32 slideState = _labels.L0042;
	uint32 periodTable = _labels.L003C;
	uint16 d3 = (uint16)(d5x2 + d5x2);

	uint16 noteWord = read16(a3);
	a3 += 2;
	noteWord = addByteToLowWord(noteWord, channelTranspose);

	write16(slideState + d3 + 2, read16(a3));
	a3 += 2;

	uint16 idx = (uint16)((noteWord & 0xFF) * 2);
	uint16 targetPeriod = read16(periodTable + idx);
	write16(slideState + d3 + 4, targetPeriod);
	write16(slideState + d3 + 0, targetPeriod);
	write16(slideState + d3 + 6, read16(a3));
	a3 += 2;

	slideArmed = 1;
}

void ElviraPrgDriver::finalizePitchSlideStep_L0016(uint16 d5x2, uint16 period) {
	uint32 slideState = _labels.L0042;
	uint16 d4 = (uint16)(d5x2 + d5x2);
	uint32 d1u32 = (uint32)read16(slideState + d4);
	int32 dividend = (int32)d1u32 - (int32)((uint32)period & 0xFFFF);
	int16 divisor = (int16)read16(slideState + d4 + 2);
	int32 quotient = 0;
	if (divisor != 0)
		quotient = dividend / (int32)divisor;
	write16(slideState + d4, (uint16)(quotient & 0xFFFF));
}

void ElviraPrgDriver::emitNoiseModeNote_L000F(uint16 d0, uint16 d5x2, uint16 d6, uint32 &a3) {
	uint16 rawNote = _labels.L003B ? read16(_labels.L003B) : 0;
	write16(_labels.L0043 + d0, rawNote);
	psgWrite(6, (uint8)(rawNote & 0xFF));

	uint16 mask = read16(_labels.L003D + d0);
	uint16 mixer = read16(_labels.L0049);
	mixer |= mask;
	uint16 d2 = shiftLeft16(mask, 3);
	d2 = (uint16)~d2;
	mixer &= d2;
	write16(_labels.L0049, mixer);
	psgWrite(7, (uint8)(mixer & 0xFF));

	uint16 slot = read16(_labels.L004B + d0);
	psgWrite((uint8)(8 + d6), read8(_labels.L004C + slot + 1));
	uint8 shape = read8(_labels.L004C + slot + 5);
	if (shape != 0) {
		psgWrite(0x0D, shape);
		psgWrite(0x0B, read8(_labels.L004C + slot + 3));
		psgWrite(0x0C, read8(_labels.L004C + slot + 2));
	}

	write32(_labels.L003F + d5x2, a3);
}

void ElviraPrgDriver::setupSequenceTableCommand_L001B(uint16 d0, uint16 d6, uint32 &a3) {
	uint16 slot = read16(_labels.L004B + d0);

	uint8 saveD7 = _savedD7;
	_savedD7 = read8(_labels.L004C + slot + 0) != 0 ? 1 : 0;

	uint32 sequenceTranspose = _labels.L0048;
	uint32 sequenceState = _labels.L0046;
	uint32 sequenceStateIndex = _labels.L0045;
	uint16 stateBase = read16(sequenceStateIndex + d0);

	uint32 periodTable = _labels.L003C;
	uint16 count = read16(a3);
	a3 += 2;
	write16(sequenceState + stateBase + 0, 0);
	uint16 countTimes2 = (uint16)(count + count);
	write16(sequenceState + stateBase + 2, countTimes2);
	uint16 halfCount = (uint16)(countTimes2 >> 1);
	write16(sequenceState + stateBase + 4, 1);
	write16(sequenceState + stateBase + 6, read16(a3));
	a3 += 2;

	for (int16 loop = (int16)(halfCount - 2); loop >= 0; --loop) {
		uint16 d4 = read16(a3);
		a3 += 2;
		d4 = addByteToLowWord(d4, read8(sequenceTranspose + d6));
		if (_savedD7 != 0) {
			write16(sequenceState + stateBase + 10, d4);
		} else {
			uint16 idx = (uint16)(d4 + d4);
			write16(sequenceState + stateBase + 10, read16(periodTable + idx));
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
		int16 delta = (int16)read16(a0);
		if (delta != 0) {
			int16 countdown = (int16)read16(a0 + 6);
			bool doStep = false;
			if (countdown < 0) {
				doStep = true;
			} else {
				countdown = (int16)(countdown - 1);
				write16(a0 + 6, (uint16)countdown);
				if (countdown < 0)
					doStep = true;
			}

			if (doStep) {
				uint16 t = (uint16)((int16)read16(a1) + (int16)read16(a0));
				WRITE_BE_UINT16(&_mem[a1], t);
				psgWrite((uint8)d2, (uint8)(t & 0xFF));
				psgWrite((uint8)(d2 + 1), (uint8)(t >> 8));

				uint16 count = (uint16)(read16(a0 + 2) - 1);
				write16(a0 + 2, count);
				if (count == 0) {
					uint16 target = read16(a0 + 4);
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
		uint16 basePeriod = read16(currentPeriods);
		write16(sequenceState + 8, basePeriod);
		currentPeriods += 2;

		uint16 seqLen = read16(sequenceState + 2);
		if (seqLen != 0) {
			uint16 countdown = (uint16)(read16(sequenceState + 4) - 1);
			write16(sequenceState + 4, countdown);
			if (countdown == 0) {
				uint16 reload = read16(sequenceState + 6);
				write16(sequenceState + 4, reload);

				uint16 idx = read16(sequenceState + 0);
				uint16 tablePeriod = read16(sequenceState + 8 + idx);
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
				write16(sequenceState + 0, nextIdx);
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

				if (read16(sequenceState + 2) == 0) {
					uint16 out = (uint16)((int16)read16(currentPeriods) + (int16)(int8)pos);
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
				uint16 mixer = (uint16)(read16(_labels.L0049) | mixerMask);
				write16(_labels.L0049, mixer);
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

const uint32 AtariSTYMStream::YmVolume4to5[32] = {
	0, 1, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31,
	0, 1, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, 29, 31
};


const int AtariSTYMStream::YmEnvDef[16][3] = {
	{ ENV_GODOWN, ENV_DOWN, ENV_DOWN },
	{ ENV_GODOWN, ENV_DOWN, ENV_DOWN },
	{ ENV_GODOWN, ENV_DOWN, ENV_DOWN },
	{ ENV_GODOWN, ENV_DOWN, ENV_DOWN },
	{ ENV_GOUP, ENV_DOWN, ENV_DOWN },
	{ ENV_GOUP, ENV_DOWN, ENV_DOWN },
	{ ENV_GOUP, ENV_DOWN, ENV_DOWN },
	{ ENV_GOUP, ENV_DOWN, ENV_DOWN },
	{ ENV_GODOWN, ENV_GODOWN, ENV_GODOWN },
	{ ENV_GODOWN, ENV_DOWN, ENV_DOWN },
	{ ENV_GODOWN, ENV_GOUP, ENV_GODOWN },
	{ ENV_GODOWN, ENV_UP, ENV_UP },
	{ ENV_GOUP, ENV_GOUP, ENV_GOUP },
	{ ENV_GOUP, ENV_UP, ENV_UP },
	{ ENV_GOUP, ENV_GODOWN, ENV_GOUP },
	{ ENV_GOUP, ENV_DOWN, ENV_DOWN }
};


 /* Sixteen level by Three voice YM2149 volume_table[C][B][A]
 * Data measured by Paulo Simoes. Copyright 2012 Paulo Simoes.
 */

uint16 AtariSTYMStream::YmEnvWaves[16][32 * 3];

const uint16 AtariSTYMStream::volumeTable[16][16][16] =
{
	{
		{     0,   120,   251,   385,   598,   840,  1258,  1725,  2536,  3483,  5151,  7054, 10367, 14634, 22931, 34469 },
		{   120,   283,   419,   556,   771,  1009,  1433,  1894,  2702,  3645,  5310,  7207, 10508, 14757, 23015, 34480 },
		{   251,   419,   553,   692,   903,  1147,  1565,  2026,  2833,  3775,  5439,  7325, 10619, 14857, 23082, 34493 },
		{   385,   556,   692,   828,  1038,  1286,  1704,  2164,  2967,  3906,  5564,  7450, 10733, 14958, 23156, 34509 },
		{   598,   771,   903,  1038,  1261,  1502,  1925,  2382,  3176,  4113,  5764,  7649, 10915, 15123, 23272, 34538 },
		{   840,  1009,  1147,  1286,  1502,  1747,  2167,  2626,  3417,  4348,  5985,  7851, 11109, 15302, 23400, 34570 },
		{  1258,  1433,  1565,  1704,  1925,  2167,  2572,  3019,  3812,  4743,  6367,  8205, 11457, 15616, 23639, 34646 },
		{  1725,  1894,  2026,  2164,  2382,  2626,  3019,  3487,  4266,  5173,  6792,  8517, 11852, 15979, 23925, 34754 },
		{  2536,  2702,  2833,  2967,  3176,  3417,  3812,  4266,  5048,  5947,  7542,  9227, 12543, 16613, 24440, 34992 },
		{  3483,  3645,  3775,  3906,  4113,  4348,  4743,  5173,  5947,  6847,  8353, 10089, 13357, 17288, 25084, 35355 },
		{  5151,  5310,  5439,  5564,  5764,  5985,  6367,  6792,  7542,  8353,  9780, 11549, 14769, 18591, 26237, 36119 },
		{  7054,  7207,  7325,  7450,  7649,  7851,  8205,  8517,  9227, 10089, 11549, 13292, 16452, 20206, 27628, 37143 },
		{ 10367, 10508, 10619, 10733, 10915, 11109, 11457, 11852, 12543, 13357, 14769, 16452, 19431, 23228, 30481, 39499 },
		{ 14634, 14757, 14857, 14958, 15123, 15302, 15616, 15979, 16613, 17288, 18591, 20206, 23228, 26871, 34097, 42635 },
		{ 22931, 23015, 23082, 23156, 23272, 23400, 23639, 23925, 24440, 25084, 26237, 27628, 30481, 34097, 40524, 48336 },
		{ 34469, 34480, 34493, 34509, 34538, 34570, 34646, 34754, 34992, 35355, 36119, 37143, 39499, 42635, 48336, 55377 }
	},
	{
		{   120,   283,   419,   556,   771,  1009,  1433,  1894,  2702,  3645,  5310,  7207, 10508, 14757, 23015, 34480 },
		{   283,   453,   589,   723,   941,  1179,  1601,  2059,  2864,  3811,  5472,  7358, 10646, 14884, 23101, 34500 },
		{   419,   589,   727,   863,  1077,  1316,  1737,  2194,  2998,  3938,  5599,  7478, 10755, 14980, 23168, 34509 },
		{   556,   723,   863,   997,  1212,  1453,  1874,  2331,  3130,  4067,  5726,  7602, 10872, 15084, 23243, 34526 },
		{   771,   941,  1077,  1212,  1427,  1674,  2089,  2551,  3345,  4273,  5916,  7792, 11056, 15245, 23354, 34556 },
		{  1009,  1179,  1316,  1453,  1674,  1915,  2333,  2788,  3579,  4509,  6142,  7993, 11249, 15426, 23486, 34585 },
		{  1433,  1601,  1737,  1874,  2089,  2333,  2735,  3188,  3976,  4898,  6516,  8315, 11594, 15735, 23718, 34662 },
		{  1894,  2059,  2194,  2331,  2551,  2788,  3188,  3649,  4423,  5330,  6948,  8656, 11984, 16094, 24004, 34771 },
		{  2702,  2864,  2998,  3130,  3345,  3579,  3976,  4423,  5202,  6104,  7684,  9368, 12668, 16724, 24512, 35007 },
		{  3645,  3811,  3938,  4067,  4273,  4509,  4898,  5330,  6104,  6999,  8449, 10221, 13483, 17379, 25149, 35366 },
		{  5310,  5472,  5599,  5726,  5916,  6142,  6516,  6948,  7684,  8449,  9917, 11680, 14885, 18689, 26297, 36126 },
		{  7207,  7358,  7478,  7602,  7792,  7993,  8315,  8656,  9368, 10221, 11680, 13417, 16558, 20295, 27677, 37144 },
		{ 10508, 10646, 10755, 10872, 11056, 11249, 11594, 11984, 12668, 13483, 14885, 16558, 19523, 23300, 30508, 39500 },
		{ 14757, 14884, 14980, 15084, 15245, 15426, 15735, 16094, 16724, 17379, 18689, 20295, 23300, 26918, 34108, 42636 },
		{ 23015, 23101, 23168, 23243, 23354, 23486, 23718, 24004, 24512, 25149, 26297, 27677, 30508, 34108, 40527, 48337 },
		{ 34480, 34500, 34509, 34526, 34556, 34585, 34662, 34771, 35007, 35366, 36126, 37144, 39500, 42636, 48337, 55377 }
	},
	{
		{   251,   419,   553,   692,   903,  1147,  1565,  2026,  2833,  3775,  5439,  7325, 10619, 14857, 23082, 34493 },
		{   419,   589,   727,   863,  1077,  1316,  1737,  2194,  2998,  3938,  5599,  7478, 10755, 14980, 23168, 34509 },
		{   553,   727,   863,   999,  1213,  1454,  1873,  2329,  3129,  4071,  5727,  7600, 10871, 15085, 23243, 34524 },
		{   692,   863,   999,  1135,  1351,  1592,  2011,  2466,  3264,  4199,  5849,  7724, 10990, 15187, 23315, 34541 },
		{   903,  1077,  1213,  1351,  1567,  1811,  2230,  2689,  3476,  4402,  6041,  7910, 11165, 15342, 23425, 34568 },
		{  1147,  1316,  1454,  1592,  1811,  2054,  2469,  2924,  3716,  4640,  6265,  8109, 11365, 15526, 23554, 34601 },
		{  1565,  1737,  1873,  2011,  2230,  2469,  2873,  3323,  4105,  5023,  6639,  8395, 11700, 15831, 23786, 34675 },
		{  2026,  2194,  2329,  2466,  2689,  2924,  3323,  3777,  4553,  5456,  7069,  8770, 12097, 16188, 24070, 34781 },
		{  2833,  2998,  3129,  3264,  3476,  3716,  4105,  4553,  5330,  6231,  7804,  9480, 12776, 16820, 24574, 35020 },
		{  3775,  3938,  4071,  4199,  4402,  4640,  5023,  5456,  6231,  7115,  8554, 10329, 13586, 17460, 25207, 35381 },
		{  5439,  5599,  5727,  5849,  6041,  6265,  6639,  7069,  7804,  8554, 10029, 11784, 14980, 18770, 26344, 36134 },
		{  7325,  7478,  7600,  7724,  7910,  8109,  8395,  8770,  9480, 10329, 11784, 13519, 16643, 20370, 27719, 37151 },
		{ 10619, 10755, 10871, 10990, 11165, 11365, 11700, 12097, 12776, 13586, 14980, 16643, 19601, 23359, 30533, 39501 },
		{ 14857, 14980, 15085, 15187, 15342, 15526, 15831, 16188, 16820, 17460, 18770, 20370, 23359, 26957, 34125, 42637 },
		{ 23082, 23168, 23243, 23315, 23425, 23554, 23786, 24070, 24574, 25207, 26344, 27719, 30533, 34125, 40529, 48338 },
		{ 34493, 34509, 34524, 34541, 34568, 34601, 34675, 34781, 35020, 35381, 36134, 37151, 39501, 42637, 48338, 55378 }
	},
	{
		{   385,   556,   692,   828,  1038,  1286,  1704,  2164,  2967,  3906,  5564,  7450, 10733, 14958, 23156, 34509 },
		{   556,   723,   863,   997,  1212,  1453,  1874,  2331,  3130,  4067,  5726,  7602, 10872, 15084, 23243, 34526 },
		{   692,   863,   999,  1135,  1351,  1592,  2011,  2466,  3264,  4199,  5849,  7724, 10990, 15187, 23315, 34541 },
		{   828,   997,  1135,  1276,  1482,  1731,  2146,  2605,  3397,  4324,  5970,  7841, 11105, 15285, 23389, 34561 },
		{  1038,  1212,  1351,  1482,  1703,  1945,  2363,  2819,  3606,  4535,  6167,  8026, 11271, 15445, 23493, 34587 },
		{  1286,  1453,  1592,  1731,  1945,  2185,  2605,  3053,  3844,  4769,  6395,  8222, 11476, 15625, 23625, 34620 },
		{  1704,  1874,  2011,  2146,  2363,  2605,  3004,  3452,  4232,  5154,  6762,  8488, 11803, 15932, 23851, 34692 },
		{  2164,  2331,  2466,  2605,  2819,  3053,  3452,  3909,  4682,  5580,  7190,  8888, 12204, 16280, 24134, 34798 },
		{  2967,  3130,  3264,  3397,  3606,  3844,  4232,  4682,  5456,  6350,  7915,  9593, 12882, 16907, 24637, 35034 },
		{  3906,  4067,  4199,  4324,  4535,  4769,  5154,  5580,  6350,  7235,  8667, 10439, 13680, 17548, 25267, 35392 },
		{  5564,  5726,  5849,  5970,  6167,  6395,  6762,  7190,  7915,  8667, 10135, 11887, 15073, 18851, 26391, 36142 },
		{  7450,  7602,  7724,  7841,  8026,  8222,  8488,  8888,  9593, 10439, 11887, 13620, 16729, 20446, 27762, 37157 },
		{ 10733, 10872, 10990, 11105, 11271, 11476, 11803, 12204, 12882, 13680, 15073, 16729, 19676, 23419, 30559, 39503 },
		{ 14958, 15084, 15187, 15285, 15445, 15625, 15932, 16280, 16907, 17548, 18851, 20446, 23419, 26998, 34139, 42639 },
		{ 23156, 23243, 23315, 23389, 23493, 23625, 23851, 24134, 24637, 25267, 26391, 27762, 30559, 34139, 40534, 48339 },
		{ 34509, 34526, 34541, 34561, 34587, 34620, 34692, 34798, 35034, 35392, 36142, 37157, 39503, 42639, 48339, 55378 }
	},
	{
		{   598,   771,   903,  1038,  1261,  1502,  1925,  2382,  3176,  4113,  5764,  7649, 10915, 15123, 23272, 34538 },
		{   771,   941,  1077,  1212,  1427,  1674,  2089,  2551,  3345,  4273,  5916,  7792, 11056, 15245, 23354, 34556 },
		{   903,  1077,  1213,  1351,  1567,  1811,  2230,  2689,  3476,  4402,  6041,  7910, 11165, 15342, 23425, 34568 },
		{  1038,  1212,  1351,  1482,  1703,  1945,  2363,  2819,  3606,  4535,  6167,  8026, 11271, 15445, 23493, 34587 },
		{  1261,  1427,  1567,  1703,  1916,  2162,  2575,  3032,  3817,  4738,  6364,  8201, 11449, 15605, 23608, 34613 },
		{  1502,  1674,  1811,  1945,  2162,  2399,  2817,  3263,  4051,  4974,  6588,  8362, 11657, 15784, 23741, 34649 },
		{  1925,  2089,  2230,  2363,  2575,  2817,  3212,  3664,  4440,  5348,  6954,  8660, 11979, 16087, 23965, 34720 },
		{  2382,  2551,  2689,  2819,  3032,  3263,  3664,  4111,  4880,  5781,  7385,  9072, 12371, 16431, 24240, 34823 },
		{  3176,  3345,  3476,  3606,  3817,  4051,  4440,  4880,  5653,  6548,  8104,  9776, 13045, 17045, 24740, 35060 },
		{  4113,  4273,  4402,  4535,  4738,  4974,  5348,  5781,  6548,  7426,  8845, 10614, 13842, 17683, 25362, 35414 },
		{  5764,  5916,  6041,  6167,  6364,  6588,  6954,  7385,  8104,  8845, 10314, 12061, 15219, 18983, 26463, 36154 },
		{  7649,  7792,  7910,  8026,  8201,  8362,  8660,  9072,  9776, 10614, 12061, 13783, 16862, 20567, 27833, 37166 },
		{ 10915, 11056, 11165, 11271, 11449, 11657, 11979, 12371, 13045, 13842, 15219, 16862, 19799, 23517, 30606, 39513 },
		{ 15123, 15245, 15342, 15445, 15605, 15784, 16087, 16431, 17045, 17683, 18983, 20567, 23517, 27066, 34164, 42641 },
		{ 23272, 23354, 23425, 23493, 23608, 23741, 23965, 24240, 24740, 25362, 26463, 27833, 30606, 34164, 40537, 48340 },
		{ 34538, 34556, 34568, 34587, 34613, 34649, 34720, 34823, 35060, 35414, 36154, 37166, 39513, 42641, 48340, 55379 }
	},
	{
		{   840,  1009,  1147,  1286,  1502,  1747,  2167,  2626,  3417,  4348,  5985,  7851, 11109, 15302, 23400, 34570 },
		{  1009,  1179,  1316,  1453,  1674,  1915,  2333,  2788,  3579,  4509,  6142,  7993, 11249, 15426, 23486, 34585 },
		{  1147,  1316,  1454,  1592,  1811,  2054,  2469,  2924,  3716,  4640,  6265,  8109, 11365, 15526, 23554, 34601 },
		{  1286,  1453,  1592,  1731,  1945,  2185,  2605,  3053,  3844,  4769,  6395,  8222, 11476, 15625, 23625, 34620 },
		{  1502,  1674,  1811,  1945,  2162,  2399,  2817,  3263,  4051,  4974,  6588,  8362, 11657, 15784, 23741, 34649 },
		{  1747,  1915,  2054,  2185,  2399,  2640,  3054,  3501,  4282,  5198,  6806,  8530, 11855, 15961, 23871, 34688 },
		{  2167,  2333,  2469,  2605,  2817,  3054,  3449,  3898,  4667,  5571,  7172,  8864, 12172, 16261, 24094, 34758 },
		{  2626,  2788,  2924,  3053,  3263,  3501,  3898,  4342,  5110,  6002,  7594,  9274, 12564, 16600, 24361, 34858 },
		{  3417,  3579,  3716,  3844,  4051,  4282,  4667,  5110,  5878,  6764,  8289,  9978, 13233, 17183, 24857, 35090 },
		{  4348,  4509,  4640,  4769,  4974,  5198,  5571,  6002,  6764,  7634,  9046, 10809, 14023, 17843, 25474, 35441 },
		{  5985,  6142,  6265,  6395,  6588,  6806,  7172,  7594,  8289,  9046, 10510, 12249, 15389, 19129, 26571, 36174 },
		{  7851,  7993,  8109,  8222,  8362,  8530,  8864,  9274,  9978, 10809, 12249, 13962, 17018, 20703, 27915, 37183 },
		{ 11109, 11249, 11365, 11476, 11657, 11855, 12172, 12564, 13233, 14023, 15389, 17018, 19935, 23632, 30665, 39517 },
		{ 15302, 15426, 15526, 15625, 15784, 15961, 16261, 16600, 17183, 17843, 19129, 20703, 23632, 27149, 34193, 42643 },
		{ 23400, 23486, 23554, 23625, 23741, 23871, 24094, 24361, 24857, 25474, 26571, 27915, 30665, 34193, 40540, 48341 },
		{ 34570, 34585, 34601, 34620, 34649, 34688, 34758, 34858, 35090, 35441, 36174, 37183, 39517, 42643, 48341, 55379 }
	},
	{
		{  1258,  1433,  1565,  1704,  1925,  2167,  2572,  3019,  3812,  4743,  6367,  8205, 11457, 15616, 23639, 34646 },
		{  1433,  1601,  1737,  1874,  2089,  2333,  2735,  3188,  3976,  4898,  6516,  8315, 11594, 15735, 23718, 34662 },
		{  1565,  1737,  1873,  2011,  2230,  2469,  2873,  3323,  4105,  5023,  6639,  8395, 11700, 15831, 23786, 34675 },
		{  1704,  1874,  2011,  2146,  2363,  2605,  3004,  3452,  4232,  5154,  6762,  8488, 11803, 15932, 23851, 34692 },
		{  1925,  2089,  2230,  2363,  2575,  2817,  3212,  3664,  4440,  5348,  6954,  8660, 11979, 16087, 23965, 34720 },
		{  2167,  2333,  2469,  2605,  2817,  3054,  3449,  3898,  4667,  5571,  7172,  8864, 12172, 16261, 24094, 34758 },
		{  2572,  2735,  2873,  3004,  3212,  3449,  3854,  4302,  5067,  5961,  7550,  9224, 12519, 16573, 24328, 34836 },
		{  3019,  3188,  3323,  3452,  3664,  3898,  4302,  4742,  5503,  6387,  7962,  9634, 12895, 16899, 24587, 34932 },
		{  3812,  3976,  4105,  4232,  4440,  4667,  5067,  5503,  6267,  7143,  8571, 10335, 13563, 17414, 25071, 35162 },
		{  4743,  4898,  5023,  5154,  5348,  5571,  5961,  6387,  7143,  8004,  9406, 11158, 14338, 18124, 25673, 35504 },
		{  6367,  6516,  6639,  6762,  6954,  7172,  7550,  7962,  8571,  9406, 10856, 12580, 15691, 19396, 26692, 36217 },
		{  8205,  8315,  8395,  8488,  8660,  8864,  9224,  9634, 10335, 11158, 12580, 14275, 17244, 20952, 28071, 37216 },
		{ 11457, 11594, 11700, 11803, 11979, 12172, 12519, 12895, 13563, 14338, 15691, 17244, 20186, 23839, 30781, 39535 },
		{ 15616, 15735, 15831, 15932, 16087, 16261, 16573, 16899, 17414, 18124, 19396, 20952, 23839, 27307, 34258, 42647 },
		{ 23639, 23718, 23786, 23851, 23965, 24094, 24328, 24587, 25071, 25673, 26692, 28071, 30781, 34258, 40553, 48342 },
		{ 34646, 34662, 34675, 34692, 34720, 34758, 34836, 34932, 35162, 35504, 36217, 37216, 39535, 42647, 48342, 55380 }
	},
	{
		{  1725,  1894,  2026,  2164,  2382,  2626,  3019,  3487,  4266,  5173,  6792,  8517, 11852, 15979, 23925, 34754 },
		{  1894,  2059,  2194,  2331,  2551,  2788,  3188,  3649,  4423,  5330,  6948,  8656, 11984, 16094, 24004, 34771 },
		{  2026,  2194,  2329,  2466,  2689,  2924,  3323,  3777,  4553,  5456,  7069,  8770, 12097, 16188, 24070, 34781 },
		{  2164,  2331,  2466,  2605,  2819,  3053,  3452,  3909,  4682,  5580,  7190,  8888, 12204, 16280, 24134, 34798 },
		{  2382,  2551,  2689,  2819,  3032,  3263,  3664,  4111,  4880,  5781,  7385,  9072, 12371, 16431, 24240, 34823 },
		{  2626,  2788,  2924,  3053,  3263,  3501,  3898,  4342,  5110,  6002,  7594,  9274, 12564, 16600, 24361, 34858 },
		{  3019,  3188,  3323,  3452,  3664,  3898,  4302,  4742,  5503,  6387,  7962,  9634, 12895, 16899, 24587, 34932 },
		{  3487,  3649,  3777,  3909,  4111,  4342,  4742,  5177,  5934,  6818,  8332, 10028, 13268, 17198, 24845, 35038 },
		{  4266,  4423,  4553,  4682,  4880,  5110,  5503,  5934,  6687,  7559,  8964, 10727, 13927, 17732, 25320, 35258 },
		{  5173,  5330,  5456,  5580,  5781,  6002,  6387,  6818,  7559,  8353,  9797, 11537, 14695, 18443, 25905, 35591 },
		{  6792,  6948,  7069,  7190,  7385,  7594,  7962,  8332,  8964,  9797, 11242, 12949, 16028, 19695, 26891, 36270 },
		{  8517,  8656,  8770,  8888,  9072,  9274,  9634, 10028, 10727, 11537, 12949, 14625, 17520, 21231, 28260, 37278 },
		{ 11852, 11984, 12097, 12204, 12371, 12564, 12895, 13268, 13927, 14695, 16028, 17520, 20469, 24080, 30926, 39567 },
		{ 15979, 16094, 16188, 16280, 16431, 16600, 16899, 17198, 17732, 18443, 19695, 21231, 24080, 27498, 34345, 42662 },
		{ 23925, 24004, 24070, 24134, 24240, 24361, 24587, 24845, 25320, 25905, 26891, 28260, 30926, 34345, 40575, 48344 },
		{ 34754, 34771, 34781, 34798, 34823, 34858, 34932, 35038, 35258, 35591, 36270, 37278, 39567, 42662, 48344, 55381 }
	},
	{
		{  2536,  2702,  2833,  2967,  3176,  3417,  3812,  4266,  5048,  5947,  7542,  9227, 12543, 16613, 24440, 34992 },
		{  2702,  2864,  2998,  3130,  3345,  3579,  3976,  4423,  5202,  6104,  7684,  9368, 12668, 16724, 24512, 35007 },
		{  2833,  2998,  3129,  3264,  3476,  3716,  4105,  4553,  5330,  6231,  7804,  9480, 12776, 16820, 24574, 35020 },
		{  2967,  3130,  3264,  3397,  3606,  3844,  4232,  4682,  5456,  6350,  7915,  9593, 12882, 16907, 24637, 35034 },
		{  3176,  3345,  3476,  3606,  3817,  4051,  4440,  4880,  5653,  6548,  8104,  9776, 13045, 17045, 24740, 35060 },
		{  3417,  3579,  3716,  3844,  4051,  4282,  4667,  5110,  5878,  6764,  8289,  9978, 13233, 17183, 24857, 35090 },
		{  3812,  3976,  4105,  4232,  4440,  4667,  5067,  5503,  6267,  7143,  8571, 10335, 13563, 17414, 25071, 35162 },
		{  4266,  4423,  4553,  4682,  4880,  5110,  5503,  5934,  6687,  7559,  8964, 10727, 13927, 17732, 25320, 35258 },
		{  5048,  5202,  5330,  5456,  5653,  5878,  6267,  6687,  7432,  8260,  9665, 11412, 14575, 18313, 25781, 35482 },
		{  5947,  6104,  6231,  6350,  6548,  6764,  7143,  7559,  8260,  8997, 10490, 12209, 15324, 19011, 26331, 35795 },
		{  7542,  7684,  7804,  7915,  8104,  8289,  8571,  8964,  9665, 10490, 11913, 13590, 16629, 20233, 27289, 36423 },
		{  9227,  9368,  9480,  9593,  9776,  9978, 10335, 10727, 11412, 12209, 13590, 15237, 18080, 21736, 28624, 37418 },
		{ 12543, 12668, 12776, 12882, 13045, 13233, 13563, 13927, 14575, 15324, 16629, 18080, 20977, 24523, 31218, 39649 },
		{ 16613, 16724, 16820, 16907, 17045, 17183, 17414, 17732, 18313, 19011, 20233, 21736, 24523, 27863, 34544, 42695 },
		{ 24440, 24512, 24574, 24637, 24740, 24857, 25071, 25320, 25781, 26331, 27289, 28624, 31218, 34544, 40634, 48348 },
		{ 34992, 35007, 35020, 35034, 35060, 35090, 35162, 35258, 35482, 35795, 36423, 37418, 39649, 42695, 48348, 55382 }
	},
	{
		{  3483,  3645,  3775,  3906,  4113,  4348,  4743,  5173,  5947,  6847,  8353, 10089, 13357, 17288, 25084, 35355 },
		{  3645,  3811,  3938,  4067,  4273,  4509,  4898,  5330,  6104,  6999,  8449, 10221, 13483, 17379, 25149, 35366 },
		{  3775,  3938,  4071,  4199,  4402,  4640,  5023,  5456,  6231,  7115,  8554, 10329, 13586, 17460, 25207, 35381 },
		{  3906,  4067,  4199,  4324,  4535,  4769,  5154,  5580,  6350,  7235,  8667, 10439, 13680, 17548, 25267, 35392 },
		{  4113,  4273,  4402,  4535,  4738,  4974,  5348,  5781,  6548,  7426,  8845, 10614, 13842, 17683, 25362, 35414 },
		{  4348,  4509,  4640,  4769,  4974,  5198,  5571,  6002,  6764,  7634,  9046, 10809, 14023, 17843, 25474, 35441 },
		{  4743,  4898,  5023,  5154,  5348,  5571,  5961,  6387,  7143,  8004,  9406, 11158, 14338, 18124, 25673, 35504 },
		{  5173,  5330,  5456,  5580,  5781,  6002,  6387,  6818,  7559,  8353,  9797, 11537, 14695, 18443, 25905, 35591 },
		{  5947,  6104,  6231,  6350,  6548,  6764,  7143,  7559,  8260,  8997, 10490, 12209, 15324, 19011, 26331, 35795 },
		{  6847,  6999,  7115,  7235,  7426,  7634,  8004,  8353,  8997,  9829, 11306, 12998, 16067, 19696, 26808, 36099 },
		{  8353,  8449,  8554,  8667,  8845,  9046,  9406,  9797, 10490, 11306, 12706, 14348, 17258, 20884, 27805, 36672 },
		{ 10089, 10221, 10329, 10439, 10614, 10809, 11158, 11537, 12209, 12998, 14348, 15967, 18756, 22352, 29098, 37647 },
		{ 13357, 13483, 13586, 13680, 13842, 14023, 14338, 14695, 15324, 16067, 17258, 18756, 21599, 25072, 31613, 39810 },
		{ 17288, 17379, 17460, 17548, 17683, 17843, 18124, 18443, 19011, 19696, 20884, 22352, 25072, 28332, 34841, 42772 },
		{ 25084, 25149, 25207, 25267, 25362, 25474, 25673, 25905, 26331, 26808, 27805, 29098, 31613, 34841, 40758, 48353 },
		{ 35355, 35366, 35381, 35392, 35414, 35441, 35504, 35591, 35795, 36099, 36672, 37647, 39810, 42772, 48353, 55383 }
	},
	{
		{  5151,  5310,  5439,  5564,  5764,  5985,  6367,  6792,  7542,  8353,  9780, 11549, 14769, 18591, 26237, 36119 },
		{  5310,  5472,  5599,  5726,  5916,  6142,  6516,  6948,  7684,  8449,  9917, 11680, 14885, 18689, 26297, 36126 },
		{  5439,  5599,  5727,  5849,  6041,  6265,  6639,  7069,  7804,  8554, 10029, 11784, 14980, 18770, 26344, 36134 },
		{  5564,  5726,  5849,  5970,  6167,  6395,  6762,  7190,  7915,  8667, 10135, 11887, 15073, 18851, 26391, 36142 },
		{  5764,  5916,  6041,  6167,  6364,  6588,  6954,  7385,  8104,  8845, 10314, 12061, 15219, 18983, 26463, 36154 },
		{  5985,  6142,  6265,  6395,  6588,  6806,  7172,  7594,  8289,  9046, 10510, 12249, 15389, 19129, 26571, 36174 },
		{  6367,  6516,  6639,  6762,  6954,  7172,  7550,  7962,  8571,  9406, 10856, 12580, 15691, 19396, 26692, 36217 },
		{  6792,  6948,  7069,  7190,  7385,  7594,  7962,  8332,  8964,  9797, 11242, 12949, 16028, 19695, 26891, 36270 },
		{  7542,  7684,  7804,  7915,  8104,  8289,  8571,  8964,  9665, 10490, 11913, 13590, 16629, 20233, 27289, 36423 },
		{  8353,  8449,  8554,  8667,  8845,  9046,  9406,  9797, 10490, 11306, 12706, 14348, 17258, 20884, 27805, 36672 },
		{  9780,  9917, 10029, 10135, 10314, 10510, 10856, 11242, 11913, 12706, 14111, 15703, 18497, 22076, 28804, 37321 },
		{ 11549, 11680, 11784, 11887, 12061, 12249, 12580, 12949, 13590, 14348, 15703, 17212, 19984, 23485, 30030, 38230 },
		{ 14769, 14885, 14980, 15073, 15219, 15389, 15691, 16028, 16629, 17258, 18497, 19984, 22737, 26094, 32422, 40256 },
		{ 18591, 18689, 18770, 18851, 18983, 19129, 19396, 19695, 20233, 20884, 22076, 23485, 26094, 29241, 35499, 43067 },
		{ 26237, 26297, 26344, 26391, 26463, 26571, 26692, 26891, 27289, 27805, 28804, 30030, 32422, 35499, 41131, 48449 },
		{ 36119, 36126, 36134, 36142, 36154, 36174, 36217, 36270, 36423, 36672, 37321, 38230, 40256, 43067, 48449, 55385 }
	},
	{
		{  7054,  7207,  7325,  7450,  7649,  7851,  8205,  8517,  9227, 10089, 11549, 13292, 16452, 20206, 27628, 37143 },
		{  7207,  7358,  7478,  7602,  7792,  7993,  8315,  8656,  9368, 10221, 11680, 13417, 16558, 20295, 27677, 37144 },
		{  7325,  7478,  7600,  7724,  7910,  8109,  8395,  8770,  9480, 10329, 11784, 13519, 16643, 20370, 27719, 37151 },
		{  7450,  7602,  7724,  7841,  8026,  8222,  8488,  8888,  9593, 10439, 11887, 13620, 16729, 20446, 27762, 37157 },
		{  7649,  7792,  7910,  8026,  8201,  8362,  8660,  9072,  9776, 10614, 12061, 13783, 16862, 20567, 27833, 37166 },
		{  7851,  7993,  8109,  8222,  8362,  8530,  8864,  9274,  9978, 10809, 12249, 13962, 17018, 20703, 27915, 37183 },
		{  8205,  8315,  8395,  8488,  8660,  8864,  9224,  9634, 10335, 11158, 12580, 14275, 17244, 20952, 28071, 37216 },
		{  8517,  8656,  8770,  8888,  9072,  9274,  9634, 10028, 10727, 11537, 12949, 14625, 17520, 21231, 28260, 37278 },
		{  9227,  9368,  9480,  9593,  9776,  9978, 10335, 10727, 11412, 12209, 13590, 15237, 18080, 21736, 28624, 37418 },
		{ 10089, 10221, 10329, 10439, 10614, 10809, 11158, 11537, 12209, 12998, 14348, 15967, 18756, 22352, 29098, 37647 },
		{ 11549, 11680, 11784, 11887, 12061, 12249, 12580, 12949, 13590, 14348, 15703, 17212, 19984, 23485, 30030, 38230 },
		{ 13292, 13417, 13519, 13620, 13783, 13962, 14275, 14625, 15237, 15967, 17212, 18672, 21436, 24840, 31211, 39115 },
		{ 16452, 16558, 16643, 16729, 16862, 17018, 17244, 17520, 18080, 18756, 19984, 21436, 24097, 27224, 33479, 40993 },
		{ 20206, 20295, 20370, 20446, 20567, 20703, 20952, 21231, 21736, 22352, 23485, 24840, 27224, 30386, 36330, 43638 },
		{ 27628, 27677, 27719, 27762, 27833, 27915, 28071, 28260, 28624, 29098, 30030, 31211, 33479, 36330, 41775, 48724 },
		{ 37143, 37144, 37151, 37157, 37166, 37183, 37216, 37278, 37418, 37647, 38230, 39115, 40993, 43638, 48724, 55407 }
	},
	{
		{ 10367, 10508, 10619, 10733, 10915, 11109, 11457, 11852, 12543, 13357, 14769, 16452, 19431, 23228, 30481, 39499 },
		{ 10508, 10646, 10755, 10872, 11056, 11249, 11594, 11984, 12668, 13483, 14885, 16558, 19523, 23300, 30508, 39500 },
		{ 10619, 10755, 10871, 10990, 11165, 11365, 11700, 12097, 12776, 13586, 14980, 16643, 19601, 23359, 30533, 39501 },
		{ 10733, 10872, 10990, 11105, 11271, 11476, 11803, 12204, 12882, 13680, 15073, 16729, 19676, 23419, 30559, 39503 },
		{ 10915, 11056, 11165, 11271, 11449, 11657, 11979, 12371, 13045, 13842, 15219, 16862, 19799, 23517, 30606, 39513 },
		{ 11109, 11249, 11365, 11476, 11657, 11855, 12172, 12564, 13233, 14023, 15389, 17018, 19935, 23632, 30665, 39517 },
		{ 11457, 11594, 11700, 11803, 11979, 12172, 12519, 12895, 13563, 14338, 15691, 17244, 20186, 23839, 30781, 39535 },
		{ 11852, 11984, 12097, 12204, 12371, 12564, 12895, 13268, 13927, 14695, 16028, 17520, 20469, 24080, 30926, 39567 },
		{ 12543, 12668, 12776, 12882, 13045, 13233, 13563, 13927, 14575, 15324, 16629, 18080, 20977, 24523, 31218, 39649 },
		{ 13357, 13483, 13586, 13680, 13842, 14023, 14338, 14695, 15324, 16067, 17258, 18756, 21599, 25072, 31613, 39810 },
		{ 14769, 14885, 14980, 15073, 15219, 15389, 15691, 16028, 16629, 17258, 18497, 19984, 22737, 26094, 32422, 40256 },
		{ 16452, 16558, 16643, 16729, 16862, 17018, 17244, 17520, 18080, 18756, 19984, 21436, 24097, 27224, 33479, 40993 },
		{ 19431, 19523, 19601, 19676, 19799, 19935, 20186, 20469, 20977, 21599, 22737, 24097, 26557, 29604, 35584, 42710 },
		{ 23228, 23300, 23359, 23419, 23517, 23632, 23839, 24080, 24523, 25072, 26094, 27224, 29604, 32602, 38145, 45081 },
		{ 30481, 30508, 30533, 30559, 30606, 30665, 30781, 30926, 31218, 31613, 32422, 33479, 35584, 38145, 43319, 49733 },
		{ 39499, 39500, 39501, 39503, 39513, 39517, 39535, 39567, 39649, 39810, 40256, 40993, 42710, 45081, 49733, 55763 }
	},
	{
		{ 14634, 14757, 14857, 14958, 15123, 15302, 15616, 15979, 16613, 17288, 18591, 20206, 23228, 26871, 34097, 42635 },
		{ 14757, 14884, 14980, 15084, 15245, 15426, 15735, 16094, 16724, 17379, 18689, 20295, 23300, 26918, 34108, 42636 },
		{ 14857, 14980, 15085, 15187, 15342, 15526, 15831, 16188, 16820, 17460, 18770, 20370, 23359, 26957, 34125, 42637 },
		{ 14958, 15084, 15187, 15285, 15445, 15625, 15932, 16280, 16907, 17548, 18851, 20446, 23419, 26998, 34139, 42639 },
		{ 15123, 15245, 15342, 15445, 15605, 15784, 16087, 16431, 17045, 17683, 18983, 20567, 23517, 27066, 34164, 42641 },
		{ 15302, 15426, 15526, 15625, 15784, 15961, 16261, 16600, 17183, 17843, 19129, 20703, 23632, 27149, 34193, 42643 },
		{ 15616, 15735, 15831, 15932, 16087, 16261, 16573, 16899, 17414, 18124, 19396, 20952, 23839, 27307, 34258, 42647 },
		{ 15979, 16094, 16188, 16280, 16431, 16600, 16899, 17198, 17732, 18443, 19695, 21231, 24080, 27498, 34345, 42662 },
		{ 16613, 16724, 16820, 16907, 17045, 17183, 17414, 17732, 18313, 19011, 20233, 21736, 24523, 27863, 34544, 42695 },
		{ 17288, 17379, 17460, 17548, 17683, 17843, 18124, 18443, 19011, 19696, 20884, 22352, 25072, 28332, 34841, 42772 },
		{ 18591, 18689, 18770, 18851, 18983, 19129, 19396, 19695, 20233, 20884, 22076, 23485, 26094, 29241, 35499, 43067 },
		{ 20206, 20295, 20370, 20446, 20567, 20703, 20952, 21231, 21736, 22352, 23485, 24840, 27224, 30386, 36330, 43638 },
		{ 23228, 23300, 23359, 23419, 23517, 23632, 23839, 24080, 24523, 25072, 26094, 27224, 29604, 32602, 38145, 45081 },
		{ 26871, 26918, 26957, 26998, 27066, 27149, 27307, 27498, 27863, 28332, 29241, 30386, 32602, 35434, 40696, 47134 },
		{ 34097, 34108, 34125, 34139, 34164, 34193, 34258, 34345, 34544, 34841, 35499, 36330, 38145, 40696, 45450, 51542 },
		{ 42635, 42636, 42637, 42639, 42641, 42643, 42647, 42662, 42695, 42772, 43067, 43638, 45081, 47134, 51542, 56999 }
	},
	{
		{ 22931, 23015, 23082, 23156, 23272, 23400, 23639, 23925, 24440, 25084, 26237, 27628, 30481, 34097, 40524, 48336 },
		{ 23015, 23101, 23168, 23243, 23354, 23486, 23718, 24004, 24512, 25149, 26297, 27677, 30508, 34108, 40527, 48337 },
		{ 23082, 23168, 23243, 23315, 23425, 23554, 23786, 24070, 24574, 25207, 26344, 27719, 30533, 34125, 40529, 48338 },
		{ 23156, 23243, 23315, 23389, 23493, 23625, 23851, 24134, 24637, 25267, 26391, 27762, 30559, 34139, 40534, 48339 },
		{ 23272, 23354, 23425, 23493, 23608, 23741, 23965, 24240, 24740, 25362, 26463, 27833, 30606, 34164, 40537, 48340 },
		{ 23400, 23486, 23554, 23625, 23741, 23871, 24094, 24361, 24857, 25474, 26571, 27915, 30665, 34193, 40540, 48341 },
		{ 23639, 23718, 23786, 23851, 23965, 24094, 24328, 24587, 25071, 25673, 26692, 28071, 30781, 34258, 40553, 48342 },
		{ 23925, 24004, 24070, 24134, 24240, 24361, 24587, 24845, 25320, 25905, 26891, 28260, 30926, 34345, 40575, 48344 },
		{ 24440, 24512, 24574, 24637, 24740, 24857, 25071, 25320, 25781, 26331, 27289, 28624, 31218, 34544, 40634, 48348 },
		{ 25084, 25149, 25207, 25267, 25362, 25474, 25673, 25905, 26331, 26808, 27805, 29098, 31613, 34841, 40758, 48353 },
		{ 26237, 26297, 26344, 26391, 26463, 26571, 26692, 26891, 27289, 27805, 28804, 30030, 32422, 35499, 41131, 48449 },
		{ 27628, 27677, 27719, 27762, 27833, 27915, 28071, 28260, 28624, 29098, 30030, 31211, 33479, 36330, 41775, 48724 },
		{ 30481, 30508, 30533, 30559, 30606, 30665, 30781, 30926, 31218, 31613, 32422, 33479, 35584, 38145, 43319, 49733 },
		{ 34097, 34108, 34125, 34139, 34164, 34193, 34258, 34345, 34544, 34841, 35499, 36330, 38145, 40696, 45450, 51542 },
		{ 40524, 40527, 40529, 40534, 40537, 40540, 40553, 40575, 40634, 40758, 41131, 41775, 43319, 45450, 49971, 55452 },
		{ 48336, 48337, 48338, 48339, 48340, 48341, 48342, 48344, 48348, 48353, 48449, 48724, 49733, 51542, 55452, 60505 }
	},
	{
		{ 34469, 34480, 34493, 34509, 34538, 34570, 34646, 34754, 34992, 35355, 36119, 37143, 39499, 42635, 48336, 55377 },
		{ 34480, 34500, 34509, 34526, 34556, 34585, 34662, 34771, 35007, 35366, 36126, 37144, 39500, 42636, 48337, 55377 },
		{ 34493, 34509, 34524, 34541, 34568, 34601, 34675, 34781, 35020, 35381, 36134, 37151, 39501, 42637, 48338, 55378 },
		{ 34509, 34526, 34541, 34561, 34587, 34620, 34692, 34798, 35034, 35392, 36142, 37157, 39503, 42639, 48339, 55378 },
		{ 34538, 34556, 34568, 34587, 34613, 34649, 34720, 34823, 35060, 35414, 36154, 37166, 39513, 42641, 48340, 55379 },
		{ 34570, 34585, 34601, 34620, 34649, 34688, 34758, 34858, 35090, 35441, 36174, 37183, 39517, 42643, 48341, 55379 },
		{ 34646, 34662, 34675, 34692, 34720, 34758, 34836, 34932, 35162, 35504, 36217, 37216, 39535, 42647, 48342, 55380 },
		{ 34754, 34771, 34781, 34798, 34823, 34858, 34932, 35038, 35258, 35591, 36270, 37278, 39567, 42662, 48344, 55381 },
		{ 34992, 35007, 35020, 35034, 35060, 35090, 35162, 35258, 35482, 35795, 36423, 37418, 39649, 42695, 48348, 55382 },
		{ 35355, 35366, 35381, 35392, 35414, 35441, 35504, 35591, 35795, 36099, 36672, 37647, 39810, 42772, 48353, 55383 },
		{ 36119, 36126, 36134, 36142, 36154, 36174, 36217, 36270, 36423, 36672, 37321, 38230, 40256, 43067, 48449, 55385 },
		{ 37143, 37144, 37151, 37157, 37166, 37183, 37216, 37278, 37418, 37647, 38230, 39115, 40993, 43638, 48724, 55407 },
		{ 39499, 39500, 39501, 39503, 39513, 39517, 39535, 39567, 39649, 39810, 40256, 40993, 42710, 45081, 49733, 55763 },
		{ 42635, 42636, 42637, 42639, 42641, 42643, 42647, 42662, 42695, 42772, 43067, 43638, 45081, 47134, 51542, 56999 },
		{ 48336, 48337, 48338, 48339, 48340, 48341, 48342, 48344, 48348, 48353, 48449, 48724, 49733, 51542, 55452, 60505 },
		{ 55377, 55377, 55378, 55378, 55379, 55379, 55380, 55381, 55382, 55383, 55385, 55407, 55763, 56999, 60505, 65119 }
	}
};

uint16 AtariSTYMStream::ymout5_u16[32][32][32];
int16 *AtariSTYMStream::ymout5 = (int16 *)AtariSTYMStream::ymout5_u16;
bool AtariSTYMStream::_tablesBuilt = false;

uint16 AtariSTYMStream::mergeVoice(uint16 c, uint16 b, uint16 a) {
	return (uint16)((c << 10) | (b << 5) | a);
}

void AtariSTYMStream::envBuild() {
	for (int env = 0; env < 16; env++) {
		for (int block = 0; block < 3; block++) {
			int vol = 0;
			int inc = 0;
			switch (YmEnvDef[env][block]) {
			case ENV_GODOWN:
				vol = 31;
				inc = -1;
				break;
			case ENV_GOUP:
				vol = 0;
				inc = 1;
				break;
			case ENV_DOWN:
				vol = 0;
				inc = 0;
				break;
			case ENV_UP:
				vol = 31;
				inc = 0;
				break;
			default:
				vol = 0;
				inc = 0;
				break;
			}

			for (int i = 0; i < 32; i++) {
				YmEnvWaves[env][block * 32 + i] = mergeVoice((uint16)vol, (uint16)vol, (uint16)vol);
				vol += inc;
			}
		}
	}
}

void AtariSTYMStream::interpolateVolumetable(uint16 volumetable[32][32][32]) {
	for (int i = 1; i < 32; i += 2) {
		for (int j = 1; j < 32; j += 2) {
			for (int k = 1; k < 32; k += 2) {
				volumetable[i][j][k] = volumeTable[(i - 1) / 2][(j - 1) / 2][(k - 1) / 2];
			}
			volumetable[i][j][0] = volumetable[i][j][1];
			volumetable[i][j][1] = volumetable[i][j][3];
			volumetable[i][j][3] = (uint16)(0.5 + std::sqrt((double)volumetable[i][j][1] * (double)volumetable[i][j][5]));
			for (int k = 2; k < 32; k += 2)
				volumetable[i][j][k] = (uint16)(0.5 + std::sqrt((double)volumetable[i][j][k - 1] * (double)volumetable[i][j][k + 1]));
		}
		for (int k = 0; k < 32; k++) {
			volumetable[i][0][k] = volumetable[i][1][k];
			volumetable[i][1][k] = volumetable[i][3][k];
			volumetable[i][3][k] = (uint16)(0.5 + std::sqrt((double)volumetable[i][1][k] * (double)volumetable[i][5][k]));
		}
		for (int j = 2; j < 32; j += 2) {
			for (int k = 0; k < 32; k++)
				volumetable[i][j][k] = (uint16)(0.5 + std::sqrt((double)volumetable[i][j - 1][k] * (double)volumetable[i][j + 1][k]));
		}
	}

	for (int j = 0; j < 32; j++) {
		for (int k = 0; k < 32; k++) {
			volumetable[0][j][k] = volumetable[1][j][k];
			volumetable[1][j][k] = volumetable[3][j][k];
			volumetable[3][j][k] = (uint16)(0.5 + std::sqrt((double)volumetable[1][j][k] * (double)volumetable[5][j][k]));
		}
	}

	for (int i = 2; i < 32; i += 2) {
		for (int j = 0; j < 32; j++) {
			for (int k = 0; k < 32; k++)
				volumetable[i][j][k] = (uint16)(0.5 + std::sqrt((double)volumetable[i - 1][j][k] * (double)volumetable[i + 1][j][k]));
		}
	}
}

void AtariSTYMStream::normalise5bitTable(uint16 *in5bit, int16 *out5bit, unsigned int level) {
	unsigned int minv = 0xffffffffu;
	unsigned int maxv = 0;

	for (int i = 0; i < 32 * 32 * 32; i++) {
		unsigned int v = in5bit[i];
		if (v < minv)
			minv = v;
		if (v > maxv)
			maxv = v;
	}

	const double scale = (maxv != minv) ? ((double)level / (double)(maxv - minv)) : 1.0;
	for (int i = 0; i < 32 * 32 * 32; i++) {
		double v = (double)in5bit[i];
		v = (v - (double)minv) * scale;
		int iv = (int)std::lround(v);
		if (iv > 32767)
			iv = 32767;
		if (iv < -32768)
			iv = -32768;
		out5bit[i] = (int16)iv;
	}
}

void AtariSTYMStream::initOnce() {
	if (_tablesBuilt)
		return;
	_tablesBuilt = true;

	envBuild();
	interpolateVolumetable(ymout5_u16);
	normalise5bitTable(&ymout5_u16[0][0][0], ymout5, 0x7fff);
}

uint16 AtariSTYMStream::tonePer(uint8 rHigh, uint8 rLow) {
	uint16 per = (uint16)(((uint16)rHigh & 0x0f) << 8) | (uint16)rLow;
	return per;
}

uint16 AtariSTYMStream::noisePer(uint8 rNoise) {
	return (uint16)(rNoise & 0x1f);
}

uint16 AtariSTYMStream::envPer(uint8 rHigh, uint8 rLow) {
	return (uint16)(((uint16)rHigh << 8) | (uint16)rLow);
}

uint32 AtariSTYMStream::rndCompute() {
	uint32 b = (((_rndRack >> 16) ^ (_rndRack >> 13)) & 1);
	_rndRack = (_rndRack >> 1) | (b << 16);
	return (_rndRack & 1) ? 0xffff : 0;
}


void AtariSTYMStream::setOutputRate(int outputRate) {
	if (outputRate <= 0)
		outputRate = 44100;
	_rate = outputRate;

	_posFractWeightedN = 0;
	_YMBuffer250PosRead = 0;
	_YMBuffer250PosWrite = 0;
}

void AtariSTYMStream::reset() {
	memset(_soundRegs, 0, sizeof(_soundRegs));

	_toneAPer = _toneBPer = _toneCPer = 1;
	_toneACount = _toneBCount = _toneCCount = 0;
	_toneAVal = _toneBVal = _toneCVal = YM_SQUARE_UP;

	_noisePer = 1;
	_noiseCount = 0;
	_noiseVal = 0;

	_envPer = 1;
	_envCount = 0;
	_envPos = 0;
	_envShape = 0;

	_mixerTA = _mixerTB = _mixerTC = 0;
	_mixerNA = _mixerNB = _mixerNC = 0;

	_rndRack = 1;
	_freqDiv2 = 0;

	_envMask3Voices = 0;
	_vol3Voices = 0;

	_posFractWeightedN = 0;
	_YMBuffer250PosRead = 0;
	_YMBuffer250PosWrite = 0;

	for (int r = 0; r < ARRAYSIZE(_soundRegs); r++)
		writeReg(r, 0);
}

void AtariSTYMStream::writeReg(int reg, uint8 data) {
	switch (reg) {
	case 0:
		_soundRegs[0] = data;
		_toneAPer = tonePer(_soundRegs[1], _soundRegs[0]);
		break;
	case 1:
		_soundRegs[1] = data & 0x0f;
		_toneAPer = tonePer(_soundRegs[1], _soundRegs[0]);
		break;
	case 2:
		_soundRegs[2] = data;
		_toneBPer = tonePer(_soundRegs[3], _soundRegs[2]);
		break;
	case 3:
		_soundRegs[3] = data & 0x0f;
		_toneBPer = tonePer(_soundRegs[3], _soundRegs[2]);
		break;
	case 4:
		_soundRegs[4] = data;
		_toneCPer = tonePer(_soundRegs[5], _soundRegs[4]);
		break;
	case 5:
		_soundRegs[5] = data & 0x0f;
		_toneCPer = tonePer(_soundRegs[5], _soundRegs[4]);
		break;
	case 6:
		_soundRegs[6] = data & 0x1f;
		_noisePer = noisePer(_soundRegs[6]);
		break;

	case 7:
		_soundRegs[7] = data & 0x3f;
		_mixerTA = (data & (1 << 0)) ? 0xffff : 0;
		_mixerTB = (data & (1 << 1)) ? 0xffff : 0;
		_mixerTC = (data & (1 << 2)) ? 0xffff : 0;
		_mixerNA = (data & (1 << 3)) ? 0xffff : 0;
		_mixerNB = (data & (1 << 4)) ? 0xffff : 0;
		_mixerNC = (data & (1 << 5)) ? 0xffff : 0;
		break;

	case 8:
		_soundRegs[8] = data & 0x1f;
		if (data & 0x10) {
			_envMask3Voices |= YM_MASK_A;
			_vol3Voices &= ~YM_MASK_A;
		} else {
			_envMask3Voices &= ~YM_MASK_A;
			_vol3Voices &= ~YM_MASK_A;
			_vol3Voices |= YmVolume4to5[_soundRegs[8]];
		}
		break;

	case 9:
		_soundRegs[9] = data & 0x1f;
		if (data & 0x10) {
			_envMask3Voices |= YM_MASK_B;
			_vol3Voices &= ~YM_MASK_B;
		} else {
			_envMask3Voices &= ~YM_MASK_B;
			_vol3Voices &= ~YM_MASK_B;
			_vol3Voices |= (YmVolume4to5[_soundRegs[9]]) << 5;
		}
		break;

	case 10:
		_soundRegs[10] = data & 0x1f;
		if (data & 0x10) {
			_envMask3Voices |= YM_MASK_C;
			_vol3Voices &= ~YM_MASK_C;
		} else {
			_envMask3Voices &= ~YM_MASK_C;
			_vol3Voices &= ~YM_MASK_C;
			_vol3Voices |= (YmVolume4to5[_soundRegs[10]]) << 10;
		}
		break;

	case 11:
		_soundRegs[11] = data;
		_envPer = envPer(_soundRegs[12], _soundRegs[11]);
		break;

	case 12:
		_soundRegs[12] = data;
		_envPer = envPer(_soundRegs[12], _soundRegs[11]);
		break;

	case 13:
		_soundRegs[13] = data & 0x0f;
		_envPos = 0;
		_envCount = 0;
		_envShape = _soundRegs[13];
		break;

	default:
		break;
	}
}


void AtariSTYMStream::generate(int16 *dst, int count) {
	if (count <= 0)
		return;

	const int desired250Unclamped = (int)std::ceil((double)count * (double)YM_ATARI_CLOCK_COUNTER / (double)_rate) + 32;
	const int desired250 = (desired250Unclamped < (YM_BUFFER_250_SIZE - 1)) ? desired250Unclamped : (YM_BUFFER_250_SIZE - 1);

	const int available250 = (_YMBuffer250PosWrite - _YMBuffer250PosRead) & YM_BUFFER_250_SIZE_MASK;
	if (available250 < desired250) {
		const int missing = desired250 - available250;
		doSamples250(missing);
	}

	for (int i = 0; i < count; i++)
		dst[i] = (int16)nextSample();
}


void AtariSTYMStream::doSamples250(int samplesToGenerate250) {
	int pos = _YMBuffer250PosWrite;

	for (int n = 0; n < samplesToGenerate250; n++) {
		_freqDiv2 ^= 1;
		if (_freqDiv2 == 0)
			_noiseCount++;

		if (_noiseCount >= _noisePer) {
			_noiseCount = 0;
			_noiseVal = (uint16)rndCompute();
		}

		_toneACount++;
		if (_toneACount >= _toneAPer) {
			_toneACount = 0;
			_toneAVal ^= YM_SQUARE_UP;
		}

		_toneBCount++;
		if (_toneBCount >= _toneBPer) {
			_toneBCount = 0;
			_toneBVal ^= YM_SQUARE_UP;
		}

		_toneCCount++;
		if (_toneCCount >= _toneCPer) {
			_toneCCount = 0;
			_toneCVal ^= YM_SQUARE_UP;
		}

		_envCount += 1;
		if (_envCount >= _envPer) {
			_envCount = 0;
			_envPos += 1;
			if ((int)_envPos >= 3 * 32)
				_envPos -= 2 * 32;
		}

		uint16 env3 = YmEnvWaves[_envShape][_envPos];
		env3 &= _envMask3Voices;

		uint32 bt = ((uint32)_toneAVal | _mixerTA) & ((uint32)_noiseVal | _mixerNA);
		uint16 tone3 = (uint16)(bt & YM_MASK_1VOICE);

		bt = ((uint32)_toneBVal | _mixerTB) & ((uint32)_noiseVal | _mixerNB);
		tone3 |= (uint16)((bt & YM_MASK_1VOICE) << 5);

		bt = ((uint32)_toneCVal | _mixerTC) & ((uint32)_noiseVal | _mixerNC);
		tone3 |= (uint16)((bt & YM_MASK_1VOICE) << 10);

		tone3 &= (uint16)(env3 | _vol3Voices);

		_YMBuffer250[pos] = ymout5[(int)tone3];
		pos = (pos + 1) & YM_BUFFER_250_SIZE_MASK;
	}

	_YMBuffer250PosWrite = pos;
}

int16 AtariSTYMStream::nextSample() {
	const uint32 intervalFract = (uint32)(((uint64)YM_ATARI_CLOCK_COUNTER * 0x10000ULL) / (uint64)_rate);
	int64 total = 0;

	if (((_YMBuffer250PosWrite - _YMBuffer250PosRead) & YM_BUFFER_250_SIZE_MASK) == 0) {
		_posFractWeightedN = 0;
		return 0;
	}

	if (_posFractWeightedN) {
		total += ((int64)_YMBuffer250[_YMBuffer250PosRead]) * (int64)(0x10000 - _posFractWeightedN);
		_YMBuffer250PosRead = (_YMBuffer250PosRead + 1) & YM_BUFFER_250_SIZE_MASK;
		_posFractWeightedN -= 0x10000;

		if (((_YMBuffer250PosWrite - _YMBuffer250PosRead) & YM_BUFFER_250_SIZE_MASK) == 0) {
			_posFractWeightedN = 0;
			return 0;
		}
	}

	_posFractWeightedN += intervalFract;

	while (_posFractWeightedN & 0xffff0000) {
		total += ((int64)_YMBuffer250[_YMBuffer250PosRead]) * 0x10000;
		_YMBuffer250PosRead = (_YMBuffer250PosRead + 1) & YM_BUFFER_250_SIZE_MASK;
		_posFractWeightedN -= 0x10000;

		if (((_YMBuffer250PosWrite - _YMBuffer250PosRead) & YM_BUFFER_250_SIZE_MASK) == 0) {
			_posFractWeightedN = 0;
			return 0;
		}
	}

	if (_posFractWeightedN) {
		total += ((int64)_YMBuffer250[_YMBuffer250PosRead]) * (int64)_posFractWeightedN;
	}

	return (int16)(total / (int64)intervalFract);
}

AtariSTYMStream::AtariSTYMStream(Common::SeekableReadStream *stream, uint32 outputRate)
	: _stream(stream), _rate(outputRate) {
	if (!_stream)
		error("AtariST YM Stream: null stream");

	_stream->seek(0, SEEK_SET);
	const uint32 sz = (uint32)_stream->size();
	_data.resize(sz);
	if (sz) {
		if (_stream->read(&_data[0], sz) != sz)
			error("AtariST YM Stream: read failed");
	}
	_pos = 0;
	_mode = kModeElvira2PKD;
	_frameHz = kDefaultFrameHz;

	_rate = (outputRate == 0) ? 44100 : outputRate;
	_samplesPerFrame = (uint32)((_rate + (_frameHz / 2)) / _frameHz);

	setOutputRate((int)_rate);
	initOnce();
	reset();
	resetSynth();
}

AtariSTYMStream::AtariSTYMStream(Common::SeekableReadStream *stream, uint32 outputRate, uint16 elvira1Tune)
	: _stream(stream), _rate(outputRate), _elvira1Tune(elvira1Tune) {
	if (!_stream)
		error("AtariST YM Stream: null stream");

	_stream->seek(0, SEEK_SET);
	const uint32 sz = (uint32)_stream->size();
	_data.resize(sz);
	if (sz) {
		if (_stream->read(&_data[0], sz) != sz)
			error("AtariST YM Stream: read failed");
	}
	_pos = 0;
	_mode = kModeElvira1PRG;
	_frameHz = kElvira1PrgFrameHz;

	_rate = (outputRate == 0) ? 44100 : outputRate;
	_samplesPerFrame = (uint32)((_rate + (_frameHz / 2)) / _frameHz);

	setOutputRate((int)_rate);
	initOnce();
	reset();
	_elviraPrgDriver = new ElviraPrgDriver(this, _data);
	if (!_elviraPrgDriver->isValid()) {
		delete _elviraPrgDriver;
		_elviraPrgDriver = nullptr;
		_isValid = false;
		_ended = true;
		_samplesLeftInWait = 0;
		return;
	}
	resetSynth();
}

AtariSTYMStream::~AtariSTYMStream() {
	delete _elviraPrgDriver;
}


void AtariSTYMStream::resetSynth() {
	_samplesLeftInWait = 0;
	_ended = false;
	reset();

	if (_mode == kModeElvira1PRG) {
		if (_elviraPrgDriver) {
			_elviraPrgDriver->init(_elvira1Tune);
		} else {
			_ended = true;
			_samplesLeftInWait = 0;
		}
		return;
	}

	parseUntilWait();
}

void AtariSTYMStream::parseUntilWait() {
	if (_mode == kModeElvira1PRG) {
		if (_elviraPrgDriver) {
			_elviraPrgDriver->vbl();
			_samplesLeftInWait = _samplesPerFrame;
		} else {
			_ended = true;
			_samplesLeftInWait = 0;
		}
		return;
	}

	if (_ended)
		return;

	while (_pos + 1 < _data.size()) {
		uint8 r = _data[_pos++];
		uint8 v = _data[_pos++];

		if (r == 0xFF) {
			if (v == 0) {
				_pos = 0;
				resetSynth();
				return;
			}

			_samplesLeftInWait = (uint32)v * _samplesPerFrame;
			return;
		}

		writeReg((int)r, v);
	}

	_ended = true;
	_samplesLeftInWait = 0;
}

int AtariSTYMStream::readBuffer(int16 *buffer, const int numSamples) {
	int produced = 0;

	while (produced < numSamples) {

		if (_samplesLeftInWait == 0) {
			parseUntilWait();
			if (_samplesLeftInWait == 0 && _ended)
				break;
		}

		const int want = numSamples - produced;
		const uint32 chunk = MIN<uint32>((uint32)want, _samplesLeftInWait);

		generate(buffer + produced, (int)chunk);

		produced += chunk;
		_samplesLeftInWait -= chunk;
	}

	return produced;
}

} // namespace Audio
