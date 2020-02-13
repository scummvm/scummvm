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

#include "common/file.h"
#include "titanic/support/rect.h"
#include "common/savefile.h"
#include "common/stream.h"
#include "common/zlib.h"
#include "titanic/support/string.h"

namespace Titanic {

class Decompressor;
class DecompressorData;

/**
 * Simple ScummVM File descendent that throws a wobbly if
 * the file it tries to open isn't present
 */
class File : public Common::File {
public:
	bool open(const Common::String &filename) override;
};

/**
 * This class implements basic reading and writing to files
 */
class SimpleFile {
private:
	/**
	 * Skip over any pending spaces
	 */
	void skipSpaces();
protected:
	Common::SeekableReadStream *_inStream;
	Common::OutSaveFile *_outStream;
	int _lineCount;
public:
	SimpleFile();
	virtual ~SimpleFile();

	operator Common::SeekableReadStream &() { return *_inStream; }
	operator Common::WriteStream &() { return *_outStream; }

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
	virtual size_t write(const void *src, size_t count) const;

	/**
	 * Seek
	 */
	virtual void seek(int offset, int origin);
	/**
	 * Read a byte
	 */
	byte readByte();

	/**
	 * Read a 16-bit LE number
	 */
	uint readUint16LE();

	/**
	 * Read a 32-bit LE number
	 */
	uint readUint32LE();

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
	Point readPoint();

	/**
	 * Read in a rect
	 */
	Rect readRect();

	/**
	 * Rect in a bounds
	 */
	Rect readBounds();

	/**
	 * Read a string and copy it into an optionally passed buffer
	 */
	void readBuffer(char *buffer = nullptr, size_t count = 0);

	/**
	 * Scan in values from the file
	 */
	bool scanf(const char *format, ...);

	/**
	 * Write out a byte
	 */
	void writeByte(byte b) { write(&b, 1); }

	/**
	 * Write out a raw 16-bit LE number
	 */
	void writeUint16LE(uint val);

	/**
	 * Write out a raw 32-bit LE number
	 */
	void writeUint32LE(uint val);

	/**
	 * Write a string line
	 */
	void writeLine(const CString &str) const;

	/**
	 * Write a string
	 */
	void writeString(const CString &str) const;

	/**
	 * Write a quoted string
	 */
	void writeQuotedString(const CString &str) const;

	/**
	 * Write a quoted string line
	 */
	void writeQuotedLine(const CString &str, int indent) const;

	/**
	 * Write a number to file
	 */
	void writeNumber(int val) const;

	/**
	 * Write a number line to file
	 */
	void writeNumberLine(int val, int indent) const;

	/**
	 * Write a floating point number
	 */
	void writeFloat(double val) const;

	/**
	 * Write a floating point number as a line
	 */
	void writeFloatLine(double val, int indent) const;

	/**
	 * Write out a point line
	 */
	void writePoint(const Point &pt, int indent)const;

	/**
	 * Write out a rect line
	 */
	void writeRect(const Rect &r, int indent) const;

	/**
	 * Write out a bounds line
	 */
	void writeBounds(const Rect &r, int indent) const;

	/**
	 * Write out a string using a format specifier, just like fprintf
	 */
	void writeFormat(const char *format, ...) const;

	/**
	 * Write out a number of tabs to form an indent in the output
	 */
	void writeIndent(uint indent) const;

	/**
	 * Validates that the following non-space character is either
	 * an opening or closing squiggly bracket denoting a class
	 * definition start or end. Returns true if it's a class start
	 */
	bool isClassStart();

	/**
	 * Write the starting header for a class definition
	 */
	void writeClassStart(const CString &classStr, int indent);

	/**
	 * Write out the ending footer for a class definition
	 */
	void writeClassEnd(int indent);

	/**
	 * Return true if the stream has finished being read
	 */
	bool eos() const {
		assert(_inStream);
		return _inStream->pos() >= _inStream->size();
	}
};

/**
 * Derived file that handles compressed files
 */
class CompressedFile : public SimpleFile {
public:
	CompressedFile() : SimpleFile() {}
	~CompressedFile() override {}

	/**
	 * Set up a stream for read access
	 */
	void open(Common::SeekableReadStream *stream) override {
		SimpleFile::open(Common::wrapCompressedReadStream(stream));
	}

	/**
	 * Set up a stream for write access
	 */
	void open(Common::OutSaveFile *stream) override {
		SimpleFile::open(new Common::OutSaveFile(Common::wrapCompressedWriteStream(stream)));
	}
};

/**
 * Derived file that handles WAD archives containing multiple files
 */
class StdCWadFile : public SimpleFile {
public:
	StdCWadFile() : SimpleFile() {}
	~StdCWadFile() override {}

	/**
	 * Open up the specified file
	 */
	virtual bool open(const Common::String &filename);

	/**
	 * Unsupported open method from parent class
	 */
	void open(Common::SeekableReadStream *stream) override {}

	/**
	 * Unsupported open method from parent class
	 */
	void open(Common::OutSaveFile *stream) override {}

	/**
	 * Return a reference to the read stream
	 */
	Common::SeekableReadStream *readStream() const { return _inStream; }
};

/**
 * General purpose support method for reading an ASCIIZ string from a stream
 */
CString readStringFromStream(Common::SeekableReadStream *s);

} // End of namespace Titanic

#endif /* TITANIC_SIMPLE_FILE_H */
