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

#define kLineMaxSize  512
#define kBufferSize   2048

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
	static uint16 _maxLineLen;
	CFile(const char *name, Crypt *crpt);
	virtual ~CFile();
	long mark();
	long seek(long pos);
};

} // End of namespace CGE

#endif
