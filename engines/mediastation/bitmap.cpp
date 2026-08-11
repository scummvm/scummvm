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
#include "mediastation/mediastation.h"

namespace MediaStation {

ImageInfo::ImageInfo(Chunk &chunk) {
	_imageDataStartOffset = chunk.readTypedUint16();
	_dimensions = chunk.readTypedGraphicSize();
	_compressionType = static_cast<BitmapCompressionType>(chunk.readTypedUint16());
	_stride = chunk.readTypedUint16();
	debugC(5, kDebugLoading, "%s: imageDataStartOffset: 0x%x, _compressionType: 0x%x, _stride: %d",
		__func__, _imageDataStartOffset, static_cast<uint>(_compressionType), _stride);
}

PixMapImage::PixMapImage(Chunk &chunk, const ImageInfo &imageInfo, bool decompressInPlace) : _imageInfo(imageInfo) {
	if (stride() < width()) {
		warning("%s: Got stride less than width", __func__);
	}

	// Make sure we are at the start of the image data.
	uint imageDataStartPos = chunk.startPos() + _imageInfo._imageDataStartOffset;
	chunk.seek(imageDataStartPos);

	if (chunk.bytesRemaining() > 0) {
		if (isCompressed()) {
			_compressedStream = chunk.readStream(chunk.bytesRemaining());
			if (decompressInPlace) {
				decompress();
			}
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

PixMapImage::PixMapImage(const ImageInfo &imageInfo, bool decompressInPlace) : _imageInfo(imageInfo) {
	_image.create(stride(), height(), Graphics::PixelFormat::createFormatCLUT8());
}

PixMapImage::~PixMapImage() {
	delete _compressedStream;
	_compressedStream = nullptr;
}

bool PixMapImage::isCompressed() const {
	return (getCompressionType() != kUncompressedBitmap) && \
		(getCompressionType() != kUncompressedTransparentBitmap);
}

void PixMapImage::decompress() {
	if (getCompressionType() != kRle8BitmapCompression) {
		return;
	} else if (_compressedStream == nullptr) {
		warning("%s: No compressed data to decompress", __func__);
		return;
	}

	// Decompress the image and then delete the compressed stream.
	_image = g_engine->getDisplayManager()->decompressRle8Bitmap(this, nullptr, nullptr);
	delete _compressedStream;
	_compressedStream = nullptr;

	_imageInfo._compressionType = kUncompressedBitmap;
}

} // End of namespace MediaStation
