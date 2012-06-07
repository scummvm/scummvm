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

#ifndef TONY_UTILS_H
#define TONY_UTILS_H

#include "common/scummsys.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"
#include "tony/mpal/memory.h"

namespace Tony {

using namespace ::Tony::MPAL;

/**
 * Data stream for reading data
 */
class RMDataStream {
protected:
	const byte *_buf;
	int _length;
	int _pos;
	bool _bError;
	int _ecode;

public:
	enum RMDSPos {
		CUR,
		START,
		END
	};

private:
	enum {
		SIZENOTKNOWN = 0x7FFFFFFF
	};

public:
	// Constructor and destructor
	RMDataStream();
	virtual ~RMDataStream();

	// Loading buffer
	void openBuffer(const byte *buf, int size = SIZENOTKNOWN);
	void close(void);

	// Attributei
	int length();
	virtual int pos();

	// EOF
	virtual bool isEOF();

	// Read methods
	friend RMDataStream &operator>>(RMDataStream &df, char &var);
	friend RMDataStream &operator>>(RMDataStream &df, byte &var);
	friend RMDataStream &operator>>(RMDataStream &df, uint16 &var);
	friend RMDataStream &operator>>(RMDataStream &df, int16 &var);
	friend RMDataStream &operator>>(RMDataStream &df, int &var);
	friend RMDataStream &operator>>(RMDataStream &df, uint32 &var);

	// General read
	virtual bool read(void *buf, int size);

	// Skipping & Seeking
	virtual RMDataStream &operator+=(int nBytes);
	virtual int seek(int nBytes, RMDSPos origin = CUR);

	// Error handling
	void setError(int ecode);
	int getError();
	bool isError();
};


/**
 * Data stream per lettura di dati aperto da file
 */
class RMFileStream : public RMDataStream {
private:
	byte *_buf;

public:
	RMFileStream();
	virtual ~RMFileStream();

	// Methods for opening file
	bool openFile(const char *lpFN);
	bool openFile(Common::File &file);

	void close(void);
};


class RMFileStreamSlow : public RMDataStream {
private:
	Common::SeekableReadStream *_stream;
public:
	RMFileStreamSlow();
	virtual ~RMFileStreamSlow();

	bool openFile(const char *lpFN);
	bool openFile(Common::File &file);

	void close(void);

	RMDataStream &operator+=(int nBytes);
	int seek(int nBytes, RMDSPos where = CUR);

	int pos();
	virtual bool isEOF();

	bool read(void *buf, int size);

	friend RMFileStreamSlow &operator>>(RMFileStreamSlow &df, char &var);
	friend RMFileStreamSlow &operator>>(RMFileStreamSlow &df, byte &var);
	friend RMFileStreamSlow &operator>>(RMFileStreamSlow &df, uint16 &var);
	friend RMFileStreamSlow &operator>>(RMFileStreamSlow &df, int16 &var);
	friend RMFileStreamSlow &operator>>(RMFileStreamSlow &df, int &var);
	friend RMFileStreamSlow &operator>>(RMFileStreamSlow &df, uint32 &var);
};

/**
 * String class
 */
class RMString {
private:
	char *_string;
	int _length;
	int _realLength;

public:
	RMString();
	~RMString();

	// Assignment constructors
	RMString(const RMString &str);
	RMString(const char *str);
	RMString(const int ch);

	// General methods
	int length() const;
	void compact();

	// Access characters within string
	char getAt(int nIndex);
	void setAt(int nIndex, char c);
	char &operator[](int nIndex);

	// String cast
	operator char *() const;

	// String assignments
	const RMString &operator=(const RMString &str);
	const RMString &operator=(const char *str);
	const RMString &operator=(const int ch);

	// String concatenation
	const RMString &operator+=(RMString &str);
	const RMString &operator+=(const char *str);
	const RMString &operator+=(const int ch);

	// Concatentation of string or character
	friend RMString operator+(const RMString &str1, const RMString &str2);

	friend RMString operator+(RMString &str, const int ch);
	friend RMString operator+(const int ch, RMString &str);

	friend RMString operator+(RMString &str, const char *s);
	friend RMString operator+(const char *s, RMString &str);

	// Extraction from data streams
	friend RMDataStream &operator>>(RMDataStream &df, RMString &var);

	// String formatting
	void format(const char *str, ...);

private:
	void resize(int size, bool bMantain = false);
	void connect(const char *str, int size);
};

/**
 * Point class
 */
class RMPoint {
public:
	int x, y;

public:
	// Constructor
	RMPoint();
	RMPoint(const RMPoint &p);
	RMPoint(int x1, int y1);

	// Copy
	RMPoint &operator=(RMPoint p);

	// Set
	void set(int x1, int y1) {
		x = x1;
		y = y1;
	}

	// Offset
	void offset(int xOff, int yOff);
	void offset(const RMPoint &p);
	friend RMPoint operator+(RMPoint p1, RMPoint p2);
	friend RMPoint operator-(RMPoint p1, RMPoint p2);
	RMPoint &operator+=(RMPoint p);
	RMPoint &operator-=(RMPoint p);
	RMPoint operator-();

	// Comparison
	bool operator==(RMPoint p);
	bool operator!=(RMPoint p);

	// Casting a POINT
	operator Common::Point() const;

	// Extraction from data streams
	friend RMDataStream &operator>>(RMDataStream &ds, RMPoint &p);
};

class RMRect {
public:
	int x1, y1;
	int x2, y2;

public:
	RMRect();
	RMRect(int x1, int y1, int x2, int y2);
	RMRect(const RMPoint &p1, const RMPoint &p2);
	RMRect(const RMRect &rc);

	// Attributes
	RMPoint &topLeft();
	RMPoint &bottomRight();
	RMPoint center();
	int width() const;
	int height() const;
	bool isEmpty() const;
	int size() const;

	// Set
	void setRect(int x1, int y1, int x2, int y2);
	void setRect(const RMPoint &p1, const RMPoint &p2);
	void setEmpty(void);

	// Copiers
	void setRect(const RMRect &rc);
	void copyRect(const RMRect &rc);
	const RMRect &operator=(const RMRect &rc);

	// Offset
	void offset(int xOff, int yOff);
	void offset(const RMPoint &p);
	friend RMRect operator+(const RMRect &rc, RMPoint p);
	friend RMRect operator-(const RMRect &rc, RMPoint p);
	friend RMRect operator+(RMPoint p, const RMRect &rc);
	friend RMRect operator-(RMPoint p, const RMRect &rc);
	const RMRect &operator+=(RMPoint p);
	const RMRect &operator-=(RMPoint p);

	// Comparison
	bool operator==(const RMRect &rc);
	bool operator!=(const RMRect &rc);

	// Normalise
	void normalizeRect();

	// Point in rect
	bool ptInRect(const RMPoint &pt) {
		return (pt.x >= x1 && pt.x <= x2 && pt.y >= y1 && pt.y <= y2);
	}

	// Extract from data stream
	friend RMDataStream &operator>>(RMDataStream &ds, RMRect &rc);
};

/**
 * Resource update manager
 */
class RMResUpdate {
	struct ResUpdInfo {
		uint32 dwRes;
		uint32 offset;
		uint32 size;
		uint32 cmpSize;
	};

	uint32 _numUpd;
	ResUpdInfo *_infos;
	Common::File _hFile;

public:
	RMResUpdate();
	~RMResUpdate();

	void init(const Common::String &fileName);
	HGLOBAL queryResource(uint32 dwRes);
};

} // End of namespace Tony

#endif /* TONY_H */
