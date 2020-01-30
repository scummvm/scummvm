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
 * $URL: https://scummvm.svn.sourceforge.net/svnroot/scummvm/scummvm/trunk/backends/fs/stdiostream.h $
 * $Id: stdiostream.h 44276 2009-09-23 16:11:23Z joostp $
 *
 */

#ifndef STDIOSTREAM_H
#define STDIOSTREAM_H

#include "ultima/shared/std/string.h"

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/sound/mixer/types.h"
#include "decoder/wave/stream.h"

class StdioStream : public Common::SeekableReadStream {
protected:
	/** File handle to the actual file. */
	NuvieIOFileRead *_handle;
	DisposeAfterUse::Flag _disposeMemory;

public:
	/**
	 * Given a path, invokes NuvieIOFileRead class on that path and wrap the result in a
	 * StdioStream instance.
	 */
	static StdioStream *makeFromPath(const Std::string &path);

	StdioStream(NuvieIOFileRead *handle, DisposeAfterUse::Flag disposeMemory = DisposeAfterUse::NO);
	virtual ~StdioStream();

	virtual bool err() const;
	virtual void clearErr();
	virtual bool eos() const;

	virtual uint32 write(const void *dataPtr, uint32 dataSize);
	virtual bool flush();

	virtual sint32 pos() const;
	virtual sint32 size() const;
	virtual bool seek(sint32 offs, int whence = SEEK_SET);
	virtual uint32 read(void *dataPtr, uint32 dataSize);
};

#endif
