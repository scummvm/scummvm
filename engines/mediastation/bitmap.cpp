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

#include "mediastation/datum.h"
#include "mediastation/bitmap.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

BitmapHeader::BitmapHeader(Chunk &chunk) {
	uint headerSizeInBytes = Datum(chunk, kDatumTypeUint16_1).u.i;
	debugC(5, kDebugLoading, "BitmapHeader::BitmapHeader(): headerSize = 0x%x", headerSizeInBytes);
	_dimensions = Datum(chunk).u.point;
	_compressionType = BitmapCompressionType(Datum(chunk, kDatumTypeUint16_1).u.i);
	debugC(5, kDebugLoading, "BitmapHeader::BitmapHeader(): _compressionType = 0x%x", static_cast<uint>(_compressionType));
	// TODO: Figure out what this is.
	// This has something to do with the width of the bitmap but is always
	// a few pixels off from the width. And in rare cases it seems to be
	// the true width!
	unk2 = Datum(chunk, kDatumTypeUint16_1).u.i;
}

BitmapHeader::~BitmapHeader() {
	delete _dimensions;
	_dimensions = nullptr;
}

bool BitmapHeader::isCompressed() {
	return (_compressionType != kUncompressedBitmap1) && (_compressionType != kUncompressedBitmap2);
}

Bitmap::Bitmap(Chunk &chunk, BitmapHeader *bitmapHeader) :
	_bitmapHeader(bitmapHeader) {
	// The header must be constructed beforehand.
	uint16 width = _bitmapHeader->_dimensions->x;
	uint16 height = _bitmapHeader->_dimensions->y;
	_surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_surface.setTransparentColor(0);
	uint8 *pixels = (uint8 *)_surface.getPixels();
	if (_bitmapHeader->isCompressed()) {
		// DECOMPRESS THE IMAGE.
		// chunk.skip(chunk.bytesRemaining());
		debugC(5, kDebugLoading, "Bitmap::Bitmap(): Decompressing bitmap");
		decompress(chunk);
	} else {
		// READ THE UNCOMPRESSED IMAGE DIRECTLY.
		// TODO: Understand why we need to ignore these 2 bytes.
		chunk.skip(2);
		chunk.read(pixels, chunk.bytesRemaining());
	}
}

Bitmap::~Bitmap() {
	delete _bitmapHeader;
	_bitmapHeader = nullptr;
}

uint16 Bitmap::width() {
	return _bitmapHeader->_dimensions->x;
}

uint16 Bitmap::height() {
	return _bitmapHeader->_dimensions->y;
}

void Bitmap::decompress(Chunk &chunk) {
	// GET THE COMPRESSED DATA.
	uint compressedImageDataSizeInBytes = chunk.bytesRemaining();
	char *compressedImageStart = new char[compressedImageDataSizeInBytes];
	char *compressedImage = compressedImageStart;
	chunk.read(compressedImage, compressedImageDataSizeInBytes);

	// MAKE SURE WE READ PAST THE FIRST 2 BYTES.
	char *compressedImageDataStart = compressedImage;
	if ((*compressedImage++ == 0) && (*compressedImage++ == 0)) {
		// This condition is empty, we just put it first since this is the expected case
		// and the negated logic would be not as readable.
	} else {
		compressedImage = compressedImageDataStart;
	}
	char *compressedImageDataEnd = compressedImage + compressedImageDataSizeInBytes;

	// GET THE DECOMPRESSED PIXELS BUFFER.
	// Media Station has 8 bits per pixel, so the decompression buffer is
	// simple.
	// TODO: Do we have to set the pixels ourselves?
	char *decompressedImage = (char *)_surface.getPixels();

	// DECOMPRESS THE RLE-COMPRESSED BITMAP STREAM.
	// TODO: Comemnted out becuase transparency runs not supported yet,
	// and there were compiler warnings about these variables not being used.
	// bool transparencyRunEverRead = false;
	// size_t transparencyRunTopYCoordinate = 0;
	// size_t transparencyRunLeftXCoordinate = 0;
	bool imageFullyRead = false;
	size_t currentYCoordinate = 0;
	while (currentYCoordinate < height()) {
		size_t currentXCoordinate = 0;
		bool readingTransparencyRun = false;
		while (true) {
			uint8_t operation = *compressedImage++;
			if (operation == 0x00) {
				// ENTER CONTROL MODE.
				operation = *compressedImage++;
				if (operation == 0x00) {
					// MARK THE END OF THE LINE.
					// Also check if the image is finished being read.
					if (compressedImage >= compressedImageDataEnd) {
						imageFullyRead = true;
					}
					break;
				} else if (operation == 0x01) {
					// MARK THE END OF THE IMAGE.
					// TODO: When is this actually used?
					imageFullyRead = true;
					break;
				} else if (operation == 0x02) {
					// MARK THE START OF A KEYFRAME TRANSPARENCY REGION.
					// Until a color index other than 0x00 (usually white) is read on this line,
					// all pixels on this line will be marked transparent.
					// If no transparency regions are present in this image, all 0x00 color indices are treated
					// as transparent. Otherwise, only the 0x00 color indices within transparency regions
					// are considered transparent. Only intraframes (frames that are not keyframes) have been
					// observed to have transparency regions, and these intraframes have them so the keyframe
					// can extend outside the boundary of the intraframe and
					// still be removed.
					//
					// TODO: Comemnted out becuase transparency runs not
					// supported yet, and there were compiler warnings about
					// these variables being set but not used.
					// readingTransparencyRun = true;
					// transparencyRunTopYCoordinate = currentYCoordinate;
					// transparencyRunLeftXCoordinate = currentXCoordinate;
					// transparencyRunEverRead = true;
				} else if (operation == 0x03) {
					// ADJUST THE PIXEL POSITION.
					// This permits jumping to a different part of the same row without
					// needing a run of pixels in between. But the actual data consumed
					// seems to actually be higher this way, as you need the control byte
					// first.
					// So to skip 10 pixels using this approach, you would encode 00 03 0a 00.
					// But to "skip" 10 pixels by encoding them as blank (0xff), you would encode 0a ff.
					// What gives? I'm not sure.
					uint8_t x_change = *compressedImage++;
					currentXCoordinate += x_change;
					uint8_t y_change = *compressedImage++;
					currentYCoordinate += y_change;
				} else if (operation >= 0x04) {
					// READ A RUN OF UNCOMPRESSED PIXELS.
					size_t yOffset = currentYCoordinate * width();
					size_t runStartingOffset = yOffset + currentXCoordinate;
					char *runStartingPointer = decompressedImage + runStartingOffset;
					uint8_t runLength = operation;
					memcpy(runStartingPointer, compressedImage, runLength);
					compressedImage += operation;
					currentXCoordinate += operation;

					if (((uintptr_t)compressedImage) % 2 == 1) {
						compressedImage++;
					}
				}
			} else {
				// READ A RUN OF LENGTH ENCODED PIXELS.
				size_t yOffset = currentYCoordinate * width();
				size_t runStartingOffset = yOffset + currentXCoordinate;
				char *runStartingPointer = decompressedImage + runStartingOffset;
				uint8_t colorIndexToRepeat = *compressedImage++;
				uint8_t repetitionCount = operation;
				memset(runStartingPointer, colorIndexToRepeat, repetitionCount);
				currentXCoordinate += repetitionCount;

				if (readingTransparencyRun) {
					// TODO: This code is comemnted out becuase the engine
					// doesn't support the keyframes/transparency regions on
					// movies yet. However, only some movies have this to start with.

					// GET THE TRANSPARENCY RUN STARTING OFFSET.
					// size_t transparencyRunYOffset = transparencyRunTopYCoordinate * width();
					// size_t transparencyRunStartOffset = transparencyRunYOffset + transparencyRunLeftXCoordinate;
					// size_t transparencyRunEndingOffset = yOffset + currentXCoordinate;
					// size_t transparency_run_length = transparencyRunEndingOffset - transparencyRunStartOffset;
					// char *transparencyRunSrcPointer = keyframe_image + runStartingOffset;
					// char *transparencyRunDestPointer = decompressedImage + runStartingOffset;

					// COPY THE TRANSPARENT AREA FROM THE KEYFRAME.
					// The "interior" of transparency regions is always encoded by a single run of
					// pixels, usually 0x00 (white).
					// memcpy(transparencyRunDestPointer, transparencyRunSrcPointer, transparency_run_length);
					readingTransparencyRun = false;
				}
			}
		}

		currentYCoordinate++;
		if (imageFullyRead) {
			break;
		}
	}
	delete[] compressedImageStart;
}

}
