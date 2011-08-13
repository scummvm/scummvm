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

#include "cge/btfile.h"
#include "common/system.h"
#include "common/str.h"
#include "cge/cge.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/memstream.h"

namespace CGE {

BtFile::BtFile(const char *name, IOMode mode, Crypt *crpt)
	: IoHand(name, mode, crpt) {
	debugC(1, kCGEDebugFile, "BtFile::BtFile(%s, %d, crpt)", name, mode);

	for (int i = 0; i < kBtLevel; i++) {
		_buff[i]._page = new BtPage;
		_buff[i]._pgNo = kBtValNone;
		_buff[i]._indx = -1;
		_buff[i]._updt = false;
		assert(_buff[i]._page != NULL);
	}
}


BtFile::~BtFile() {
	debugC(1, kCGEDebugFile, "BtFile::~BtFile()");
	for (int i = 0; i < kBtLevel; i++) {
		putPage(i, false);
		delete _buff[i]._page;
	}
}


void BtFile::putPage(int lev, bool hard) {
	debugC(1, kCGEDebugFile, "BtFile::putPage(%d, %s)", lev, hard ? "true" : "false");

	if (hard || _buff[lev]._updt) {
		seek(_buff[lev]._pgNo * kBtSize);
		write((uint8 *) _buff[lev]._page, kBtSize);
		_buff[lev]._updt = false;
	}
}


BtPage *BtFile::getPage(int lev, uint16 pgn) {
	debugC(1, kCGEDebugFile, "BtFile::getPage(%d, %d)", lev, pgn);

	if (_buff[lev]._pgNo != pgn) {
		int32 pos = pgn * kBtSize;
		putPage(lev, false);
		_buff[lev]._pgNo = pgn;
		if (size() > pos) {
			seek((uint32) pgn * kBtSize);

			// Read in the page
			byte buffer[kBtSize];
			int bytesRead = read(buffer, kBtSize);

			// Unpack it into the page structure
			Common::MemoryReadStream stream(buffer, bytesRead, DisposeAfterUse::NO);
			_buff[lev]._page->read(stream);

			_buff[lev]._updt = false;
		} else {
			memset(&_buff[lev]._page, 0, kBtSize);
			_buff[lev]._page->_hea._count = 0;
			_buff[lev]._page->_hea._down = kBtValNone;
			_buff[lev]._updt = true;
		}
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


int keycomp(const void *k1, const void *k2) {
	return scumm_strnicmp((const char *) k1, (const char*) k2, kBtKeySize);
}

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

} // End of namespace CGE
