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

constexpr int BANK_SIZE = 0x2000;  // reserved size of each interlaced CGA bank
constexpr int BYTES_PER_ROW = 80;  // 320 pixels / 4 pixels-per-byte

// CGA mode 4, palette 1, border/background color 0 (black)
static const byte CGA_PALETTE1[4 * 3] = {
	0x00, 0x00, 0x00, // 0: black
	0x55, 0xff, 0xff, // 1: light cyan
	0xff, 0x55, 0xff, // 2: light magenta
	0xff, 0xff, 0xff  // 3: white
};

bool PicDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();
	_surface.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	byte *output_buffer = (byte *)_surface.getPixels();

	Common::Array<byte> data;
	data.resize(stream.size());
	stream.read(&data[0], stream.size());

	for (int y = 0; y < 200; ++y) {
		int bank = y & 1;
		const byte *row = &data[bank * BANK_SIZE + (y / 2) * BYTES_PER_ROW];
		byte *outputRow = output_buffer + y * 320;

		for (int byteX = 0; byteX < BYTES_PER_ROW; ++byteX) {
			byte b = row[byteX];
			outputRow[byteX * 4 + 0] = (b >> 6) & 3;
			outputRow[byteX * 4 + 1] = (b >> 4) & 3;
			outputRow[byteX * 4 + 2] = (b >> 2) & 3;
			outputRow[byteX * 4 + 3] = b & 3;
		}
	}

	_palette = Graphics::Palette(CGA_PALETTE1, 4);
	return true;
}

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima
