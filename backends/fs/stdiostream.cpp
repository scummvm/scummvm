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

#if !defined(DISABLE_STDIO_FILESTREAM)

// Disable symbol overrides so that we can use FILE, fopen etc.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

// for Windows unicode fopen(): _wfopen()
#if defined(WIN32) && defined(UNICODE)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "backends/platform/sdl/win32/win32_wrapper.h"
#endif

// Include this after windows.h so we don't get a warning for redefining ARRAYSIZE
#include "backends/fs/stdiostream.h"

StdioStream::StdioStream(void *handle) : _handle(handle) {
	assert(handle);
}

StdioStream::~StdioStream() {
	fclose((FILE *)_handle);
}

bool StdioStream::err() const {
	return ferror((FILE *)_handle) != 0;
}

void StdioStream::clearErr() {
	clearerr((FILE *)_handle);
}

bool StdioStream::eos() const {
	return feof((FILE *)_handle) != 0;
}

int64 StdioStream::pos() const {
#if defined(WIN32)
	return _ftelli64((FILE *)_handle);
#elif defined(HAS_FSEEKO_OFFT_64)
	return ftello((FILE *)_handle);
#elif defined(HAS_FSEEKO64)
	return ftello64((FILE *)_handle);
#else
	return ftell((FILE *)_handle);
#endif
}

int64 StdioStream::size() const {
#if defined(WIN32)
	int64 oldPos = _ftelli64((FILE *)_handle);
	_fseeki64((FILE *)_handle, 0, SEEK_END);
	int64 length = _ftelli64((FILE *)_handle);
	_fseeki64((FILE *)_handle, oldPos, SEEK_SET);
#elif defined(HAS_FSEEKO_OFFT_64)
	int64 oldPos = ftello((FILE *)_handle);
	fseeko((FILE *)_handle, 0, SEEK_END);
	int64 length = ftello((FILE *)_handle);
	fseeko((FILE *)_handle, oldPos, SEEK_SET);
#elif defined(HAS_FSEEKO64)
	int64 oldPos = ftello64((FILE *)_handle);
	fseeko64((FILE *)_handle, 0, SEEK_END);
	int64 length = ftello64((FILE *)_handle);
	fseeko64((FILE *)_handle, oldPos, SEEK_SET);
#else
	int64 oldPos = ftell((FILE *)_handle);
	fseek((FILE *)_handle, 0, SEEK_END);
	int64 length = ftell((FILE *)_handle);
	fseek((FILE *)_handle, oldPos, SEEK_SET);
#endif

	return length;
}

bool StdioStream::seek(int64 offs, int whence) {
#if defined(WIN32)
	return _fseeki64((FILE *)_handle, offs, whence) == 0;
#elif defined(HAS_FSEEKO_OFFT_64)
	return fseeko((FILE *)_handle, offs, whence) == 0;
#elif defined(HAS_FSEEKO64)
	return fseeko64((FILE *)_handle, offs, whence) == 0;
#else
	return fseek((FILE *)_handle, offs, whence) == 0;
#endif
}

uint32 StdioStream::read(void *ptr, uint32 len) {
	return fread((byte *)ptr, 1, len, (FILE *)_handle);
}

bool StdioStream::setBufferSize(uint32 bufferSize) {
	if (bufferSize != 0) {
		return setvbuf((FILE *)_handle, nullptr, _IOFBF, bufferSize) == 0;
	} else {
		return setvbuf((FILE *)_handle, nullptr, _IONBF, 0) == 0;
	}
}

uint32 StdioStream::write(const void *ptr, uint32 len) {
	return fwrite(ptr, 1, len, (FILE *)_handle);
}

bool StdioStream::flush() {
	return fflush((FILE *)_handle) == 0;
}

StdioStream *StdioStream::makeFromPathHelper(const Common::String &path, bool writeMode,
		StdioStream *(*factory)(void *handle)) {
#if defined(WIN32) && defined(UNICODE)
	wchar_t *wPath = Win32::stringToTchar(path);
	FILE *handle = _wfopen(wPath, writeMode ? L"wb" : L"rb");
	free(wPath);
#elif defined(HAS_FOPEN64)
	FILE *handle = fopen64(path.c_str(), writeMode ? "wb" : "rb");
#else
	FILE *handle = fopen(path.c_str(), writeMode ? "wb" : "rb");
#endif

	if (handle)
		return factory(handle);
	return nullptr;
}

#endif
