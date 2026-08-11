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

#include "got/gfx/gfx_chunks.h"
#include "common/file.h"
#include "got/utils/compression.h"

namespace Got {
namespace Gfx {

void GraphicChunk::load(Common::SeekableReadStream *src, const byte *data) {
	_compressMode = src->readUint16LE();
	_offset = src->readUint32LE();
	_uncompressedSize = src->readUint16LE();
	_compressedSize = src->readUint16LE();
	_width = src->readUint16LE();
	_height = src->readUint16LE();

	_data = data + _offset;
}

void GraphicChunk::enable() {
	// Data already uncompressed, nothing further needed
	if (_compressMode == UNCOMPRESSED)
		return;

	_decompressedData.resize(_uncompressedSize);

	switch (_compressMode) {
	case LZSS:
		decompressLzss(_data, &_decompressedData[0], _uncompressedSize);
		break;

	case RLE:
		decompressRle(_data, &_decompressedData[0], _uncompressedSize);
		break;

	default:
		error("Unknown compression type %d", _compressMode);
	}

	// Mark the entry as uncompressed, and point to the data
	_compressMode = UNCOMPRESSED;
	_data = &_decompressedData[0];
}

GraphicChunk::operator const Graphics::ManagedSurface() const {
	Graphics::ManagedSurface s;
	s.w = s.pitch = _width;
	s.h = _height;
	s.format = Graphics::PixelFormat::createFormatCLUT8();
	s.setPixels(const_cast<byte *>(_data));

	return s;
}

GraphicChunk::operator const Gfx::Palette63() const {
	return Gfx::Palette63(_data);
}

void GfxChunks::load() {
	// Get stream to access images
	Common::SeekableReadStream *f = getStream();

#if 0
    Common::DumpFile df;
    if (df.open("got.gfx")) {
        df.writeStream(f);
        f->seek(0);
    }
    df.close();
#endif

	// Keep a copy in memory for decoding images as needed
	_data = new byte[f->size()];
	f->read(_data, f->size());

	// Set the number of images
	f->seek(0);
	_chunks.resize(f->readUint16LE());

	// Iterate through loading the image metrics
	for (uint i = 0; i < _chunks.size(); ++i)
		_chunks[i].load(f, _data);

	// Close the file
	delete f;

	// Decompress two ranges of chunks by default
	for (uint i = 0; i < 34; ++i)
		_chunks[i].enable();
}

GraphicChunk &GfxChunks::operator[](uint idx) {
	GraphicChunk &chunk = _chunks[idx];
	chunk.enable();

	return chunk;
}

Common::SeekableReadStream *GfxChunks::getStream() const {
	// Check for stand-alone graphics file
	Common::File *f = new Common::File;
	if (f->open("got.gfx"))
		return f;
	delete f;

	// Check for got.exe executable
	Common::File fExe;
	if (!fExe.open("got.exe"))
		error("Could not locate got graphics");

	if (fExe.readUint16BE() != MKTAG16('M', 'Z'))
		error("Invalid exe header");

	const int lastPageSize = fExe.readUint16LE();
	const int totalPages = fExe.readUint16LE();
	const int offset = lastPageSize ? ((totalPages - 1) << 9) + lastPageSize : totalPages << 9;

	fExe.seek(offset);
	if (fExe.readUint16BE() != MKTAG16(0xe2, 0x4a))
		error("Invalid embedded graphics signature");

	return fExe.readStream(fExe.size() - fExe.pos());
}

} // namespace Gfx
} // namespace Got
