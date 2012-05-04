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
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Utils.CPP............  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c                                  *
 *  z$$beu     .ue="  $  "=e..    .zed$$c                                 *
 *      "#$e z$*"   .  `.   ^*Nc e$""                                     *
 *         "$$".  .r"   ^4.  .^$$"                                        *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

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
	const byte *m_buf;
    int m_length;
    int m_pos;
    bool m_bError;
    int m_ecode;

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
    void OpenBuffer(const byte *buf, int size = SIZENOTKNOWN);
	void Close(void);

    // Attributi
    int Length();
    virtual int Pos();

    // EOF
    virtual bool IsEOF();

    // Read methods
    friend RMDataStream &operator>>(RMDataStream &df, char &var);
    friend RMDataStream &operator>>(RMDataStream &df, byte &var);
    friend RMDataStream &operator>>(RMDataStream &df, uint16 &var);
    friend RMDataStream &operator>>(RMDataStream &df, int16 &var);
    friend RMDataStream &operator>>(RMDataStream &df, int &var);
    friend RMDataStream &operator>>(RMDataStream &df, uint32 &var);

    // Lettura generica
    virtual bool Read(void *buf, int size);

    // Skipping & Seeking
    virtual RMDataStream &operator+=(int nBytes);
    virtual int Seek(int nBytes, RMDSPos origin = CUR);

    // Gestione errori
    void SetError(int ecode);
    int GetError();
    bool IsError();
};


/**
 * Data stream per lettura di dati aperto da file
 */
class RMFileStream : public RMDataStream {
private:
	byte *m_buf;

public:
	RMFileStream();
	virtual ~RMFileStream();

	// Apre lo stream da file
	bool OpenFile(const char *lpFN);
	bool OpenFile(Common::File &file);

	void Close(void);
};


class RMFileStreamSlow : public RMDataStream {
private:
	Common::SeekableReadStream *_stream;
public:
	RMFileStreamSlow();
	virtual ~RMFileStreamSlow();

	bool OpenFile(const char *lpFN);
	bool OpenFile(Common::File &file);

	void Close(void);

    RMDataStream& operator+=(int nBytes);
    int Seek(int nBytes, RMDSPos where = CUR);

	int Pos();
	virtual bool IsEOF();

	bool Read(void *buf, int size);

    friend RMFileStreamSlow& operator>>(RMFileStreamSlow &df, char &var);
    friend RMFileStreamSlow& operator>>(RMFileStreamSlow &df, byte &var);
    friend RMFileStreamSlow& operator>>(RMFileStreamSlow &df, uint16 &var);
    friend RMFileStreamSlow& operator>>(RMFileStreamSlow &df, int16 &var);
    friend RMFileStreamSlow& operator>>(RMFileStreamSlow &df, int &var);
    friend RMFileStreamSlow& operator>>(RMFileStreamSlow &df, uint32 &var);
};

/**
 * String class
 */
class RMString {
private:
	char *m_string;
	int m_length;
    int m_realLength;

public:
    RMString();
    ~RMString();

    // Assignment constructors
    RMString(const RMString &str);
    RMString(const char *str);
    RMString(const int ch);

    // Metodi generici
    int Length();
    void Compact();

    // Access characters within string
    char GetAt(int nIndex);
    void SetAt(int nIndex, char c);
    char& operator[](int nIndex);

    // String cast
    operator char*() const;

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

    friend RMString operator+(RMString& str, const int ch);
    friend RMString operator+(const int ch, RMString &str);

    friend RMString operator+(RMString &str, const char *s);
    friend RMString operator+(const char *s, RMString &str);

    // Estrazione da data stream
    friend RMDataStream& operator>>(RMDataStream& df, RMString &var);

	// Formattazione di stringa
	void Format(const char *str, ...);

private:
    void Resize(int size, bool bMantain = false);
    void Connect(const char* str, int size);
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

    // Copia
    RMPoint& operator=(RMPoint p);

	// Set
	void Set(int x1, int y1) { x=x1; y=y1; }

    // Offset
    void Offset(int xOff, int yOff);
    void Offset(RMPoint p);
    friend RMPoint operator+(RMPoint p1, RMPoint p2);
    friend RMPoint operator-(RMPoint p1, RMPoint p2);
    RMPoint &operator+=(RMPoint p);
    RMPoint &operator-=(RMPoint p);
    RMPoint operator-();

    // Confronti
    bool operator==(RMPoint p);
    bool operator!=(RMPoint p);

    // Casting a POINT
	operator Common::Point() const;

    // Extraction from data streams
    friend RMDataStream& operator>>(RMDataStream &ds, RMPoint &p);
};

class RMRect {
public:
	int x1,y1;
    int x2,y2;

public:
    RMRect();
    RMRect(int x1, int y1, int x2, int y2);
    RMRect(RMPoint p1, RMPoint p2);
    RMRect(const RMRect &rc);

    // Attributes
    RMPoint &TopLeft();
    RMPoint &BottomRight();
    RMPoint Center();
    int Width() const;
    int Height() const;
    bool IsEmpty() const;
	int Size() const;

    // Set
    void SetRect(int x1, int y1, int x2, int y2);
    void SetRect(RMPoint p1, RMPoint p2);
    void SetEmpty(void);

    // Copiers
    void SetRect(const RMRect &rc);
    void CopyRect(const RMRect &rc);
    const RMRect &operator=(const RMRect &rc);

    // Offset
    void Offset(int xOff, int yOff);
    void Offset(RMPoint p);
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
    void NormalizeRect();

	// Point in rect
	bool PtInRect(RMPoint pt) { return (pt.x>=x1 && pt.x<=x2 && pt.y>=y1 && pt.y<=y2); }

    // Extract from data stream
    friend RMDataStream &operator>>(RMDataStream& ds, RMRect &rc);
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

	void Init(const Common::String &fileName);
	HGLOBAL QueryResource(uint32 dwRes);
};

} // End of namespace Tony

#endif /* TONY_H */
