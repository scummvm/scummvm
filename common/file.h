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

#ifndef COMMON_FILE_H
#define COMMON_FILE_H

#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"

class FilesystemNode;

namespace Common {

class File : public SeekableReadStream, public WriteStream {
protected:
	/** File handle to the actual file; 0 if no file is open. */
	void *_handle;

	/** Status flag which tells about recent I/O failures. */
	bool _ioFailed;

	/** The name of this file, for debugging. */
	String _name;

private:
	// Disallow copying File objects. There is not strict reason for this,
	// except that so far we never had real need for such a feature, and
	// code that accidentally copied File objects tended to break in strange
	// ways.
	File(const File &f);
	File &operator =(const File &f);

public:
	enum AccessMode {
		kFileReadMode = 1,
		kFileWriteMode = 2
	};

	static void addDefaultDirectory(const String &directory);
	static void addDefaultDirectoryRecursive(const String &directory, int level = 4, const String &prefix = "");

	static void addDefaultDirectory(const FilesystemNode &directory);
	static void addDefaultDirectoryRecursive(const FilesystemNode &directory, int level = 4, const String &prefix = "");

	static void resetDefaultDirectories();

	File();
	virtual ~File();

	/**
	 * Checks if a given file exists in any of the current default paths
	 * (those were/are added by addDefaultDirectory and/or
	 * addDefaultDirectoryRecursive).
	 *
	 * @param filename: the file to check for
	 * @return: true if the file exists, else false
	 */
	static bool exists(const String &filename);

	virtual bool open(const String &filename, AccessMode mode = kFileReadMode);
	virtual bool open(const FilesystemNode &node, AccessMode mode = kFileReadMode);

	virtual void close();

	/**
	 * Checks if the object opened a file successfully.
	 *
	 * @return: true if any file is opened, false otherwise.
	 */
	bool isOpen() const;

	/**
	 * Returns the filename of the opened file.
	 *
	 * @return: the filename
	 */
	const char *name() const { return _name.c_str(); }

	bool ioFailed() const;
	void clearIOFailed();
	bool eos() const { return eof(); }

	/**
	 * Checks for end of file.
	 *
	 * @return: true if the end of file is reached, false otherwise.
	 */
	virtual bool eof() const;

	virtual uint32 pos() const;
	virtual uint32 size() const;
	void seek(int32 offs, int whence = SEEK_SET);
	uint32 read(void *dataPtr, uint32 dataSize);
	uint32 write(const void *dataPtr, uint32 dataSize);
};

} // End of namespace Common

#endif
