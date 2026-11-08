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

#include "tesoro/core/animimage.h"

#include "common/endian.h"
#include "common/textconsole.h"

namespace Tesoro {

static const uint32 kHeaderSize = 0x10;
static const uint32 kSpanSize = 8;
static const uint32 kItemSize = 8;

uint32 AnimImage::pixelOffset() const {
	return READ_LE_UINT32(&_data[0]);
}

byte *AnimImage::pixelData() {
	if (_data.size() < 8)
		return nullptr;
	const uint32 offset = pixelOffset(), bytes = pixelBytes();
	return offset <= _data.size() && bytes <= _data.size() - offset ? &_data[offset] : nullptr;
}

uint32 AnimImage::pixelBytes() const {
	return READ_LE_UINT32(&_data[4]);
}

uint16 AnimImage::type() const {
	return READ_LE_UINT16(&_data[0xc]);
}

bool AnimImage::load(Common::SeekableReadStream &stream) {
	_data.clear();

	const uint32 size = stream.readUint32LE();
	if (stream.err() || stream.eos() || size < kHeaderSize || size > (uint32)(stream.size() - stream.pos()))
		return false;

	_data.resize(size);
	if (stream.read(&_data[0], size) != size) {
		_data.clear();
		return false;
	}
	return convert();
}

bool AnimImage::convert() {
	byte *blob = &_data[0];
	const uint32 size = _data.size();

	if (pixelOffset() > size) {
		_data.clear();
		return false;
	}

	const uint32 spanCount = READ_LE_UINT16(blob + 0xa);
	if (kHeaderSize + spanCount * kSpanSize > size) {
		_data.clear();
		return false;
	}

	const uint16 imageType = type();
	for (uint32 s = 0; s < spanCount; ++s) {
		const byte *span = blob + kHeaderSize + s * kSpanSize;
		const uint32 items = READ_LE_UINT32(span);
		const uint32 count = READ_LE_UINT16(span + 4);
		if ((uint64)items + (uint64)count * kItemSize > size) {
			_data.clear();
			return false;
		}

		for (uint32 k = 0; k < count && imageType != kLiteral; ++k) {
			byte *item = blob + items + k * kItemSize;
			const uint32 value = READ_LE_UINT32(item);
			if (imageType == kShadow)
				WRITE_LE_UINT32(item, (value & 0x1f) << 2);
		}
	}
	return true;
}

} // End of namespace Tesoro
