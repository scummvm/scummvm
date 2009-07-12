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

#include "backends/platform/ps2/fileio.h"

#include <tamtypes.h>
#include <kernel.h>
#include <fileio.h>
#include <assert.h>
#include <string.h>
#include "backends/platform/ps2/asyncfio.h"
#include "engines/engine.h"
#include "common/file.h"
#include "eecodyvdfs.h"
#include "common/config-manager.h"
#include "backends/platform/ps2/ps2debug.h"
#include "backends/platform/ps2/systemps2.h"

#define __PS2_FILE_SEMA__ 1
// #define __PS2_FILE_DEBUG 1
// #define __PS2_CACHE_DEBUG__ 1

#define PS2_CACHE_MAX (128 * 1024)
#define PS2_CACHE_CHK (16 * 1024)

extern OSystem_PS2 *g_systemPs2;

uint32 _rseek;

AsyncFio fio;

Ps2File::Ps2File(void) {
	_fd = -1;
	_fileSize = 0;
	_filePos = 0;
	_cacheSize = 0;
	_cachePos = 0;
	_eof = false;

	// _cache = (uint8 *)malloc(PS2_CACHE_MAX);

	_cacheBuf = (uint8*)memalign(64, CACHE_SIZE * 2);

	_cacheOpRunning = 0;
	_filePos = _physFilePos = _cachePos = 0;
	_fileSize = _bytesInCache = _cacheOfs = 0;
	_cacheOpRunning = false;
	_readBytesBlock = 0;
	_stream = true;

#ifdef __PS2_FILE_SEMA__
	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_sema = CreateSema(&newSema);
	assert(_sema >= 0);
#endif
}

Ps2File::~Ps2File(void) {
	uint32 w;
	if (_fd >= 0) {

		if (_mode != O_RDONLY) {
			fio.seek(_fd, 0, SEEK_SET);
			fio.write(_fd, _cacheBuf, _filePos);
			w = fio.sync(_fd);
			printf("flushed wbuf: %x of %x\n", w, _filePos);
		}

		fio.close(_fd);
		uint32 r = fio.sync(_fd);
		printf("close [%d] - sync'd = %d\n", _fd, r);
	}

	// free(_cache);
	free(_cacheBuf);

#ifdef __PS2_FILE_SEMA__
	DeleteSema(_sema);
#endif
}

bool Ps2File::open(const char *name, int mode) {
#if 1
	_fd = fio.open(name, mode);

	printf("open %s [%d]\n", name, _fd);

	if (_fd >= 0) {
		_mode = mode;
		_filePos = 0;

		if (_mode == O_RDONLY) {
			_fileSize = fio.seek(_fd, 0, SEEK_END);
			fio.seek(_fd, 0, SEEK_SET);
		}
		else
			_fileSize = 0;

		printf("  _mode = %x\n", _mode);
		printf("  _fileSize = %d\n", _fileSize);
		// printf("  _filePos = %d\n", _filePos);

		return true;
	}

	return false;
#else
	uint32 r;

	// hack: FIO does not reports size for RW (?)
	_fd = fio.open(name, O_RDONLY);
	if (_fd >= 0) {
		_fileSize = fio.seek(_fd, 0, SEEK_END);
		fio.seek(_fd, 0, SEEK_SET); /* rewind ! */

		if (_fileSize && mode != O_RDONLY) {
			fio.read(_fd, _cacheBuf, _fileSize);
			r = fio.sync(_fd);
			printf(" sz=%d, read=%d\n", _fileSize, r);
			assert(r == _fileSize);
		}

		fio.close(_fd);
	}
	else
		_fileSize = 0; /* new file */

	_fd = fio.open(name, mode);

	printf("open %s [%d]\n", name, _fd);

	if (_fd >= 0) {
		_mode = mode;
		_filePos = 0;

		if (_fileSize) { /* existing data */
			if (mode == O_RDONLY) {
				/* DANGER: for w* modes it will truncate your fine files */
				fio.seek(_fd, 0, SEEK_SET);
			}
			else if (_mode & O_APPEND) {
				fio.seek(_fd, 0, _fileSize);
				_filePos = _fileSize;
			}
			#if 0 /* file already trunc'd when opened as w* -> moved up */
			if (mode != O_RDONLY) {
				fio.read(_fd, _cacheBuf, _fileSize);
				r = fio.sync(_fd);
				printf(" sz=%d, read=%d\n", _fileSize, r);
				assert(r == _fileSize);
				// _fileSize = fio.seek(_fd, 0, SEEK_END);
			}
			#endif
		}

		printf("  _mode = %x\n", _mode);
		printf("  _fileSize = %d\n", _fileSize);
		printf("  _filePos = %d\n", _filePos);

		return true;
	} else
		return false;
#endif
}

int32 Ps2File::tell(void) {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif
	int32 res = _filePos;
#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif
	return res;
}

int32 Ps2File::size(void) {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif
	int32 res = _fileSize;
#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif
	return res;
}

bool Ps2File::eof(void) {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif
	bool res = _eof; // (_filePos == _fileSize);
	// bool res = (_filePos >= _fileSize);
#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);

	// printf(" EOF [%d] : %d of %d  -> %d\n", _fd, _filePos, _fileSize, res);
#endif
	return res;
}

bool Ps2File::getErr(void) {
	return _eof;
}

void Ps2File::setErr(bool err) {
	_eof = err;
}

int Ps2File::seek(int32 offset, int origin) {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif
	_rseek = 0;
	int seekDest;
	int res = -1;
	switch (origin) {
		case SEEK_SET:
			seekDest = offset;
			break;
		case SEEK_CUR:
			seekDest = _filePos + offset;
			break;
		case SEEK_END:
			seekDest = _fileSize + offset;
			break;
		default:
			seekDest = -1;
			break;
	}
	if ((seekDest >= 0) && (seekDest <= (int)_fileSize)) {
		// _rseek = fio.sync(_fd);
		_filePos = seekDest;
		// fio.seek(_fd, _filePos, SEEK_SET);
		// fio.sync(_fd);
		// _cacheSize = 0;
		_eof = false;
		res = 0;
	}
	else _eof = true;

	// printf("seek [%d]  %d  %d\n", _fd, offset, origin);
	// printf("  res = %d\n", res);

#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif

	return res;
}

void Ps2File::cacheReadAhead(void) {
	if (_cacheOpRunning) {
		// there's already some cache read running
		if (fio.poll(_fd)) // did it finish?
			cacheReadSync(); // yes.
	}
	if ((!_cacheOpRunning) && ((_readBytesBlock >= CACHE_READ_THRESHOLD) || _stream) && fio.fioAvail()) {
		// the engine seems to do sequential reads and there are no other I/Os going on. read ahead.
		uint32 cachePosEnd = _cachePos + _bytesInCache;

		if (_cachePos > _filePos)
			return; // there was a seek in the meantime, don't cache.
		if (cachePosEnd - _filePos >= CACHE_FILL_MIN)
			return; // cache is full enough.
		if (cachePosEnd == _fileSize)
			return; // can't read beyond EOF.

		assert(cachePosEnd < _fileSize);

		if (_cachePos + _bytesInCache <= _filePos) {
			_cacheOfs = _bytesInCache = 0;
			_cachePos = cachePosEnd = _filePos & ~READ_ALIGN_MASK;
			assert(_filePos == _physFilePos);
		} else {
			uint32 cacheDiff = _filePos - _cachePos;
			assert(_bytesInCache >= cacheDiff);
			cacheDiff &= ~READ_ALIGN_MASK;
			_bytesInCache -= cacheDiff;
			_cachePos += cacheDiff;
			_cacheOfs = (_cacheOfs + cacheDiff) % CACHE_SIZE;
		}

		if (_physFilePos != cachePosEnd) {
			sioprintf("unexpected _physFilePos %d cache %d %d\n", _physFilePos, _cacheOfs, _bytesInCache);
			// assert(!(cachePosEnd & READ_ALIGN_MASK)); // romeo
			_physFilePos = fio.seek(_fd, cachePosEnd, SEEK_SET);
			if (_physFilePos != cachePosEnd) {
				sioprintf("cache seek error: seek to %d instead of %d, fs = %d\n", _physFilePos, cachePosEnd, _fileSize);
				return;
			}
		}

		uint32 cacheDest = (_cacheOfs + _bytesInCache) % CACHE_SIZE;
		uint32 cacheRead = CACHE_SIZE - _bytesInCache;
		if (cacheDest + cacheRead > CACHE_SIZE)
			cacheRead = CACHE_SIZE - cacheDest;
		if (cacheRead > MAX_READ_STEP)
			cacheRead = MAX_READ_STEP;

		assert((!(cacheRead & READ_ALIGN_MASK)) && cacheRead);

		_cacheOpRunning = true;
		fio.read(_fd, _cacheBuf + cacheDest, cacheRead);
	}
}

void Ps2File::cacheReadSync(void) {
	if (_cacheOpRunning) {
		int res = fio.sync(_fd);
		assert(res >= 0);
		_bytesInCache += res;
		_physFilePos += res;
		_cacheOpRunning = false;
	}
}

uint32 Ps2File::read(void *dest, uint32 len) {
	// uint32 r=0, d=0, ds=0, sz=0;
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif

#ifdef __PS2_FILE_DEBUG__
	printf("read (1) : _filePos = %d\n", _filePos);
	printf("read (1) : _cachePos = %d\n", _cachePos);
#endif

	if (_filePos >= _fileSize) {
		_eof = true;
#ifdef __PS2_FILE_SEMA__
    SignalSema(_sema);
#endif
		return 0;
	}

	if ((_filePos+len) > _fileSize) {
		len = _fileSize-_filePos;
		_eof = true;
	}

	uint8 *destBuf = (uint8*)dest;
	if ((_filePos < _cachePos) || (_filePos + len > _cachePos + _bytesInCache))
		cacheReadSync(); // we have to read from CD, sync cache.

	while (len && (_filePos != _fileSize)) {
		if ((_filePos >= _cachePos) && (_filePos < _cachePos + _bytesInCache)) { // read from cache
			uint32 staPos = (_cacheOfs + (_filePos - _cachePos)) % CACHE_SIZE;
			uint32 cpyLen = _bytesInCache - (_filePos - _cachePos);
			if (cpyLen > len)
				cpyLen = len;
			if (staPos + cpyLen > CACHE_SIZE)
				cpyLen = CACHE_SIZE - staPos;

			assert(cpyLen);
			memcpy(destBuf, _cacheBuf + staPos, cpyLen);
			_filePos += cpyLen;
			destBuf += cpyLen;
			_readBytesBlock += len;
			len -= cpyLen;
		} else { // cache miss
			assert(!_cacheOpRunning);
			if (_physFilePos != _filePos) {
				if ((_filePos < _physFilePos) || (_filePos > _physFilePos + (CACHE_SIZE / 2)))
					_readBytesBlock = 0; // reset cache hit count

				_physFilePos = _filePos & ~READ_ALIGN_MASK;
				if (fio.seek(_fd, _physFilePos, SEEK_SET) != (int)_physFilePos)
					break; // read beyond EOF
			}

			int doRead = len + (_filePos - _physFilePos);
			doRead = (doRead + READ_ALIGN_MASK) & ~READ_ALIGN_MASK;

			if (doRead > MAX_READ_STEP)
				doRead = MAX_READ_STEP;
			if (doRead < 2048)
				doRead = 2048;

			fio.read(_fd, _cacheBuf, doRead);
			_cachePos = _physFilePos;
			_cacheOfs = 0;
			_bytesInCache = fio.sync(_fd);
			_physFilePos += _bytesInCache;
			if (!_bytesInCache)
				break; // EOF
		}
	}
	cacheReadAhead();
	SignalSema(_sema);
	return destBuf - (uint8*)dest;
}

uint32 Ps2File::write(const void *src, uint32 len) {
#ifdef __PS2_FILE_SEMA__
	WaitSema(_sema);
#endif

	memcpy(&_cacheBuf[_filePos], src, len);
	_filePos += len;

#ifdef __PS2_FILE_SEMA__
	SignalSema(_sema);
#endif

	return len;
}

FILE *ps2_fopen(const char *fname, const char *mode) {
	Ps2File *file = new Ps2File();
	int _mode = O_RDONLY;

	printf("fopen(%s, %s)\n", fname, mode);

	if (mode[0] == 'r' && mode [1] == 'w')
		_mode = O_RDWR;
	else if (mode[0] == 'w')
		_mode = O_WRONLY | O_CREAT;
	else if (mode[0] == 'a')
		_mode = O_RDWR | O_CREAT | O_APPEND;

	if (file->open(fname, _mode))
		return (FILE *)file;
	else
		return NULL;
}

int ps2_fclose(FILE *stream) {
	Ps2File *file = (Ps2File*)stream;

	delete file;

	return 0;
}

int ps2_fseek(FILE *stream, long offset, int origin) {
	return ((Ps2File*)stream)->seek(offset, origin);
}

uint32 ps2_ftell(FILE *stream) {
	return ((Ps2File*)stream)->tell();
}

int ps2_feof(FILE *stream) {
	return ((Ps2File*)stream)->eof();
}

size_t ps2_fread(void *buf, size_t r, size_t n, FILE *stream) {
	assert(r != 0);
	return ((Ps2File*)stream)->read(buf, r * n) / r;
}

int ps2_fgetc(FILE *stream) {
	uint8 temp;
	if (((Ps2File*)stream)->read(&temp, 1))
		return temp;
	else
		return EOF;
}

size_t ps2_fwrite(const void *buf, size_t r, size_t n, FILE *stream) {
	assert(r != 0);
	return ((Ps2File*)stream)->write(buf, r * n) / r;
}

int ps2_fputc(int c, FILE *stream) {
	if (((Ps2File*)stream)->write(&c, 1) == 1)
		return c;
	else
		return -1;
}

int ps2_fputs(const char *s, FILE *stream) {
	int len = strlen(s);
	if (ps2_fwrite(s, 1, len, stream) == (size_t)len)
		return len;
	else
		return EOF;
}

int ps2_fprintf(FILE *pOut, const char *zFormat, ...) {
	va_list ap;
	char resStr[2048];

	va_start(ap,zFormat);
	int res = vsnprintf(resStr, 2048, zFormat, ap);
	va_end(ap);
	if ((pOut == stderr) || (pOut == stdout)) {
		printf("%s", resStr);
		sioprintf("%s", resStr);
	} else
		res = ps2_fwrite(resStr, 1, res, pOut);

	return res;
}

int ps2_fflush(FILE *stream) {
	// printf("fflush not implemented\n");
	return 0;
}

int ps2_ferror(FILE *stream) {
	int err = ((Ps2File*)stream)->getErr();

	if (err)
		printf("ferror -> %d\n", err);

	return err;
}

void ps2_clearerr(FILE *stream) {
	((Ps2File*)stream)->setErr(false);
}
