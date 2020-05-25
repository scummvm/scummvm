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

/*
 * This code is based on original Sfinx source code
 * Copyright (c) 1994-1997 Janusz B. Wisniewski and L.K. Avalon
 */

#ifndef CGE2_FILEIO_H
#define CGE2_FILEIO_H

#include "common/file.h"

namespace CGE2 {

class CGE2Engine;

#define kBtSize       2048
#define kBtKeySize    13
#define kBtLevel      2
#define kBtInnerCount ((kBtSize - 4 /*sizeof(Header) */) / (kBtKeySize + 2 /*sizeof(Inner) */))
#define kBtLeafCount  ((kBtSize - 4 /*sizeof(Header) */) / (kBtKeySize + 4 + 4 /*sizeof(BtKeypack) */))
#define kBtValNone    0xFFFF
#define kBtValRoot    0
#define kCatName      "VOL.CAT"
#define kDatName      "VOL.DAT"
#define kCryptSeed    0xA5

enum ID {
	kIdNear, kIdMTake, kIdFTake, kIdPhase, kIdSeq,
	kIdName, kIdType, kIdFront, kIdEast,
	kIdPortable, kIdTransparent,
	kIdNone = -1
};

struct BtKeypack {
	char _key[kBtKeySize];
	uint32 _pos;
	uint32 _size;
};

struct Inner {
	uint8  _key[kBtKeySize];
	uint16 _down;
};

struct Header {
	uint16 _count;
	uint16 _down;
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

	void readBTree(Common::ReadStream &s);
};

class ResourceManager {
private:
	struct {
		BtPage *_page;
		uint16 _pageNo;
		int _index;
	} _buff[kBtLevel];

	BtPage *getPage(int level, uint16 pageId);
	uint16 catRead(byte *buf, uint16 length);
	Common::File *_catFile;
	Common::File *_datFile;
	void xCrypt(byte *buf, uint16 length);
public:
	ResourceManager();
	~ResourceManager();
	uint16 read(byte *buf, uint16 length);
	bool seek(int32 offs, int whence = SEEK_SET);

	BtKeypack *find(const char *key);
	bool exist(const char *name);
};

class EncryptedStream {
private:
	CGE2Engine *_vm;
	Common::SeekableReadStream *_readStream;
	int _lineCount;
	bool _error;
public:
	EncryptedStream(CGE2Engine *vm, const char *name);
	~EncryptedStream();
	bool err();
	bool eos();
	bool seek(int32 offset);
	int32 pos();
	int32 size();
	uint32 read(byte *dataPtr, uint32 dataSize);
	int16 readSint16LE();
	uint32 readUint32LE();
	Common::String readLine();
	int getLineCount() { return _lineCount; }

	static const char *kIdTab[];
};

} // End of namespace CGE2

#endif // CGE2_FILEIO_H
