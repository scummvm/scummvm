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
#include "common/textconsole.h"

#include "eem/animation.h"
#include "eem/detection.h"

namespace EEM {

void decodeAnmFrameRLE(const byte *src, uint srcSize, byte *dst, uint dstSize) {
	const byte *srcEnd = src + srcSize;
	byte *dstEnd = dst + dstSize;

	while (dst < dstEnd && src < srcEnd) {
		const byte op = *src++;

		if (op == 0x80) {
			// Extended opcode: read 16-bit length argument.
			if (src + 2 > srcEnd)
				break;
			const uint16 n = (uint16)src[0] | ((uint16)src[1] << 8);
			src += 2;

			if ((n & 0x8000) == 0) {
				// Skip n bytes — preserves previous-frame pixels.
				if (n >= (uint)(dstEnd - dst))
					break;
				dst += n;
			} else if ((n & 0x4000) == 0) {
				// Long literal copy: (n & 0x7FFF) bytes from src to dst.
				uint16 cnt = n & 0x7FFF;
				while (cnt-- && dst < dstEnd && src < srcEnd)
					*dst++ = *src++;
			} else {
				// Long fill: (n & 0x3FFF) bytes with the next single byte.
				if (src >= srcEnd)
					break;
				const byte val = *src++;
				uint16 cnt = n & 0x3FFF;
				while (cnt-- && dst < dstEnd)
					*dst++ = val;
			}
		} else if (op == 0) {
			// Short fill: byte count, then byte value.
			if (src + 2 > srcEnd)
				break;
			const byte cnt = src[0];
			const byte val = src[1];
			src += 2;
			for (byte i = 0; i < cnt && dst < dstEnd; i++)
				*dst++ = val;
		} else if ((op & 0x80) == 0) {
			// Short literal copy: `op` bytes (1..0x7F) from src to dst.
			byte cnt = op;
			while (cnt-- && dst < dstEnd && src < srcEnd)
				*dst++ = *src++;
		} else {
			// Short skip: (op & 0x7F) bytes — preserves previous-frame pixels.
			const uint skip = op & 0x7F;
			if (skip >= (uint)(dstEnd - dst))
				break;
			dst += skip;
		}
	}
}

bool ANMDecoder::open(const Common::Path &path) {
	close();

	if (!_file.open(path)) {
		warning("ANMDecoder: cannot open %s", path.toString().c_str());
		return false;
	}

	if (_file.read(_palette, sizeof(_palette)) != sizeof(_palette)) {
		warning("ANMDecoder: short palette read on %s", path.toString().c_str());
		close();
		return false;
	}

	_frameCount = _file.readUint16LE();
	if (_frameCount == 0 || _frameCount > 1024) {
		warning("ANMDecoder: implausible frame count %u in %s",
				_frameCount, path.toString().c_str());
		close();
		return false;
	}

	_file.skip(2);                     // header[+0]: ignored
	_height = _file.readUint16LE();    // header[+2]
	_width  = _file.readUint16LE();    // header[+4]
	_file.skip(6);                     // header[+6..+10]: ignored

	if (_width == 0 || _height == 0) {
		warning("ANMDecoder: zero dimensions in %s", path.toString().c_str());
		close();
		return false;
	}

	_lengths.resize(_frameCount);
	for (uint16 i = 0; i < _frameCount; i++)
		_lengths[i] = _file.readUint16LE();

	_buffer.resize((uint32)_width * _height);
	memset(_buffer.data(), 0, _buffer.size());
	_nextFrameIdx = 0;

	debugC(1, kDebugGfx, "ANMDecoder: %s opened, %u frames, %ux%u",
		   path.toString().c_str(), _frameCount, _width, _height);
	return true;
}

void ANMDecoder::close() {
	if (_file.isOpen())
		_file.close();
	_lengths.clear();
	_buffer.clear();
	_packed.clear();
	_frameCount = _width = _height = _nextFrameIdx = 0;
}

void ANMDecoder::getPalette8(byte *out) const {
	for (uint i = 0; i < sizeof(_palette); i++)
		out[i] = (byte)(_palette[i] << 2);
}

void ANMDecoder::seedFrameBuffer(const byte *pixels, uint pitch) {
	if (!pixels || _buffer.empty() || _width == 0 || _height == 0)
		return;

	for (uint y = 0; y < _height; y++)
		memcpy(_buffer.data() + y * _width, pixels + y * pitch, _width);
}

const byte *ANMDecoder::nextFrame() {
	if (_nextFrameIdx >= _frameCount)
		return nullptr;

	const uint16 packedSize = _lengths[_nextFrameIdx];
	if (_packed.size() < packedSize)
		_packed.resize(packedSize);

	if (_file.read(_packed.data(), packedSize) != packedSize) {
		warning("ANMDecoder: short read on frame %u", _nextFrameIdx);
		return nullptr;
	}

	decodeAnmFrameRLE(_packed.data(), packedSize, _buffer.data(), _buffer.size());
	_nextFrameIdx++;
	return _buffer.data();
}

} // End of namespace EEM
