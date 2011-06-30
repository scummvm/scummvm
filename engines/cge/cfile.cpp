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
#include <fcntl.h>
#include <string.h>
#include "common/system.h"

namespace CGE {

IoBuf::IoBuf(IOMODE mode, CRYPT *crpt)
	: IoHand(mode, crpt),
	  _bufMark(0),
	  _ptr(0),
	  _lim(0) {
	_buff = farnew(uint8, IOBUF_SIZE);
	if (_buff == NULL)
		error("No core for I/O");
}


IoBuf::IoBuf(const char *name, IOMODE mode, CRYPT *crpt)
	: IoHand(name, mode, crpt),
	  _bufMark(0),
	  _ptr(0),
	  _lim(0) {
	_buff = farnew(uint8, IOBUF_SIZE);
	if (_buff == NULL)
		error("No core for I/O [%s]", name);
}

IoBuf::~IoBuf(void) {
	if (Mode > REA)
		writeBuff();
	if (_buff)
		free(_buff);
}


void IoBuf::readBuff(void) {
	_bufMark = IoHand::mark();
	_lim = IoHand::read(_buff, IOBUF_SIZE);
	_ptr = 0;
}


void IoBuf::writeBuff(void) {
	if (_lim) {
		IoHand::write(_buff, _lim);
		_bufMark = IoHand::mark();
		_lim = 0;
	}
}


uint16 IoBuf::read(void *buf, uint16 len) {
	uint16 total = 0;
	while (len) {
		if (_ptr >= _lim)
			readBuff();
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
	uint16 total = 0;

	while (total < LINE_MAX - 2) {
		if (_ptr >= _lim)
			readBuff();
		uint8 *p = _buff + _ptr;
		uint16 n = _lim - _ptr;
		if (n) {
			if (total + n >= LINE_MAX - 2)
				n = LINE_MAX - 2 - total;
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
					readBuff();
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
	uint16 tot = 0;
	while (len) {
		uint16 n = IOBUF_SIZE - _lim;
		if (n > len)
			n = len;
		if (n) {
			memcpy(_buff + _lim, buf, n);
			_lim += n;
			len -= n;
			buf = (uint8 *)buf + n;
			tot += n;
		} else
			writeBuff();
	}
	return tot;
}


uint16 IoBuf::write(uint8 *buf) {
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
	if (_ptr >= _lim) {
		readBuff();
		if (_lim == 0)
			return -1;
	}
	return _buff[_ptr++];
}


void IoBuf::write(uint8 b) {
	if (_lim >= IOBUF_SIZE)
		writeBuff();
	_buff[_lim++] = b;
}


uint16  CFile::_maxLineLen   = LINE_MAX;


CFile::CFile(const char *name, IOMODE mode, CRYPT *crpt)
	: IoBuf(name, mode, crpt) {
}


CFile::~CFile(void) {
}


void CFile::flush(void) {
	if (Mode > REA)
		writeBuff();
	else
		_lim = 0;

	/*
	_BX = Handle;
	_AH = 0x68;       // Flush buffer
	asm   int 0x21
	*/
	warning("FIXME: CFILE::Flush");
}


long CFile::mark(void) {
	return _bufMark + ((Mode > REA) ? _lim : _ptr);
}


long CFile::seek(long pos) {
	if (pos >= _bufMark && pos < _bufMark + _lim) {
		((Mode == REA) ? _ptr : _lim) = (uint16)(pos - _bufMark);
		return pos;
	} else {
		if (Mode > REA)
			writeBuff();
		else
			_lim = 0;

		_ptr = 0;
		return _bufMark = IoHand::seek(pos);
	}
}


void CFile::append(CFile &f) {
	seek(size());
	if (f.Error == 0) {
		while (true) {
			if ((_lim = f.IoHand::read(_buff, IOBUF_SIZE)) == IOBUF_SIZE)
				writeBuff();
			else
				break;
			if ((Error = f.Error) != 0)
				break;
		}
	}
}

} // End of namespace CGE
