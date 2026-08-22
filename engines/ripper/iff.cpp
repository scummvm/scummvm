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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/iff.h"

#include "common/array.h"
#include "common/formats/iff_container.h"

namespace Ripper {

void IFFDecoder::loadPBMBitmap(Common::SeekableReadStream &stream, byte *data, uint16 outPitch) {
	const Image::IFFDecoder::Header *header = getHeader();
	// ReadIffBitmapHeaderAndPayloadBuffer at 0x6a6d4 rounds each PBM row to an even byte count.
	const uint16 scanlinePitch = (header->width + 1) & ~1;
	Common::Array<byte> scanline(scanlinePitch);

	for (uint16 i = 0; i < header->height; ++i) {
		if (header->compression) {
			Common::PackBitsReadStream packStream(stream);
			packStream.read(scanline.data(), scanlinePitch);
		} else {
			stream.read(scanline.data(), scanlinePitch);
		}

		memcpy(data, scanline.data(), outPitch);
		data += outPitch;
	}
}

} // End of namespace Ripper
