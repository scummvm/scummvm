/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef COMMON_FILE_H
#define COMMON_FILE_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

namespace Common {

class File : public SeekableReadStream, public WriteStream {
protected:
	/** POSIX file handle to the actual file; 0 if no file is open. */
	FILE *_handle;

	/** Status flag which tells about recent I/O failures. */
	bool _ioFailed;

	/** Simple ref-counter for File objects. */
	int32 _refcount;

	/** The name of this file, for debugging. */
	String _name;


	static StringList _defaultDirectories;

public:
	enum AccessMode {
		kFileReadMode = 1,
		kFileWriteMode = 2
	};

	static void addDefaultDirectory(const String &directory);
	static void resetDefaultDirectories();

	File();
	virtual ~File();

	void incRef();
	void decRef();

	virtual bool open(const char *filename, AccessMode mode = kFileReadMode, const char *directory = NULL);
	static bool exists(const char *filename, const char *directory = NULL);

	virtual void close();
	bool isOpen() const;
	bool ioFailed() const;
	void clearIOFailed();
	bool eos() const { return eof(); }
	bool eof() const;
	uint32 pos() const;
	uint32 size() const;
	const char *name() const { return _name.c_str(); }
	void seek(int32 offs, int whence = SEEK_SET);
	uint32 read(void *dataPtr, uint32 dataSize);
	uint32 write(const void *dataPtr, uint32 dataSize);
};

} // End of namespace Common

#endif
