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

#ifndef BACKENDS_FS_POSIX_POSIXIOSTREAM_H
#define BACKENDS_FS_POSIX_POSIXIOSTREAM_H

#include "backends/fs/stdiostream.h"

/**
 * A file input / output stream using POSIX interfaces
 */
class PosixIoStream final : public StdioStream {
public:
	static StdioStream *makeFromPath(const Common::String &path, bool writeMode) {
		return StdioStream::makeFromPathHelper(path, writeMode, [](void *handle) -> StdioStream * {
			return new PosixIoStream(handle);
		});
	}
	PosixIoStream(void *handle);

	int64 size() const override;
};

#endif
