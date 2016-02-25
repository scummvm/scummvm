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

#ifndef TITANIC_SIMPLE_FILE_H
#define TITANIC_SIMPLE_FILE_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/queue.h"
#include "common/rect.h"
#include "common/savefile.h"
#include "titanic/string.h"

namespace Titanic {

class Decompressor;
class DecompressorData;

class SimpleFile {
protected:
	Common::File _file;
	Common::SeekableReadStream *_inStream;
	Common::OutSaveFile *_outStream;
	int _lineCount;
public:
	SimpleFile();
	virtual ~SimpleFile();

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
	 * Read from the file with validation
	 */
	virtual void safeRead(void *dst, size_t count);

	/**
	 * Read from the file
	 */
	virtual size_t unsafeRead(void *dst, size_t count);

	/**
	 * Write out data
	 */
	virtual size_t write(const void *src, size_t count);

	/**
	 * Read a string from the file
	 */
	CString readString();

	/**
	 * Read a number from the file
	 */
	int readNumber();

	/**
	 * Read a floating point number from the file
	 */
	double readFloat();

	/**
	 * Read in a point
	 */
	Common::Point readPoint();

	/**
	 * Read in a rect
	 */
	Common::Rect readRect();

	/**
	 * Read a string and copy it into an optionally passed buffer
	 */
	void readBuffer(char *buffer = nullptr, size_t count = 0);

	/**
	 * Write a string line
	 */
	void writeLine(const CString &str);

	/**
	 * Write a string
	 */
	void writeString(const CString &str);

	/**
	 * Write a quoted string
	 */
	void writeQuotedString(const CString &str);

	/**
	 * Write a quoted string line
	 */
	void writeQuotedLine(const CString &str, int indent);

	/**
	 * Write a number to file
	 */
	void writeNumber(int val);

	/**
	 * Write a number line to file
	 */
	void writeNumberLine(int val, int indent);

	/**
	 * Write out a point line
	 */
	void writePoint(const Common::Point &pt, int indent);

	/**
	 * Write out a rect line
	 */
	void writeRect(const Common::Rect &r, int indent);

	/**
	 * Write out a number of tabs to form an indent in the output
	 */
	void writeIndent(uint indent);

	/**
	 * Validates that the following non-space character is either
	 * an opening or closing squiggly bracket denoting a class
	 * definition start or end. Returns true if it's a class start
	 */
	bool IsClassStart();

	/**
	 * Write the starting header for a class definition
	 */
	void writeClassStart(const CString &classStr, int indent);

	/**
	 * Write out the ending footer for a class definition
	 */
	void writeClassEnd(int indent);
};

} // End of namespace Titanic

#endif /* TITANIC_SIMPLE_FILE_H */
