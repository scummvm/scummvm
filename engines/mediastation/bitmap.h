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

#include "graphics/managed_surface.h"

#include "mediastation/chunk.h"
#include "mediastation/assetheader.h"

#ifndef MEDIASTATION_BITMAP_H
#define MEDIASTATION_BITMAP_H

namespace MediaStation {

class BitmapHeader {
public:
	enum class CompressionType {
		UNCOMPRESSED = 0,
		RLE_COMPRESSED = 1,
		UNK1 = 6,
		UNCOMPRESSED_2 = 7,
	};

	BitmapHeader(Chunk &chunk);
	~BitmapHeader();

	bool isCompressed();

	Common::Point *dimensions = nullptr;
	CompressionType compression_type;
	uint unk2;
};

class Bitmap {
public:
	BitmapHeader *_bitmapHeader = nullptr;

	Bitmap(Chunk &chunk, BitmapHeader *bitmapHeader);
	~Bitmap();

	uint16 width();
	uint16 height();
	Graphics::ManagedSurface surface;

private:
	void decompress(Chunk &chunk);
};

}

#endif