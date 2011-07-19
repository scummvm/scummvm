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

#ifndef __CGE_CFILE__
#define __CGE_CFILE__

#include "cge/general.h"

namespace CGE {

#define LINE_MAX    512
#define IOBUF_SIZE  2048

#define CFREAD(x)   read((uint8 *)(x),sizeof(*(x)))


class IoBuf : public IoHand {
protected:
	uint8 *_buff;
	uint16 _ptr;
	uint16 _lim;
	long _bufMark;
	uint16 _seed;
	CRYPT *_crypt;
	virtual void readBuf();
	virtual void writeBuf();
public:
	IoBuf(IOMODE mode, CRYPT *crpt = NULL);
	IoBuf(const char *name, IOMODE mode, CRYPT *crpt = NULL);
	virtual ~IoBuf();
	uint16 read(void *buf, uint16 len);
	uint16 read(uint8 *buf);
	int read();
	uint16 write(void *buf, uint16 len);
	uint16 write(uint8 *buf);
	void write(uint8 b);
};


class CFile : public IoBuf {
public:
	static uint16 _maxLineLen;
	CFile(const char *name, IOMODE mode = REA, CRYPT *crpt = NULL);
	virtual ~CFile();
	void flush();
	long mark();
	long seek(long pos);
	void append(CFile &f);
};

} // End of namespace CGE

#endif
