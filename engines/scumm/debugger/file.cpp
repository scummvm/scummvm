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

#include "common/endian.h"
#include "common/fs.h"

#include "scumm/debugger/file.h"

namespace Scumm {

namespace Editor {

File::File()
	: _encByte(0),
	  _stream(nullptr) {
}

File::~File() {
	close();
}

bool File::open(const Common::Path &path, byte encByte) {
	close();

	_path = path;
	_encByte = encByte;
	_stream = Common::FSNode(path).createReadStream();

	return _stream != nullptr;
}

void File::close() {
	delete _stream;
	_stream = nullptr;
}

const Common::Path &File::getPath() const {
	return _path;
}

int64 File::pos() const {
	return _stream ? _stream->pos() : 0;
}

int64 File::size() const {
	return _stream ? _stream->size() : 0;
}

bool File::seek(int64 offs, int whence) {
	return _stream ? _stream->seek(offs, whence) : false;
}

uint32 File::read(void *dataPtr, uint32 dataSize) {
	if (!_stream)
		return 0;

	uint32 bytesRead = _stream->read(dataPtr, dataSize);
	byte *p = (byte *)dataPtr;
	for (uint32 i = 0; i < bytesRead; ++i)
		p[i] ^= _encByte;

	return bytesRead;
}

byte File::readByte() {
	byte b = 0;
	read(&b, 1);
	return b;
}

uint16 File::readUint16LE() {
	byte buf[2];
	read(buf, 2);
	return READ_LE_UINT16(buf);
}

uint32 File::readUint32LE() {
	byte buf[4];
	read(buf, 4);
	return READ_LE_UINT32(buf);
}

uint16 File::readUint16BE() {
	byte buf[2];
	read(buf, 2);
	return READ_BE_UINT16(buf);
}

uint32 File::readUint32BE() {
	byte buf[4];
	read(buf, 4);
	return READ_BE_UINT32(buf);
}

} // End of namespace Editor

} // End of namespace Scumm
