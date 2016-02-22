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

#ifndef TITANIC_COMPRESSED_FILE_H
#define TITANIC_COMPRESSED_FILE_H

#include "common/scummsys.h"
#include "common/file.h"
#include "titanic/compression.h"
#include "titanic/simple_file.h"
#include "titanic/string.h"

namespace Titanic {

enum CompressedFileMode { COMPMODE_NONE, COMPMODE_WRITE, COMPMODE_READ };

/**
 * Derived file that handles compressed files
 */
class CompressedFile : public SimpleFile {
private:
	Compression _compression;
	CompressedFileMode _fileMode;
	byte _writeBuffer[516];
	byte *_dataStartPtr;
	byte *_dataPtr;
	int _dataRemaining;
	int _dataMaxSize;
	int _dataCount;

	/**
	 * Decompress data from the source file
	 */
	void decompress();
public:
	CompressedFile();
	virtual ~CompressedFile();

	/**
	 * Open a file for access
	 */
	virtual void open(const Common::String &name);

	/**
	 * Set up a stream for read access
	 */
	virtual void open(Common::SeekableReadStream *stream);

	/**
	 * Set up a stream for write access
	 */
	virtual void open(Common::OutSaveFile *stream);

	/**
	 * Close the file
	 */
	virtual void close();

	/**
	 * Read from the file
	 */
	virtual size_t unsafeRead(void *dst, size_t count);
};

} // End of namespace Titanic

#endif /* TITANIC_COMPRESSED_FILE_H */
