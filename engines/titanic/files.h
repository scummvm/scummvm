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

#ifndef TITANIC_FILES_H
#define TITANIC_FILES_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/queue.h"
#include "titanic/string.h"

namespace Titanic {

enum FileMode { FILE_READ = 1, FILE_WRITE = 2 };

class SimpleFile {
protected:
	Common::File _file;
public:
	SimpleFile();
	virtual ~SimpleFile();

	/**
	 * Open a file for access
	 */
	virtual void open(const Common::String &name, FileMode mode = FILE_READ);

	/**
	 * Close the file
	 */
	virtual void close();

	/**
	 * Read from the file with validation
	 */
	virtual void safeRead(void *dst, size_t count);

	/**
	 * Read from the file
	 */
	virtual size_t unsafeRead(void *dst, size_t count);

	/**
	 * Read a string from the file
	 */
	virtual CString readString();

	/**
	 * Read a number from the file
	 */
	virtual int readNumber();

	/**
	 * Read a floating point number from the file
	 */
	virtual double readFloat();
};

class CompressedFile : public SimpleFile {
private:
	Common::Queue<byte> _queue;
	int _field48;
	int _isReading;
	int _field260;
	int _mode;

	/**
	 * Decompress data from the source file
	 */
	void decompress();

	void validate(int *mode, const char *version, int v3);

	int validate2(int *mode, int v15, const char *version, int v3);
public:
	CompressedFile();
	virtual ~CompressedFile();

	/**
	 * Open a file for access
	 */
	virtual void open(const Common::String &name, FileMode mode = FILE_READ);

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

#endif /* TITANIC_FILES_H */
