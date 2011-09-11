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
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#ifndef CGE_FILEIO_H
#define CGE_FILEIO_H

#include "cge/general.h"
#include "common/stream.h"

namespace CGE {

#define kBtSize       1024
#define kBtKeySize    13
#define kBtLevel      2
#define kBtInnerCount ((kBtSize - 4 /*sizeof(Hea) */) / (kBtKeySize + 2 /*sizeof(Inner) */))
#define kBtLeafCount  ((kBtSize - 4 /*sizeof(Hea) */) / (kBtKeySize + 4 + 2 /*sizeof(BtKeypack) */))
#define kBtValNone    0xFFFF
#define kBtValRoot    0
#define kLineMaxSize  512
#define kBufferSize   2048
#define kCatName      "VOL.CAT"
#define kDatName      "VOL.DAT"

struct BtKeypack {
	char _key[kBtKeySize];
	uint32 _mark;
	uint16 _size;
};

struct Inner {
	uint8  _key[kBtKeySize];
	uint16 _down;
};

struct Header {
	uint16 _count;
	uint16 _down;
};

class IoHand {
protected:
	uint16 _seed;
	Crypt *_crypt;
public:
	Common::File *_file;
	uint16 _error;

	IoHand(const char *name, Crypt crypt);
	IoHand(Crypt *crypt);
	virtual ~IoHand();
	uint16 read(void *buf, uint16 len);
	long mark();
	long size();
	long seek(long pos);
};

class IoBuf : public IoHand {
protected:
	uint8 *_buff;
	uint16 _ptr;
	uint16 _lim;
	long _bufMark;
	virtual void readBuf();
public:
	IoBuf(Crypt *crpt);
	IoBuf(const char *name, Crypt *crpt);
	virtual ~IoBuf();
	uint16 read(void *buf, uint16 len);
	uint16 read(uint8 *buf);
	int read();
};


class CFile : public IoBuf {
public:
	CFile(const char *name, Crypt *crpt);
	virtual ~CFile();
	long mark();
	long seek(long pos);
};

struct BtPage {
	Header _header;
	union {
		// dummy filler to make proper size of union
		uint8 _data[kBtSize - 4]; /* 4 is the size of struct Header */
		// inner version of data: key + word-sized page link
		Inner _inner[kBtInnerCount];
		// leaf version of data: key + all user data
		BtKeypack _leaf[kBtLeafCount];
	};

	void read(Common::ReadStream &s);
};

class BtFile : public IoHand {
	struct {
		BtPage *_page;
		uint16 _pgNo;
		int _indx;
	} _buff[kBtLevel];

	BtPage *getPage(int lev, uint16 pgn);
public:
	BtFile(const char *name, Crypt *crpt);
	virtual ~BtFile();
	BtKeypack *find(const char *key);
	bool exist(const char *name);
};

class EncryptedStream {
private:
	Common::SeekableReadStream *_readStream;
	bool _error;
public:
	EncryptedStream(const char *name);
	~EncryptedStream();
	bool err();
	bool eos();
	bool seek(int32 offset);
	int32 pos();
	int32 size();
	uint32 read(void *dataPtr, uint32 dataSize);
	Common::String readLine();
};

extern CFile *_dat;
extern BtFile *_cat;

} // End of namespace CGE

#endif
