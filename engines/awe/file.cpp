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

#include "awe/file.h"

namespace Awe {
#ifdef DEPRECATED

struct File_impl {
	bool _ioErr;
	File_impl() : _ioErr(false) {}
	virtual bool open(const char *path, const char *mode) = 0;
	virtual void close() = 0;
	virtual void seek(int32 off) = 0;
	virtual void read(void *ptr, uint32 size) = 0;
	virtual void write(void *ptr, uint32 size) = 0;
};

struct stdFile : File_impl {
	FILE *_fp;
	stdFile() : _fp(0) {}
	bool open(const char *path, const char *mode) {
		_ioErr = false;
		_fp = fopen(path, mode);
		return (_fp != NULL);
	}
	void close() {
		if (_fp) {
			fclose(_fp);
			_fp = 0;
		}
	}
	void seek(int32 off) {
		if (_fp) {
			fseek(_fp, off, SEEK_SET);
		}
	}
	void read(void *ptr, uint32 size) {
		if (_fp) {
			uint32 r = fread(ptr, 1, size, _fp);
			if (r != size) {
				_ioErr = true;
			}
		}
	}
	void write(void *ptr, uint32 size) {
		if (_fp) {
			uint32 r = fwrite(ptr, 1, size, _fp);
			if (r != size) {
				_ioErr = true;
			}
		}
	}
};

struct zlibFile : File_impl {
	gzFile _fp;
	zlibFile() : _fp(0) {}
	bool open(const char *path, const char *mode) {
		_ioErr = false;
		_fp = gzopen(path, mode);
		return (_fp != NULL);
	}
	void close() {
		if (_fp) {
			gzclose(_fp);
			_fp = 0;
		}
	}
	void seek(int32 off) {
		if (_fp) {
			gzseek(_fp, off, SEEK_SET);
		}
	}
	void read(void *ptr, uint32 size) {
		if (_fp) {
			uint32 r = gzread(_fp, ptr, size);
			if (r != size) {
				_ioErr = true;
			}
		}
	}
	void write(void *ptr, uint32 size) {
		if (_fp) {
			uint32 r = gzwrite(_fp, ptr, size);
			if (r != size) {
				_ioErr = true;
			}
		}
	}
};

File::File(bool gzipped) {
	if (gzipped) {
		_impl = new zlibFile;
	} else {
		_impl = new stdFile;
	}
}

File::~File() {
	_impl->close();
	delete _impl;
}

bool File::open(const char *filename, const char *directory, const char *mode) {	
	_impl->close();
	char buf[512];
	sprintf(buf, "%s/%s", directory, filename);
	char *p = buf + strlen(directory) + 1;
	string_lower(p);
	bool opened = _impl->open(buf, mode);
	if (!opened) { // let's try uppercase
		string_upper(p);
		opened = _impl->open(buf, mode);
	}
	return opened;
}

void File::close() {
	_impl->close();
}

bool File::ioErr() const {
	return _impl->_ioErr;
}

void File::seek(int32 off) {
	_impl->seek(off);
}

void File::read(void *ptr, uint32 size) {
	_impl->read(ptr, size);
}

uint8 File::readByte() {
	uint8 b;
	read(&b, 1);
	return b;
}

uint16 File::readUint16BE() {
	uint8 hi = readByte();
	uint8 lo = readByte();
	return (hi << 8) | lo;
}

uint32 File::readUint32BE() {
	uint16 hi = readUint16BE();
	uint16 lo = readUint16BE();
	return (hi << 16) | lo;
}

void File::write(void *ptr, uint32 size) {
	_impl->write(ptr, size);
}

void File::writeByte(uint8 b) {
	write(&b, 1);
}

void File::writeUint16BE(uint16 n) {
	writeByte(n >> 8);
	writeByte(n & 0xFF);
}

void File::writeUint32BE(uint32 n) {
	writeUint16BE(n >> 16);
	writeUint16BE(n & 0xFFFF);
}
#endif

} // namespace Awe
