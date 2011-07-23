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

IoBuf::IoBuf(IOMode mode, CRYPT *crpt)
	: IoHand(mode, crpt),
	  _bufMark(0),
	  _ptr(0),
	  _lim(0) {
	debugC(1, kCGEDebugFile, "IoBuf::IoBuf(%d, crpt)", mode);

	_buff = (uint8 *) malloc(sizeof(uint8) * kBufferSize);
	if (_buff == NULL)
		error("No core for I/O");
}


IoBuf::IoBuf(const char *name, IOMode mode, CRYPT *crpt)
	: IoHand(name, mode, crpt),
	  _bufMark(0),
	  _ptr(0),
	  _lim(0) {
	debugC(1, kCGEDebugFile, "IoBuf::IoBuf(%s, %d, crpt)", name, mode);

	_buff = (uint8 *) malloc(sizeof(uint8) * kBufferSize);
	if (_buff == NULL)
		error("No core for I/O [%s]", name);
}

IoBuf::~IoBuf() {
	debugC(6, kCGEDebugFile, "IoBuf::~IoBuf()");

	if (_mode != kModeRead)
		writeBuf();
	free(_buff);
}


void IoBuf::readBuf() {
	debugC(4, kCGEDebugFile, "IoBuf::readBuf()");

	_bufMark = IoHand::mark();
	_lim = IoHand::read(_buff, kBufferSize);
	_ptr = 0;
}


void IoBuf::writeBuf() {
	debugC(4, kCGEDebugFile, "IoBuf::writeBuf()");

	if (_lim) {
		IoHand::write(_buff, _lim);
		_bufMark = IoHand::mark();
		_lim = 0;
	}
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


uint16 IoBuf::write(void *buf, uint16 len) {
	debugC(1, kCGEDebugFile, "IoBuf::write(buf, %d)", len);

	uint16 tot = 0;
	while (len) {
		uint16 n = kBufferSize - _lim;
		if (n > len)
			n = len;
		if (n) {
			memcpy(_buff + _lim, buf, n);
			_lim += n;
			len -= n;
			buf = (uint8 *)buf + n;
			tot += n;
		} else
			writeBuf();
	}
	return tot;
}


uint16 IoBuf::write(uint8 *buf) {
	debugC(1, kCGEDebugFile, "IoBuf::write(buf)");

	uint16 len = 0;
	if (buf) {
		len = strlen((const char *) buf);
		if (len)
			if (buf[len - 1] == '\n')
				--len;
		len = write(buf, len);
		if (len) {
			static char EOL[] = "\r\n";
			uint16 n = write(EOL, sizeof(EOL) - 1);
			len += n;
		}
	}
	return len;
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


void IoBuf::write(uint8 b) {
	debugC(1, kCGEDebugFile, "IoBuf::write(%d)", b);

	if (_lim >= kBufferSize)
		writeBuf();
	_buff[_lim++] = b;
}


uint16  CFile::_maxLineLen   = kLineMaxSize;


CFile::CFile(const char *name, IOMode mode, CRYPT *crpt)
	: IoBuf(name, mode, crpt) {
	debugC(1, kCGEDebugFile, "CFile::CFile(%s, %d, crpt)", name, mode);
}


CFile::~CFile() {
}


void CFile::flush() {
	debugC(1, kCGEDebugFile, "CFile::flush()");

	if (_mode != kModeRead)
		writeBuf();
	else
		_lim = 0;

	/*
	_BX = Handle;
	_AH = 0x68;       // Flush buffer
	asm   int 0x21
	*/
	warning("FIXME: CFILE::Flush");
}


long CFile::mark() {
	debugC(5, kCGEDebugFile, "CFile::mark()");

	return _bufMark + ((_mode != kModeRead) ? _lim : _ptr);
}


long CFile::seek(long pos) {
	debugC(1, kCGEDebugFile, "CFile::seek(%ld)", pos);

	if (pos >= _bufMark && pos < _bufMark + _lim) {
		((_mode == kModeRead) ? _ptr : _lim) = (uint16)(pos - _bufMark);
		return pos;
	} else {
		if (_mode != kModeRead)
			writeBuf();
		else
			_lim = 0;

		_ptr = 0;
		return _bufMark = IoHand::seek(pos);
	}
}


void CFile::append(CFile &f) {
	debugC(1, kCGEDebugFile, "CFile::append(f)");

	seek(size());
	if (f._error == 0) {
		while (true) {
			if ((_lim = f.IoHand::read(_buff, kBufferSize)) == kBufferSize)
				writeBuf();
			else
				break;
			if ((_error = f._error) != 0)
				break;
		}
	}
}

} // End of namespace CGE
