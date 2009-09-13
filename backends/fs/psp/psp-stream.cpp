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

#include "backends/fs/psp/psp-stream.h"
#include "backends/platform/psp/trace.h"
#include <errno.h>

PSPIoStream::PSPIoStream(const Common::String &path, bool writeMode)
: StdioStream((void *)1), _path(path), _writeMode(writeMode) {

	assert(!path.empty());

	_handle = (void *)0;		// Need to do this since base class asserts not 0.

	PowerMan.registerSuspend(this);	 // Register with the powermanager to be suspended

}

PSPIoStream::~PSPIoStream() {
	PowerMan.unregisterSuspend(this); // Unregister with powermanager to be suspended
								 // Must do this before fclose() or resume() will reopen.

	fclose((FILE *)_handle);
}

// Function to open the file pointed to by the path.
//
//
void * PSPIoStream::open() {
	if (PowerMan.beginCriticalSection()==PowerManager::Blocked) {
		// No need to open. Just return the _handle resume() already opened.
		PSPDebugTrace("Suspended in PSPIoStream::open\n");
	} else {
		_handle = fopen(_path.c_str(), _writeMode ? "wb" : "rb"); 	// open
	}
	
	PowerMan.endCriticalSection();

	return _handle;
}

bool PSPIoStream::err() const {
	bool ret;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::err()\n");

	ret = ferror((FILE *)_handle) != 0;

	PowerMan.endCriticalSection();

	return ret;
}

void PSPIoStream::clearErr() {
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::clearErr()\n");

	clearerr((FILE *)_handle);

	PowerMan.endCriticalSection();
}

bool PSPIoStream::eos() const {
	bool ret;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::eos()\n");

	ret = feof((FILE *)_handle) != 0;

	PowerMan.endCriticalSection();

	return ret;
}

int32 PSPIoStream::pos() const {
	int32 ret;

	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::pos()\n");

	ret = ftell((FILE *)_handle);

	PowerMan.endCriticalSection();

	return ret;
}


int32 PSPIoStream::size() const {
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::size()\n");

	int32 oldPos = ftell((FILE *)_handle);
	fseek((FILE *)_handle, 0, SEEK_END);
	int32 length = ftell((FILE *)_handle);
	fseek((FILE *)_handle, oldPos, SEEK_SET);

	PowerMan.endCriticalSection();

	return length;
}

bool PSPIoStream::seek(int32 offs, int whence) {
	int ret = 0;

	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::seek()\n");

	ret = fseek((FILE *)_handle, offs, whence);

	PowerMan.endCriticalSection();

	return ret == 0;
}

uint32 PSPIoStream::read(void *ptr, uint32 len) {
	int ret = 0;

	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::read()\n");

	ret = fread((byte *)ptr, 1, len, (FILE *)_handle);
	
	PowerMan.endCriticalSection();
	
	return ret;
}

uint32 PSPIoStream::write(const void *ptr, uint32 len) {
	int ret = 0;

	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::read()\n");

	ret = fwrite(ptr, 1, len, (FILE *)_handle);

	PowerMan.endCriticalSection();

	return ret;
}

bool PSPIoStream::flush() {
	int ret = 0;

	// Check if we can access the file
	if (PowerMan.beginCriticalSection() == PowerManager::Blocked)
		PSPDebugTrace("Suspended in PSPIoStream::read()\n");

	ret = fflush((FILE *)_handle);

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
	if (_handle > 0) {
		_pos = ftell((FILE *)_handle);	// Save our position
		fclose((FILE *)_handle);		// close our file descriptor
		_handle = 0;					// Set handle to null
	}

	return 0;
}

/*
 *  Function to resume the IO stream (called by Power Manager)
 */
int PSPIoStream::resume() {
	int ret = 0;

	// We reopen our file descriptor
	_handle = fopen(_path.c_str(), _writeMode ? "wb" : "rb");
	if (_handle <= 0) {
		PSPDebugTrace("PSPIoStream::resume(): Couldn't reopen file %s\n", _path.c_str());
		ret = -1;
	}

	// Resume our previous position
	if(_handle > 0) fseek((FILE *)_handle, _pos, SEEK_SET);

	return ret;
}

#endif /* __PSP__ */
