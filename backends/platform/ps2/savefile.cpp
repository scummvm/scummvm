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

#include <tamtypes.h>
#include <kernel.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <fileio.h>
#include <malloc.h>
#include <ucl/ucl.h>
#include <libmc.h>
#include "backends/platform/ps2/savefile.h"
#include "backends/platform/ps2/rawsavefile.h"
#include "backends/platform/ps2/Gs2dScreen.h"
#include "backends/platform/ps2/systemps2.h"
#include "backends/platform/ps2/savefilemgr.h"
#include "backends/platform/ps2/ps2debug.h"
#include "backends/fs/abstract-fs.h"

#define UCL_MAGIC 0x314C4355

AutoSaveFile::AutoSaveFile(Ps2SaveFileManager *saveMan, const char *filename) {
	strcpy(_fileName, filename);
	_saveMan = saveMan;
	_bufSize = 65536;
	_buf = (uint8*)memalign(64, _bufSize);
	_bufPos = 0;
}

AutoSaveFile::~AutoSaveFile(void) {
	_saveMan->writeSaveNonblocking(_fileName, _buf, _bufPos);
	free(_buf);
}

uint32 AutoSaveFile::write(const void *ptr, uint32 size) {
	uint32 bytesFree = _bufSize - _bufPos;
	if (bytesFree < size) {
		uint32 allocBytes = (size > 32 * 1024) ? size : 32 * 1024;
		_bufSize += allocBytes;
		_buf = (uint8*)realloc(_buf, _bufSize);
		bytesFree = _bufSize - _bufPos;
	}
	memcpy(_buf + _bufPos, ptr, size);
	_bufPos += size;
	return size;
}


UclInSaveFile::UclInSaveFile(const char *filename, Gs2dScreen *screen, McAccess *mcAccess) : RawReadFile(mcAccess) {
	_screen = screen;

	_ioFailed = true;

	if (bufOpen(filename)) {
		if ((_size > 8) && (*(uint32 *)_buf == UCL_MAGIC)) {
			uint32 resSize = *(uint32 *)(_buf + 4);
			uint8 *decBuf = (uint8 *)malloc(resSize + 2048);
			int res = ucl_nrv2e_decompress_8(_buf + 8, _size - 8, decBuf, &resSize, NULL);
			if ((res >= 0) && (resSize == *(uint32 *)(_buf + 4))) {
				free(_buf);
				_buf = decBuf;
				_size = resSize;
				_ioFailed = false;
				_pos = 0;
			} else
				free(decBuf);
		}
	}
	if (_ioFailed) {
		if (_buf)
			free(_buf);
		_buf = NULL;
		_size = -1;
	}
}

UclInSaveFile::~UclInSaveFile(void) {
	_screen->wantAnim(false);
}

bool UclInSaveFile::ioFailed(void) const {
	return _ioFailed;
}

void UclInSaveFile::clearIOFailed(void) {
	_ioFailed = false;
}

bool UclInSaveFile::eos(void) const {
	return bufTell() == bufSize();
}

int32 UclInSaveFile::pos(void) const {
	return bufTell();
}

int32 UclInSaveFile::size(void) const {
	return bufSize();
}

bool UclInSaveFile::seek(int pos, int whence) {
	bufSeek(pos, whence);
	return true;
}

uint32 UclInSaveFile::read(void *ptr, uint32 size) {
	return (uint32)bufRead(ptr, (int)size);
}

bool UclInSaveFile::skip(uint32 offset) {
	bufSeek(offset, SEEK_CUR);
	return true;s
}

UclOutSaveFile::UclOutSaveFile(const char *filename, OSystem_PS2 *system, Gs2dScreen *screen, McAccess *mc) : RawWriteFile(mc) {
	_screen = screen;
	_system = system;
	strcpy(_fileName, filename);

	_ioFailed = !bufOpen(filename);

	_wasFlushed = false;
}

UclOutSaveFile::~UclOutSaveFile(void) {
	if (_pos != 0) {
		printf("Engine didn't call SaveFile::flush()\n");
		flush();
		if (ioFailed()) {
			// unable to save to memory card and it's too late to return an error code to the engine
			_system->msgPrintf(5000, "!WARNING!\nCan't write to memory card.\nGame was NOT saved.");
			printf("~UclOutSaveFile: Flush failed!\n");
		}
	}
	_screen->wantAnim(false);
}

uint32 UclOutSaveFile::write(const void *ptr, uint32 size) {
	bufWrite(ptr, (int)size);
	return size;
}

bool UclOutSaveFile::ioFailed(void) const {
	return _ioFailed;
}

void UclOutSaveFile::clearIOFailed(void) {
	_ioFailed = false;
}

bool UclOutSaveFile::flush(void) {
	if (_pos != 0) {
		if (_wasFlushed) {
			printf("Multiple calls to UclOutSaveFile::flush!\n");
			_ioFailed = true;
			return false;
		}
		uint32 compSize = _pos * 2;
		uint8 *compBuf = (uint8*)memalign(64, compSize + 8);
		*(uint32*)(compBuf + 0) = UCL_MAGIC;
		*(uint32*)(compBuf + 4) = _pos; // uncompressed size
		int res = ucl_nrv2e_99_compress(_buf, _pos, compBuf + 8, &compSize, NULL, 10, NULL, NULL);
		assert(res >= 0);

		free(_buf);
		_buf = compBuf;
		_size = _pos * 2;
		_pos = compSize + 8;
		if (!bufFlush()) {
			printf("UclOutSaveFile::flush failed!\n");
			_ioFailed = true;
			removeFile();
		}
		_wasFlushed = true;
	}
	return true;
}

/* ----------------------------------------- Glue Classes for POSIX Memory Card Access ----------------------------------------- */

Ps2McReadFile::Ps2McReadFile(Ps2SaveFileManager *saveMan) : RawReadFile(saveMan->getMcAccess()), Ps2File(-1) {
}

Ps2McReadFile::~Ps2McReadFile(void) {
}

bool Ps2McReadFile::open(const char *name) {
	return bufOpen(name);
}

uint32 Ps2McReadFile::read(void *dest, uint32 len) {
	return (uint32)bufRead(dest, (int)len);
}

uint32 Ps2McReadFile::write(const void *src, uint32 len) {
	printf("Write access on Ps2McReadFile!\n");
	return 0;
}

int32 Ps2McReadFile::tell(void) {
	return bufTell();
}

int32 Ps2McReadFile::size(void) {
	return bufSize();
}

int Ps2McReadFile::seek(int32 offset, int origin) {
	return bufSeek(offset, origin);
}

bool Ps2McReadFile::eof(void) {
	return bufTell() == bufSize();
}


Ps2McWriteFile::Ps2McWriteFile(Ps2SaveFileManager *saveMan) : RawWriteFile(saveMan->getMcAccess()), Ps2File(-1) {
}

Ps2McWriteFile::~Ps2McWriteFile() {
}

bool Ps2McWriteFile::open(const char *name) {
	return bufOpen(name);
}

uint32 Ps2McWriteFile::read(void *dest, uint32 len) {
	printf("Read request on Ps2McWriteFile!\n");
	return 0;
}

uint32 Ps2McWriteFile::write(const void *src, uint32 len) {
	bufWrite(src, (int)len);
	return len;
}

int32 Ps2McWriteFile::tell(void) {
	return bufTell();
}

int32 Ps2McWriteFile::size(void) {
	return bufTell();
}

int Ps2McWriteFile::seek(int32 offset, int origin) {
	printf("SEEK Request on Ps2McWriteFile!\n");
	SleepThread();
	return 0;
}

bool Ps2McWriteFile::eof(void) {
	return true;
}
