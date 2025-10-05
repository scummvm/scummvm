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
#include "mediastation/asset.h"

namespace MediaStation {

enum BitmapCompressionType {
	kUncompressedBitmap = 0,
	kRle8BitmapCompression = 1,
	kCccBitmapCompression = 5,
	kCccTransparentBitmapCompression = 6,
	kUncompressedTransparentBitmap = 7,
};

class BitmapHeader {
public:
	BitmapHeader(Chunk &chunk);

	Common::Point _dimensions;
	BitmapCompressionType _compressionType = kUncompressedBitmap;
	int16 _stride = 0;
};

class Bitmap {
public:
	Bitmap(Chunk &chunk, BitmapHeader *bitmapHeader);
	virtual ~Bitmap();

	bool isCompressed() const;
	BitmapCompressionType getCompressionType() const { return _bitmapHeader->_compressionType; }
	int16 width() const { return _bitmapHeader->_dimensions.x; }
	int16 height() const { return _bitmapHeader->_dimensions.y; }
	int16 stride() const { return _bitmapHeader->_stride; }

	Common::SeekableReadStream *_compressedStream = nullptr;
	Graphics::ManagedSurface _image;

private:
	BitmapHeader *_bitmapHeader = nullptr;
	uint _unk1 = 0;
};

} // End of namespace MediaStation

#endif
