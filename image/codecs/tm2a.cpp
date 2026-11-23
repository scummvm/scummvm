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

#include "image/codecs/tm2a.h"

#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "graphics/yuv_to_rgb.h"

namespace Image {

// Horizontal log2 sampling intervals and vertical sampling intervals for Y and UV.
static const byte kSampling[21][4] = {
	{0, 1, 1, 1}, {0, 1, 1, 2}, {0, 1, 2, 4}, {1, 1, 2, 4},
	{0, 2, 2, 4}, {1, 2, 2, 4}, {2, 2, 2, 4}, {1, 4, 2, 4},
	{2, 4, 2, 4}, {2, 8, 3, 8}, {3, 4, 3, 8}, {3, 8, 3, 8},
	{0, 1, 1, 4}, {0, 1, 2, 2}, {0, 2, 1, 4}, {1, 1, 2, 2},
	{1, 4, 2, 8}, {2, 2, 3, 4}, {2, 4, 3, 8}, {0, 1, 3, 8},
	{1, 2, 3, 8}
};

TM2ADecoder::TM2ADecoder(int width, int height) :
		_width(width), _height(height), _pixelFormat(Graphics::PixelFormat::createFormatRGBA32()),
		_modeCount(0), _data(nullptr), _dataEnd(nullptr),
		_tokens(nullptr), _tokensEnd(nullptr), _failed(false) {
}

TM2ADecoder::~TM2ADecoder() {
	_surface.free();
}

bool TM2ADecoder::setOutputPixelFormat(const Graphics::PixelFormat &format) {
	if (format.bytesPerPixel != 2 && format.bytesPerPixel != 4)
		return false;
	_pixelFormat = format;
	return true;
}

bool TM2ADecoder::readHeader(const byte *data, uint32 size) {
	if (size < 18 || READ_BE_UINT16(data) != _height || READ_BE_UINT16(data + 2) != _width ||
		data[4] != 1 || !data[5] || data[5] > ARRAYSIZE(_modes) ||
		data[8] != 0 || READ_BE_UINT16(data + 9) != 1)
		return false;

	// Skip the encoder identifier and optional extension, followed by a checksum.
	uint32 pos = 12 + data[11];
	if (pos + 6 > size || data[pos] != 0 || pos + 6 + data[pos + 1] != size)
		return false;
	_modeCount = data[5];
	return true;
}

bool TM2ADecoder::readCodebook(const byte *data, uint32 size) {
	if (size < 8)
		return false;
	uint count = READ_BE_UINT16(data + 5);
	if (!count || count > ARRAYSIZE(_codebook))
		return false;

	uint32 pos = 8;
	for (uint i = 0; i < count; ++i) {
		if (pos == size)
			return false;
		uint length = data[pos++];
		if (!length || length > 8 || length > size - pos)
			return false;
		_codeLengths[i] = length;
		for (uint j = 0; j < length; ++j) {
			if (data[pos] >= 128)
				return false;
			_codebook[i][j] = data[pos++];
		}
	}
	return pos == size;
}

bool TM2ADecoder::readChunks(Common::Array<byte> &packet) {
	_data = _dataEnd = nullptr;
	_tokens = _tokensEnd = nullptr;
	_modeCount = 0;
	_failed = false;
	memset(_modes, 0, sizeof(_modes));
	memset(_deltaCounts, 0, sizeof(_deltaCounts));
	memset(_codeLengths, 0, sizeof(_codeLengths));
	bool header = false, tiles = false, codebook = false;
	byte key[4] = {};
	uint32 pos = 0;
	while (pos < packet.size()) {
		if (packet.size() - pos < 8)
			return false;
		uint32 tag = READ_BE_UINT32(&packet[pos]);
		uint32 size = READ_BE_UINT32(&packet[pos + 4]);
		pos += 8;
		if (size > packet.size() - pos)
			return false;
		byte *data = packet.begin() + pos;
		pos += size;

		if (tag == 0xa0000106) {
			if (_data || pos != size + 8 || size <= 4 || READ_BE_UINT32(data) != size - 4)
				return false;
			_data = data + 4;
			_dataEnd = data + size;
			uint32 value = size - 4;
			for (int i = 0; i < 3; ++i)
				value = (value << 1) | (((value >> 21) ^ (value >> 31) ^ (value >> 1) ^ ~value) & 1);
			WRITE_BE_UINT32(key, value);
			continue;
		}
		if (!_data)
			return false;
		if (tag != 0xa000010a) {
			for (uint32 i = 0; i < size; ++i)
				data[i] ^= key[i & 3];
		}

		switch (tag) {
		case 0xa0000115:
			if (header || !readHeader(data, size))
				return false;
			header = true;
			break;
		case 0xa0000109:
			if (tiles || size != 3 || data[0] != 8 || data[1] != _width / 8 || data[2] != 1)
				return false;
			tiles = true;
			break;
		case 0xa0000102: {
			if (size < 2 || data[0] > 1 || !data[1] || data[1] > 128 || size != 2U + data[1] * 2U)
				return false;
			uint table = data[0];
			if (_deltaCounts[table])
				return false;
			_deltaCounts[table] = data[1];
			for (uint i = 0; i < data[1]; ++i)
				_deltas[table][i] = (int16)READ_BE_UINT16(data + 2 + i * 2);
			break;
		}
		case 0xa000010b:
			if (size != 4 || !data[0] || data[0] > ARRAYSIZE(kSampling) ||
				data[1] != 0 || data[2] != 1 || data[3] >= _modeCount || _modes[data[3]])
				return false;
			_modes[data[3]] = data[0];
			break;
		case 0xa000010a:
			if (codebook || !readCodebook(data, size))
				return false;
			codebook = true;
			break;
		default:
			return false;
		}
	}
	return header && tiles && codebook && _deltaCounts[0] && _deltaCounts[1];
}

byte TM2ADecoder::readToken() {
	if (_failed)
		return 0;
	if (_tokens == _tokensEnd) {
		if (_data == _dataEnd) {
			_failed = true;
			return 0;
		}
		uint code = *_data++;
		if (!_codeLengths[code]) {
			_failed = true;
			return 0;
		}
		_tokens = _codebook[code];
		_tokensEnd = _tokens + _codeLengths[code];
	}
	return *_tokens++;
}

int16 TM2ADecoder::readDelta(uint table) {
	uint index = readToken();
	if (index >= _deltaCounts[table]) {
		_failed = true;
		return 0;
	}
	return _deltas[table][index];
}

bool TM2ADecoder::decodePixels() {
	_planes.resize(_width * _height * 2);
	_row.resize(_width / 2);
	_blocks.resize(_width / 8);
	for (uint i = 0; i < _row.size(); ++i)
		_row[i] = 0x80808080;

	byte *yPlane = _planes.begin();
	byte *uPlane = yPlane + _width * _height;
	byte *vPlane = uPlane + _width * _height / 2;
	for (int band = 0; band < _height; band += 8) {
		for (uint b = 0; b < _blocks.size(); ++b) {
			uint slot = readToken();
			uint reference = readToken();
			if (slot >= _modeCount || !_modes[slot])
				return false;
			_blocks[b] = _modes[slot] - 1;
			// These intra frames predict from the preceding row, with zero displacement.
			if (reference == 1) {
				if (readToken() != 0 || readToken() != 0)
					return false;
			} else if (reference != 0 || b == 0) {
				return false;
			}
		}
		for (uint y = 0; y < 8; ++y) {
			uint32 lumaPred = 0, chromaPred = 0;
			for (uint b = 0; b < _blocks.size(); ++b) {
				const byte *mode = kSampling[_blocks[b]];
				if (y % mode[1]) {
					lumaPred = chromaPred = 0;
					continue;
				}
				bool chroma = y % mode[3] == 0;
				if (!chroma)
					chromaPred = 0;
				uint step = MAX(2U, 1U << mode[0]);
				for (uint x = 0; x < 8; x += step) {
					uint index = b * 4 + x / 2;
					uint32 base = _row[index];
					bool readChroma = chroma && x % (1U << mode[2]) == 0;
					if (chroma) {
						if (mode[0])
							base = (base & 0xff00ffff) | ((base & 0xff) << 16);
						if (readChroma) {
							chromaPred += (uint32)readDelta(1) << 8;
							chromaPred += (uint32)readDelta(1) << 24;
						} else {
							base = (base & 0x00ff00ff) | (_row[index - 1] & 0xff00ff00);
						}
					}
					// Packed YUYV arithmetic deliberately preserves carries between components.
					lumaPred += (uint32)readDelta(0) * 0x10001;
					uint32 value = base + lumaPred;
					if (!mode[0]) {
						uint32 delta = (uint32)readDelta(0);
						lumaPred += delta * 0x10001;
						value += delta << 16;
					}
					if (readChroma)
						value += chromaPred;
					for (uint j = 0; j < step / 2; ++j)
						_row[index + j] = value;
				}
			}
			if (_failed)
				return false;
			for (uint i = 0; i < _row.size(); ++i) {
				uint32 value = _row[i];
				*yPlane++ = value;
				*yPlane++ = value >> 16;
				*uPlane++ = value >> 8;
				*vPlane++ = value >> 24;
			}
		}
	}
	return true;
}

const Graphics::Surface *TM2ADecoder::decodeFrame(Common::SeekableReadStream &stream) {
	if (_width <= 0 || _height <= 0 || _width > 2040 || _height > 32760 ||
		(_width & 7) || (_height & 7)) {
		warning("TM2A: unsupported dimensions %dx%d", _width, _height);
		return nullptr;
	}
	int64 size = stream.size() - stream.pos();
	if (size < 12 || size > 0x7fffffff)
		return nullptr;
	Common::Array<byte> packet;
	packet.resize(size);
	if (stream.read(packet.begin(), packet.size()) != packet.size() ||
		!readChunks(packet) || !decodePixels()) {
		warning("TM2A: invalid or unsupported frame");
		return nullptr;
	}
	if (!_surface.getPixels() || _surface.format != _pixelFormat) {
		_surface.free();
		_surface.create(_width, _height, _pixelFormat);
	}
	const byte *y = _planes.begin();
	const byte *u = y + _width * _height;
	const byte *v = u + _width * _height / 2;
	YUVToRGBMan.convert422(&_surface, Graphics::YUVToRGBManager::kScaleITU,
	                      y, u, v, _width, _height, _width, _width / 2);
	return &_surface;
}

} // End of namespace Image
