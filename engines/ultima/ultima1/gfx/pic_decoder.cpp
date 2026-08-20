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
#include "ultima/ultima1/gfx/pic_decoder.h"

namespace Ultima {
namespace Ultima1 {
namespace Gfx {

// 16 six-bit EGA palette register values for the castle picture
// (its pixel data assumes this palette, not the standard one)
static const byte CASTLE_PALETTE_REGISTERS[16] = {
	0, 24, 7, 63, 63, 34, 58, 14, 20, 7, 61, 59, 1, 57, 7, 63
};

// Decodes a 6-bit EGA palette register value (bits: R2 G2 B2 R1 G1 B1)
// into 8-bit RGB, per-channel value = primary*0xAA + secondary*0x55
static void decodeEGARegister(byte v, byte &r, byte &g, byte &b) {
	r = ((v >> 2) & 1) * 0xAA + ((v >> 5) & 1) * 0x55;
	g = ((v >> 1) & 1) * 0xAA + ((v >> 4) & 1) * 0x55;
	b = ((v >> 0) & 1) * 0xAA + ((v >> 3) & 1) * 0x55;
}

bool PicDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();
	_surface.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	byte *output_buffer = (byte *)_surface.getPixels();

	// Read in the source data
	Common::Array<byte> ega_data;
	ega_data.resize(stream.size());
	stream.read(&ega_data[0], stream.size());

	// 320x200, 4 bits/pixel, 2 pixels packed per byte (high nibble = left
	// pixel, low nibble = right pixel), stored linearly top-to-bottom -
	// 160 bytes/row
	for (int y = 0; y < 200; ++y) {
		const byte *row = &ega_data[y * 160];
		byte *outputRow = output_buffer + y * 320;

		for (int byteX = 0; byteX < 160; ++byteX) {
			byte byte_data = row[byteX];
			outputRow[byteX * 2] = byte_data >> 4;
			outputRow[byteX * 2 + 1] = byte_data & 0x0f;
		}
	}

	byte rgb[16 * 3];
	for (int i = 0; i < 16; ++i)
		decodeEGARegister(CASTLE_PALETTE_REGISTERS[i], rgb[i * 3], rgb[i * 3 + 1], rgb[i * 3 + 2]);
	_palette = Graphics::Palette(rgb, 16);

	return true;
}

} // namespace Gfx
} // namespace Ultima1
} // namespace Ultima
