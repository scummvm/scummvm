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

#include "cge/cfile.h"
#include "common/system.h"
#include "cge/cge.h"
#include "common/debug.h"
#include "common/debug-channels.h"

namespace CGE {

IoBuf::IoBuf(Crypt *crypt)
	: IoHand(crypt),
	  _bufMark(0),
	  _ptr(0),
	  _lim(0) {
	debugC(1, kCGEDebugFile, "IoBuf::IoBuf(crypt)");

	_buff = (uint8 *) malloc(sizeof(uint8) * kBufferSize);
	assert(_buff != NULL);
}

IoBuf::IoBuf(const char *name, Crypt *crypt)
	: IoHand(name, crypt),
	  _bufMark(0),
	  _ptr(0),
	  _lim(0) {
	debugC(1, kCGEDebugFile, "IoBuf::IoBuf(%s, %d, crypt)", name);

	_buff = (uint8 *) malloc(sizeof(uint8) * kBufferSize);
	assert(_buff != NULL);
}

IoBuf::~IoBuf() {
	debugC(6, kCGEDebugFile, "IoBuf::~IoBuf()");
	free(_buff);
}

void IoBuf::readBuf() {
	debugC(4, kCGEDebugFile, "IoBuf::readBuf()");

	_bufMark = IoHand::mark();
	_lim = IoHand::read(_buff, kBufferSize);
	_ptr = 0;
}

uint16 IoBuf::read(void *buf, uint16 len) {
	debugC(4, kCGEDebugFile, "IoBuf::read(buf, %d)", len);

	uint16 total = 0;
	while (len) {
		if (_ptr >= _lim)
			readBuf();
		uint16 n = _lim - _ptr;
		if (n) {
			if (len < n)
				n = len;
			memcpy(buf, _buff + _ptr, n);
			buf = (uint8 *)buf + n;
			len -= n;
			total += n;
			_ptr += n;
		} else
			break;
	}
	return total;
}

uint16 IoBuf::read(uint8 *buf) {
	debugC(3, kCGEDebugFile, "IoBuf::read(buf)");

	uint16 total = 0;

	while (total < kLineMaxSize - 2) {
		if (_ptr >= _lim)
			readBuf();
		uint8 *p = _buff + _ptr;
		uint16 n = _lim - _ptr;
		if (n) {
			if (total + n >= kLineMaxSize - 2)
				n = kLineMaxSize - 2 - total;
			uint8 *eol = (uint8 *) memchr(p, '\r', n);
			if (eol)
				n = (uint16)(eol - p);
			uint8 *eof = (uint8 *) memchr(p, '\32', n);
			if (eof) { // end-of-file
				n = (uint16)(eof - p);
				_ptr = (uint16)(eof - _buff);
			}
			if (n)
				memcpy(buf, p, n);
			buf += n;
			total += n;
			if (eof)
				break;
			_ptr += n;
			if (eol) {
				_ptr++;
				*(buf++) = '\n';
				total++;
				if (_ptr >= _lim)
					readBuf();
				if (_ptr < _lim)
					if (_buff[_ptr] == '\n')
						++_ptr;
				break;
			}
		} else
			break;
	}
	*buf = '\0';
	return total;
}

int IoBuf::read() {
	debugC(1, kCGEDebugFile, "IoBuf::read()");

	if (_ptr >= _lim) {
		readBuf();
		if (_lim == 0)
			return -1;
	}
	return _buff[_ptr++];
}

uint16 CFile::_maxLineLen = kLineMaxSize;

CFile::CFile(const char *name, Crypt *crypt)
	: IoBuf(name, crypt) {
	debugC(1, kCGEDebugFile, "CFile::CFile(%s, crypt)", name);
}

CFile::~CFile() {
}

long CFile::mark() {
	debugC(5, kCGEDebugFile, "CFile::mark()");

	return _bufMark + _ptr;
}

long CFile::seek(long pos) {
	debugC(1, kCGEDebugFile, "CFile::seek(%ld)", pos);

	if (pos >= _bufMark && pos < _bufMark + _lim) {
		_ptr = (uint16)(pos - _bufMark);
		return pos;
	} else {
		_lim = 0;
		_ptr = 0;
		return _bufMark = IoHand::seek(pos);
	}
}

} // End of namespace CGE
