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

#include "titanic/compressed_file.h"

namespace Titanic {

#define BUFFER_SIZE 1024

CompressedFile::CompressedFile() : SimpleFile() {
	_readStream = nullptr;
	_writeStream = nullptr;
}

CompressedFile::~CompressedFile() {
}

void CompressedFile::open(const Common::String &name) {
	SimpleFile::open(name);
	_readStream = Common::wrapCompressedReadStream(&_file);
}

void CompressedFile::open(Common::SeekableReadStream *stream) {
	SimpleFile::open(stream);
	_readStream = Common::wrapCompressedReadStream(&_file);
}

void CompressedFile::open(Common::OutSaveFile *stream) {
	SimpleFile::open(stream);
	_writeStream = Common::wrapCompressedWriteStream(stream);
}

void CompressedFile::close() {
	delete _readStream;
	delete _writeStream;
	_readStream = nullptr;
	_writeStream = nullptr;

	SimpleFile::close();
}

size_t CompressedFile::unsafeRead(void *dst, size_t count) {
	assert(_readStream);
	if (count == 0)
		return 0;

	// Read data and decompress
	return _readStream->read(dst, count);
}

size_t CompressedFile::write(const void *src, size_t count) {
	return _writeStream->write(src, count);
}

} // End of namespace Titanic
