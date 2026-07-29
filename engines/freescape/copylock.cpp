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
	// Dark Side, Amiga (Stampede cover disk, issue 1), DARKSIDE
	{ "87310b48b108374a45709885e45a7c2a", kCipherShift,
	  { 0x00000000, 0x3688589d, 0x003ffe40, 0x1fffc015, 0x735067a3, 0x4281be60, 0x00000000 } },
	{ nullptr, kCipherRotate, { 0, 0, 0, 0, 0, 0, 0 } }
};

// The executable the wrapper sits in: a GEMDOS program on the Atari ST, a hunk
// one on the Amiga.
enum CopylockContainer {
	kContainerGemdos = 0,
	kContainerHunk = 1
};

// Layout of the protection, recovered from the file itself.
struct CopylockLayout {
	CopylockContainer container;
	uint32 textOffset; // the segment the protection lives in
	uint32 textSize;
	uint32 magic;      // key of the Trace Vector Decoder
	uint32 progOffset; // start of the wrapped program, from the TEXT segment
	uint32 progSize;   // number of encrypted bytes
	CopylockCipher cipher;
};

const uint32 kHunkHeader = 0x000003f3;
const uint32 kHunkCode = 0x000003e9;
const uint32 kHunkHeaderSize = 6 * 4; // one hunk, no resident libraries

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

// The TEXT of a GEMDOS program, or the single code hunk of an Amiga executable.
bool findTextSegment(const Common::Array<byte> &data, CopylockLayout &layout) {
	if (data.size() >= 0x1c && READ_BE_UINT16(data.data()) == 0x601a) {
		layout.container = kContainerGemdos;
		layout.textOffset = 0x1c;
		layout.textSize = readUint32(data.data(), 2);
		return layout.textSize >= 0x100 && layout.textOffset + layout.textSize <= data.size();
	}

	// A hunk header of one code hunk, then that hunk: 0x3E9 and its length in
	// longwords, this one without the memory flags the header carries.
	if (data.size() >= kHunkHeaderSize + 8 && readUint32(data.data(), 0) == kHunkHeader &&
			readUint32(data.data(), 4) == 0 && readUint32(data.data(), 8) == 1 &&
			readUint32(data.data(), kHunkHeaderSize) == kHunkCode) {
		layout.container = kContainerHunk;
		layout.textOffset = kHunkHeaderSize + 8;
		layout.textSize = 4 * readUint32(data.data(), kHunkHeaderSize + 4);
		return layout.textSize >= 0x100 && layout.textOffset + layout.textSize <= data.size();
	}

	return false;
}

static bool readLayout(Common::SeekableReadStream *file, Common::Array<byte> &data, CopylockLayout &layout) {
	file->seek(0);
	data.resize(file->size());
	if (file->read(data.data(), data.size()) != data.size())
		return false;
	file->seek(0);

	if (!findTextSegment(data, layout))
		return false;

	const byte *text = data.data() + layout.textOffset;
	uint32 start;
	if (!findProtection(text, layout.textSize, start, layout.magic))
		return false;

	return findDecoder(text, start, layout.magic, layout);
}

bool Copylock::isProtected(Common::SeekableReadStream *file) {
	Common::Array<byte> data;
	CopylockLayout layout;
	return readLayout(file, data, layout);
}

Common::SeekableReadStream *unwrapGemdos(const Common::Array<byte> &data,
		const CopylockLayout &layout, const CopylockKey &key) {
	uint32 progOffset = layout.textOffset + layout.progOffset;
	if (progOffset + layout.progSize > data.size()) {
		warning("Copylock: the wrapped program does not fit in the file");
		return nullptr;
	}

	debugC(1, kFreescapeDebugParser, "Copylock: program at 0x%x, %d bytes encrypted",
		progOffset, layout.progSize);

	uint32 available = data.size() - progOffset;
	byte *prog = (byte *)malloc(available);
	memcpy(prog, data.data() + progOffset, available);
	decodeProgram(prog, layout.progSize, key);

	uint32 size = READ_BE_UINT16(prog) == 0x601a ? programSize(prog, available) : 0;
	if (!size) {
		warning("Copylock: decryption did not yield a GEMDOS program");
		free(prog);
		return nullptr;
	}

	return new Common::MemoryReadStream(prog, size, DisposeAfterUse::YES);
}

// Here the displacement of the decoding loop is relative to where the protection
// runs, not to the file, so it cannot locate the program. The key stream is a
// ring anchored at the start of the code hunk though, so the hunk is decoded as
// a whole and the program found by the hunk header it carries.
Common::SeekableReadStream *unwrapHunk(const Common::Array<byte> &data,
		const CopylockLayout &layout, const CopylockKey &key) {
	byte *code = (byte *)malloc(layout.textSize);
	memcpy(code, data.data() + layout.textOffset, layout.textSize);
	decodeProgram(code, layout.textSize, key);

	uint32 offset = 0;
	uint32 hunkSize = 0;
	for (; offset + 8 <= layout.textSize; offset += 4) {
		if (readUint32(code, offset) != kHunkCode)
			continue;
		hunkSize = 4 * readUint32(code, offset + 4);
		if (hunkSize && offset + 8 + hunkSize <= layout.textSize)
			break;
		hunkSize = 0;
	}

	if (!hunkSize) {
		warning("Copylock: decryption did not yield a hunk executable");
		free(code);
		return nullptr;
	}

	debugC(1, kFreescapeDebugParser, "Copylock: program at 0x%x, %d bytes of code",
		layout.textOffset + offset, hunkSize);

	// The wrapper holds the hunks but not the header of the executable they came
	// from, which is rebuilt so the result reads like the unprotected release.
	uint32 imageSize = layout.textSize - offset;
	byte *prog = (byte *)malloc(kHunkHeaderSize + imageSize);
	WRITE_BE_UINT32(prog, kHunkHeader);
	WRITE_BE_UINT32(prog + 4, 0);  // no resident library
	WRITE_BE_UINT32(prog + 8, 1);  // one hunk, first and last
	WRITE_BE_UINT32(prog + 12, 0);
	WRITE_BE_UINT32(prog + 16, 0);
	WRITE_BE_UINT32(prog + 20, hunkSize / 4);
	memcpy(prog + kHunkHeaderSize, code + offset, imageSize);
	free(code);

	return new Common::MemoryReadStream(prog, kHunkHeaderSize + imageSize, DisposeAfterUse::YES);
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

	if (layout.container == kContainerHunk)
		return unwrapHunk(data, layout, *key);

	return unwrapGemdos(data, layout, *key);
}

} // End of namespace Freescape
