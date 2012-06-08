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
 */

/*
 * This code is based on original Tony Tough source code
 *
 * Copyright (c) 1997-2003 Nayma Software
 */

#include "tony/utils.h"
#include "tony/tony.h"
#include "tony/mpal/lzo.h"

namespace Tony {

/****************************************************************************\
*       RMString methods
\****************************************************************************/

/**
 * Constructor
 */
RMString::RMString() {
	_string = NULL;
	_length = 0;
	_realLength = 0;
}

/**
 * Destructor
 */
RMString::~RMString() {
	if (_string != NULL)
		delete[] _string;
}

/**
 * Copy constructor
 */
RMString::RMString(const RMString &str) {
	// Richiama l'overload su '=' per copiare
	_string = NULL;
	_length = 0;
	_realLength = 0;
	*this = str;
}

/**
 * Constructor from a char *
 */
RMString::RMString(const char *str) {
	// Use the overloaded '=' when copying
	_string = NULL;
	_length = 0;
	_realLength = 0;
	*this = str;
}

/**
 * Constructor with a single passed character
 */
RMString::RMString(const int ch) {
	// Use the overloaded '=' when copying
	_string = NULL;
	_length = 0;
	_realLength = 0;
	*this = ch;
}

/**
 * Returns the length of the string
 * @returns                 Length
 */
int RMString::length() const {
	return _length;
}

/**
 * Gets the character at the given index
 * @param nIndex            Position of the character to return
 * @returns                 Character
 */
char RMString::getAt(int nIndex) {
	assert(nIndex < _length);
	return _string[nIndex];
}

/**
 * Sets the character at the given index
 * @param nIndex            Position of the character to change
 * @param c                 Character
 */
void RMString::setAt(int nIndex, char c) {
	assert(nIndex < _length);
	_string[nIndex] = c;
}

/**
 * Overloaded square brackets operator for accessing characters within the string
 * @param nIndex            Position of the charactre to reference
 * @params                  Reference to the character
 */
char &RMString::operator[](int nIndex) {
	assert(nIndex < _length);
	return _string[nIndex];
}

/**
 * Copies a string
 * @param str               String to copy
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator=(const RMString &str) {
	// Set the new length
	_length = str._length;

	// If the source is empty, then destroy the current string buffer
	if (_length == 0) {
		if (_realLength > 0) {
			delete[] _string;
			_string = NULL;
			_realLength = 0;
		}
	} else {
		// Resize if necessary
		resize(_length + 1);

		// Copy the string
		Common::copy(str._string, str._string + _length + 1, _string);
	}

	return *this;
}

/**
 * Copies a char * string
 * @param str               String to copy
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator=(const char *str) {
	// If the source is empty, then destroy the current string buffer
	if (str == NULL) {
		if (_realLength > 0) {
			delete[] _string;
			_string = NULL;
			_realLength = _length = 0;
		}
	} else {
		// Calculate the new length
		_length = strlen(str);

		// Resize if necessary
		resize(_length + 1);

		// Copy the string
		Common::copy(str, str + _length + 1, _string);
	}

	return *this;
}

/**
 * Forms a string from a passed character
 * @param ch                Character to copy
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator=(const int ch) {
	if (ch == '\0')  {
		// Destroy the current string
		if (_realLength > 0) {
			delete [] _string;
			_string = NULL;
			_length = _realLength = 0;
		}
	} else {
		// Resize if necessary
		resize(2);

		_string[0] = ch;
		_string[1] = '\0';
		_length = 1;
	}

	return *this;
}

/**
 * Concatenate a string into the current one
 * @param str               String to concatenate
 * @param size              Length of the string
 */
void RMString::connect(const char *str, int size) {
	int nlen;

	if (size > 0) {
		// Calculate the new lenght
		nlen = _length + size;

		// Resize
		resize(nlen + 1, true);

		// Linkage with '\0'
		Common::copy(str, str + size + 1, _string + _length);

		// Save the new length
		_length = nlen;
	}
}

/**
 * Concatenate a string
 * @param str               String to concatenate
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator+=(RMString &str) {
	connect(str, str.length());
	return *this;
}

/**
 * Concatenate a string
 * @param str               String to concatenate
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator+=(const char *str) {
	connect(str, strlen(str));
	return *this;
}

/**
 * Concatenate a character
 * @param ch                Character to concatenate
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator+=(const int ch) {
	char str[2];

	// Create a simple string buffer to hold the single character
	str[0] = ch;
	str[1] = '\0';

	connect(str, 1);
	return *this;
}

/**
 * Casts a string as char *
 * @returns                 char * reference to string
 */
RMString::operator char *() const {
	return _string;
}

/**
 * Resize a string as necessary
 * @param size              New size necessary (in bytes)
 * @param bMaintain         If true we must keep the original string,
                            if false we can destroy.
 */
void RMString::resize(int size, bool bMantain) {
	if (_realLength == 0) {
		_string = new char[size];
		_realLength = size;
	} else if (size > _realLength) {
		if (bMantain) {
			char *app;

			app = new char[size];
			Common::copy(_string, _string + _length + 1, app);
			delete[] _string;
			_string = app;
		} else {
			delete[] _string;
			_string = new char[size];
		}
	}
}

/**
 * Compacts the string to occupy less memory if possible.
 */
void RMString::compact(void) {
	if (_realLength + 1 > _length) {
		char *app;

		app = new char[_length + 1];
		Common::copy(_string, _string + _length + 1, app);

		delete[] _string;
		_string = app;
	}
}

/**
 * Operator to concatenate two strings
 */
RMString operator+(const RMString &str1, const RMString &str2) {
	RMString ret(str1);

	return (ret += str2);
}

/**
 * Operator to concatenate a character to a string
 */
RMString operator+(RMString &str, const int ch) {
	RMString ret(str);

	return (ret += ch);
}

RMString operator+(const int ch, RMString &str) {
	RMString ret(ch);

	return (ret += str);
}

/**
 * Operator to concatenate a char * string to an RMString
 */
RMString operator+(RMString &str, const char *s) {
	RMString ret(str);

	return (ret += s);
}

RMString operator+(const char *s, RMString &str) {
	RMString ret(s);

	return (ret += str);
}

/**
 * Extracts a string from a data stream
 * @param df                data stream
 * @param var               String
 */
RMDataStream &operator>>(RMDataStream &df, RMString &var) {
	uint8 len;
	int i;

	df >> len;
	var.resize(len + 1);
	var._length = len + 1;

	for (i = 0; i < len; i++)
		df >> var[i];

	var[i] = '\0';
	var._length = len;

	return df;
}

/**
 * Formats a string
 */
void RMString::format(const char *str, ...) {
	static char buf[2048];
	va_list argList;

	va_start(argList, str);
	vsprintf(buf, str, argList);
	va_end(argList);
	*this = buf;
}

/****************************************************************************\
*       RMFileStreamSlow Methods
\****************************************************************************/

RMFileStreamSlow::RMFileStreamSlow() : RMDataStream() {
	_stream = NULL;
}

RMFileStreamSlow::~RMFileStreamSlow() {
	close();
}

void RMFileStreamSlow::close() {
	delete _stream;
}

bool RMFileStreamSlow::openFile(Common::File &file) {
	_stream = file.readStream(file.size());

	_length = _stream->pos();

	return true;
}


bool RMFileStreamSlow::openFile(const char *lpFN) {
	// Open file for reading
	Common::File f;
	if (!f.open(lpFN))
		return false;

	_length = f.size();
	_stream = f.readStream(f.size());

	return true;
}


RMDataStream &RMFileStreamSlow::operator+=(int nBytes) {
	seek(nBytes);
	return *this;
}

int RMFileStreamSlow::pos() {
	return _stream->pos();
}

bool RMFileStreamSlow::isEOF() {
	return (pos() >= _length);
}


int RMFileStreamSlow::seek(int nBytes, RMDSPos where) {
	switch (where) {
	case START:
		return _stream->seek(nBytes);

	case END:
		return _stream->seek(nBytes, SEEK_END);

	case CUR:
		return _stream->seek(nBytes, SEEK_CUR);

	default:
		return 0;
	}
}


bool RMFileStreamSlow::read(void *buf, int size) {
	uint32 dwRead;

	dwRead = _stream->read(buf, size);
	return ((int)dwRead == size);
}


RMFileStreamSlow &operator>>(RMFileStreamSlow &df, char &var) {
	df.read(&var, 1);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, byte &var) {
	df.read(&var, 1);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, uint16 &var) {
	uint16 v;
	df.read(&v, 2);
	v = FROM_LE_16(v);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, int16 &var) {
	uint16 v;
	df.read(&v, 2);
	var = (int16)FROM_LE_16(v);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, int &var) {
	int v;
	df.read(&v, 4);
	var = FROM_LE_32(v);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, uint32 &var) {
	uint32 v;
	df.read(&v, 4);
	var = FROM_LE_32(v);
	return df;
}


/****************************************************************************\
*       RMDataStream methods
\****************************************************************************/

/**
 * Constructor
 */
RMDataStream::RMDataStream() {
	_length = 0;
	_pos = 0;
	_bError = false;
}

/**
 * Destructor
 */
RMDataStream::~RMDataStream() {
	close();
}

/**
 * Close a stream
 */
void RMDataStream::close(void) {
	_length = 0;
	_pos = 0;
}

/**
 * Takes the address of the buffer from which will be read the data.
 * @param lpBuf         Data buffer
 * @param size          Size of the buffer
 * @remarks             If the length of the buffer is not known, and cannot be
 *                      specified, then EOF() and Seek() to end won't work.
 */
void RMDataStream::openBuffer(const byte *lpBuf, int size) {
	_length = size;
	_buf = lpBuf;
	_bError = false;
	_pos = 0;
}

/**
 * Returns the length of the stream
 * @returns             Stream length
 */
int RMDataStream::length() {
	return _length;
}

/**
 * Determines if the end of the stream has been reached
 * @returns             true if end of stream reached, false if not
 */
bool RMDataStream::isEOF() {
	return (_pos >= _length);
}

/**
 * Extracts data from the stream
 * @param df                Stream
 * @param var               Variable of a supported type
 * @returns                 Value read from the stream
 */
RMDataStream &operator>>(RMDataStream &df, char &var) {
	df.read(&var, 1);
	return df;
}

/**
 * Extracts data from the stream
 * @param df                Stream
 * @param var               Variable of a supported type
 * @returns                 Value read from the stream
 */
RMDataStream &operator>>(RMDataStream &df, uint8 &var) {
	df.read(&var, 1);
	return df;
}

/**
 * Extracts data from the stream
 * @param df                Stream
 * @param var               Variable of a supported type
 * @returns                 Value read from the stream
 */
RMDataStream &operator>>(RMDataStream &df, uint16 &var) {
	uint16 v;
	df.read(&v, 2);

	var = FROM_LE_16(v);
	return df;
}

/**
 * Extracts data from the stream
 * @param df                Stream
 * @param var               Variable of a supported type
 * @returns                 Value read from the stream
 */
RMDataStream &operator>>(RMDataStream &df, int16 &var) {
	uint16 v;
	df.read(&v, 2);

	var = (int16)FROM_LE_16(v);
	return df;
}

/**
 * Extracts data from the stream
 * @param df                Stream
 * @param var               Variable of a supported type
 * @returns                 Value read from the stream
 */
RMDataStream &operator>>(RMDataStream &df, int &var) {
	uint32 v;
	df.read(&v, 4);

	var = (int)FROM_LE_32(v);
	return df;
}

/**
 * Extracts data from the stream
 * @param df                Stream
 * @param var               Variable of a supported type
 * @returns                 Value read from the stream
 */
RMDataStream &operator>>(RMDataStream &df, uint32 &var) {
	uint32 v;
	df.read(&v, 4);

	var = FROM_LE_32(v);
	return df;
}

/**
 * Reads a series of data from the stream in a buffer
 * @param lpBuf             Data buffer
 * @param size              Size of the buffer
 * @returns                 true if we have reached the end, false if not
 */
bool RMDataStream::read(void *lpBuf, int size) {
	byte *dest = (byte *)lpBuf;

	if ((_pos + size) > _length) {
		Common::copy(_buf + _pos, _buf + _pos + (_length - _pos), dest);

		return true;
	} else {
		Common::copy(_buf + _pos, _buf + _pos + size, dest);

		_pos += size;
		return false;
	}
}

/**
 * Skips a number of bytes in the stream
 * @param nBytres           Number of bytes to skip
 * @returns                 The stream
 */
RMDataStream &RMDataStream::operator+=(int nBytes) {
	_pos += nBytes;
	return *this;
}

/**
 * Seeks to a position within the stream
 * @param nBytes            Number of bytes from specified origin
 * @param origin            Origin to do offset from
 * @returns                 The absolute current position in bytes
 */
int RMDataStream::seek(int nBytes, RMDSPos origin) {
	switch (origin) {
	case CUR:
		break;

	case START:
		_pos = 0;
		break;

	case END:
		if (_length == SIZENOTKNOWN)
			return _pos;
		_pos = _length;
		break;
	}

	_pos += nBytes;
	return _pos;
}

/**
 * Returns the current position of the stream
 * @returns                 The current position
 */
int RMDataStream::pos() {
	return _pos;
}

/**
 * Check if an error occurred during reading the stream
 * @returns                 true if there was an error, false otherwise
 */
bool RMDataStream::isError() {
	return _bError;
}

/**
 * Sets an error code for the stream
 * @param code              Error code
 */
void RMDataStream::setError(int code) {
	_bError = true;
	_ecode = code;
}

/**
 * Returns the error code for the stream
 * @returns                 Error code
 */
int RMDataStream::getError() {
	return _ecode;
}

/****************************************************************************\
*       RMPoint methods
\****************************************************************************/

/**
 * Constructor
 */
RMPoint::RMPoint() {
	x = y = 0;
}

/**
 * Copy constructor
 */
RMPoint::RMPoint(const RMPoint &p) {
	x = p.x;
	y = p.y;
}

/**
 * Constructor with integer parameters
 */
RMPoint::RMPoint(int x1, int y1) {
	x = x1;
	y = y1;
}

/**
 * Copy operator
 */
RMPoint &RMPoint::operator=(RMPoint p) {
	x = p.x;
	y = p.y;

	return *this;
}

/**
 * Offsets the point by another point
 */
void RMPoint::offset(const RMPoint &p) {
	x += p.x;
	y += p.y;
}

/**
 * Offsets the point by a specified offset
 */
void RMPoint::offset(int xOff, int yOff) {
	x += xOff;
	y += yOff;
}

/**
 * Sums together two points
 */
RMPoint operator+(RMPoint p1, RMPoint p2) {
	RMPoint p(p1);

	return (p += p2);
}

/**
 * Subtracts two points
 */
RMPoint operator-(RMPoint p1, RMPoint p2) {
	RMPoint p(p1);

	return (p -= p2);
}

/**
 * Sum (offset) of a point
 */
RMPoint &RMPoint::operator+=(RMPoint p) {
	offset(p);
	return *this;
}

/**
 * Subtract (offset) of a point
 */
RMPoint &RMPoint::operator-=(RMPoint p) {
	offset(-p);
	return *this;
}

/**
 * Inverts a point
 */
RMPoint RMPoint::operator-() {
	RMPoint p;

	p.x = -x;
	p.y = -y;

	return p;
}

/**
 * Equality operator
 */
bool RMPoint::operator==(RMPoint p) {
	return ((x == p.x) && (y == p.y));
}

/**
 * Not equal operator
 */
bool RMPoint::operator!=(RMPoint p) {
	return ((x != p.x) || (y != p.y));
}

/**
 * Reads a point from a stream
 */
RMDataStream &operator>>(RMDataStream &ds, RMPoint &p) {
	ds >> p.x >> p.y;
	return ds;
}

/****************************************************************************\
*       RMRect methods
\****************************************************************************/

RMRect::RMRect() {
	setEmpty();
}

void RMRect::setEmpty(void) {
	x1 = y1 = x2 = y2 = 0;
}

RMRect::RMRect(const RMPoint &p1, const RMPoint &p2) {
	setRect(p1, p2);
}

RMRect::RMRect(int X1, int Y1, int X2, int Y2) {
	setRect(X1, Y1, X2, Y2);
}

RMRect::RMRect(const RMRect &rc) {
	copyRect(rc);
}

void RMRect::setRect(const RMPoint &p1, const RMPoint &p2) {
	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;
}

void RMRect::setRect(int X1, int Y1, int X2, int Y2) {
	x1 = X1;
	y1 = Y1;
	x2 = X2;
	y2 = Y2;
}

void RMRect::setRect(const RMRect &rc) {
	copyRect(rc);
}

void RMRect::copyRect(const RMRect &rc) {
	x1 = rc.x1;
	y1 = rc.y1;
	x2 = rc.x2;
	y2 = rc.y2;
}

RMPoint &RMRect::topLeft() {
	// FIXME: This seems very bad
	return *((RMPoint *)this);
}

RMPoint &RMRect::bottomRight() {
	// FIXME: This seems very bad
	return *((RMPoint *)this + 1);
}

RMPoint RMRect::center() {
	return RMPoint((x2 - x1) / 2, (y2 - y1) / 2);
}

int RMRect::width() const {
	return x2 - x1;
}

int RMRect::height() const {
	return y2 - y1;
}

int RMRect::size() const {
	return width() * height();
}

bool RMRect::isEmpty() const {
	return (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0);
}

const RMRect &RMRect::operator=(const RMRect &rc) {
	copyRect(rc);
	return *this;
}

void RMRect::offset(int xOff, int yOff) {
	x1 += xOff;
	y1 += yOff;
	x2 += xOff;
	y2 += yOff;
}

void RMRect::offset(const RMPoint &p) {
	x1 += p.x;
	y1 += p.y;
	x2 += p.x;
	y2 += p.y;
}

const RMRect &RMRect::operator+=(RMPoint p) {
	offset(p);
	return *this;
}

const RMRect &RMRect::operator-=(RMPoint p) {
	offset(-p);
	return *this;
}

RMRect operator+(const RMRect &rc, RMPoint p) {
	RMRect r(rc);
	return (r += p);
}

RMRect operator-(const RMRect &rc, RMPoint p) {
	RMRect r(rc);

	return (r -= p);
}

RMRect operator+(RMPoint p, const RMRect &rc) {
	RMRect r(rc);

	return (r += p);
}

RMRect operator-(RMPoint p, const RMRect &rc) {
	RMRect r(rc);

	return (r += p);
}

bool RMRect::operator==(const RMRect &rc) {
	return ((x1 == rc.x1) && (y1 == rc.y1) && (x2 == rc.x2) && (y2 == rc.y2));
}

bool RMRect::operator!=(const RMRect &rc) {
	return ((x1 != rc.x1) || (y1 != rc.y1) || (x2 != rc.x2) || (y2 != rc.y2));
}

void RMRect::normalizeRect(void) {
	setRect(MIN(x1, x2), MIN(y1, y2), MAX(x1, x2), MAX(y1, y2));
}

RMDataStream &operator>>(RMDataStream &ds, RMRect &rc) {
	ds >> rc.x1 >> rc.y1 >> rc.x2 >> rc.y2;
	return ds;
}


/****************************************************************************\
*       Resource Update
\****************************************************************************/

RMResUpdate::RMResUpdate() {
	_infos = NULL;
}

RMResUpdate::~RMResUpdate() {
	if (_infos) {
		delete[] _infos;
		_infos = NULL;
	}

	if (_hFile.isOpen())
		_hFile.close();
}

void RMResUpdate::init(const Common::String &fileName) {
	// Open the resource update file
	if (!_hFile.open(fileName))
		// It doesn't exist, so exit immediately
		return;

	uint8 version;
	uint32 i;

	version = _hFile.readByte();
	_numUpd = _hFile.readUint32LE();

	_infos = new ResUpdInfo[_numUpd];

	// Load the index of the resources in the file
	for (i = 0; i < _numUpd; ++i) {
		ResUpdInfo &info = _infos[i];

		info.dwRes = _hFile.readUint32LE();
		info.offset = _hFile.readUint32LE();
		info.size = _hFile.readUint32LE();
		info.cmpSize = _hFile.readUint32LE();
	}
}

HGLOBAL RMResUpdate::queryResource(uint32 dwRes) {
	// If there isn't an update file, return NULL
	if (!_hFile.isOpen())
		return NULL;

	uint32 i;
	for (i = 0; i < _numUpd; ++i)
		if (_infos[i].dwRes == dwRes)
			// Found the index
			break;

	if (i == _numUpd)
		// Couldn't find a matching resource, so return NULL
		return NULL;

	const ResUpdInfo &info = _infos[i];
	byte *cmpBuf = new byte[info.cmpSize];
	uint32 dwRead;

	// Move to the correct offset and read in the compressed data
	_hFile.seek(info.offset);
	dwRead = _hFile.read(cmpBuf, info.cmpSize);

	if (info.cmpSize > dwRead) {
		// Error occurred reading data, so return NULL
		delete[] cmpBuf;
		return NULL;
	}

	// Allocate space for the output resource
	HGLOBAL destBuf = globalAllocate(0, info.size);
	byte *lpDestBuf = (byte *)globalLock(destBuf);
	uint32 dwSize;

	// Decompress the data
	lzo1x_decompress(cmpBuf, info.cmpSize, lpDestBuf, &dwSize);

	// Delete buffer for compressed data
	delete [] cmpBuf;

	// Return the resource
	globalUnlock(destBuf);
	return destBuf;
}

} // End of namespace Tony
