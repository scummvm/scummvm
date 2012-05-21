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
	m_string = NULL;
	m_length = 0;
	m_realLength = 0;
}

/**
 * Destructor
 */
RMString::~RMString() {
	if (m_string != NULL)
		delete[] m_string;
}

/**
 * Copy constructor
 */
RMString::RMString(const RMString &str) {
	// Richiama l'overload su '=' per copiare
	m_string = NULL;
	m_length = 0;
	m_realLength = 0;
	*this = str;
}

/**
 * Constructor from a char *
 */
RMString::RMString(const char *str) {
	// Use the overloaded '=' when copying
	m_string = NULL;
	m_length = 0;
	m_realLength = 0;
	*this = str;
}

/**
 * Constructor with a single passed character
 */
RMString::RMString(const int ch) {
	// Use the overloaded '=' when copying
	m_string = NULL;
	m_length = 0;
	m_realLength = 0;
	*this = ch;
}

/**
 * Returns the length of the string
 * @returns                 Length
 */
int RMString::Length() const {
	return m_length;
}

/**
 * Gets the character at the given index
 * @param nIndex            Position of the character to return
 * @returns                 Character
 */
char RMString::GetAt(int nIndex) {
	assert(nIndex < m_length);
	return m_string[nIndex];
}

/**
 * Sets the character at the given index
 * @param nIndex            Position of the character to change
 * @param c                 Character
 */
void RMString::SetAt(int nIndex, char c) {
	assert(nIndex < m_length);
	m_string[nIndex] = c;
}

/**
 * Overloaded square brackets operator for accessing characters within the string
 * @param nIndex            Position of the charactre to reference
 * @params                  Reference to the character
 */
char &RMString::operator[](int nIndex) {
	assert(nIndex < m_length);
	return m_string[nIndex];
}

/**
 * Copies a string
 * @param str               String to copy
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator=(const RMString &str) {
	// Set the new length
	m_length = str.m_length;

	// If the source is empty, then destroy the current string buffer
	if (m_length == 0) {
		if (m_realLength > 0) {
			delete[] m_string;
			m_string = NULL;
			m_realLength = 0;
		}
	} else {
		// Resize if necessary
		Resize(m_length + 1);

		// Copy the string
		Common::copy(str.m_string, str.m_string + m_length + 1, m_string);
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
		if (m_realLength > 0) {
			delete[] m_string;
			m_string = NULL;
			m_realLength = m_length = 0;
		}
	} else {
		// Calculate the new length
		m_length = strlen(str);

		// Resize if necessary
		Resize(m_length + 1);

		// Copy the string
		Common::copy(str, str + m_length + 1, m_string);
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
		if (m_realLength > 0) {
			delete [] m_string;
			m_string = NULL;
			m_length = m_realLength = 0;
		}
	} else {
		// Resize if necessary
		Resize(2);

		m_string[0] = ch;
		m_string[1] = '\0';
		m_length = 1;
	}

	return *this;
}

/**
 * Concatenate a string into the current one
 * @param str               String to concatenate
 * @param size              Length of the string
 */
void RMString::Connect(const char *str, int size) {
	int nlen;

	if (size > 0) {
		// Calculate the new lenght
		nlen = m_length + size;

		// Resize
		Resize(nlen + 1, true);

		// Linkage with '\0'
		Common::copy(str, str + size + 1, m_string + m_length);

		// Save the new length
		m_length = nlen;
	}
}

/**
 * Concatenate a string
 * @param str               String to concatenate
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator+=(RMString &str) {
	Connect(str, str.Length());
	return *this;
}

/**
 * Concatenate a string
 * @param str               String to concatenate
 * @returns                 Refrence to our string
 */
const RMString &RMString::operator+=(const char *str) {
	Connect(str, strlen(str));
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

	Connect(str, 1);
	return *this;
}

/**
 * Casts a string as char *
 * @returns                 char * reference to string
 */
RMString::operator char *() const {
	return m_string;
}

/**
 * Resize a string as necessary
 * @param size              New size necessary (in bytes)
 * @param bMaintain         If true we must keep the original string,
                            if false we can destroy.
 */
void RMString::Resize(int size, bool bMantain) {
	if (m_realLength == 0) {
		m_string = new char[size];
		m_realLength = size;
	} else if (size > m_realLength) {
		if (bMantain) {
			char *app;

			app = new char[size];
			Common::copy(m_string, m_string + m_length + 1, app);
			delete[] m_string;
			m_string = app;
		} else {
			delete[] m_string;
			m_string = new char[size];
		}
	}
}

/**
 * Compacts the string to occupy less memory if possible.
 */
void RMString::Compact(void) {
	if (m_realLength + 1 > m_length) {
		char *app;

		app = new char[m_length + 1];
		Common::copy(m_string, m_string + m_length + 1, app);

		delete[] m_string;
		m_string = app;
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
	var.Resize(len + 1);
	var.m_length = len + 1;

	for (i = 0; i < len; i++)
		df >> var[i];

	var[i] = '\0';
	var.m_length = len;

	return df;
}

/**
 * Formats a string
 */
void RMString::Format(const char *str, ...) {
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
	Close();
}

void RMFileStreamSlow::Close() {
	delete _stream;
}

bool RMFileStreamSlow::OpenFile(Common::File &file) {
	_stream = file.readStream(file.size());

	m_length = _stream->pos();

	return true;
}


bool RMFileStreamSlow::OpenFile(const char *lpFN) {
	// Open file for reading
	Common::File f;
	if (!f.open(lpFN))
		return false;

	m_length = f.size();
	_stream = f.readStream(f.size());

	return true;
}


RMDataStream &RMFileStreamSlow::operator+=(int nBytes) {
	Seek(nBytes);
	return *this;
}

int RMFileStreamSlow::Pos() {
	return _stream->pos();
}

bool RMFileStreamSlow::IsEOF() {
	return (Pos() >= m_length);
}


int RMFileStreamSlow::Seek(int nBytes, RMDSPos where) {
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


bool RMFileStreamSlow::Read(void *buf, int size) {
	uint32 dwRead;

	dwRead = _stream->read(buf, size);
	return ((int)dwRead == size);
}


RMFileStreamSlow &operator>>(RMFileStreamSlow &df, char &var) {
	df.Read(&var, 1);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, byte &var) {
	df.Read(&var, 1);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, uint16 &var) {
	uint16 v;
	df.Read(&v, 2);
	v = FROM_LE_16(v);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, int16 &var) {
	uint16 v;
	df.Read(&v, 2);
	var = (int16)FROM_LE_16(v);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, int &var) {
	int v;
	df.Read(&v, 4);
	var = FROM_LE_32(v);
	return df;
}

RMFileStreamSlow &operator>>(RMFileStreamSlow &df, uint32 &var) {
	uint32 v;
	df.Read(&v, 4);
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
	m_length = 0;
	m_pos = 0;
	m_bError = false;
}

/**
 * Destructor
 */
RMDataStream::~RMDataStream() {
	Close();
}

/**
 * Close a stream
 */
void RMDataStream::Close(void) {
	m_length = 0;
	m_pos = 0;
}

/**
 * Takes the address of the buffer from which will be read the data.
 * @param lpBuf         Data buffer
 * @param size          Size of the buffer
 * @remarks             If the length of the buffer is not known, and cannot be
 *                      specified, then EOF() and Seek() to end won't work.
 */
void RMDataStream::OpenBuffer(const byte *lpBuf, int size) {
	m_length = size;
	m_buf = lpBuf;
	m_bError = false;
	m_pos = 0;
}

/**
 * Returns the length of the stream
 * @returns             Stream length
 */
int RMDataStream::Length() {
	return m_length;
}

/**
 * Determines if the end of the stream has been reached
 * @returns             true if end of stream reached, false if not
 */
bool RMDataStream::IsEOF() {
	return (m_pos >= m_length);
}

/**
 * Extracts data from the stream
 * @param df                Stream
 * @param var               Variable of a supported type
 * @returns                 Value read from the stream
 */
RMDataStream &operator>>(RMDataStream &df, char &var) {
	df.Read(&var, 1);
	return df;
}

/**
 * Extracts data from the stream
 * @param df                Stream
 * @param var               Variable of a supported type
 * @returns                 Value read from the stream
 */
RMDataStream &operator>>(RMDataStream &df, uint8 &var) {
	df.Read(&var, 1);
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
	df.Read(&v, 2);

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
	df.Read(&v, 2);

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
	df.Read(&v, 4);

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
	df.Read(&v, 4);

	var = FROM_LE_32(v);
	return df;
}

/**
 * Reads a series of data from the stream in a buffer
 * @param lpBuf             Data buffer
 * @param size              Size of the buffer
 * @returns                 true if we have reached the end, false if not
 */
bool RMDataStream::Read(void *lpBuf, int size) {
	byte *dest = (byte *)lpBuf;

	if ((m_pos + size) > m_length) {
		Common::copy(m_buf + m_pos, m_buf + m_pos + (m_length - m_pos), dest);

		return true;
	} else {
		Common::copy(m_buf + m_pos, m_buf + m_pos + size, dest);

		m_pos += size;
		return false;
	}
}

/**
 * Skips a number of bytes in the stream
 * @param nBytres           Number of bytes to skip
 * @returns                 The stream
 */
RMDataStream &RMDataStream::operator+=(int nBytes) {
	m_pos += nBytes;
	return *this;
}

/**
 * Seeks to a position within the stream
 * @param nBytes            Number of bytes from specified origin
 * @param origin            Origin to do offset from
 * @returns                 The absolute current position in bytes
 */
int RMDataStream::Seek(int nBytes, RMDSPos origin) {
	switch (origin) {
	case CUR:
		break;

	case START:
		m_pos = 0;
		break;

	case END:
		if (m_length == SIZENOTKNOWN)
			return m_pos;
		m_pos = m_length;
		break;
	}

	m_pos += nBytes;
	return m_pos;
}

/**
 * Returns the current position of the stream
 * @returns                 The current position
 */
int RMDataStream::Pos() {
	return m_pos;
}

/**
 * Check if an error occurred during reading the stream
 * @returns                 true if there was an error, false otherwise
 */
bool RMDataStream::IsError() {
	return m_bError;
}

/**
 * Sets an error code for the stream
 * @param code              Error code
 */
void RMDataStream::SetError(int code) {
	m_bError = true;
	m_ecode = code;
}

/**
 * Returns the error code for the stream
 * @returns                 Error code
 */
int RMDataStream::GetError() {
	return m_ecode;
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
void RMPoint::Offset(const RMPoint &p) {
	x += p.x;
	y += p.y;
}

/**
 * Offsets the point by a specified offset
 */
void RMPoint::Offset(int xOff, int yOff) {
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
	Offset(p);
	return *this;
}

/**
 * Subtract (offset) of a point
 */
RMPoint &RMPoint::operator-=(RMPoint p) {
	Offset(-p);
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
	SetEmpty();
}

void RMRect::SetEmpty(void) {
	x1 = y1 = x2 = y2 = 0;
}

RMRect::RMRect(const RMPoint &p1, const RMPoint &p2) {
	SetRect(p1, p2);
}

RMRect::RMRect(int X1, int Y1, int X2, int Y2) {
	SetRect(X1, Y1, X2, Y2);
}

RMRect::RMRect(const RMRect &rc) {
	CopyRect(rc);
}

void RMRect::SetRect(const RMPoint &p1, const RMPoint &p2) {
	x1 = p1.x;
	y1 = p1.y;
	x2 = p2.x;
	y2 = p2.y;
}

void RMRect::SetRect(int X1, int Y1, int X2, int Y2) {
	x1 = X1;
	y1 = Y1;
	x2 = X2;
	y2 = Y2;
}

void RMRect::SetRect(const RMRect &rc) {
	CopyRect(rc);
}

void RMRect::CopyRect(const RMRect &rc) {
	x1 = rc.x1;
	y1 = rc.y1;
	x2 = rc.x2;
	y2 = rc.y2;
}

RMPoint &RMRect::TopLeft() {
	// FIXME: This seems very bad
	return *((RMPoint *)this);
}

RMPoint &RMRect::BottomRight() {
	// FIXME: This seems very bad
	return *((RMPoint *)this + 1);
}

RMPoint RMRect::Center() {
	return RMPoint((x2 - x1) / 2, (y2 - y1) / 2);
}

int RMRect::Width() const {
	return x2 - x1;
}

int RMRect::Height() const {
	return y2 - y1;
}

int RMRect::Size() const {
	return Width() * Height();
}

bool RMRect::IsEmpty() const {
	return (x1 == 0 && y1 == 0 && x2 == 0 && y2 == 0);
}

const RMRect &RMRect::operator=(const RMRect &rc) {
	CopyRect(rc);
	return *this;
}

void RMRect::Offset(int xOff, int yOff) {
	x1 += xOff;
	y1 += yOff;
	x2 += xOff;
	y2 += yOff;
}

void RMRect::Offset(const RMPoint &p) {
	x1 += p.x;
	y1 += p.y;
	x2 += p.x;
	y2 += p.y;
}

const RMRect &RMRect::operator+=(RMPoint p) {
	Offset(p);
	return *this;
}

const RMRect &RMRect::operator-=(RMPoint p) {
	Offset(-p);
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

void RMRect::NormalizeRect(void) {
	SetRect(MIN(x1, x2), MIN(y1, y2), MAX(x1, x2), MAX(y1, y2));
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

void RMResUpdate::Init(const Common::String &fileName) {
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

HGLOBAL RMResUpdate::QueryResource(uint32 dwRes) {
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
	HGLOBAL destBuf = GlobalAllocate(info.size);
	byte *lpDestBuf = (byte *)GlobalLock(destBuf);
	uint32 dwSize;

	// Decompress the data
	lzo1x_decompress(cmpBuf, info.cmpSize, lpDestBuf, &dwSize);

	// Delete buffer for compressed data
	delete [] cmpBuf;

	// Return the resource
	GlobalUnlock(destBuf);
	return lpDestBuf;
}

} // End of namespace Tony
