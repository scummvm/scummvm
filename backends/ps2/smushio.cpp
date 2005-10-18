/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 *
 */

/*
	The Smush player uses at least two handles for accessing the same SMUSH file,
	to avoid threading issues.	One handle for video, one for audio apparently.
	Each of the handles always skips the data that the other one read before
	(or will read later).

	This behaviour makes it difficult to do read ahead caching without reading
	any given file twice, so this class tries to "reunite" the read accesses and
	do the necessary caching.
*/

#include "backends/ps2/fileio.h"
#include "backends/ps2/asyncfio.h"
#include <kernel.h>
#include <assert.h>
#include <string.h>
#include <sio.h>

extern AsyncFio fio;
extern void sioprintf(const char *zFormat, ...);

#define SMUSH_CACHE_SIZE (5 * 1024 * 1024)
#define READ_STEP (32 * 1024)
#define SMUSH_IN_USE -2

class SmushReader {
public:
	SmushReader(void);
	~SmushReader(void);
	int open(const char *name);
	void close(void);
	uint32 read(void *dest, uint32 from, uint32 len);
	void virtSeek(uint32 from, uint32 to);
	uint32 size(void);
	bool keepOpened(void);
private:
	void processCache(bool sync);
	char _fname[256];
	int _sema, _fd, _refCount;
	uint8 *_cacheBuf;
	volatile uint32 _cacheFilePos, _bytesInCache, _fileSize, _cacheBufOfs;
	volatile uint32 _lastRead[2];
	volatile bool _cacheOp;
};

SmushReader::SmushReader(void) {
	_cacheBuf = NULL;
	_fd = -1;
	_refCount = 0;

	ee_sema_t newSema;
	newSema.init_count = 1;
	newSema.max_count = 1;
	_sema = CreateSema(&newSema);
	assert(_sema >= 0);
}

SmushReader::~SmushReader(void) {
    DeleteSema(_sema);
}

int SmushReader::open(const char *name) {
	WaitSema(_sema);
	if (_refCount) {
		if (stricmp(_fname, name)) {
			sioprintf("SmushReader is already open to file\n%s\nGot open request for %s", _fname, name);
			SignalSema(_sema);
			return SMUSH_IN_USE;
		}
	} else {
		assert(_fd < 0);
		_fd = fio.open(name, O_RDONLY);
		if (_fd < 0) {
			SignalSema(_sema);
			return -1;
		}
		_fileSize = fio.seek(_fd, 0, SEEK_END);
		fio.seek(_fd, 0, SEEK_SET);

		_cacheBuf = (uint8*)malloc(SMUSH_CACHE_SIZE);
		if (!_cacheBuf) {
			sioprintf("Smush Reader ran out of memory");
			fio.close(_fd);
			_fd = -1;
			SignalSema(_sema);
			return -1;
		}
		_lastRead[0] = _lastRead[1] = 0;
		_cacheBufOfs = _bytesInCache = _cacheFilePos = 0;
		fio.read(_fd, _cacheBuf, READ_STEP);
		_cacheOp = true;
		strcpy(_fname, name);
	}
	_refCount++;
	sioprintf("SmushReader %s ref count %d", _fname, _refCount);
	SignalSema(_sema);
	return 0;
}

void SmushReader::close(void) {
	WaitSema(_sema);
	sioprintf("Closing Ref to %s", _fname);
	assert(_refCount > 0);
	_refCount--;
	if (!_refCount) {
		sioprintf("SmushReader: All references to %s closed", _fname);
		processCache(true);
		_fname[0] = '\0';
		fio.close(_fd);
		_fd = -1;
		free(_cacheBuf);
		_cacheBuf = NULL;
	}
	SignalSema(_sema);
}

#define MIN(a, b) ((a < b) ? (a) : (b))

void SmushReader::processCache(bool sync) {
	if (_cacheOp) {
		if (sync || fio.poll(_fd)) { // has the transfer finished or were we told to wait for it to finish?
			int rdRes = fio.sync(_fd);
			assert(rdRes >= 0);
            _bytesInCache += rdRes;
			_cacheOp = false;
		}
	} else if (!sync) {
		if (_cacheFilePos + _bytesInCache == _fileSize)
			return;

		uint32 rdPos = MIN(_lastRead[0], _lastRead[1]);

		int cacheOfs = (rdPos - _cacheFilePos) & ~0xF; // we'd like to keep the buffer aligned to 16 bytes
		if (cacheOfs < 0) {
			sioprintf("ERROR: smush cache too far ahead!");
			return;
		}

		if (_bytesInCache - cacheOfs < SMUSH_CACHE_SIZE - READ_STEP) {
			// we want to do some more reading
			if (_bytesInCache > cacheOfs) {
				_bytesInCache -= cacheOfs;
				_cacheBufOfs  += cacheOfs;
				_cacheFilePos += cacheOfs;
			} else {
				sioprintf("cache underrun!");
				_bytesInCache = 0;
				_cacheBufOfs  = 0;
				_cacheFilePos = rdPos;
			}

			uint32 bufEndPos = (_cacheBufOfs + _bytesInCache) % SMUSH_CACHE_SIZE;
			uint32 readLen = SMUSH_CACHE_SIZE - bufEndPos;
			if (readLen > READ_STEP)
				readLen = READ_STEP;

			fio.read(_fd, _cacheBuf + bufEndPos, readLen);
			_cacheOp = true;
		}
	}
}

uint32 SmushReader::read(void *dest, uint32 from, uint32 len) {
	uint8 *destBuf = (uint8*)dest;
	WaitSema(_sema);
	if ((from >= _cacheFilePos) && (from + len <= _cacheFilePos + _bytesInCache))
		processCache(false);
	else {
		processCache(true); // we'll have to read, sync cache before.
	}
	uint32 readEnds = from + len;
	if (from == _lastRead[0])
		_lastRead[0] += len;
	else if (from == _lastRead[1])
		_lastRead[1] += len;
	else {
		if ((_lastRead[0] > readEnds) && (_lastRead[1] < readEnds)) {
			_lastRead[1] = readEnds;
		} else if ((_lastRead[0] < readEnds) && (_lastRead[1] > readEnds)) {
			_lastRead[0] = readEnds;
		} else {
			if ((_lastRead[0] < readEnds) && (_lastRead[1] < readEnds)) {
				if (_lastRead[0] < _lastRead[1])
					_lastRead[0] = readEnds;
				else
					_lastRead[1] = readEnds;
			} else
				sioprintf("unexpected readend: %d / %d => %d", _lastRead[0], _lastRead[1], readEnds);
		}
	}

	while (len) {
		while (len && (from >= _cacheFilePos) && (from < _cacheFilePos + _bytesInCache)) {
			uint32 cpyOfs = ((from - _cacheFilePos) + _cacheBufOfs) % SMUSH_CACHE_SIZE;
			uint32 cpyLen = _bytesInCache - (from - _cacheFilePos);
			if (cpyLen > len)
				cpyLen = len;
			if (cpyOfs + cpyLen > SMUSH_CACHE_SIZE)
				cpyLen = SMUSH_CACHE_SIZE - cpyOfs;
			memcpy(destBuf, _cacheBuf + cpyOfs, cpyLen);
			destBuf += cpyLen;
			from    += cpyLen;
			len     -= cpyLen;
		}
		if (len) {
			sioprintf("Smush cache missed: read %d -> %d, cache %d -> %d", from, len, _cacheFilePos, _bytesInCache);
			assert(fio.seek(_fd, 0, SEEK_CUR) == _cacheFilePos + _bytesInCache);
			fio.seek(_fd, from, SEEK_SET);
			int rdRes;
			do {
				fio.read(_fd, destBuf, len);
				rdRes = fio.sync(_fd);
                destBuf += rdRes;
				from += rdRes;
				len -= rdRes;
			} while (len && rdRes);
			fio.seek(_fd, _cacheFilePos + _bytesInCache, SEEK_SET);
			break;
		}
	}
	processCache(false);
	SignalSema(_sema);
	return destBuf - (uint8*)dest;
}

void SmushReader::virtSeek(uint32 from, uint32 to) {
	WaitSema(_sema);
	if (_lastRead[0] == from)
		_lastRead[0] = to;
	else if (_lastRead[1] == from)
		_lastRead[1] = to;
	SignalSema(_sema);
}

uint32 SmushReader::size(void) {
	assert(_fd >= 0);
	return _fileSize;
}

bool SmushReader::keepOpened(void) {
	return _refCount > 0;
}

#define MAX_READERS 3

static SmushReader *g_smushReaders[MAX_READERS] = { NULL, NULL, NULL };

static int g_openSema = -1;

Ps2SmushFile::Ps2SmushFile(int64 cacheId) : Ps2File(cacheId) {
    _filePos = _fileSize = 0;
	_id = -1;
	if (g_openSema < 0) {
		ee_sema_t newSema;
		newSema.init_count = 1;
		newSema.max_count = 1;
		g_openSema = CreateSema(&newSema);
		assert(g_openSema >= 0);
	}
}

Ps2SmushFile::~Ps2SmushFile(void) {
	WaitSema(g_openSema);
	if (_id >= 0) {
		g_smushReaders[_id]->close();
		if (!g_smushReaders[_id]->keepOpened()) {
			delete g_smushReaders[_id];
			g_smushReaders[_id] = NULL;
		}
	}
	SignalSema(g_openSema);
}

bool Ps2SmushFile::open(const char *name) {
	WaitSema(g_openSema);
	int opSlot = MAX_READERS;
	for (int i = 0; i < MAX_READERS; i++) {
		if (g_smushReaders[i]) {
			sioprintf("attaching to reader in slot %d", i);
			if (g_smushReaders[i]->open(name) == 0) {
				_id = i;
				_fileSize = g_smushReaders[i]->size();
				sioprintf("attach ok");
				break;
			}
		} else if (opSlot == MAX_READERS)
			opSlot = i;
	}
	if (_id < 0) { // smush file wasn't opened before
		sioprintf("creating new reader in slot %d", opSlot);
		if (opSlot < MAX_READERS) {
			g_smushReaders[opSlot] = new SmushReader();
			if (g_smushReaders[opSlot]->open(name) == 0) {
				_id = opSlot;
				_fileSize = g_smushReaders[opSlot]->size();
			} else {
				// can't open file
				delete g_smushReaders[opSlot];
				g_smushReaders[opSlot] = NULL;
			}
		} else
			printf("Ran out of reader slots\n");
	}
	SignalSema(g_openSema);
	return (_id >= 0);
}

uint32 Ps2SmushFile::read(void *dest, uint32 len) {
	int res = g_smushReaders[_id]->read(dest, _filePos, len);
	_filePos += res;
	return res;
}

uint32 Ps2SmushFile::write(const void *src, uint32 len) {
	printf("ERROR: Received write request on Smush reader\n");
	SleepThread();
	return 0;
}

uint32 Ps2SmushFile::tell(void) {
	return _filePos;
}

uint32 Ps2SmushFile::size(void) {
	return _fileSize;
}

int Ps2SmushFile::seek(int32 offset, int origin) {
	int32 res;
	switch (origin) {
		case SEEK_SET:
			res = offset;
			break;
		case SEEK_CUR:
			res = _filePos + offset;
			break;
		case SEEK_END:
			res = _fileSize + offset;
			break;
		default:
			return -1;
	}
	if ((res >= 0) && (res <= _fileSize)) {
		if (offset != 0)
			g_smushReaders[_id]->virtSeek(_filePos, res);
		_filePos = res;
		return 0;
	}
	return -1;
}

bool Ps2SmushFile::eof(void) {
	return _filePos == _fileSize;
}

