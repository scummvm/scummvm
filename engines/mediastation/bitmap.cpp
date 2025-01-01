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
	/*uint header_size_in_bytes =*/ Datum(chunk, DatumType::UINT16_1).u.i;
	dimensions = Datum(chunk).u.point;
	compression_type = BitmapHeader::CompressionType(Datum(chunk, DatumType::UINT16_1).u.i);
	debugC(5, kDebugLoading, "BitmapHeader::BitmapHeader(): _compressionType = 0x%x", compression_type);
	// TODO: Figure out what this is.
	// This has something to do with the width of the bitmap but is always
	// a few pixels off from the width. And in rare cases it seems to be
	// the true width!
	unk2 = Datum(chunk, DatumType::UINT16_1).u.i;
}

BitmapHeader::~BitmapHeader() {
	delete dimensions;
	dimensions = nullptr;
}

bool BitmapHeader::isCompressed() {
	return (compression_type != BitmapHeader::CompressionType::UNCOMPRESSED) &&
	       (compression_type != BitmapHeader::CompressionType::UNCOMPRESSED_2);
}

Bitmap::Bitmap(Chunk &chunk, BitmapHeader *bitmapHeader) :
	_bitmapHeader(bitmapHeader) {
	// The header must be constructed beforehand.
	uint16 width = _bitmapHeader->dimensions->x;
	uint16 height = _bitmapHeader->dimensions->y;
	surface.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	uint8 *pixels = (uint8 *)surface.getPixels();
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
	return _bitmapHeader->dimensions->x;
}

uint16 Bitmap::height() {
	return _bitmapHeader->dimensions->y;
}

void Bitmap::decompress(Chunk &chunk) {
	// GET THE COMPRESSED DATA.
	uint compressed_image_data_size_in_bytes = chunk.bytesRemaining();
	char *compressed_image_start = new char[compressed_image_data_size_in_bytes];
	char *compressed_image = compressed_image_start;
	chunk.read(compressed_image, compressed_image_data_size_in_bytes);

	// MAKE SURE WE READ PAST THE FIRST 2 BYTES.
	char *compressed_image_data_start = compressed_image;
	if ((*compressed_image++ == 0) && (*compressed_image++ == 0)) {
		// This condition is empty, we just put it first since this is the expected case
		// and the negated logic would be not as readable.
	} else {
		compressed_image = compressed_image_data_start;
	}
	char *compressed_image_data_end = compressed_image + compressed_image_data_size_in_bytes;

	// GET THE DECOMPRESSED PIXELS BUFFER.
	// Media Station has 8 bits per pixel, so the decompression buffer is
	// simple.
	// TODO: Do we have to set the pixels ourselves?
	char *decompressed_image = (char *)surface.getPixels();

	// DECOMPRESS THE RLE-COMPRESSED BITMAP STREAM.
	bool transparency_run_ever_read = false;
	size_t transparency_run_top_y_coordinate = 0;
	size_t transparency_run_left_x_coordinate = 0;
	bool image_fully_read = false;
	size_t current_y_coordinate = 0;
	while (current_y_coordinate < height()) {
		size_t current_x_coordinate = 0;
		bool reading_transparency_run = false;
		while (true) {
			uint8_t operation = *compressed_image++;
			if (operation == 0x00) {
				// ENTER CONTROL MODE.
				operation = *compressed_image++;
				if (operation == 0x00) {
					// MARK THE END OF THE LINE.
					// Also check if the image is finished being read.
					if (compressed_image >= compressed_image_data_end) {
						image_fully_read = true;
					}
					break;
				} else if (operation == 0x01) {
					// MARK THE END OF THE IMAGE.
					// TODO: When is this actually used?
					image_fully_read = true;
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
					reading_transparency_run = true;
					transparency_run_top_y_coordinate = current_y_coordinate;
					transparency_run_left_x_coordinate = current_x_coordinate;
					transparency_run_ever_read = true;
				} else if (operation == 0x03) {
					// ADJUST THE PIXEL POSITION.
					// This permits jumping to a different part of the same row without
					// needing a run of pixels in between. But the actual data consumed
					// seems to actually be higher this way, as you need the control byte
					// first.
					// So to skip 10 pixels using this approach, you would encode 00 03 0a 00.
					// But to "skip" 10 pixels by encoding them as blank (0xff), you would encode 0a ff.
					// What gives? I'm not sure.
					uint8_t x_change = *compressed_image++;
					current_x_coordinate += x_change;
					uint8_t y_change = *compressed_image++;
					current_y_coordinate += y_change;
				} else if (operation >= 0x04) {
					// READ A RUN OF UNCOMPRESSED PIXELS.
					size_t y_offset = current_y_coordinate * width();
					size_t run_starting_offset = y_offset + current_x_coordinate;
					char *run_starting_pointer = decompressed_image + run_starting_offset;
					uint8_t run_length = operation;
					memcpy(run_starting_pointer, compressed_image, run_length);
					compressed_image += operation;
					current_x_coordinate += operation;

					if (((uintptr_t)compressed_image) % 2 == 1) {
						compressed_image++;
					}
				}
			} else {
				// READ A RUN OF LENGTH ENCODED PIXELS.
				size_t y_offset = current_y_coordinate * width();
				size_t run_starting_offset = y_offset + current_x_coordinate;
				char *run_starting_pointer = decompressed_image + run_starting_offset;
				uint8_t color_index_to_repeat = *compressed_image++;
				uint8_t repetition_count = operation;
				memset(run_starting_pointer, color_index_to_repeat, repetition_count);
				current_x_coordinate += repetition_count;

				if (reading_transparency_run) {
					// GET THE TRANSPARENCY RUN STARTING OFFSET.
					size_t transparency_run_y_offset = transparency_run_top_y_coordinate * width();
					size_t transparency_run_start_offset = transparency_run_y_offset + transparency_run_left_x_coordinate;
					size_t transparency_run_ending_offset = y_offset + current_x_coordinate;
					size_t transparency_run_length = transparency_run_ending_offset - transparency_run_start_offset;
					// char *transparency_run_src_pointer = keyframe_image + run_starting_offset;
					// char *transparency_run_dest_pointer = decompressed_image + run_starting_offset;

					// COPY THE TRANSPARENT AREA FROM THE KEYFRAME.
					// The "interior" of transparency regions is always encoded by a single run of
					// pixels, usually 0x00 (white).
					// memcpy(transparency_run_dest_pointer, transparency_run_src_pointer, transparency_run_length);
					reading_transparency_run = false;
				}
			}
		}

		current_y_coordinate++;
		if (image_fully_read) {
			break;
		}
	}
	delete[] compressed_image_start;
}

}
