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

#include "common/file.h"
#include "got/gfx/images.h"
#include "got/utils/compression.h"

namespace Got {
namespace Gfx {

void GraphicChunk::load(Common::SeekableReadStream *src, const byte *data) {
	_data = data;

	_compressMode = src->readUint16LE();
	_offset = src->readUint32LE();
	_uncompressedSize = src->readUint16LE();
	_compressedSize = src->readUint16LE();
	_width = src->readUint16LE();
	_height = src->readUint16LE();
}

GraphicChunk::operator Graphics::ManagedSurface &() {
	assert(_height != 0 && _width != 0);

	// Create the managed surface
	_image.create(_width, _height);

	const byte *src = _data + _offset;

	size_t size = MIN((int)_uncompressedSize, _width * _height);

	switch (_compressMode) {
	case UNCOMPRESSED:
		// Uncompressed image, simply read it in
		Common::copy(src, src + size, (byte *)_image.getPixels());
		break;

	case LZSS:
		lzss_decompress(src + _offset, (byte *)_image.getPixels(), size);
		break;

	case RLE:
		rle_decompress(src + _offset, (byte *)_image.getPixels(), size);
		break;

	default:
		error("Unknown compression type %d", _compressMode);
		break;
	}

	// Return a reference to the now loaded surface
	return _image;
}


void GfxChunks::load() {
	// Get stream to access images
	Common::SeekableReadStream *f = getStream();

	// Keep a copy in memory for decoding images as needed
	_data = new byte[f->size()];
	f->read(_data, f->size());

	// Set the number of images
	f->seek(0);
	_images.resize(f->readUint16LE());

	// Iterate through loading the image metrics
	for (uint i = 0; i < _images.size(); ++i)
		_images[i].load(f, _data);

	// Close the file
	delete f;
}

Common::SeekableReadStream *GfxChunks::getStream() const {
	// Check for stand-alone graphics file
	Common::File *f = new Common::File;
	if (f->open("graphics.got"))
		return f;
	delete f;

	// Check for got.exe executable
	Common::File fExe;
	if (!fExe.open("got.exe"))
		error("Could not locate got graphics");

	if (fExe.readUint16BE() != MKTAG16('M', 'Z'))
		error("Invalid exe header");

	int lastPageSize = fExe.readUint16LE();
	int totalPages = fExe.readUint16LE();
	int offset = lastPageSize ? ((totalPages - 1) << 9) + lastPageSize :
		totalPages << 9;

	fExe.seek(offset);
	if (fExe.readUint16BE() != MKTAG16(0xe2, 0x4a))
		error("Invalid embedded graphics signature");

	return fExe.readStream(fExe.size() - fExe.pos());
}

} // namespace Gfx
} // namespace Got
