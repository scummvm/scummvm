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
#ifdef __PSP__

#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>

#include "backends/platform/psp/powerman.h"
#include "backends/fs/psp/psp-stream.h"

#include <errno.h>

//#define __PSP_PRINT_TO_FILE__
//#define __PSP_DEBUG_FUNCS__ /* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */
#include "backends/platform/psp/trace.h"

PSPIoStream::PSPIoStream(const Common::String &path, bool writeMode)
		: StdioStream((void *)1), _path(path), _writeMode(writeMode) {
	DEBUG_ENTER_FUNC();

	assert(!path.empty());

	_handle = (void *)0;		// Need to do this since base class asserts not 0.
	_ferror = false;
	_feof = false;
	_pos = 0;

	/* for error checking */
	_errorSuspend = 0;
	_errorSource = 0;
	_errorPos = 0;
	_errorHandle = 0;
	_suspendCount = 0;

	DEBUG_EXIT_FUNC();
}

PSPIoStream::~PSPIoStream() {
	DEBUG_ENTER_FUNC();

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");

	PowerMan.unregisterSuspend(this); // Unregister with powermanager to be suspended
	// Must do this before fclose() or resume() will reopen.

	fclose((FILE *)_handle);	// We don't need a critical section(?). Worst case, the handle gets closed on its own

	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
}

/* Function to open the file pointed to by the path.
 *
 */
void *PSPIoStream::open() {
	DEBUG_ENTER_FUNC();
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked) {
		// No need to open. Just return the _handle resume() already opened.
		PSP_DEBUG_PRINT_FUNC("Suspended\n");
	}

	_handle = fopen(_path.c_str(), _writeMode ? "wb" : "rb"); 	// open

	PowerMan.registerSuspend(this);	 // Register with the powermanager to be suspended

	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return _handle;
}

bool PSPIoStream::err() const {
	DEBUG_ENTER_FUNC();
	if (_ferror)
		PSP_ERROR("mem_ferror[%d], source[%d], suspend error[%d], pos[%d], _errorPos[%d], _errorHandle[%p], suspendCount[%d]\n",
		          _ferror, _errorSource, _errorSuspend, _pos, _errorPos, _errorHandle, _suspendCount);

	DEBUG_EXIT_FUNC();
	return _ferror;
}

void PSPIoStream::clearErr() {
	_ferror = false;	// Remove regular error bit
}

bool PSPIoStream::eos() const {
	return _feof;
}

int32 PSPIoStream::pos() const {
	return _pos;
}


int32 PSPIoStream::size() const {
	DEBUG_ENTER_FUNC();
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");

	fseek((FILE *)_handle, 0, SEEK_END);
	int32 length = ftell((FILE *)_handle);
	fseek((FILE *)_handle, _pos, SEEK_SET);

	if (_pos < 0 || length < 0) {	// Check for errors
		_errorSource = 2;
		PSP_ERROR("pos[%d] or length[%d] < 0!\n", _pos, length);
		_ferror = true;
		length = -1;				// If our oldPos is bad, we want length to be bad too to signal
		clearerr((FILE *)_handle);
	}

	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return length;
}

bool PSPIoStream::seek(int32 offs, int whence) {
	DEBUG_ENTER_FUNC();

	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");

	int ret = fseek((FILE *)_handle, offs, whence);

	if (ret != 0) {
		_ferror = true;
		PSP_ERROR("fseek returned with [%d], non-zero\n", ret);
		clearerr((FILE *)_handle);
		_feof = feof((FILE *)_handle);
		_errorSource = 3;
	} else {					// everything ok
		_feof = false;		// Reset eof flag since we know it was ok
	}

	_pos = ftell((FILE *)_handle);	// update pos

	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return (ret == 0);
}

uint32 PSPIoStream::read(void *ptr, uint32 len) {
	DEBUG_ENTER_FUNC();
	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");

	PSP_DEBUG_PRINT_FUNC("filename[%s], len[%d]\n", _path.c_str(), len);

	size_t ret = fread((byte *)ptr, 1, len, (FILE *)_handle);

	_pos += ret;	// Update pos

	if (ret != len) {	// Check for eof
		_feof = feof((FILE *)_handle);
		if (!_feof) {	// It wasn't an eof. Must be an error
			_ferror = true;
			clearerr((FILE *)_handle);
			_pos = ftell((FILE *)_handle);	// Update our position
			_errorSource = 4;
			PSP_ERROR("fread returned ret[%d] instead of len[%d]\n", ret, len);
		}
	}

	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return ret;
}

uint32 PSPIoStream::write(const void *ptr, uint32 len) {
	DEBUG_ENTER_FUNC();
	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");

	PSP_DEBUG_PRINT_FUNC("filename[%s], len[%d]\n", _path.c_str(), len);

	size_t ret = fwrite(ptr, 1, len, (FILE *)_handle);

	_pos += ret;

	if (ret != len) {	// Set error
		_ferror = true;
		clearerr((FILE *)_handle);
		_pos = ftell((FILE *)_handle);	// Update pos
		_errorSource = 5;
		PSP_ERROR("fwrite returned[%d] instead of len[%d]\n", ret, len);
	}

	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return ret;
}

bool PSPIoStream::flush() {
	DEBUG_ENTER_FUNC();
	// Enter critical section
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");

	int ret = fflush((FILE *)_handle);

	if (ret != 0) {
		_ferror = true;
		clearerr((FILE *)_handle);
		_errorSource = 6;
		PSP_ERROR("fflush returned ret[%u]\n", ret);
	}

	PowerMan.endCriticalSection();

	DEBUG_EXIT_FUNC();
	return (ret == 0);
}

// For the PSP, since we're building in suspend support, we moved opening
// the actual file to an open function since we need an actual PSPIoStream object to suspend.
//
PSPIoStream *PSPIoStream::makeFromPath(const Common::String &path, bool writeMode) {
	DEBUG_ENTER_FUNC();
	PSPIoStream *stream = new PSPIoStream(path, writeMode);

	if (stream->open() <= 0) {
		delete stream;
		stream = 0;
	}

	DEBUG_EXIT_FUNC();
	return stream;
}

/*
 *  Function to suspend the IO stream (called by PowerManager)
 *  we can have no output here
 */
int PSPIoStream::suspend() {
	DEBUG_ENTER_FUNC();
	_suspendCount++;

	if (_handle > 0 && _pos < 0) {	/* check for error */
		_errorSuspend = SuspendError;
		_errorPos = _pos;
		_errorHandle = _handle;
	}

	if (_handle > 0) {
		fclose((FILE *)_handle);		// close our file descriptor
		_handle = (void *)0xFFFFFFFF;	// Set handle to non-null invalid value so makeFromPath doesn't return error
	}

	DEBUG_EXIT_FUNC();
	return 0;
}

/*
 *  Function to resume the IO stream (called by Power Manager)
 */
int PSPIoStream::resume() {
	DEBUG_ENTER_FUNC();
	int ret = 0;
	_suspendCount--;

	// We reopen our file descriptor
	_handle = fopen(_path.c_str(), _writeMode ? "wb" : "rb");
	if (_handle <= 0) {
		PSP_ERROR("Couldn't reopen file %s\n", _path.c_str());
	}

	// Resume our previous position
	if (_handle > 0 && _pos > 0) {
		ret = fseek((FILE *)_handle, _pos, SEEK_SET);

		if (ret != 0) {		// Check for problem
			_errorSuspend = ResumeError;
			_errorPos = _pos;
			_errorHandle = _handle;
		}
	}
	DEBUG_EXIT_FUNC();
	return ret;
}


#endif /* __PSP__ */
