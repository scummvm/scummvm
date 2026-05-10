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

#ifndef MEDIASTATION_BITMAP_H
#define MEDIASTATION_BITMAP_H

#include "common/rect.h"
#include "graphics/managed_surface.h"

#include "mediastation/datafile.h"
#include "mediastation/actor.h"

namespace MediaStation {

enum BitmapCompressionType {
	kUncompressedBitmap = 0,
	kRle8BitmapCompression = 1,
	kCccBitmapCompression = 5,
	kCccTransparentBitmapCompression = 6,
	kUncompressedTransparentBitmap = 7,
};

class ImageInfo {
public:
	ImageInfo() = default;
	ImageInfo(Chunk &chunk);

	Common::Point _dimensions;
	BitmapCompressionType _compressionType = kUncompressedBitmap;
	int16 _stride = 0;
	uint _imageDataStartOffset = 0;
};

class PixMapImage {
public:
	PixMapImage(Chunk &chunk, const ImageInfo &imageInfo, bool decompressInPlace = false);
	PixMapImage(const ImageInfo &imageInfo, bool decompressInPlace = false);
	virtual ~PixMapImage();

	bool isCompressed() const;
	BitmapCompressionType getCompressionType() const { return _imageInfo._compressionType; }
	void setCompressionType(BitmapCompressionType compressionType) { _imageInfo._compressionType = compressionType; }
	int16 width() const { return _imageInfo._dimensions.x; }
	int16 height() const { return _imageInfo._dimensions.y; }
	int16 stride() const { return _imageInfo._stride; }

	Common::SeekableReadStream *_compressedStream = nullptr;
	Graphics::ManagedSurface _image;

private:
	ImageInfo _imageInfo;
	void decompress();
};

} // End of namespace MediaStation

#endif
