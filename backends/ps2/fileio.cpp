/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "backends/ps2/fileio.h"

#include <tamtypes.h>
#include <kernel.h>
#include <fileio.h>
#include <assert.h>
#include <string.h>
//#include <fileXio_rpc.h>
#include <cdvd_rpc.h>
#include "backends/ps2/asyncfio.h"
#include "base/engine.h"
#include "common/file.h"

#define CACHE_BUF_SIZE (2048 * 16)
#define MAX_CACHED_FILES 8

//#define DEFAULT_MODE (FIO_S_IRUSR | FIO_S_IWUSR | FIO_S_IRGRP | FIO_S_IWGRP | FIO_S_IROTH | FIO_S_IWOTH)

extern void sioprintf(const char *zFormat, ...);

AsyncFio fio;

AccessFio::AccessFio(void) {
	_handle = -1;
}

AccessFio::~AccessFio(void) {
	if (_handle >= 0)
		fio.close(_handle);
}

int32 AccessFio::sync(void) {
	return fio.sync(_handle);
}

bool AccessFio::poll(void) {
	return fio.poll(_handle);
}

bool AccessFio::fioAvail(void) {
	return fio.fioAvail();
}

bool AccessFio::open(const char *name, int mode) {
	_handle = fio.open(name, mode);
	return (_handle >= 0);
}

void AccessFio::read(void *dest, uint32 size) {
	fio.read(_handle, dest, size);
}

void AccessFio::write(const void *src, uint32 size) {
	fio.write(_handle, src, size);
}

int AccessFio::seek(int32 offset, int whence) {
	return fio.seek(_handle, offset, whence);
}

/*class AccessFioX : public AccessFio{
public:
	AccessFioX(void);
	virtual ~AccessFioX(void);
	virtual bool open(const char *name, int mode);
	virtual int read(void *dest, uint32 size);
	virtual int write(const void *src, uint32 size);
	virtual int seek(int32 offset, int whence);
	virtual void sync(int32 *res);
};

AccessFioX::AccessFioX(void) {
	_handle = -1;
}

AccessFioX::~AccessFioX(void) {
	if (_handle >= 0)
		fileXioClose(_handle);
}

void AccessFioX::sync(int32 *res) {
	assert(false);
}

bool AccessFioX::open(const char *name, int mode) {
	_handle = fileXioOpen(name, mode, DEFAULT_MODE);
	return (_handle >= 0);
}

int AccessFioX::read(void *dest, uint32 size) {
	return fileXioRead(_handle, (unsigned char*)dest, size);
}

int AccessFioX::write(const void *src, uint32 size) {
	return fileXioWrite(_handle, (unsigned char*)src, size);
}

int AccessFioX::seek(int32 offset, int whence) {
	return fileXioLseek(_handle, offset, whence);
}*/

struct TocNode {
	char name[64];
	TocNode *next, *sub;
	bool isDir;
	uint8 nameLen;
};

class TocManager {
public:
	TocManager(void);
	~TocManager(void);
	void readEntries(const char *root);
    int64 fileExists(const char *name);
	bool haveEntries(void);
private:
	void readDir(const char *path, TocNode **node, int level);
	TocNode *_rootNode;
	char _root[256];
	uint8 _rootLen;
};

class Ps2File {
public:
	Ps2File(int64 cacheId);
	~Ps2File(void);
	bool open(const char *name, int ioMode);
	bool isOpen(void);
	uint32 read(void *dest, uint32 size);
	uint32 write(const void *src, uint32 size);
	uint32 tell(void);
	uint32 size(void);
	int seek(int32 offset, int origin);
	bool eof(void);
	AccessFio *giveHandle(void);
	int64 _cacheId;
	void setSeekReset(void);
private:
	void checkCache(void);
	void syncCache(void);
	bool _cacheOp;
	bool _seekReset;

	bool _forWriting;
	AccessFio *_handle;
	uint8 *_cacheBuf, *_cachePos;
	uint32 _filePos;
	uint32 _cacheBytesLeft;
	uint32 _fSize;
	uint32 _readBytesBlock;
};

Ps2File::Ps2File(int64 cacheId) {
	_handle = NULL;
	_cachePos = _cacheBuf = (uint8*)malloc(CACHE_BUF_SIZE);
	_fSize = _filePos = _cacheBytesLeft = 0;
	_forWriting = false;
	_readBytesBlock = 0;
	_cacheOp = false;
	_cacheId = cacheId;
	_seekReset = false;
}

Ps2File::~Ps2File(void) {
	if (_handle != NULL) {
		syncCache();
		if (_forWriting && (_cachePos != _cacheBuf)) {
			_handle->write(_cacheBuf, _cachePos - _cacheBuf);
			int res = _handle->sync();
			if (res != (_cachePos - _cacheBuf)) {
				// Fixme: writing operation failed and we noticed
				// too late to return an error to the engine. 
				printf("ERROR: flushing the cache on fclose() failed!\n");
			}
		}
		delete _handle;
	}
	free(_cacheBuf);
}

bool Ps2File::open(const char *name, int ioMode) {
	//if (strncmp(name, "pfs0", 4) == 0)
	//	_handle = new AccessFioX();
	//else
	_handle = new AccessFio();

	if (_handle->open(name, ioMode)) {
		if (ioMode == O_RDONLY) {
			_fSize = _handle->seek(0, SEEK_END);
			_handle->seek(0, SEEK_SET);
		} else {
			_cacheBytesLeft = CACHE_BUF_SIZE;
			_forWriting = true;
		}
		return true;
	} else {
		delete _handle;
		_handle = NULL;
		return false;
	}
}

void Ps2File::setSeekReset(void) {
	_seekReset = true;
}

bool Ps2File::isOpen(void) {
	return (_handle != NULL);
}

int Ps2File::seek(int32 offset, int origin) {
	assert(!_forWriting);
	syncCache();
	uint32 seekDest;
	switch (origin) {
		case SEEK_SET:
			seekDest = offset;
			break;
		case SEEK_CUR:
			if (_seekReset)
				seekDest = offset;
			else
				seekDest = _filePos + offset;
			break;
		case SEEK_END:
			seekDest = _fSize + offset;
			break;
		default:
			return -1;
	}
	_seekReset = false;
	if (seekDest <= _fSize) {
		if ((seekDest >= _filePos) && (seekDest < _filePos + _cacheBytesLeft)) {
			uint32 cacheOffset = (seekDest - _filePos);
			_cacheBytesLeft -= cacheOffset;
            _cachePos += cacheOffset;
		} else {
			_handle->seek(seekDest, SEEK_SET);
			_cacheBytesLeft = 0;
		}
		_filePos = seekDest;
		_readBytesBlock = 0;
		return 0;
	} else
		return -1;
}

bool Ps2File::eof(void) {
	if ((!_forWriting) && (_filePos == _fSize))
		return true;
	else
		return false;
}

void Ps2File::checkCache(void) {
	if (!_forWriting) {
		if (_readBytesBlock > 32768) {
			if (_cacheBytesLeft <= (CACHE_BUF_SIZE / 4)) {
				if (_cacheBytesLeft && (_cachePos != _cacheBuf)) {
					memmove(_cacheBuf, _cachePos, _cacheBytesLeft);
				}
				_cachePos = _cacheBuf;
				_handle->read(_cacheBuf + _cacheBytesLeft, CACHE_BUF_SIZE - _cacheBytesLeft);
				_cacheOp = true;
			}
		}
	}
}

void Ps2File::syncCache(void) {
	if ((!_forWriting) && _cacheOp) {
		int cacheRes = _handle->sync();
		assert(cacheRes >= 0);
		_cacheBytesLeft += cacheRes;
		_cacheOp = false;
	}
}

uint32 Ps2File::read(void *dest, uint32 size) {
	assert(!_forWriting);
	syncCache();
	if (_seekReset)
        seek(0, SEEK_SET);	
	_readBytesBlock += size;

	uint8 *destBuf = (uint8*)dest;
	while (size) {
		if (_cacheBytesLeft != 0) {
			uint32 doCopy = (size >= _cacheBytesLeft) ? _cacheBytesLeft : size;
			memcpy(destBuf, _cachePos, doCopy);
			size -= doCopy;
			_cacheBytesLeft -= doCopy;
			_cachePos += doCopy;
			destBuf += doCopy;
			_filePos += doCopy;
		}
		if (size > 0) {
			assert(_cacheBytesLeft == 0);
			if (size >= CACHE_BUF_SIZE) {
				int readRes;
				do {
					_handle->read(destBuf, size);
					readRes = _handle->sync();
					_filePos += readRes;
					destBuf += readRes;
					size -= readRes;
				} while (size && readRes);
				if (size)
					printf("read operation failed, %d bytes left to read\n", size);
				return destBuf - (uint8*)dest;
			} else {
				uint32 doRead = size;
				if ((doRead < 2048) && (_cacheId >= 0))
					doRead = 2048;
				memset(_cacheBuf, 'A', 0x20);
				_handle->read(_cacheBuf, doRead);
				_cacheBytesLeft = _handle->sync();
				_cachePos = _cacheBuf;
				if (_cacheBytesLeft == 0) {
					return destBuf - (uint8*)dest;
				}
			}
		}
	}
	checkCache();
	return destBuf - (uint8*)dest;
}

uint32 Ps2File::write(const void *src, uint32 size) {
	assert(_forWriting && (!_seekReset));
	const uint8 *srcBuf = (const uint8*)src;

	while (size) {
		uint32 doWrite = (size > _cacheBytesLeft) ? _cacheBytesLeft : size;
		memcpy(_cachePos, srcBuf, doWrite);
		_cachePos += doWrite;
		_cacheBytesLeft -= doWrite;
		size -= doWrite;
		srcBuf += doWrite;
		_filePos += doWrite;

		if (_cacheBytesLeft == 0) {
			_handle->write(_cacheBuf, CACHE_BUF_SIZE);
			int res = _handle->sync();
			if (res == CACHE_BUF_SIZE) {
				_cachePos = _cacheBuf;
				_cacheBytesLeft = CACHE_BUF_SIZE;
			} else {
				printf("cache write operation failed, only %d bytes written\n", res);
				return 0;
			}

			if (size >= CACHE_BUF_SIZE) {
				int writeRes;
				do {
					_handle->write(srcBuf, size);
					writeRes = _handle->sync();
					_filePos += writeRes;
					srcBuf += writeRes;
					size -= writeRes;
				} while (size && writeRes);
				if (size)
					printf("write operation failed, %d bytes left to write\n", size);
                return srcBuf - (uint8*)src;
			}
		}
	}
	return srcBuf - (uint8*)src;
}

uint32 Ps2File::tell(void) {
	if (_seekReset)
		seek(0, SEEK_SET);
	return _filePos;
}

uint32 Ps2File::size(void) {
	assert(!_forWriting);
	return _fSize;
}

AccessFio *Ps2File::giveHandle(void) {
	assert(_handle);
	return _handle;
}

TocManager tocManager;

struct FioHandleCache {
	Ps2File *file;
	FioHandleCache *next, *prev;
};

static FioHandleCache *cacheListStart = NULL;
static FioHandleCache *cacheListEnd = NULL;
static int cacheListLen = 0;
static int cacheListSema = -1;
static bool wantHandleCaching = true;

extern void ps2_disableHandleCaching(void) {
	wantHandleCaching = false;
}

Ps2File *findInCache(int64 id);

FILE *ps2_fopen(const char *fname, const char *mode) {
	if (cacheListSema == -1) {
		ee_sema_t newSema;
		newSema.init_count = 1;
		newSema.max_count = 1;
		cacheListSema = CreateSema(&newSema);
		assert(cacheListSema >= 0);
	}
	if (!tocManager.haveEntries() && g_engine) // read the TOC the first time the engine opens a file
		tocManager.readEntries(g_engine->getGameDataPath());

	int fioMode = 0;
	switch(*mode) {
		case 'r':
			fioMode = O_RDONLY;
			break;
		case 'w':
			fioMode = O_WRONLY | O_CREAT | O_TRUNC;
			break;
		default:
			printf("unsupported mode \"%s\" for file \"%s\"\n", mode, fname);
			return NULL;
	}
	int64 cacheId = -1;
	if ((fioMode == O_RDONLY) && tocManager.haveEntries())
		cacheId = tocManager.fileExists(fname);

	if (cacheId != 0) {
		Ps2File *file = findInCache(cacheId);
		if (file)
			return (FILE*)file;

		if ((mode[1] != 'b') && (mode[1] != '\0')) {
			printf("unsupported mode \"%s\" for file \"%s\"\n", mode, fname);
			return NULL;
		}
		file = new Ps2File(cacheId);
		if (file->open(fname, fioMode))
			return (FILE*)file;
		else
			delete file;
	}
	return NULL;
}

void checkCacheListLen(void) {
	while (cacheListLen > MAX_CACHED_FILES) {
		assert(cacheListEnd && cacheListStart);
		delete cacheListEnd->file;
		cacheListEnd->prev->next = NULL;
		FioHandleCache *temp = cacheListEnd;
		cacheListEnd = cacheListEnd->prev;
		delete temp;
		cacheListLen--;
	}
}

int ps2_fclose(FILE *stream) {
	Ps2File *file = (Ps2File*)stream;
	if ((file->_cacheId > 0) && wantHandleCaching) { // this is a file on the CD, could be smart to cache it
		FioHandleCache *newHandle = new FioHandleCache;
		newHandle->file = file;
		file->setSeekReset();

		WaitSema(cacheListSema);
		if (!cacheListEnd) {
			newHandle->prev = newHandle->next = NULL;
			cacheListEnd = cacheListStart = newHandle;
		} else {
			newHandle->prev = NULL;
			newHandle->next = cacheListStart;
			cacheListStart->prev = newHandle;
			cacheListStart = newHandle;
		}
		cacheListLen++;
		checkCacheListLen();
		SignalSema(cacheListSema);
	} else
		delete file;
    return 0;
}

Ps2File *findInCache(int64 id) {
	if ((id <= 0) || !wantHandleCaching)
		return NULL;
	WaitSema(cacheListSema);
	FioHandleCache *node = cacheListStart;
	while (node) {
		if (node->file->_cacheId == id) {
			if (node == cacheListStart)
				cacheListStart = node->next;
			if (node == cacheListEnd)
				cacheListEnd = node->prev;
			if (node->prev)
				node->prev->next = node->next;
			if (node->next)
				node->next->prev = node->prev;
			Ps2File *ret = node->file;
			delete node;
			cacheListLen--;
			SignalSema(cacheListSema);
			return ret;
		} else
			node = node->next;
	}
	SignalSema(cacheListSema);
	return NULL;
}

int ps2_fseek(FILE *stream, long offset, int origin) {
	return ((Ps2File*)stream)->seek(offset, origin);
}

long ps2_ftell(FILE *stream) {
	return ((Ps2File*)stream)->tell();
}

long ps2_fsize(FILE *stream) {
	return ((Ps2File*)stream)->size();
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

char *ps2_fgets(char *buf, int n, FILE *stream) {
	char *retVal = buf;
	while (n--) {
		if (n == 0)
			*buf = '\0';
		else {
			char c = ps2_fgetc(stream);
			if (c == EOF)
				return NULL;
			if ((c == '\r') || (c == '\n')) {
				*buf++ = '\0';
                return retVal;
			}
			*buf++ = c;
		}
	}
	return retVal;
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
	if (ps2_fwrite(s, 1, len, stream) == len)
		return len;
	else
		return EOF;
}

int ps2_fflush(FILE *stream) {
	printf("fflush not implemented\n");
	return 0;
}

TocManager::TocManager(void) {
	_rootNode = NULL;
}

TocManager::~TocManager(void) {
	// todo: write this...
}

bool TocManager::haveEntries(void) {
	return _rootNode != NULL;
}

void TocManager::readEntries(const char *root) {
    _rootLen = strlen(root);
	strcpy(_root, root);
	if (_root[_rootLen - 1] == '/') {
		_rootLen--;
		_root[_rootLen] = '\0';
	}
	readDir(_root, &_rootNode, 0);
}

#define MAX_DIR_ENTRIES 512

void TocManager::readDir(const char *path, TocNode **node, int level) {
	if (level <= 2) { // we don't scan deeper than that
		struct TocEntry tocEntries[MAX_DIR_ENTRIES];
		int files = CDVD_GetDir(path + 5, NULL, CDVD_GET_FILES_AND_DIRS, tocEntries, MAX_DIR_ENTRIES, NULL);

		for (int cnt = 0; cnt < files; cnt++) {
			if (tocEntries[cnt].filename[0] != '.') { // skip '.' and '..'
				*node = new TocNode;
				(*node)->sub = (*node)->next = NULL;

				(*node)->nameLen = strlen(tocEntries[cnt].filename);
				memcpy((*node)->name, tocEntries[cnt].filename, (*node)->nameLen + 1);

				if (tocEntries[cnt].fileProperties & 2) { // directory
					(*node)->isDir = true;
					char nextPath[256];
					sprintf(nextPath, "%s/%s", path, tocEntries[cnt].filename);
					readDir(nextPath, &((*node)->sub), level + 1);
				} else
					(*node)->isDir = false;
				node = &((*node)->next);
			}
		}
	}
}

int64 TocManager::fileExists(const char *name) {
	const char *tmpName = name;

	if (((name[_rootLen] != '/') && (name[_rootLen] != '\0')) || (strnicmp(name, _root, _rootLen) != 0)) {
		for (int i = 0; i < 8; i++)
			if (name[i] == ':')	// we don't know the content of other drives,
				return -1;		// assume file exists
			else if ((name[i] == '/') || (name[i] == '\\'))
				return 0;		// does not exists (this is probably ScummVM trying the 'current directory'.)
		return 0;
	}

	uint8 nameLen = strlen(name);

	name += _rootLen + 1;
	nameLen -= _rootLen + 1;

	TocNode *node = _rootNode;
	int64 retId = 1;
	while (node) {
		if (((name[node->nameLen] == '/') || (name[node->nameLen] == '\0')) && (strnicmp(name, node->name, node->nameLen) == 0)) {
			name += node->nameLen;
			nameLen -= node->nameLen;
			if (node->isDir) {
				if (nameLen) {
					name++;	// skip '/'
					nameLen--;
					node = node->sub;
					retId <<= 10;
				} else
					return 0; // can't open a directory with fopen()
			} else {
				if (nameLen == 0)
					return retId; // ok, found
				else
					return 0; // here's a file, but there's something left in the path
			}
		} else {
			node = node->next;
			retId++;
		}
	}
	return 0; // does not exist
}

