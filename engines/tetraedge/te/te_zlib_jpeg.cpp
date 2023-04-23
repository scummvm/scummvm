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

#include "tetraedge/te/te_zlib_jpeg.h"
#include "common/substream.h"
#include "common/compression/zlib.h"

namespace Tetraedge {

TeZlibJpeg::TeZlibJpeg() {
}

TeZlibJpeg::~TeZlibJpeg() {
}

bool TeZlibJpeg::load(Common::SeekableReadStream &stream) {
	uint32 compressedSize = stream.readUint32LE();
	if (compressedSize > stream.size()) {
		warning("[TeImage::load] invalid size %d (file size %d)", compressedSize, (int)stream.size());
		return false;
	}
	uint32 uncompressedSize = stream.readUint32LE();
	Common::SeekableSubReadStream *substream = new Common::SeekableSubReadStream(&stream, stream.pos(), stream.size());
	Common::SeekableReadStream *zlibStream = Common::wrapCompressedReadStream(substream, uncompressedSize);
	bool result = TeJpeg::load(*zlibStream);
	delete zlibStream;
	return result;
}

/*static*/
bool TeZlibJpeg::matchExtension(const Common::String &extn) {
	return extn == "jpeg.zlib";
}

} // end namespace Tetraedge
