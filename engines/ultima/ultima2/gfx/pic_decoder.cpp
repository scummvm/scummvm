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

#include "common/stream.h"
#include "ultima/ultima2/gfx/pic_decoder.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

bool PicDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();
	_surface.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	byte *output_buffer = (byte *)_surface.getPixels();

	// Read in the source data
	Common::Array<byte> cga_data;
	cga_data.resize(stream.size());
	stream.read(&cga_data[0], stream.size());

	int x, y;
	int cga_offset;
	uint8_t byte_data;
	int output_offset;

	// CGA interlaced format:
	// - First 8,192 bytes: even scanlines (0, 2, 4, ..., 198)
	// - Next 8,192 bytes: odd scanlines (1, 3, 5, ..., 199)
	// Each scanline is 80 bytes (320 pixels / 4 pixels per byte)

	// Process even scanlines (0, 2, 4, ..., 198)
	for (y = 0; y < 200; y += 2) {
		cga_offset = (y / 2) * 80;  // Even lines are in first half
		output_offset = y * 320;

		for (x = 0; x < 80; x++) {
			byte_data = cga_data[cga_offset + x];

			// Extract 4 pixels from the byte (2 bits each)
			output_buffer[output_offset + (x * 4) + 0] = (byte_data >> 6) & 0x03;
			output_buffer[output_offset + (x * 4) + 1] = (byte_data >> 4) & 0x03;
			output_buffer[output_offset + (x * 4) + 2] = (byte_data >> 2) & 0x03;
			output_buffer[output_offset + (x * 4) + 3] = (byte_data >> 0) & 0x03;
		}
	}

	// Process odd scanlines (1, 3, 5, ..., 199)
	for (y = 1; y < 200; y += 2) {
		cga_offset = 8192 + ((y / 2) * 80);  // Odd lines are in second half
		output_offset = y * 320;

		for (x = 0; x < 80; x++) {
			byte_data = cga_data[cga_offset + x];

			// Extract 4 pixels from the byte (2 bits each)
			output_buffer[output_offset + (x * 4) + 0] = (byte_data >> 6) & 0x03;
			output_buffer[output_offset + (x * 4) + 1] = (byte_data >> 4) & 0x03;
			output_buffer[output_offset + (x * 4) + 2] = (byte_data >> 2) & 0x03;
			output_buffer[output_offset + (x * 4) + 3] = (byte_data >> 0) & 0x03;
		}
	}

	return true;
}

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima
