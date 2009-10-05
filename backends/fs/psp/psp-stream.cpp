/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "backends/platform/psp/trace.h"
#include "backends/platform/psp/powerman.h"
#include "backends/fs/psp/psp-stream.h"

#include <errno.h>

PSPIoStream::PSPIoStream(const Common::String &path, bool writeMode)
: StdioStream((void *)1), _path(path), _writeMode(writeMode) {

	assert(!path.empty());

	_handle = (void *)0;		// Need to do this since base class asserts not 0.
	_ferror = false;			
	_feof = false;
	_pos = 0;

#ifdef __PSP_DEBUG_SUSPEND__		
	_errorSuspend = 0;
	_errorSource = 0;
	_errorPos = 0;
	_errorHandle = 0;
	_suspendCount = 0;
#endif
}

PSPIoStream::~PSPIoStream() {
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugSuspend("Suspended in PSPIoStream::~PSPIoStream()\n");

	PowerMan.unregisterSuspend(this); // Unregister with powermanager to be suspended
								 // Must do this before fclose() or resume() will reopen.

	fclose((FILE *)_handle);	// We don't need a critical section(?). Worst case, the handle gets closed on its own

	PowerMan.endCriticalSection();
}

// Function to open the file pointed to by the path.
//
//
void *PSPIoStream::open() {
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked) {
		// No need to open. Just return the _handle resume() already opened.
		PSPDebugSuspend("Suspended in PSPIoStream::open\n");
	} 

	_handle = fopen(_path.c_str(), _writeMode ? "wb" : "rb"); 	// open

	PowerMan.registerSuspend(this);	 // Register with the powermanager to be suspended

	PowerMan.endCriticalSection();

	return _handle;
}

bool PSPIoStream::err() const {
	if (_ferror)
		PSPDebugSuspend("In PSPIoStream::err - mem_ferror=%d, source=%d, suspend error=%d, pos=%d, _errorPos=%d, _errorHandle=%p, suspendCount=%d _handle\n", 
			_ferror, _errorSource, _errorSuspend, _pos, _errorPos, _errorHandle, _suspendCount);
			
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
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugSuspend("Suspended in PSPIoStream::size()\n");

	fseek((FILE *)_handle, 0, SEEK_END);
	int32 length = ftell((FILE *)_handle);
	fseek((FILE *)_handle, _pos, SEEK_SET);
	
	if (_pos < 0 || length < 0) {	// Check for errors
		PSPDebugSuspend("In PSPIoStream::size(). encountered an error!\n");
		_ferror = true;
		length = -1;				// If our oldPos is bad, we want length to be bad too to signal
		clearerr((FILE *)_handle);

#ifdef __PSP_DEBUG_SUSPEND__		
		_errorSource = 2;
#endif
	}

	PowerMan.endCriticalSection();

	return length;
}

bool PSPIoStream::seek(int32 offs, int whence) {
	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugSuspend("Suspended in PSPIoStream::seek()\n");

	int ret = fseek((FILE *)_handle, offs, whence);
	
	if (ret != 0) {
		_ferror = true;
		PSPDebugSuspend("In PSPIoStream::seek(). encountered an error!\n");
		clearerr((FILE *)_handle);
		_feof = feof((FILE *)_handle);

#ifdef __PSP_DEBUG_SUSPEND__				
		_errorSource = 3;
#endif
	}
	else {					// everything ok
		_feof = false;		// Reset eof flag since we know it was ok
	}
	
	_pos = ftell((FILE *)_handle);	// update pos

	PowerMan.endCriticalSection();

	return ret == 0;
}

uint32 PSPIoStream::read(void *ptr, uint32 len) {
	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugSuspend("Suspended in PSPIoStream::read()\n");

	size_t ret = fread((byte *)ptr, 1, len, (FILE *)_handle);	

	_pos += ret;	// Update pos
	
	if (ret != len) {	// Check for eof
		_feof = feof((FILE *)_handle);
		if (!_feof) {	// It wasn't an eof. Must be an error
			_ferror = true;
			clearerr((FILE *)_handle);
			_pos = ftell((FILE *)_handle);	// Update our position
			PSPDebugSuspend("In PSPIoStream::read(). encountered an error!\n");

#ifdef __PSP_DEBUG_SUSPEND__								
			_errorSource = 4;
#endif
		}
	}
	
	PowerMan.endCriticalSection();
	
	return ret;
}

uint32 PSPIoStream::write(const void *ptr, uint32 len) {
	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugSuspend("Suspended in PSPIoStream::read()\n");

	size_t ret = fwrite(ptr, 1, len, (FILE *)_handle);

	_pos += ret;

	if (ret != len) {	// Set error
		_ferror = true;
		clearerr((FILE *)_handle);
		_pos = ftell((FILE *)_handle);	// Update pos
		PSPDebugTrace("In PSPIoStream::write(). encountered an error!\n");

#ifdef __PSP_DEBUG_SUSPEND__								
		_errorSource = 5;
#endif
	}

	PowerMan.endCriticalSection();

	return ret;
}

bool PSPIoStream::flush() {
	// Enter critical section
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugSuspend("Suspended in PSPIoStream::read()\n");

	int ret = fflush((FILE *)_handle);

	if (ret != 0) {
		_ferror = true;
		clearerr((FILE *)_handle);
		PSPDebugSuspend("In PSPIoStream::flush(). encountered an error!\n");

#ifdef __PSP_DEBUG_SUSPEND__							
		_errorSource = 6;
#endif
	}

	PowerMan.endCriticalSection();

	return ret == 0;
}

// For the PSP, since we're building in suspend support, we moved opening
// the actual file to an open function since we need an actual PSPIoStream object to suspend.
//
PSPIoStream *PSPIoStream::makeFromPath(const Common::String &path, bool writeMode) {
	PSPIoStream *stream = new PSPIoStream(path, writeMode);

	if (stream->open() > 0) {
		return stream;
	} else {
		delete stream;
		return 0;
	}
}

/*
 *  Function to suspend the IO stream (called by PowerManager)
 */
int PSPIoStream::suspend() {
#ifdef __PSP_DEBUG_SUSPEND__
	_suspendCount++;

	if (_handle > 0 && _pos < 0) {
		_errorSuspend = SuspendError;
		_errorPos = _pos;
		_errorHandle = _handle;
	}	
#endif /* __PSP_DEBUG_SUSPEND__ */

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
	int ret = 0;
#ifdef __PSP_DEBUG_SUSPEND__
	_suspendCount--;
#endif
	
	// We reopen our file descriptor
	_handle = fopen(_path.c_str(), _writeMode ? "wb" : "rb");
	if (_handle <= 0) {
		PSPDebugSuspend("PSPIoStream::resume(): Couldn't reopen file %s\n", _path.c_str());
	}

	// Resume our previous position
	if (_handle > 0 && _pos > 0) {
		ret = fseek((FILE *)_handle, _pos, SEEK_SET);

#ifdef __PSP_DEBUG_SUSPEND__
		if (ret != 0) {		// Check for problem
			_errorSuspend = ResumeError;
			_errorPos = _pos;
			_errorHandle = _handle;
		}
#endif

	}

	return ret;
}


#endif /* __PSP__ */
