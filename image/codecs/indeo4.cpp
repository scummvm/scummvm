/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

/* Intel Indeo 4 decompressor, derived from ffmpeg.
 *
 * Original copyright note: * Intel Indeo 3 (IV31, IV32, etc.) video decoder for ffmpeg
 * written, produced, and directed by Alan Smithee
 */

#include "common/system.h"
#include "common/endian.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/util.h"

#include "graphics/yuv_to_rgb.h"

#include "image/codecs/indeo4.h"
#include "image/codecs/indeo/get_bits.h"

namespace Image {

Indeo4Decoder::Indeo4Decoder(uint16 width, uint16 height) {
	_pixelFormat = g_system->getScreenFormat();
	_surface = new Graphics::ManagedSurface();
	_surface->create(width, height, _pixelFormat);

}

Indeo4Decoder::~Indeo4Decoder() {
	delete _surface;
}

bool Indeo4Decoder::isIndeo4(Common::SeekableReadStream &stream) {
	// Less than 16 bytes? This can't be right
	if (stream.size() < 16)
		return false;

	// Read in the start of the data
	byte buffer[16];
	stream.read(buffer, 16);
	stream.seek(-16, SEEK_CUR);

	// Validate the first 18-bit word has the correct identifier
	Indeo::GetBits gb(buffer, 16 * 8);
	bool isIndeo4 = gb.getBits(18) == 0x3FFF8;

	return isIndeo4;
}

const Graphics::Surface *Indeo4Decoder::decodeFrame(Common::SeekableReadStream &stream) {
	// Not Indeo 4? Fail
	if (!isIndeo4(stream))
		return 0;

	// TODO
	return nullptr;
}

} // End of namespace Image
