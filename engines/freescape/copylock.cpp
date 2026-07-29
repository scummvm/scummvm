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

#include "common/debug.h"
#include "common/endian.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "common/stream.h"

#include "freescape/copylock.h"
#include "freescape/freescape.h"

namespace Freescape {

// Cipher used to wrap the program, i.e. the shape of the decoding loop found
// in the protection. Both come from Copylock series 2 (1989).
enum CopylockCipher {
	// sub.w d0,(a6)+ ; add.w d1,(a6)+
	// add.l d2,d0    ; rol.l d0,d0
	// add.l d3,d1    ; ror.l d1,d1
	// The key is d0..d3 in key[0..3].
	kCipherRotate = 0,

	// roxl.l #1,d0 ... roxl.l #1,d5 ; eor.l d0,(a6)+
	// The six registers and the X flag they rotate through form a 193 bit ring
	// which turns one bit per longword, so the key stream is the ring itself:
	// key[0..6] holds it, from the value of d0 on, one bit per iteration.
	kCipherShift = 1
};

// The registers the protection derives from the disk serial. They are constant
// for a release, see the class comment. The md5 is that of the whole file, not
// the partial one detection matches on.
struct CopylockKey {
	const char *md5;
	CopylockCipher cipher;
	uint32 key[7];
};

static const CopylockKey kCopylockKeys[] = {
	// Castle Master, Atari ST (Virtual Worlds compilation), M.PRG
	{ "2e9f0b3fe88e961851b50af9f7c77002", kCipherRotate,
	  { 0x276bd21e, 0x00000000, 0xffffe829, 0xeeefd7cb, 0, 0, 0 } },
	// Castle Master, Atari ST (Castle Master & The Crypt, Incentive), M.PRG
	{ "f580b8658e622588298d1d6ad87437fb", kCipherShift,
	  { 0x4ed7a43c, 0x0010c85c, 0x400b051c, 0x5bd5a219, 0x0900ff04, 0x00000000, 0x00000000 } },
	{ nullptr, kCipherRotate, { 0, 0, 0, 0, 0, 0, 0 } }
};

// Layout of the protection, recovered from the file itself.
struct CopylockLayout {
	uint32 magic;      // key of the Trace Vector Decoder
	uint32 progOffset; // start of the wrapped program, from the TEXT segment
	uint32 progSize;   // number of encrypted bytes
	CopylockCipher cipher;
};

static uint32 readUint32(const byte *buf, uint32 offset) {
	return READ_BE_UINT32(buf + offset);
}

// Series 2 encrypts its own loops by xor-ing each instruction with the
// preceding encrypted longword plus a magic value, which needs no execution.
static uint32 tvdKey(const byte *text, uint32 offset, uint32 magic) {
	return readUint32(text, offset - 4) + magic;
}

static void tvdDecode(const byte *text, uint32 offset, uint32 magic, uint32 *out) {
	uint32 key = tvdKey(text, offset, magic);
	out[0] = readUint32(text, offset) ^ key;
	out[1] = readUint32(text, offset + 4) ^ key;
}

// Signature of the protection: lea pc+$12,a6 / move.l -4(a6),d6 / add.l $8.l,d6
static bool findProtection(const byte *text, uint32 size, uint32 &start, uint32 &magic) {
	uint32 instr[2];

	for (uint32 i = 4; i + 20 < size; i += 2) {
		uint32 candidate = (readUint32(text, i) ^ 0x4dfa0010) - readUint32(text, i - 4);

		tvdDecode(text, i + 4, candidate, instr);
		if (instr[0] != 0x2c2efffc)
			continue;
		tvdDecode(text, i + 8, candidate, instr);
		if (instr[0] != 0xdcb90000)
			continue;

		start = i;
		magic = candidate;
		return true;
	}
	return false;
}

// The decoding routine starts with lea here(pc),a6 / adda.l #offset,a6 /
// move.l #size,d6, which gives the location and the length of the program.
static bool findDecoder(const byte *text, uint32 limit, uint32 magic, CopylockLayout &layout) {
	uint32 instr[2];

	for (uint32 i = 4; i < limit; i += 2) {
		tvdDecode(text, i, magic, instr);
		if (instr[0] != 0x4dfafffe)
			continue;

		tvdDecode(text, i + 4, magic, instr);
		if ((instr[0] >> 16) != 0xddfc)
			continue;
		uint32 offset = ((instr[0] & 0xffff) << 16) | (instr[1] >> 16);

		tvdDecode(text, i + 10, magic, instr);
		if ((instr[0] >> 16) != 0x2c3c)
			continue;
		uint32 count = ((instr[0] & 0xffff) << 16) | (instr[1] >> 16);

		// Then the decoding loop itself, which has to be one we implement
		static const uint16 rotateLoop[] = {
			0x915e, 0xd35e, 0xd082, 0xe1b8, 0xd283, 0xe2b9, 0x5986, 0x66f0
		};
		static const uint16 shiftLoop[] = {
			0xdc8e, 0xe390, 0xe391, 0xe392, 0xe393, 0xe394, 0xe395, 0xb19e, 0xbdc6, 0x66ee
		};

		for (int variant = 0; variant < 2; variant++) {
			const uint16 *loop = variant ? shiftLoop : rotateLoop;
			int length = variant ? ARRAYSIZE(shiftLoop) : ARRAYSIZE(rotateLoop);

			uint32 at = i + 16;
			bool matched = true;
			for (int j = 0; j < length && matched; j++, at += 2) {
				tvdDecode(text, at, magic, instr);
				matched = (instr[0] >> 16) == loop[j];
			}

			if (matched) {
				layout.magic = magic;
				layout.progOffset = i + offset;
				layout.progSize = count;
				layout.cipher = variant ? kCipherShift : kCipherRotate;
				return true;
			}
		}
		return false;
	}
	return false;
}

static uint32 rol32(uint32 value, uint32 count) {
	count &= 31;
	return count ? ((value << count) | (value >> (32 - count))) : value;
}

static uint32 ror32(uint32 value, uint32 count) {
	count &= 31;
	return count ? ((value >> count) | (value << (32 - count))) : value;
}

static void decodeRotate(byte *prog, uint32 size, const CopylockKey &key) {
	uint32 d0 = key.key[0], d1 = key.key[1], d2 = key.key[2], d3 = key.key[3];

	for (uint32 i = 0; i + 4 <= size; i += 4) {
		WRITE_BE_UINT16(prog + i, READ_BE_UINT16(prog + i) - (uint16)d0);
		WRITE_BE_UINT16(prog + i + 2, READ_BE_UINT16(prog + i + 2) + (uint16)d1);

		d0 += d2;
		d0 = rol32(d0, d0);
		d1 += d3;
		d1 = ror32(d1, d1);
	}
}

// The ring turns by one bit per longword, so the key of iteration i is made of
// the 32 bits it holds from position i on.
static const int kRingBits = 193;

static void decodeShift(byte *prog, uint32 size, const CopylockKey &key) {
	for (uint32 i = 0; i + 4 <= size; i += 4) {
		uint32 k = 0;
		for (int b = 0; b < 32; b++) {
			int pos = ((i / 4) + b) % kRingBits;
			k = (k << 1) | ((key.key[pos / 32] >> (31 - (pos % 32))) & 1);
		}
		WRITE_BE_UINT32(prog + i, READ_BE_UINT32(prog + i) ^ k);
	}
}

static void decodeProgram(byte *prog, uint32 size, const CopylockKey &key) {
	if (key.cipher == kCipherShift)
		decodeShift(prog, size, key);
	else
		decodeRotate(prog, size, key);
}

// Size of the unwrapped GEMDOS program, so that the tail of the protection is
// not carried over.
static uint32 programSize(const byte *prog, uint32 available) {
	uint32 size = 0x1c + readUint32(prog, 2) + readUint32(prog, 6) + readUint32(prog, 14);
	if (size + 4 > available)
		return 0;

	if (READ_BE_UINT16(prog + 26) != 0) // absflag: no relocation table
		return size;

	bool empty = readUint32(prog, size) == 0;
	size += 4;
	if (!empty) {
		while (size < available && prog[size] != 0)
			size++;
		size++;
	}
	return size <= available ? size : 0;
}

static const CopylockKey *findKey(Common::SeekableReadStream *file) {
	Common::String md5 = Common::computeStreamMD5AsString(*file, file->size());
	file->seek(0);

	for (const CopylockKey *key = kCopylockKeys; key->md5; key++) {
		if (md5 == key->md5)
			return key;
	}

	debugC(1, kFreescapeDebugParser, "Copylock: no key for md5 %s", md5.c_str());
	return nullptr;
}

static bool readLayout(Common::SeekableReadStream *file, Common::Array<byte> &data, CopylockLayout &layout) {
	file->seek(0);
	data.resize(file->size());
	if (file->read(data.data(), data.size()) != data.size())
		return false;
	file->seek(0);

	if (data.size() < 0x1c || READ_BE_UINT16(data.data()) != 0x601a)
		return false;

	uint32 textSize = readUint32(data.data(), 2);
	if (textSize < 0x100 || 0x1c + textSize > data.size())
		return false;

	const byte *text = data.data() + 0x1c;
	uint32 start;
	if (!findProtection(text, textSize, start, layout.magic))
		return false;

	return findDecoder(text, start, layout.magic, layout);
}

bool Copylock::isProtected(Common::SeekableReadStream *file) {
	Common::Array<byte> data;
	CopylockLayout layout;
	return readLayout(file, data, layout);
}

Common::SeekableReadStream *Copylock::unwrap(Common::SeekableReadStream *file) {
	Common::Array<byte> data;
	CopylockLayout layout;

	if (!readLayout(file, data, layout))
		return nullptr;

	const CopylockKey *key = findKey(file);
	if (!key)
		return nullptr;

	if (key->cipher != layout.cipher) {
		warning("Copylock: the key does not match the cipher of the file");
		return nullptr;
	}

	uint32 progOffset = 0x1c + layout.progOffset;
	if (progOffset + layout.progSize > data.size()) {
		warning("Copylock: the wrapped program does not fit in the file");
		return nullptr;
	}

	debugC(1, kFreescapeDebugParser, "Copylock: program at 0x%x, %d bytes encrypted",
		progOffset, layout.progSize);

	uint32 available = data.size() - progOffset;
	byte *prog = (byte *)malloc(available);
	memcpy(prog, data.data() + progOffset, available);
	decodeProgram(prog, layout.progSize, *key);

	uint32 size = READ_BE_UINT16(prog) == 0x601a ? programSize(prog, available) : 0;
	if (!size) {
		warning("Copylock: decryption did not yield a GEMDOS program");
		free(prog);
		return nullptr;
	}

	return new Common::MemoryReadStream(prog, size, DisposeAfterUse::YES);
}

} // End of namespace Freescape
