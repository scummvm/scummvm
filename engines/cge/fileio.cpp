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

/*-----------------------------------------------------------------------
 * IOHand
 *-----------------------------------------------------------------------*/
IoHand::IoHand(Crypt *crypt) : _error(0), _crypt(crypt), _seed(kCryptSeed) {
	_file = new Common::File();
}

IoHand::IoHand(const char *name, Crypt *crypt)
		: _error(0), _crypt(crypt), _seed(kCryptSeed) {
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
		_seed = _crypt(buf, len);
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

/*-----------------------------------------------------------------------
 * IoBuf
 *-----------------------------------------------------------------------*/
IoBuf::IoBuf(Crypt *crypt)
	: IoHand(crypt),
	  _bufMark(0),
	  _ptr(0),
	  _lim(0) {
	debugC(1, kCGEDebugFile, "IoBuf::IoBuf(crypt)");

	_buff = (uint8 *)malloc(sizeof(uint8) * kBufferSize);
	assert(_buff != NULL);
}

IoBuf::IoBuf(const char *name, Crypt *crypt)
	: IoHand(name, crypt),
	  _bufMark(0),
	  _ptr(0),
	  _lim(0) {
	debugC(1, kCGEDebugFile, "IoBuf::IoBuf(%s, crypt)", name);

	_buff = (uint8 *)malloc(sizeof(uint8) * kBufferSize);
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
CFile::CFile(const char *name, Crypt *crypt) : IoBuf(name, crypt) {
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
	_header._count = s.readUint16LE();
	_header._down = s.readUint16LE();

	if (_header._down == kBtValNone) {
		// Leaf list
		for (int i = 0; i < kBtLeafCount; ++i) {
			s.read(_leaf[i]._key, kBtKeySize);
			_leaf[i]._mark = s.readUint32LE();
			_leaf[i]._size = s.readUint16LE();
		}
	} else {
		// Root index
		for (int i = 0; i < kBtInnerCount; ++i) {
			s.read(_inner[i]._key, kBtKeySize);
			_inner[i]._down = s.readUint16LE();
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
		if (pg->_header._down != kBtValNone) {
			int i;
			for (i = 0; i < pg->_header._count; i++) {
				// Does this work, or does it have to compare the entire buffer?
				if (scumm_strnicmp((const char *)key, (const char*)pg->_inner[i]._key, kBtKeySize) < 0)
					break;
			}
			nxt = (i) ? pg->_inner[i - 1]._down : pg->_header._down;
			_buff[lev]._indx = i - 1;
			lev++;
		} else {
			int i;
			for (i = 0; i < pg->_header._count - 1; i++) {
				if (scumm_stricmp((const char *)key, (const char *)pg->_leaf[i]._key) <= 0)
					break;
			}
			_buff[lev]._indx = i;
			return &pg->_leaf[i];
		}
	}
	return NULL;
}

bool BtFile::exist(const char *name) {
	debugC(1, kCGEDebugFile, "BtFile::exist(%s)", name);

	return scumm_stricmp(find(name)->_key, name) == 0;
}

/*-----------------------------------------------------------------------
 * EncryptedStream
 *-----------------------------------------------------------------------*/
EncryptedStream::EncryptedStream(const char *name) {
	debugC(3, kCGEDebugFile, "EncryptedStream::EncryptedStream(%s)", name);

	_error = false;
	if (_dat->_error || _cat->_error)
		error("Bad volume data");
	BtKeypack *kp = _cat->find(name);
	if (scumm_stricmp(kp->_key, name) != 0)
		_error = true;

	_dat->_file->seek(kp->_mark);
	byte *dataBuffer = (byte *)malloc(kp->_size);
	_dat->_file->read(dataBuffer, kp->_size);
	XCrypt(dataBuffer, kp->_size);
	_readStream = new Common::MemoryReadStream(dataBuffer, kp->_size, DisposeAfterUse::YES);
}

uint32 EncryptedStream::read(void *dataPtr, uint32 dataSize) {
	return _readStream->read(dataPtr, dataSize);
}

bool EncryptedStream::err() {
	return (_error & _readStream->err());
}

bool EncryptedStream::eos() {
	return _readStream->eos();
}

bool EncryptedStream::seek(int32 offset) {
	return _readStream->seek(offset);
}

Common::String EncryptedStream::readLine() {
	return _readStream->readLine();
}

int32 EncryptedStream::size() {
	return _readStream->size();
}

int32 EncryptedStream::pos() {
	return _readStream->pos();
}

EncryptedStream::~EncryptedStream() {
}

} // End of namespace CGE
