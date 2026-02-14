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

#include "phoenixvr/pakf.h"
#include "common/debug.h"
#include "common/memstream.h"
#include "common/textconsole.h"

namespace PhoenixVR {

namespace {
void unpack_3(uint8 *dstBegin, uint32 dstSize, Common::SeekableReadStream &input) {
	auto *dstEnd = dstBegin + dstSize;
	auto *dst = dstBegin;
	while (true) {
		byte n = input.readByte();
		if (input.eos())
			break;
		if (n & 0x80) {
			int offset;
			byte len = (n & 0x3f) + 1;
			if (n & 0x40)
				offset = input.readByte() + 1;
			else {
				offset = input.readByte();
				offset <<= 8;
				offset |= input.readByte();
				offset += 1;
			}
			if (dst - offset < dstBegin)
				error("invalid decompression offset");
			if (dst + len > dstEnd)
				error("invalid decompression size");
			while (len--) {
				*dst = dst[-offset];
				++dst;
			}
		} else {
			unsigned len = n + 1;
			if (dst + len > dstEnd)
				error("invalid decompression size");
			while (len--) {
				*dst++ = input.readByte();
			}
		}
	}
}
} // namespace

Common::SeekableReadStream *unpack(Common::SeekableReadStream &input) {
	input.seek(0);
	byte header[0x24];
	input.read(header, sizeof(header));
	if (header[0] != 'P' || header[1] != 'A' || header[2] != 'K' || header[3] != 'F')
		error("invalid PAKF signature");

	Common::MemoryReadStreamEndian ms(header + 4, sizeof(header) - 4, false);
	auto fsize = ms.readUint32();
	if (fsize != input.size())
		error("invalid PAKF size");

	ms.skip(16); // original name
	auto method = ms.readUint32();
	auto csize = ms.readUint32();
	auto usize = ms.readUint32();
	debug("method %u, compressed size: %u, uncompressed size: %u", method, csize, usize);
	Common::SharedPtr<byte> mem(new uint8[usize], Common::ArrayDeleter<byte>());
	switch (method) {
	case 3:
		unpack_3(mem.get(), usize, input);
		break;
	default:
		error("invalid compression method %u\n", method);
	}

	return new Common::MemoryReadStream(mem, usize);
}

} // namespace PhoenixVR
