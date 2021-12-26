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

#ifndef BACKENDS_FS_ROMFSSTREAM_H
#define BACKENDS_FS_ROMFSSTREAM_H

#include "common/scummsys.h"
#include "common/noncopyable.h"
#include "common/stream.h"
#include "common/str.h"

class RomfsStream final : public Common::SeekableReadStream, public Common::SeekableWriteStream, public Common::NonCopyable {
protected:
	/** File handle to the actual file. */
	void *_handle;

public:
	/**
	 * Given a path, invokes fopen on that path and wrap the result in a
	 * RomfsStream instance.
	 */
	static RomfsStream *makeFromPath(const Common::String &path, bool writeMode);

	RomfsStream(void *handle);
	~RomfsStream() override;

	bool err() const override;
	void clearErr() override;
	bool eos() const override;

	uint32 write(const void *dataPtr, uint32 dataSize) override;
	bool flush() override;

	int64 pos() const override;
	int64 size() const override;
	bool seek(int64 offs, int whence = SEEK_SET) override;
	uint32 read(void *dataPtr, uint32 dataSize) override;
};

#endif
