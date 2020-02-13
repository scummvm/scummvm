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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "backends/fs/posix/posix-iostream.h"

#include <sys/stat.h>

PosixIoStream *PosixIoStream::makeFromPath(const Common::String &path, bool writeMode) {
	FILE *handle = fopen(path.c_str(), writeMode ? "wb" : "rb");

	if (handle)
		return new PosixIoStream(handle);

	return nullptr;
}

PosixIoStream::PosixIoStream(void *handle) :
		StdioStream(handle) {
}

int32 PosixIoStream::size() const {
	int fd = fileno((FILE *)_handle);
	if (fd == -1) {
		return StdioStream::size();
	}

	// Using fstat to obtain the file size is generally faster than fseek / ftell
	// because it does not affect the IO buffer.
	struct stat st;
	if (fstat(fd, &st) == -1) {
		return StdioStream::size();
	}

	return st.st_size;
}
