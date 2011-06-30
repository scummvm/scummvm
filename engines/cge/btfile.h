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

#ifndef __BTFILE__
#define __BTFILE__

#include "cge/general.h"

namespace CGE {

#define BT_SIZE     K(1)
#define BT_KEYLEN   13
#define BT_LEVELS   2

#define BT_NONE     0xFFFF
#define BT_ROOT     0

#include "common/pack-start.h"	// START STRUCT PACKING

struct BtKeypack {
	char _key[BT_KEYLEN];
	uint32 _mark;
	uint16 _size;
};

struct Inner {
	uint8 _key[BT_KEYLEN];
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
		uint8 _data[BT_SIZE - sizeof(Hea)];
		// inner version of data: key + word-sized page link
		Inner _inn[(BT_SIZE - sizeof(Hea)) / sizeof(Inner)];
		// leaf version of data: key + all user data
		BtKeypack _lea[(BT_SIZE - sizeof(Hea)) / sizeof(BtKeypack)];
	};
};

#include "common/pack-end.h"	// END STRUCT PACKING


class BtFile : public IoHand {
	struct {
		BtPage *_page;
		uint16 _pgNo;
		int _indx;
		bool _updt;
	} _buff[BT_LEVELS];

	void putPage(int lev, bool hard = false);
	BtPage *getPage(int lev, uint16 pgn);
public:
	BtFile(const char *name, IOMODE mode = REA, CRYPT *crpt = NULL);
	virtual ~BtFile();
	BtKeypack *find(const char *key);
	BtKeypack *next();
	void make(BtKeypack *keypack, uint16 count);
};

} // End of namespace CGE

#endif
