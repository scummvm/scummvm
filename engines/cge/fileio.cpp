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

#include "common/system.h"
#include "common/str.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/memstream.h"
#include "cge/cge.h"
#include "cge/fileio.h"

namespace CGE {

Dat *VFile::_dat = NULL;
BtFile *VFile::_cat = NULL;
VFile *VFile::_recent = NULL;

uint16 XCrypt(void *buf, uint16 siz, uint16 seed) {
	byte *b = static_cast<byte *>(buf);

	for (uint16 i = 0; i < siz; i++)
		*b++ ^= seed;
	
	return seed;
}

/*-----------------------------------------------------------------------
 * IOHand
 *-----------------------------------------------------------------------*/
IoHand::IoHand(Crypt *crypt) : XFile(), _crypt(crypt), _seed(kCryptSeed) {
	_file = new Common::File();
}

IoHand::IoHand(const char *name, Crypt *crypt)
		: XFile(), _crypt(crypt), _seed(kCryptSeed) {
	_file = new Common::File();
	_file->open(name);
}

IoHand::~IoHand() {
	_file->close();
	delete _file;
}

uint16 IoHand::read(void *buf, uint16 len) {
	if (!_file->isOpen())
		return 0;

	uint16 bytesRead = _file->read(buf, len);
	if (!bytesRead)
		error("Read %s - %d bytes", _file->getName(), len);
	if (_crypt)
		_seed = _crypt(buf, len, kCryptSeed);
	return bytesRead;
}

long IoHand::mark() {
	return _file->pos();
}

long IoHand::seek(long pos) {
	_file->seek(pos, SEEK_SET);
	return _file->pos();
}

long IoHand::size() {
	return _file->size();
}

bool IoHand::exist(const char *name) {
	return Common::File::exists(name);
}

/*-----------------------------------------------------------------------
 * IoBuf
 *-----------------------------------------------------------------------*/
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
	debugC(1, kCGEDebugFile, "IoBuf::IoBuf(%s, crypt)", name);

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

/*-----------------------------------------------------------------------
 * CFile
 *-----------------------------------------------------------------------*/
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

/*-----------------------------------------------------------------------
 * BtPage
 *-----------------------------------------------------------------------*/
void BtPage::read(Common::ReadStream &s) {
	_hea._count = s.readUint16LE();
	_hea._down = s.readUint16LE();

	if (_hea._down == kBtValNone) {
		// Leaf list
		for (int i = 0; i < kBtLeafCount; ++i) {
			s.read(_lea[i]._key, kBtKeySize);
			_lea[i]._mark = s.readUint32LE();
			_lea[i]._size = s.readUint16LE();
		}
	} else {
		// Root index
		for (int i = 0; i < kBtInnerCount; ++i) {
			s.read(_inn[i]._key, kBtKeySize);
			_inn[i]._down = s.readUint16LE();
		}
	}
}

/*-----------------------------------------------------------------------
 * BtFile
 *-----------------------------------------------------------------------*/
BtFile::BtFile(const char *name, Crypt *crpt)
	: IoHand(name, crpt) {
	debugC(1, kCGEDebugFile, "BtFile::BtFile(%s, crpt)", name);

	for (int i = 0; i < kBtLevel; i++) {
		_buff[i]._page = new BtPage;
		_buff[i]._pgNo = kBtValNone;
		_buff[i]._indx = -1;
		assert(_buff[i]._page != NULL);
	}
}

BtFile::~BtFile() {
	debugC(1, kCGEDebugFile, "BtFile::~BtFile()");
	for (int i = 0; i < kBtLevel; i++)
		delete _buff[i]._page;
}

BtPage *BtFile::getPage(int lev, uint16 pgn) {
	debugC(1, kCGEDebugFile, "BtFile::getPage(%d, %d)", lev, pgn);

	if (_buff[lev]._pgNo != pgn) {
		int32 pos = pgn * kBtSize;
		_buff[lev]._pgNo = pgn;
		assert(size() > pos);
		// In the original, there was a check verifying if the
		// purpose was to write a new file. This should only be
		// to create a new file, thus it was removed.
		seek((uint32) pgn * kBtSize);

		// Read in the page
		byte buffer[kBtSize];
		int bytesRead = read(buffer, kBtSize);

		// Unpack it into the page structure
		Common::MemoryReadStream stream(buffer, bytesRead, DisposeAfterUse::NO);
		_buff[lev]._page->read(stream);

		_buff[lev]._indx = -1;
	}
	return _buff[lev]._page;
}

BtKeypack *BtFile::find(const char *key) {
	debugC(1, kCGEDebugFile, "BtFile::find(%s)", key);

	int lev = 0;
	uint16 nxt = kBtValRoot;
	while (!_error) {
		BtPage *pg = getPage(lev, nxt);
		// search
		if (pg->_hea._down != kBtValNone) {
			int i;
			for (i = 0; i < pg->_hea._count; i++) {
				// Does this work, or does it have to compare the entire buffer?
				if (scumm_strnicmp((const char *)key, (const char*)pg->_inn[i]._key, kBtKeySize) < 0)
					break;
			}
			nxt = (i) ? pg->_inn[i - 1]._down : pg->_hea._down;
			_buff[lev]._indx = i - 1;
			lev++;
		} else {
			int i;
			for (i = 0; i < pg->_hea._count - 1; i++) {
				if (scumm_stricmp((const char *)key, (const char *)pg->_lea[i]._key) <= 0)
					break;
			}
			_buff[lev]._indx = i;
			return &pg->_lea[i];
		}
	}
	return NULL;
}

/*-----------------------------------------------------------------------
 * Dat
 *-----------------------------------------------------------------------*/
Dat::Dat(): _file(kDatName, XCrypt) {
	debugC(1, kCGEDebugFile, "Dat::Dat()");
}

/*-----------------------------------------------------------------------
 * VFile
 *-----------------------------------------------------------------------*/
void VFile::init() {
	debugC(1, kCGEDebugFile, "VFile::init()");

	_dat = new Dat();
	_cat = new BtFile(kCatName, XCrypt);
	_recent = NULL;
}

void VFile::deinit() {
	delete _dat;
	delete _cat;
}

VFile::VFile(const char *name) : IoBuf(NULL) {
	debugC(3, kCGEDebugFile, "VFile::VFile(%s)", name);

	if (_dat->_file._error || _cat->_error)
		error("Bad volume data");
	BtKeypack *kp = _cat->find(name);
	if (scumm_stricmp(kp->_key, name) != 0)
		_error = 1;
	_endMark = (_bufMark = _begMark = kp->_mark) + kp->_size;
}

VFile::~VFile() {
	if (_recent == this)
		_recent = NULL;
}

bool VFile::exist(const char *name) {
	debugC(1, kCGEDebugFile, "VFile::exist(%s)", name);

	return scumm_stricmp(_cat->find(name)->_key, name) == 0;
}

void VFile::readBuf() {
	debugC(3, kCGEDebugFile, "VFile::readBuf()");

	if (_recent != this) {
		_dat->_file.seek(_bufMark + _lim);
		_recent = this;
	}
	_bufMark = _dat->_file.mark();
	long n = _endMark - _bufMark;
	if (n > kBufferSize)
		n = kBufferSize;
	_lim = _dat->_file.read(_buff, (uint16) n);
	_ptr = 0;
}

long VFile::mark() {
	debugC(5, kCGEDebugFile, "VFile::mark()");

	return (_bufMark + _ptr) - _begMark;
}

long VFile::size() {
	debugC(1, kCGEDebugFile, "VFile::size()");

	return _endMark - _begMark;
}

long VFile::seek(long pos) {
	debugC(1, kCGEDebugFile, "VFile::seek(%ld)", pos);

	_recent = NULL;
	_lim = 0;
	return (_bufMark = _begMark + pos);
}

} // End of namespace CGE
