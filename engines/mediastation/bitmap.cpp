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

#include "mediastation/bitmap.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

BitmapHeader::BitmapHeader(Chunk &chunk) {
	uint headerSizeInBytes = chunk.readTypedUint16();
	_dimensions = chunk.readTypedGraphicSize();
	_compressionType = static_cast<BitmapCompressionType>(chunk.readTypedUint16());
	_stride = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "BitmapHeader::BitmapHeader(): headerSize: %d, _compressionType = 0x%x, _stride = %d",
		headerSizeInBytes, static_cast<uint>(_compressionType), _stride);
}

Bitmap::Bitmap(Chunk &chunk, BitmapHeader *bitmapHeader) : _bitmapHeader(bitmapHeader) {
	if (stride() < width()) {
		warning("%s: Got stride less than width", __func__);
	}

	_unk1 = chunk.readUint16LE();
	if (chunk.bytesRemaining() > 0) {
		if (isCompressed()) {
			_compressedStream = chunk.readStream(chunk.bytesRemaining());
		} else {
			_image.create(stride(), height(), Graphics::PixelFormat::createFormatCLUT8());
			if (getCompressionType() == kUncompressedTransparentBitmap)
				_image.setTransparentColor(0);
			byte *pixels = static_cast<byte *>(_image.getPixels());

			chunk.read(pixels, stride() * height());
			if (chunk.bytesRemaining() > 0) {
				warning("%s: %d bytes remaining in bitmap chunk after reading image data", __func__, chunk.bytesRemaining());
				chunk.skip(chunk.bytesRemaining());
			}
		}
	}
}

Bitmap::~Bitmap() {
	delete _bitmapHeader;
	_bitmapHeader = nullptr;

	delete _compressedStream;
	_compressedStream = nullptr;
}

bool Bitmap::isCompressed() const {
	return (getCompressionType() != kUncompressedBitmap) && \
		(getCompressionType() != kUncompressedTransparentBitmap);
}

} // End of namespace MediaStation
