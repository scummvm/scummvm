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

#ifndef PSPSTREAM_H_
#define PSPSTREAM_H_

#include <pspkerneltypes.h>
#include "backends/platform/psp/powerman.h"
//#include "common/list.h"
#include "common/noncopyable.h"
#include "common/stream.h"
#include "common/str.h"

/*
 *  Class to handle special suspend/resume needs of PSP IO Streams
 */
class PSPIoStream : public Common::SeekableReadStream, public Common::WriteStream, public Common::NonCopyable, public Suspendable {
protected:
	SceUID _handle;		// file handle
	Common::String _path;
	int _fileSize;
	bool _writeMode;	// for resuming in the right mode
	int _physicalPos;	// position in the real file
	int _pos;			// position. Sometimes virtual
	bool _inCache;		// whether we're in cache (virtual) mode
	bool _eos;			// EOS flag
	
	enum {
		SuspendError = 2,
		ResumeError = 3
	};

	enum {
		CACHE_SIZE = 1024,
		MIN_READ_SIZE = 1024	// reading less than 1024 takes exactly the same time as 1024
	};
	
	// For caching
	char *_cache;
	int _cacheStartOffset;		// starting offset of the cache. -1 when cache is invalid
	
	mutable int _ferror;		// file error state
	int _errorSuspend;			// for debugging
	mutable int _errorSource;
	int _errorPos;
	SceUID _errorHandle;
	int _suspendCount;

	bool synchronizePhysicalPos();		// synchronize the physical and virtual positions
	bool isOffsetInCache(uint32 pos);	// check if an offset is found in cache
	bool isCacheValid() { return _cacheStartOffset != -1; }
	
public:

	/**
	 * Given a path, invoke fopen on that path and wrap the result in a
	 * PSPIoStream instance.
	 */
	static PSPIoStream *makeFromPath(const Common::String &path, bool writeMode);

	PSPIoStream(const Common::String &path, bool writeMode);
	virtual ~PSPIoStream();

	void * open();		// open the file pointed to by the file path

	bool err() const;
	void clearErr();
	bool eos() const;

	virtual uint32 write(const void *dataPtr, uint32 dataSize);
	virtual bool flush();

	virtual int32 pos() const;
	virtual int32 size() const;
	virtual bool seek(int32 offs, int whence = SEEK_SET);
	virtual uint32 read(void *dataPtr, uint32 dataSize);

	int suspend();		/* Suspendable interface (power manager) */
	int resume();		/* " " */
};

#endif /* PSPSTREAM_H_ */
