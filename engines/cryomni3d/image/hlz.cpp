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

#include "cryomni3d/image/hlz.h"

#include "common/stream.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "image/codecs/codec.h"

#include "image/codecs/hlz.h"

namespace Image {

HLZFileDecoder::HLZFileDecoder() {
	_surface = nullptr;
	_codec = nullptr;
}

HLZFileDecoder::~HLZFileDecoder() {
	destroy();
}

void HLZFileDecoder::destroy() {
	delete _codec;
	_codec = nullptr;
	_surface = nullptr;
}

bool HLZFileDecoder::loadStream(Common::SeekableReadStream &stream) {
	destroy();

	stream.read(_palette, sizeof(_palette));
	uint16 width = stream.readUint16LE();
	uint16 height = stream.readUint16LE();

	if (width == 0 || height == 0) {
		return false;
	}

	_codec = new HLZDecoder(width, height);
	_surface = _codec->decodeFrame(stream);
	return true;
}

} // End of namespace Image
