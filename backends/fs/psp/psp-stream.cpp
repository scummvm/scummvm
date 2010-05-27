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

#define MIN2(a,b) ((a < b) ? a : b)
#define MIN3(a,b,c) ( (a < b) ? (a < c ? a : c) : (b < c ? b : c) )

//#define __PSP_PRINT_TO_FILE__ /* For debugging suspend stuff, we have no screen output */
//#define __PSP_DEBUG_FUNCS__ 	/* For debugging function calls */
//#define __PSP_DEBUG_PRINT__	/* For debug printouts */

#include "backends/platform/psp/trace.h"

//#define DEBUG_BUFFERS					/* to see the contents of the buffers being read */

#ifdef DEBUG_BUFFERS
void printBuffer(byte *ptr, uint32 len) {
	uint32 printLen = len <= 10 ? len : 10;
	
	for (int i = 0; i < printLen; i++) {
		PSP_INFO_PRINT("%x ", ptr[i]);		
	}
	
	if (len > 10) {
		PSP_INFO_PRINT("... ");
		for (int i = len - 10; i < len; i++)
			PSP_INFO_PRINT("%x ", ptr[i]);
	}
	
	PSP_INFO_PRINT("\n");
}
#endif


PSPIoStream::PSPIoStream(const Common::String &path, bool writeMode)
		: StdioStream((void *)1), _path(path), _writeMode(writeMode),
		  _ferror(false), _pos(0),
		  _physicalPos(0), _fileSize(0), _inCache(false), _eos(false),
		  _cacheStartOffset(-1), _cache(0),
		  _errorSuspend(0), _errorSource(0),
		  _errorPos(0), _errorHandle(0), _suspendCount(0) {
	DEBUG_ENTER_FUNC();

	// assert(!path.empty());	// do we need this?

	_handle = (void *)0;		// Need to do this since base class asserts not 0.
}

PSPIoStream::~PSPIoStream() {
	DEBUG_ENTER_FUNC();

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");

	PowerMan.unregisterSuspend(this); // Unregister with powermanager to be suspended
									  // Must do this before fclose() or resume() will reopen.

	fclose((FILE *)_handle);		  // We don't need a critical section. Worst case, the handle gets closed on its own
	
	if (_cache)
		free(_cache);

	PowerMan.endCriticalSection();
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

	if (_handle) {
		// Get the file size
		fseek((FILE *)_handle, 0, SEEK_END);	// go to the end
		_fileSize = ftell((FILE *)_handle);
		fseek((FILE *)_handle, 0, SEEK_SET);	// back to the beginning
	
		// Allocate the cache
		_cache = (char *)memalign(64, CACHE_SIZE);
	}

	PowerMan.registerSuspend(this);	 // Register with the powermanager to be suspended

	PowerMan.endCriticalSection();

	return _handle;
}

bool PSPIoStream::err() const {
	DEBUG_ENTER_FUNC();
	
	if (_ferror)	// We dump since no printing to screen with suspend
		PSP_ERROR("mem_ferror[%d], source[%d], suspend error[%d], pos[%d], \
		_errorPos[%d], _errorHandle[%p], suspendCount[%d]\n",
		          _ferror, _errorSource, _errorSuspend, _pos,
				  _errorPos, _errorHandle, _suspendCount);

	return _ferror;
}

void PSPIoStream::clearErr() {
	_ferror = false;
}

bool PSPIoStream::eos() const {
	return _eos;
}

int32 PSPIoStream::pos() const {
	return _pos;
}

int32 PSPIoStream::size() const {
	return _fileSize;
}

bool PSPIoStream::seek(int32 offs, int whence) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT_FUNC("offset[0x%x], whence[%d], _pos[0x%x], _physPos[0x%x]\n", offs, whence, _pos, _physicalPos);
	_eos = false;
	
	int32 posToSearchFor = 0;
	switch (whence) {
	case SEEK_CUR:
		posToSearchFor = _pos;
		break;
	case SEEK_END:
		posToSearchFor = _fileSize;	// unsure. Does it take us here or to EOS - 1?
		break;
	}
	posToSearchFor += offs;
	
	// Check for bad values
	if (posToSearchFor < 0) {
		_ferror = true;
		return false;
	}
	
	if (posToSearchFor > _fileSize) {
		_ferror = true;
		_eos = true;
		return false;
	}
	
	// See if we can find it in cache
	if (isOffsetInCache(posToSearchFor)) {
		PSP_DEBUG_PRINT("seek offset[0x%x] found in cache. Cache starts[0x%x]\n", posToSearchFor, _cacheStartOffset);
		_inCache = true;		
	} else {	// not in cache
		_inCache = false;		
	}	
	_pos = posToSearchFor;		
	return true;
}

uint32 PSPIoStream::read(void *ptr, uint32 len) {
	DEBUG_ENTER_FUNC();
	PSP_DEBUG_PRINT_FUNC("filename[%s], len[0x%x], ptr[%p]\n", _path.c_str(), len, ptr);

	if (_ferror || _eos)
		return 0;
		
	byte *destPtr = (byte *)ptr;
	uint32 lenFromFile = len;		// how much we read from the actual file
	uint32 lenFromCache = 0;		// how much we read from cache
	uint32 lenRemainingInFile = _fileSize - _pos;
	
	if (lenFromFile > lenRemainingInFile) {
		lenFromFile = lenRemainingInFile;
		_eos = true;
	}	
	
	// Are we in cache?
	if (_inCache && isCacheValid()) {
		uint32 offsetInCache = _pos - _cacheStartOffset;
		// We can read at most what's in the cache or the remaining size of the file
		lenFromCache = MIN2(lenFromFile, CACHE_SIZE - offsetInCache); // unsure
		
		PSP_DEBUG_PRINT("reading 0x%x bytes from cache to %p. pos[0x%x] physPos[0x%x] cacheStart[0x%x]\n", lenFromCache, destPtr, _pos, _physicalPos, _cacheStartOffset);
		
		memcpy(destPtr, &_cache[offsetInCache], lenFromCache);
		_pos += lenFromCache;		
		
		if (lenFromCache < lenFromFile) {	// there's more to copy from the file
			lenFromFile -= lenFromCache;
			lenRemainingInFile -= lenFromCache;	// since we moved pos
			destPtr += lenFromCache;
		} else {							// we're done
#ifdef DEBUG_BUFFERS
			printBuffer((byte *)ptr, len);
#endif			
			
			return lenFromCache;			// how much we actually read
		}		
	}

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");
	
	
	synchronizePhysicalPos();	// we need to update our physical position
	
	if (lenFromFile <= MIN_READ_SIZE) {	// We load the cache in case the read is small enough
		// This optimization is based on the principle that reading 1 byte is as expensive as 1000 bytes
		uint32 lenToCopyToCache = MIN2((uint32)MIN_READ_SIZE, lenRemainingInFile); // at most remaining file size
		
		PSP_DEBUG_PRINT("filling cache with 0x%x bytes from physicalPos[0x%x]. cacheStart[0x%x], pos[0x%x], fileSize[0x%x]\n", lenToCopyToCache, _physicalPos, _cacheStartOffset, _pos, _fileSize);
		
		size_t ret = fread(_cache, 1, lenToCopyToCache, (FILE *)_handle);
		if (ret != lenToCopyToCache) {
			PSP_ERROR("in filling cache, failed to get 0x%x bytes. Only got 0x%x\n", lenToCopyToCache, ret);
			_ferror = true;
			clearerr((FILE *)_handle);
		}
		_cacheStartOffset = _physicalPos;
		_inCache = true;
		
		_physicalPos += ret;
		
		PSP_DEBUG_PRINT("copying 0x%x bytes from cache to %p\n", lenFromFile, destPtr);
		
		// Copy to the destination buffer from cache
		memcpy(destPtr, _cache, lenFromFile);
		_pos += lenFromFile;
		
	} else {	// Too big for cache. No caching
		PSP_DEBUG_PRINT("reading 0x%x bytes from file to %p. Pos[0x%x], physPos[0x%x]\n", lenFromFile, destPtr, _pos, _physicalPos);
		size_t ret = fread(destPtr, 1, lenFromFile, (FILE *)_handle);

		_physicalPos += ret;	// Update pos
		_pos = _physicalPos;

		if (ret != lenFromFile) {	// error
			PSP_ERROR("fread returned [0x%x] instead of len[0x%x]\n", ret, lenFromFile);
			_ferror = true;
			clearerr((FILE *)_handle);
			_errorSource = 4;			
		}
		_inCache = false;
	}

	PowerMan.endCriticalSection();

#ifdef DEBUG_BUFFERS
	printBuffer((byte *)ptr, len);
#endif	
		
	return lenFromCache + lenFromFile;		// total of what was copied
}

// TODO: Test if seeking backwards/forwards has any effect on performance
inline bool PSPIoStream::synchronizePhysicalPos() {
	if (_pos != _physicalPos) {
		if (fseek((FILE *)_handle, _pos - _physicalPos, SEEK_CUR) != 0)
			return false;
		_physicalPos = _pos;	
	}
	
	return true;
}

inline bool PSPIoStream::isOffsetInCache(uint32 offset) {
	if (_cacheStartOffset != -1 && 
		offset >= (uint32)_cacheStartOffset && 
		offset < (uint32)(_cacheStartOffset + CACHE_SIZE))
		return true;
	return false;
}

uint32 PSPIoStream::write(const void *ptr, uint32 len) {
	DEBUG_ENTER_FUNC();
	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSP_DEBUG_PRINT_FUNC("Suspended\n");

	PSP_DEBUG_PRINT_FUNC("filename[%s], len[0x%x]\n", _path.c_str(), len);

	if (_ferror)
		return 0;
		
	_eos = false;	// we can't have eos with write
	synchronizePhysicalPos();
	
	size_t ret = fwrite(ptr, 1, len, (FILE *)_handle);

	// If we're making the file bigger, adjust the size
	if (_physicalPos + (int)ret > _fileSize)
		_fileSize = _physicalPos + ret;
	_physicalPos += ret;
	_pos = _physicalPos;
	_inCache = false;
	_cacheStartOffset = -1;	// invalidate cache

	if (ret != len) {	// Set error
		_ferror = true;
		clearerr((FILE *)_handle);
		_pos = ftell((FILE *)_handle);	// Update pos
		_errorSource = 5;
		PSP_ERROR("fwrite returned[0x%x] instead of len[0x%x]\n", ret, len);
	}

	PowerMan.endCriticalSection();

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
		PSP_ERROR("fflush returned ret[%d]\n", ret);
	}

	PowerMan.endCriticalSection();

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
		
		_physicalPos = _pos;
		_inCache = false;

		if (ret != 0) {		// Check for problem
			_errorSuspend = ResumeError;
			_errorPos = _pos;
			_errorHandle = _handle;
		}
	}
	return ret;
}


#endif /* __PSP__ */
