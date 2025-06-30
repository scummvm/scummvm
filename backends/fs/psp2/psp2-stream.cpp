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
#ifdef PSP2

#include "backends/fs/psp2/psp2-stream.h"
#include "common/debug.h"

#include <psp2common/kernel/iofilemgr.h>
#include <psp2/io/stat.h>
#include <psp2/io/fcntl.h> 

#define MIN2(a,b) ((a < b) ? a : b)
#define MIN3(a,b,c) ( (a < b) ? (a < c ? a : c) : (b < c ? b : c) )


// Class Psp2IoStream ------------------------------------------------

Psp2IoStream::Psp2IoStream(const Common::String &path, bool writeMode)
		: _handle(0), _path(path), _fileSize(0), _writeMode(writeMode),
		  _physicalPos(0), _pos(0), _eos(false),	_error(false),
		  _errorSuspend(0), _errorSource(0), _errorPos(0), _errorHandle(0), _suspendCount(0) {

	//assert(!path.empty());	// do we need this?
}

Psp2IoStream::~Psp2IoStream() {
	if (PowerMan.beginCriticalSection())
	    debug(8, "suspended");

	PowerMan.unregisterForSuspend(this); 			// Unregister with powermanager to be suspended
													// Must do this before fclose() or resume() will reopen.
	sceIoClose(_handle);

	PowerMan.endCriticalSection();
}

/* Function to open the file pointed to by the path.
 *
 */
SceUID Psp2IoStream::open() {
	if (PowerMan.beginCriticalSection()) {
		// No need to open? Just return the _handle resume() already opened
		debug(8, "suspended");
	}

	_handle = sceIoOpen(_path.c_str(), _writeMode ? SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC : SCE_O_RDONLY, 0777);
	if (_handle <= 0) {
		_error = true;
		_handle = 0;
	}

	// Get the file size. This way is much faster than going to the end of the file and back
	SceIoStat stat;
	sceIoGetstat(_path.c_str(), &stat);
	_fileSize = stat.st_size;	// 4GB file (32 bits) is big enough for us

	debug(8, "%s filesize[%d]", _path.c_str(), _fileSize);

	PowerMan.registerForSuspend(this);	 // Register with the powermanager to be suspended

	PowerMan.endCriticalSection();

	return _handle;
}

bool Psp2IoStream::err() const {
	if (_error)	// We dump since no printing to screen with suspend callback
		debug(8, "mem_error[%d], source[%d], suspend error[%d], pos[%d],"
				  "_errorPos[%d], _errorHandle[%d], suspendCount[%d]",
		          _error, _errorSource, _errorSuspend, _pos,
				  _errorPos, _errorHandle, _suspendCount);

	return _error;
}

void Psp2IoStream::clearErr() {
	_error = false;
}

bool Psp2IoStream::eos() const {
	return _eos;
}

int64 Psp2IoStream::pos() const {
	return _pos;
}

int64 Psp2IoStream::size() const {
	return _fileSize;
}

bool Psp2IoStream::physicalSeekFromCur(int32 offset) {

	int ret = sceIoLseek32(_handle, offset, SCE_SEEK_CUR);

	if (ret < 0) {
		_error = true;
		debug(8, "failed to seek in file[%s] to [%x]. Error[%x]", _path.c_str(), offset, ret);
		return false;
	}
	_physicalPos += offset;
	return true;
}

bool Psp2IoStream::seek(int64 offs, int whence) {
	debug(8, "offset[0x%llx], whence[%d], _pos[0x%x], _physPos[0x%x]", offs, whence, _pos, _physicalPos);
	_eos = false;

	int32 posToSearchFor = 0;
	switch (whence) {
	case SEEK_CUR:
		posToSearchFor = _pos;
		break;
	case SEEK_END:
		posToSearchFor = _fileSize;
		break;
	}
	posToSearchFor += offs;

	// Check for bad values
	if (posToSearchFor < 0) {
		_error = true;
		return false;
	} else if (posToSearchFor > _fileSize) {
		_error = true;
		_eos = true;
		return false;
	}

	_pos = posToSearchFor;

	return true;
}

uint32 Psp2IoStream::read(void *ptr, uint32 len) {
	debug(8, "filename[%s], len[0x%x], ptr[%p], _pos[%x], _physPos[%x]", _path.c_str(), len, ptr, _pos, _physicalPos);

	if (_error || _eos || len <= 0)
		return 0;

	uint32 lenRemainingInFile = _fileSize - _pos;

	// check for getting EOS
	if (len > lenRemainingInFile) {
		len = lenRemainingInFile;
		_eos = true;
	}

	if (PowerMan.beginCriticalSection())
	    debug(8, "suspended");

	// check if we need to seek
	if (_pos != _physicalPos) {
		debug(8, "seeking from %x to %x", _physicalPos, _pos);
		if (!physicalSeekFromCur(_pos - _physicalPos)) {
			_error = true;
			return 0;
		}
	}

	int ret = sceIoRead(_handle, ptr, len);

	PowerMan.endCriticalSection();

	_physicalPos += ret;	// Update position
	_pos = _physicalPos;

	if (ret != (int)len) {	// error
		debug(8, "sceIoRead returned [0x%x] instead of len[0x%x]", ret, len);
		_error = true;
		_errorSource = 4;
	}
	return ret;
}

uint32 Psp2IoStream::write(const void *ptr, uint32 len) {
	debug(8, "filename[%s], len[0x%x], ptr[%p], _pos[%x], _physPos[%x]", _path.c_str(), len, ptr, _pos, _physicalPos);

	if (!len || _error)		// we actually get some calls with len == 0!
		return 0;

	_eos = false;			// we can't have eos with write

	if (PowerMan.beginCriticalSection())
	    debug(8, "suspended");

	// check if we need to seek
	if (_pos != _physicalPos)
		if (!physicalSeekFromCur(_pos - _physicalPos)) {
			_error = true;
			return 0;
		}

	int ret = sceIoWrite(_handle, ptr, len);

	PowerMan.endCriticalSection();

	if (ret != (int)len) {
		_error = true;
		_errorSource = 5;
		debug(8, "sceIoWrite returned[0x%x] instead of len[0x%x]", ret, len);
	}

	_physicalPos += ret;
	_pos = _physicalPos;

	if (_pos > _fileSize)
		_fileSize = _pos;

	return ret;
}

bool Psp2IoStream::flush() {
	return true;
}

// For the PSP2, since we're building in suspend support, we moved opening
// the actual file to an open function since we need an actual Psp2IoStream object to suspend.
//
Psp2IoStream *Psp2IoStream::makeFromPath(const Common::String &path, bool writeMode) {
	Psp2IoStream *stream = new Psp2IoStream(path, writeMode);

	if (stream->open() <= 0) {
		delete stream;
		stream = 0;
	}

	return stream;
}

/*
 *  Function to suspend the IO stream
 *  we can have no output here
 */
int Psp2IoStream::suspend() {
	_suspendCount++;

	if (_handle > 0 && _pos < 0) {	/* check for error */
		_errorSuspend = SuspendError;
		_errorPos = _pos;
		_errorHandle = _handle;
	}

	if (_handle > 0) {
		sceIoClose(_handle);		// close our file descriptor
		_handle = 0xFFFFFFFF;		// Set handle to non-null invalid value so makeFromPath doesn't return error
	}

	return 0;
}

/*
 *  Function to resume the IO stream (called by Power Manager)
 */
int Psp2IoStream::resume() {
	int ret = 0;
	_suspendCount--;

	// We reopen our file descriptor
	_handle = sceIoOpen(_path.c_str(), _writeMode ? SCE_O_RDWR | SCE_O_CREAT : SCE_O_RDONLY, 0777); 	// open
	if (_handle <= 0) {
		_errorSuspend = ResumeError;
		_errorPos = _pos;
	}

	// Resume our previous position if needed
	if (_handle > 0 && _pos > 0) {
		ret = sceIoLseek32(_handle, _pos, SCE_SEEK_SET);

		_physicalPos = _pos;

		if (ret < 0) {		// Check for problem
			_errorSuspend = ResumeError;
			_errorPos = _pos;
			_errorHandle = _handle;
		}
	}
	return ret;
}

#endif /* PSP2 */
