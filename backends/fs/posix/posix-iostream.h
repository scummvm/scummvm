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

#ifndef BACKENDS_FS_POSIX_POSIXIOSTREAM_H
#define BACKENDS_FS_POSIX_POSIXIOSTREAM_H

#include "backends/fs/stdiostream.h"

/**
 * A file input / output stream using POSIX interfaces
 */
class PosixIoStream : public StdioStream {
public:
#if defined(ANDROID_PLAIN_PORT)
	bool createdWithSAF;
	Common::String hackyfilename;
#endif

	static PosixIoStream *makeFromPath(const Common::String &path, bool writeMode);
	PosixIoStream(void *handle);
#if defined(ANDROID_PLAIN_PORT)
	PosixIoStream(void *handle, bool bCreatedWithSAF, Common::String sHackyFilename);
	~PosixIoStream();
#endif

	int32 size() const override;
};

#endif
