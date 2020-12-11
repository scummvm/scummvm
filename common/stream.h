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

#ifndef COMMON_STREAM_H
#define COMMON_STREAM_H

#include "common/endian.h"
#include "common/scummsys.h"
#include "common/str.h"

namespace Common {

/**
 * @defgroup common_stream Streams
 * @ingroup common
 *
 * @brief API for managing readable and writable data streams.
 *
 * @{
 */

class ReadStream;
class SeekableReadStream;

/**
 * Virtual base class for both ReadStream and WriteStream.
 */
class Stream {
public:
	virtual ~Stream() {}

	/**
	 * Return true if an I/O failure occurred.
	 *
	 * This flag is never cleared automatically. In order to clear it,
	 * the client code must call clearErr() explicitly.
	 *
	 * @note The semantics of any implementation of this method is
	 * supposed to match that of ISO C ferror().
	 */
	virtual bool err() const { return false; }

	/**
	 * Reset the I/O error status as returned by err().
	 *
	 * For a ReadStream, also reset the end-of-stream status returned by eos().
	 *
	 * @note The semantics of any implementation of this method is
	 * supposed to match that of ISO C clearerr().
	 */
	virtual void clearErr() {}
};

/**
 * Generic interface for a writable data stream.
 */
class WriteStream : virtual public Stream {
public:
	/**
	 * Write data into the stream. Subclasses must implement this
	 * method. All other write methods are implemented using it.
	 *
	 * @note The semantics of any implementation of this method is
	 * supposed to match that of ISO C fwrite().
	 *
	 * @param dataPtr	Pointer to the data to be written.
	 * @param dataSize	Number of bytes to be written.
	 *
	 * @return The number of bytes that were actually written.
	 */
	virtual uint32 write(const void *dataPtr, uint32 dataSize) = 0;

	/**
	 * Commit any buffered data to the underlying channel or
	 * storage medium. Unbuffered streams can use the default
	 * implementation.
	 *
	 * @note The semantics of any implementation of this method is
	 * supposed to match that of ISO C fflush().
	 *
	 * @return True on success, false in case of a failure.
	 */
	virtual bool flush() { return true; }

	/**
	 * Finalize and close this stream. 
	 *
	 * Call this method right before this stream instance is deleted.
	 * The goal is to enable the client code to detect
	 * and handle I/O errors that might occur when closing 
	 * (and flushing, if buffered) the stream.
	 *
	 * After this method has been called, no further writes can be
	 * performed on the stream. Calling err() is allowed.
	 *
	 * By default, this just flushes the stream.
	 */
	virtual void finalize() {
		flush();
	}

	/**
	* Obtain the current value of the stream position indicator.
	*
	* @return The current position indicator, or -1 if an error occurred.
	 */
	virtual int32 pos() const = 0;

	/**
	 * @name Functions for writing data
	 *
	 * The following methods all have default implementations.
	 * Subclasses need not (and should not) overload them.
	 * @{
	 */
 
	/**
	 * Write the given byte to the current position in the stream.
	 */
	void writeByte(byte value) {
		write(&value, 1);
	}
	/**
	 * Write the given signed byte to the current position in the stream.
	 */
	void writeSByte(int8 value) {
		write(&value, 1);
	}
	/**
	 * Write an unsigned 16-bit word stored in little endian order into the stream.
	 */
	void writeUint16LE(uint16 value) {
		value = TO_LE_16(value);
		write(&value, 2);
	}
	/**
	 * Write an unsigned 32-bit word stored in little endian order into the stream.
	 */
	void writeUint32LE(uint32 value) {
		value = TO_LE_32(value);
		write(&value, 4);
	}
	/**
	 * Write an unsigned 64-bit word stored in little endian order into the stream.
	 */
	void writeUint64LE(uint64 value) {
		value = TO_LE_64(value);
		write(&value, 8);
	}
	/**
	 * Write an unsigned 16-bit word stored in big endian order into the stream.
	 */
	void writeUint16BE(uint16 value) {
		value = TO_BE_16(value);
		write(&value, 2);
	}
	/**
	 * Write an unsigned 32-bit word stored in big endian order into the stream.
	 */
	void writeUint32BE(uint32 value) {
		value = TO_BE_32(value);
		write(&value, 4);
	}
	/**
	 * Write an unsigned 64-bit word stored in big endian order into the stream.
	 */
	void writeUint64BE(uint64 value) {
		value = TO_BE_64(value);
		write(&value, 8);
	}
	/**
	 * Write a signed 16-bit word stored in little endian order into the stream.
	 */
	FORCEINLINE void writeSint16LE(int16 value) {
		writeUint16LE((uint16)value);
	}
	/**
	 * Write a signed 32-bit word stored in little endian order into the stream.
	 */
	FORCEINLINE void writeSint32LE(int32 value) {
		writeUint32LE((uint32)value);
	}
	/**
	 * Write a signed 64-bit word stored in little endian order into the stream.
	 */
	FORCEINLINE void writeSint64LE(int64 value) {
		writeUint64LE((uint64)value);
	}
	/**
	 * Write a signed 16-bit word stored in big endian order into the stream.
	 */
	FORCEINLINE void writeSint16BE(int16 value) {
		writeUint16BE((uint16)value);
	}
	/**
	 * Write a signed 32-bit word stored in big endian order into the stream.
	 */
	FORCEINLINE void writeSint32BE(int32 value) {
		writeUint32BE((uint32)value);
	}
	/**
	 * Write a signed 64-bit word stored in big endian order into the stream.
	 */
	FORCEINLINE void writeSint64BE(int64 value) {
		writeUint64BE((uint64)value);
	}


	/**
	 * Write a 32-bit floating point value
	 * stored in little endian (LSB first) order into the stream.
	 */
	FORCEINLINE void writeFloatLE(float value) {
		uint32 n;

		memcpy(&n, &value, 4);

		writeUint32LE(n);
	}


	/**
	 * Write a 32-bit floating point value
	 * stored in big endian order into the stream. 
	 */
	FORCEINLINE void writeFloatBE(float value) {
		uint32 n;

		memcpy(&n, &value, 4);

		writeUint32BE(n);
	}

	/**
	 * Write a 64-bit floating point value (with decimals)
	 * stored in little endian (LSB first) order into the stream.
	 */
	FORCEINLINE void writeDoubleLE(double value) {
		uint64 n;

		memcpy(&n, &value, 8);

		writeUint64LE(n);
	}


	/**
	 * Write the given 64-bit floating point value (with decimals) 
	 * stored in big endian order into the stream.
	 */
	FORCEINLINE void writeDoubleBE(double value) {
		uint64 n;

		memcpy(&n, &value, 8);

		writeUint64BE(n);
	}

	/**
	 * Write at most @p dataSize of data from another stream into this one,
	 * starting from the current stream position.
	 *
	 * @return The number of bytes written into the stream.
	 */
	uint32 writeStream(ReadStream *stream, uint32 dataSize);
	/**
	 * Write data from another stream into this one,
	 * starting from its current position to the end of the stream.
	 *
	 * @return The number of bytes written into the stream.
	 */
	uint32 writeStream(SeekableReadStream *stream);

	/**
	 * Write the given string to the stream.
	 * This writes str.size() characters, but no terminating zero byte.
	 */
	void writeString(const String &str);
	/** @} */
};

/**
 * Derived abstract base class for write streams that are seekable.
 */
class SeekableWriteStream : public WriteStream {
public:
	/**
	 * Set the stream position indicator for the stream. 
	 *
	 * The new position, measured in bytes, is obtained by adding offset bytes
	 * to the position specified by whence. If whence is set to SEEK_SET, SEEK_CUR,
	 * or SEEK_END, the offset is relative to the start of the file, the current
	 * position indicator, or end-of-stream, respectively. A successful call
	 * to the seek() method clears the end-of-stream indicator for the stream.
	 *
	 * @note The semantics of any implementation of this method is
	 * supposed to match that of ISO C fseek().
	 *
	 * @param offset        The relative offset in bytes.
	 * @param whence        The seek reference: SEEK_SET, SEEK_CUR, or SEEK_END.
	 *
	 * @return True on success, false in case of a failure.
	 */
	virtual bool seek(int32 offset, int whence = SEEK_SET) = 0;

	/**
	 * Obtain the current size of the stream, measured in bytes.
	 *
	 * If this value is unknown or cannot be computed, -1 is returned.
	 *
	 * @return The size of the stream, or -1 if an error occurred.
	 */
	virtual int32 size() const = 0;
};

/**
 * Generic interface for a readable data stream.
 */
class ReadStream : virtual public Stream {
public:
	ReadStream() {}

	/**
	 * Return true if a read failed because the stream end has been reached.
	 *
	 * This flag is cleared by clearErr().
	 * For a SeekableReadStream, the flag is also cleared by a successful seek.
	 *
	 * @note The semantics of any implementation of this method is
	 * supposed to match that of ISO C feof(). In particular, in a stream
	 * with N bytes, reading exactly N bytes from the start should *not*
	 * set eos; only reading *beyond* the available data should set it.
	 */
	virtual bool eos() const = 0;

	/**
	 * Read data from the stream.
	 *
	 * Subclasses must implement this method.
	 * All other read methods are implemented using it.
	 *
	 * @note The semantics of any implementation of this method is
	 * supposed to match that of ISO C fread(), in particular where
	 * it concerns setting error and end of file/stream flags.
	 *
	 * @param dataPtr	Pointer to a buffer into which the data is read.
	 * @param dataSize	Number of bytes to be read.
	 *
	 * @return The number of bytes that were actually read.
	 */
	virtual uint32 read(void *dataPtr, uint32 dataSize) = 0;

	/**
	 * @name Functions for reading data
	 *
	 * The following methods all have default implementations.
	 * Subclasses in general should not overload them.
	 * @{
	 */

	/**
	 * Read an unsigned byte from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	byte readByte() {
		byte b = 0; // FIXME: remove initialisation
		read(&b, 1);
		return b;
	}

	/**
	 * Read a signed byte from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int8 readSByte() {
		return (int8)readByte();
	}

	/**
	 * Read an unsigned 16-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	uint16 readUint16LE() {
		uint16 val;
		read(&val, 2);
		return FROM_LE_16(val);
	}

	/**
	 * Read an unsigned 32-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	uint32 readUint32LE() {
		uint32 val;
		read(&val, 4);
		return FROM_LE_32(val);
	}

	/**
	 * Read an unsigned 64-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	uint64 readUint64LE() {
		uint64 val;
		read(&val, 8);
		return FROM_LE_64(val);
	}

	/**
	 * Read an unsigned 16-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	uint16 readUint16BE() {
		uint16 val;
		read(&val, 2);
		return FROM_BE_16(val);
	}

	/**
	 * Read an unsigned 32-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	uint32 readUint32BE() {
		uint32 val;
		read(&val, 4);
		return FROM_BE_32(val);
	}

	/**
	 * Read an unsigned 64-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	uint64 readUint64BE() {
		uint64 val;
		read(&val, 8);
		return FROM_BE_64(val);
	}

	/**
	 * Read a signed 16-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int16 readSint16LE() {
		return (int16)readUint16LE();
	}

	/**
	 * Read a signed 32-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int32 readSint32LE() {
		return (int32)readUint32LE();
	}

	/**
	 * Read a signed 64-bit word stored in little endian (LSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int64 readSint64LE() {
		return (int64)readUint64LE();
	}

	/**
	 * Read a signed 16-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int16 readSint16BE() {
		return (int16)readUint16BE();
	}

	/**
	 * Read a signed 32-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int32 readSint32BE() {
		return (int32)readUint32BE();
	}

	/**
	 * Read a signed 64-bit word stored in big endian (MSB first) order
	 * from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE int64 readSint64BE() {
		return (int64)readUint64BE();
	}

	/**
	 * Read a 32-bit floating point value stored in little endian (LSB first)
	 * order from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE float readFloatLE() {
		uint32 n = readUint32LE();
		float f;

		memcpy(&f, &n, 4);

		return f;
	}

	/**
	 * Read a 32-bit floating point value stored in big endian
	 * order from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE float readFloatBE() {
		uint32 n = readUint32BE();
		float f;

		memcpy(&f, &n, 4);

		return f;
	}


	/**
	 * Read a 64-bit floating point value stored in little endian (LSB first)
	 * order from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE double readDoubleLE() {
		uint64 n = readUint64LE();
		double d;

		memcpy(&d, &n, 8);

		return d;
	}

	/**
	 * Read a 64-bit floating point value stored in big endian
	 * order from the stream and return it.
	 *
	 * Performs no error checking. The return value is undefined
	 * if a read error occurred (for which the client code can check by
	 * calling err() and eos() ).
	 */
	FORCEINLINE double readDoubleBE() {
		uint64 n = readUint64BE();
		double d;

		memcpy(&d, &n, 8);

		return d;
	}

	/**
	 * Read the specified amount of data into a malloc'ed buffer
	 * which is then wrapped into a MemoryReadStream.
	 *
	 * The returned stream might contain less data than requested
	 * if reading more data failed. This is because of an I/O error or because
	 * the end of the stream was reached. It can be determined by
	 * calling err() and eos().
	 */
	SeekableReadStream *readStream(uint32 dataSize);

	/**
	 * Reads in a terminated string. Upon successful completion,
	 * return a string with the content of the line, *without*
	 * the terminating character.
	 *
	 * @param terminator	The terminating character to use.
	 */
	String readString(char terminator = 0);

	/**
	 * Read a string in Pascal format, that is, one byte is
	 * string length, followed by string data.
	 *
	 * @param transformCR	If set (default), then transform \\r into \\n.
	 */
	Common::String readPascalString(bool transformCR = true);
	/** @} */
};

/**
 * Interface for a seekable and readable data stream.
 *
 * @todo Get rid of SEEK_SET, SEEK_CUR, or SEEK_END, use our own constants.
 */
class SeekableReadStream : virtual public ReadStream {
public:

	/**
	 * Obtain the current value of the stream position indicator.
	 *
	 * @return The current position indicator, or -1 if an error occurred.
	 */
	virtual int32 pos() const = 0;

	/**
	 * Obtain the total size of the stream, measured in bytes.
	 * If this value is unknown or cannot be computed, -1 is returned.
	 *
	 * @return The size of the stream, or -1 if an error occurred.
	 */
	virtual int32 size() const = 0;

	/**
	 * Set the stream position indicator for the stream.
	 *
	 * The new position, measured in bytes, is obtained by adding offset bytes
	 * to the position specified by whence. If whence is set to SEEK_SET, SEEK_CUR,
	 * or SEEK_END, the offset is relative to the start of the file, the current
	 * position indicator, or end-of-stream, respectively. A successful call
	 * to the seek() method clears the end-of-stream indicator for the stream.
	 *
	 * @note The semantics of any implementation of this method is
	 * supposed to match that of ISO C fseek().
	 *
	 * @param offset	Relative offset in bytes.
	 * @param whence	Seek reference: SEEK_SET, SEEK_CUR, or SEEK_END.
	 *
	 * @return True on success, false in case of a failure.
	 */
	virtual bool seek(int32 offset, int whence = SEEK_SET) = 0;

	/**
	 * Skip the given number of bytes in the stream.
	 * 
	 * This is equivalent to calling:
	 * @code
	 * seek(offset, SEEK_CUR)
	 * @endcode
	 * to add the given number of bytes to the current position indicator of the stream.
	 *
	 * @return True on success, false in case of a failure.
	 */
	virtual bool skip(uint32 offset) { return seek(offset, SEEK_CUR); }

	/**
	 * Read at most one less than the number of characters specified
	 * by @p bufSize from the stream and store them in the string buffer.
	 *
	 * Reading stops when the end of a line is reached (CR, CR/LF, or LF),
	 * and at end-of-stream or error. The newline, if any, is retained (CR
	 * and CR/LF are translated to ``LF = 0xA = '\n'``). If any characters
	 * are read and there is no error, a `\0` character is appended
	 * to end the string.
	 *
	 * Upon successful completion, return a pointer to the string. If
	 * end-of-stream occurs before any characters are read, returns NULL
	 * and the buffer contents remain unchanged. If an error occurs,
	 * returns NULL and the buffer contents are indeterminate.
	 * This method does not distinguish between end-of-stream and error;
	 * callers must use err() or eos() to determine which occurred.
	 *
	 * @note This method is closely modeled after the standard fgets()
	 *       function from stdio.h.
	 *
	 * @param s         The buffer to store into.
	 * @param bufSize	Size of the buffer.
	 * @param handleCR	If set (default), then CR and CR/LF are handled, as well as LF.
	 *
	 * @return Pointer to the read string, or NULL if an error occurred.
	 */
	virtual char *readLine(char *s, size_t bufSize, bool handleCR = true);


	/**
	 * Read a full line and returns it as a Common::String.
	 *
	 * Reading stops when the end of a line is reached (CR, CR/LF, or LF),
	 * and at end-of-stream or error.
	 *
	 * Upon successful completion, return a string with the content
	 * of the line, *without* the end of a line marker. This method
	 * does not indicate whether an error occurred. Callers must use
	 * err() or eos() to determine whether an exception occurred.
	 *
	 * @param handleCR	If set (default), then CR and CR/LF are handled, as well as LF.
	 */
	virtual String readLine(bool handleCR = true);

	/**
	 * Print a hexdump of the stream while maintaing position. The number
	 * of bytes per line is customizable.
	 *
	 * @param len           Length of this data.
	 * @param bytesPerLine	Number of bytes to print per line (default: 16).
	 * @param startOffset	Shift the shown offsets by the starting offset (default: 0).
	 */
	void hexdump(int len, int bytesPerLine = 16, int startOffset = 0);
};

/**
 * ReadStream mixin subclass that adds non-endian read
 * methods whose endianness is set during the stream creation.
 */
class ReadStreamEndian : virtual public ReadStream {
private:
	const bool _bigEndian;

public:
	/**
	 * Set the endianness of the read stream.
	 *
	 * @param bigEndian If true, create a big endian stream.
	 *                  If false, create a little endian stream.
	 */
	ReadStreamEndian(bool bigEndian) : _bigEndian(bigEndian) {}
	/**
	 * Return true if data is encoded in big endian order.
	 */
	bool isBE() const { return _bigEndian; }
	/**
	 * Read an unsigned 16-bit word using the stream endianness
	 * and return it in native endianness.
	 */
	uint16 readUint16() {
		uint16 val;
		read(&val, 2);
		return (_bigEndian) ? FROM_BE_16(val) : FROM_LE_16(val);
	}
	/**
	 * Read an unsigned 32-bit word using the stream endianness
	 * and return it in native endianness.
	 */
	uint32 readUint32() {
		uint32 val;
		read(&val, 4);
		return (_bigEndian) ? FROM_BE_32(val) : FROM_LE_32(val);
	}
	/**
	 * Read an unsigned 64-bit word using the stream endianness
	 * and return it in native endianness.
	 */
	uint64 readUint64() {
		uint64 val;
		read(&val, 8);
		return (_bigEndian) ? FROM_BE_64(val) : FROM_LE_64(val);
	}
	/**
	 * Read a signed 16-bit word using the stream endianness
	 * and return it in native endianness.
	 */
	FORCEINLINE int16 readSint16() {
		return (int16)readUint16();
	}
	/**
	 * Read a signed 32-bit word using the stream endianness
	 * and return it in native endianness.
	 */
	FORCEINLINE int32 readSint32() {
		return (int32)readUint32();
	}
	/**
	 * Read a signed 64-bit word using the stream endianness
	 * and return it in native endianness.
	 */
	FORCEINLINE int64 readSint64() {
		return (int64)readUint64();
	}
};

/**
 * SeekableReadStream subclass that adds non-endian read
 * methods whose endianness is set during the stream creation.
 */
class SeekableReadStreamEndian : virtual public SeekableReadStream, virtual public ReadStreamEndian {
public:
	/**
	 * Set the endianness of the read stream.
	 *
	 * @param bigEndian If true, create a big endian stream.
	 *                  If false, create a little endian stream.
	 */
	SeekableReadStreamEndian(bool bigEndian) : ReadStreamEndian(bigEndian) {}
};

/** @} */

} // End of namespace Common

#endif
