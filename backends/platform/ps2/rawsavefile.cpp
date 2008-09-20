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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "backends/platform/ps2/rawsavefile.h"
#include "backends/platform/ps2/savefilemgr.h"

RawReadFile::RawReadFile(McAccess *mcAccess) {
	_mcAccess = mcAccess;
	_size = -1;
	_pos = 0;
	_buf = NULL;
	_eof = false;
}

RawReadFile::~RawReadFile(void) {
	if (_buf)
		free(_buf);
}

bool RawReadFile::bufOpen(const char *path) {
	int handle = _mcAccess->open(path, O_RDONLY);
	printf("RawReadFile Open: \"%s\" => %d\n", path, handle);
	if (handle >= 0) {
		_size = _mcAccess->size(handle);
		_buf = (uint8 *)memalign(64, _size);
		int res = _mcAccess->read(handle, _buf, _size);
		if (res != _size) {
			free(_buf);
			_size = -1;
			_buf = NULL;
		}
		_mcAccess->close(handle);
	} else {
		_size = -1;
		_buf = NULL;
	}
	_pos = 0;
	return (_buf != NULL);
}

int RawReadFile::bufTell(void) const {
	return _pos;
}

int RawReadFile::bufSeek(int ofs, int whence) {
	switch (whence) {
		case SEEK_SET:
			_pos = ofs;
			break;
		case SEEK_CUR:
			_pos += ofs;
			break;
		case SEEK_END:
			_pos = _size + ofs;
			break;
	}
	if (_pos < 0)
		_pos = 0;
	else if (_pos > _size)
		_pos = _size;

	_eof = false;
	return _pos;
}

int RawReadFile::bufRead(void *dest, int size) {
	if (_pos + size > _size) {
		size = _size - _pos;
		_eof = true;
	}
	memcpy(dest, _buf + _pos, size);
	_pos += size;
	return size;
}

int RawReadFile::bufSize(void) const {
	return _size;
}

bool RawReadFile::bufEof(void) const {
	return _eof;
}

void RawReadFile::bufClearErr(void) const {
	_eof = false;
}

RawWriteFile::RawWriteFile(McAccess *mcAccess) {
	_mcAccess = mcAccess;
	_size = 64 * 1024;

	_buf = (uint8 *)memalign(64, _size);
	_pos = 0;
	_handle = -1;
}

RawWriteFile::~RawWriteFile() {
	if (_pos != 0) {
		printf("RawWriteFile d'tor: file wasn't flushed!\n");
		bufFlush();
	}
	free(_buf);
	if (_handle >= 0)
		_mcAccess->close(_handle);
}

bool RawWriteFile::bufOpen(const char *path) {
	_handle = _mcAccess->open(path, O_WRONLY | O_CREAT);
	strcpy(_filename, path);
	return (_handle >= 0);
}

void RawWriteFile::bufWrite(const void *buf, int len) {
	while (_pos + len > _size) {
		_size = _size * 2;
		_buf = (uint8 *)realloc(_buf, _size);
	}
	memcpy(_buf + _pos, buf, len);
	_pos += len;
}

bool RawWriteFile::bufFlush(void) {
	int result = _mcAccess->write(_handle, _buf, _pos);
	if (_pos != result) {
		if (result > 0) {
			memmove(_buf, _buf + result, _pos - result);
			_pos -= result;
		}
		return false;
	}
	_pos = 0;
	return true;
}

int RawWriteFile::bufTell(void) const {
	return _pos;
}

void RawWriteFile::removeFile(void) {
	if (_handle >= 0)
		_mcAccess->close(_handle);
	_handle = -1;
	_pos = 0;

	_mcAccess->remove(_filename);
}
