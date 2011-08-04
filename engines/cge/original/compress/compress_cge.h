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

#ifndef __COMPRESS_CGE_H
#define __COMPRESS_CGE_H

typedef unsigned char byte;
typedef unsigned char uint8;
typedef unsigned short int uint16;
typedef unsigned long int uint32;

#pragma pack(1)

/**
 * The following defines are copied from the cge engine file btfile.h
 */
#define kBtSize      1024
#define kBtKeySize   13
#define kBtLevel     2


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

#define INN_SIZE ((kBtSize - sizeof(Hea)) / sizeof(Inner))
#define LEA_SIZE ((kBtSize - sizeof(Hea)) / sizeof(BtKeypack))

struct BtPage {
	Hea _hea;
	union {
		// dummy filler to make proper size of union
		uint8 _data[kBtSize - sizeof(Hea)];
		// inner version of data: key + word-sized page link
		Inner _inn[(kBtSize - sizeof(Hea)) / sizeof(Inner)];
		// leaf version of data: key + all user data
		BtKeypack _lea[(kBtSize - sizeof(Hea)) / sizeof(BtKeypack)];
	};
};

#pragma pack()

#endif
