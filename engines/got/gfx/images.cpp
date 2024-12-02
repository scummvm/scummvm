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

namespace Got {
namespace Gfx {

void GraphicChunk::loadInfo(GfxChunks *owner, int index, Common::SeekableReadStream *src) {
	_owner = owner;
	_index = index;

	_field0 = src->readUint16LE();
	_field2 = src->readUint16LE();
	_field4 = src->readUint16LE();
	_uncompressedSize = src->readUint16LE();
	_compressedSize = src->readUint16LE();
	_width = src->readUint16LE();
	_height = src->readUint16LE();
}

void GraphicChunk::load() {
	_owner->load(_index);
}

void GraphicChunk::unload() {
	_owner->unload(_index);
}

GfxChunks::~GfxChunks() {
	delete[] _data;
}

void GfxChunks::load() {
	// Get stream to access images
	Common::SeekableReadStream *f = getStream();

	// Set the number of images
	_images.resize(f->readUint16LE());

	// Iterate through loading the image metrics
	for (uint i = 0; i < _images.size(); ++i)
		_images[i].loadInfo(this, i, f);

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

	int lastPageSize = f->readUint16LE();
	int totalPages = f->readUint16LE();
	int offset = lastPageSize ? ((totalPages - 1) << 9) + lastPageSize :
		totalPages << 9;

	fExe.seek(offset);
	if (fExe.readUint16BE() != MKTAG16(0xe2, 0x4a))
		error("Invalid embedded graphics signature");

	return fExe.readStream(fExe.size() - fExe.pos());
}

void GfxChunks::load(int index) {

}

void GfxChunks::unload(int index) {
	_images[index]._image.clear();
}

void GfxChunks::loadRange(int start, int count) {
	for (; count > 0; ++start, --count)
		load(start);
}

} // namespace Gfx
} // namespace Got
