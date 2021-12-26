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

#ifndef PSPSTREAM_H_
#define PSPSTREAM_H_

#include <pspkerneltypes.h>
#include "backends/platform/psp/powerman.h"
//#include "common/list.h"
#include "common/noncopyable.h"
#include "common/stream.h"
#include "common/str.h"

/**
 *  Class to handle special suspend/resume needs of PSP IO Streams
 */
class PspIoStream final : public Common::SeekableReadStream, public Common::SeekableWriteStream, public Common::NonCopyable, public Suspendable {
protected:
	SceUID _handle;		// file handle
	Common::String _path;
	int _fileSize;
	bool _writeMode;	// for resuming in the right mode
	int _physicalPos;	// physical position in file
	int _pos;			// position. Sometimes virtual
	bool _eos;			// EOS flag

	enum {
		SuspendError = 2,
		ResumeError = 3
	};

	// debug stuff
	mutable int _error;		// file error state
	int _errorSuspend;			// for debugging
	mutable int _errorSource;
	int _errorPos;
	SceUID _errorHandle;
	int _suspendCount;

	bool physicalSeekFromCur(int32 offset);

public:

	/**
	 * Given a path, invoke fopen on that path and wrap the result in a
	 * PspIoStream instance.
	 */
	static PspIoStream *makeFromPath(const Common::String &path, bool writeMode);

	PspIoStream(const Common::String &path, bool writeMode);
	~PspIoStream() override;

	SceUID open();		// open the file pointed to by the file path

	bool err() const override;
	void clearErr() override;
	bool eos() const override;

	uint32 write(const void *dataPtr, uint32 dataSize) override;
	bool flush() override;

	int64 pos() const override;
	int64 size() const override;
	bool seek(int64 offs, int whence = SEEK_SET) override;
	uint32 read(void *dataPtr, uint32 dataSize) override;

	// for suspending
	int suspend() override;		/* Suspendable interface (power manager) */
	int resume() override;		/* " " */
};

#endif /* PSPSTREAM_H_ */
