/* ScummVM - Graphic Adventure AweEngine
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
#include "awe/util.h"

namespace Awe {

#ifdef DEPRECATED
struct File_impl {
	bool _ioErr;
	File_impl() : _ioErr(false) {
	}
	virtual ~File_impl() {
	}
	virtual bool open(const char *path, const char *mode) = 0;
	virtual void close() = 0;
	virtual uint32 size() = 0;
	virtual void seek(int off, int whence) = 0;
	virtual int read(void *ptr, uint32 len) = 0;
	virtual int write(void *ptr, uint32 len) = 0;
};

struct stdFile : File_impl {
	FILE *_fp;
	stdFile() : _fp(0) {
	}
	bool open(const char *path, const char *mode) {
		_ioErr = false;
		_fp = fopen(path, mode);
		return (_fp != 0);
	}
	void close() {
		if (_fp) {
			fclose(_fp);
			_fp = 0;
		}
	}
	uint32 size() {
		uint32 sz = 0;
		if (_fp) {
			int pos = ftell(_fp);
			fseek(_fp, 0, SEEK_END);
			sz = ftell(_fp);
			fseek(_fp, pos, SEEK_SET);
		}
		return sz;
	}
	void seek(int off, int whence) {
		if (_fp) {
			fseek(_fp, off, whence);
		}
	}
	int read(void *ptr, uint32 len) {
		if (_fp) {
			uint32 r = fread(ptr, 1, len, _fp);
			if (r != len) {
				_ioErr = true;
			}
			return r;
		}
		return 0;
	}
	int write(void *ptr, uint32 len) {
		if (_fp) {
			uint32 r = fwrite(ptr, 1, len, _fp);
			if (r != len) {
				_ioErr = true;
			}
			return r;
		}
		return 0;
	}
};

File::File() {
	_impl = new stdFile;
}

File::~File() {
	_impl->close();
	delete _impl;
}

bool File::open(const char *filepath) {
	_impl->close();
	return _impl->open(filepath, "rb");
}

static bool getFilePathNoCase(const char *filename, const char *path, char *out) {
	bool ret = false;
	DIR *d = opendir(path);
	if (d) {
		dirent *de;
		while ((de = readdir(d)) != NULL) {
			if (de->d_name[0] == '.') {
				continue;
			}
			if (strcasecmp(de->d_name, filename) == 0) {
				sprintf(out, "%s/%s", path, de->d_name);
				ret = true;
				break;
			}
		}
		closedir(d);
	}
	return ret;
}

bool File::open(const char *filename, const char *path) {
	_impl->close();
	char filepath[MAXPATHLEN];
	if (getFilePathNoCase(filename, path, filepath)) {
		return _impl->open(filepath, "rb");
	}
	return false;
}

bool File::openForWriting(const char *filepath) {
	_impl->close();
	return _impl->open(filepath, "wb");
}

void File::close() {
	_impl->close();
}

bool File::ioErr() const {
	return _impl->_ioErr;
}

uint32 File::size() {
	return _impl->size();
}

void File::seek(int off, int whence) {
	_impl->seek(off, whence);
}

int File::read(void *ptr, uint32 len) {
	return _impl->read(ptr, len);
}

uint8 File::readByte() {
	uint8 b;
	read(&b, 1);
	return b;
}

uint16 File::readUint16LE() {
	uint8 lo = readByte();
	uint8 hi = readByte();
	return (hi << 8) | lo;
}

uint32 File::readUint32LE() {
	uint16 lo = readUint16LE();
	uint16 hi = readUint16LE();
	return (hi << 16) | lo;
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

int File::write(void *ptr, uint32 len) {
	return _impl->write(ptr, len);
}

void File::writeByte(uint8 b) {
	write(&b, 1);
}

void File::writeUint16LE(uint16 n) {
	writeByte(n & 0xFF);
	writeByte(n >> 8);
}

void File::writeUint32LE(uint32 n) {
	writeUint16LE(n & 0xFFFF);
	writeUint16LE(n >> 16);
}

void File::writeUint16BE(uint16 n) {
	writeByte(n >> 8);
	writeByte(n & 0xFF);
}

void File::writeUint32BE(uint32 n) {
	writeUint16BE(n >> 16);
	writeUint16BE(n & 0xFFFF);
}

void dumpFile(const char *filename, const uint8 *p, int size) {
	char path[MAXPATHLEN];
	snprintf(path, sizeof(path), "DUMP/%s", filename);
	FILE *fp = fopen(path, "wb");
	if (fp) {
		const int wr = fwrite(p, 1, size, fp);
		if (wr != size) {
			warning("Failed to write %d bytes (expected %d)", wr, size);
		}
		fclose(fp);
	}
}
#endif

void dumpFile(const char *filename, const uint8 *p, int size) {
	error("TODO: dumpFile - %s", filename);
}

} // namespace Awe
