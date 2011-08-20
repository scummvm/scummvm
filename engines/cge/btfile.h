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

#ifndef __CGE_BTFILE__
#define __CGE_BTFILE__

#include "cge/general.h"
#include "common/stream.h"

namespace CGE {

#define kBtSize      1024
#define kBtKeySize   13
#define kBtLevel     2
#define kBtInnerCount ((kBtSize - 4 /*sizeof(Hea) */) / (kBtKeySize + 2 /*sizeof(Inner) */))
#define kBtLeafCount ((kBtSize - 4 /*sizeof(Hea) */) / (kBtKeySize + 4 + 2 /*sizeof(BtKeypack) */))

#define kBtValNone   0xFFFF
#define kBtValRoot   0

struct BtKeypack {
	char _key[kBtKeySize];
	uint32 _mark;
	uint16 _size;
};

struct Inner {
	uint8 _key[kBtKeySize];
	uint16 _down;
};

struct Hea {
	uint16 _count;
	uint16 _down;
};

struct BtPage {
	Hea _hea;
	union {
		// dummy filler to make proper size of union
		uint8 _data[kBtSize - 4 /*sizeof(Hea) */];
		// inner version of data: key + word-sized page link
		Inner _inn[kBtInnerCount];
		// leaf version of data: key + all user data
		BtKeypack _lea[kBtLeafCount];
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
	BtKeypack *next();
};

} // End of namespace CGE

#endif
