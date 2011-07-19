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

namespace CGE {

BtFile::BtFile(const char *name, IOMODE mode, CRYPT *crpt)
	: IoHand(name, mode, crpt) {
	debugC(1, kDebugFile, "BtFile::BtFile(%s, %d, crpt)", name, mode);

	for (int i = 0; i < kBtLevel; i++) {
		_buff[i]._page = new BtPage;
		_buff[i]._pgNo = kBtValNone;
		_buff[i]._indx = -1;
		_buff[i]._updt = false;
		if (_buff[i]._page == NULL)
			error("No core");
	}
}


BtFile::~BtFile() {
	debugC(1, kDebugFile, "BtFile::~BtFile()");
	for (int i = 0; i < kBtLevel; i++) {
		putPage(i, false);
		delete _buff[i]._page;
	}
}


void BtFile::putPage(int lev, bool hard) {
	debugC(1, kDebugFile, "BtFile::putPage(%d, %s)", lev, hard ? "true" : "false");

	if (hard || _buff[lev]._updt) {
		seek(_buff[lev]._pgNo * sizeof(BtPage));
		write((uint8 *) _buff[lev]._page, sizeof(BtPage));
		_buff[lev]._updt = false;
	}
}


BtPage *BtFile::getPage(int lev, uint16 pgn) {
	debugC(1, kDebugFile, "BtFile::getPage(%d, %d)", lev, pgn);

	if (_buff[lev]._pgNo != pgn) {
		int32 pos = pgn * sizeof(BtPage);
		putPage(lev, false);
		_buff[lev]._pgNo = pgn;
		if (size() > pos) {
			seek((uint32) pgn * sizeof(BtPage));
			read((uint8 *) _buff[lev]._page, sizeof(BtPage));
			_buff[lev]._updt = false;
		} else {
			_buff[lev]._page->_hea._count = 0;
			_buff[lev]._page->_hea._down = kBtValNone;
			memset(_buff[lev]._page->_data, '\0', sizeof(_buff[lev]._page->_data));
			_buff[lev]._updt = true;
		}
		_buff[lev]._indx = -1;
	}
	return _buff[lev]._page;
}

BtKeypack *BtFile::find(const char *key) {
	debugC(1, kDebugFile, "BtFile::find(%s)", key);

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


void BtFile::make(BtKeypack *keypack, uint16 count) {
	debugC(1, kDebugFile, "BtFile::make(keypack, %d)", count);

#if kBtLevel != 2
#error This tiny BTREE implementation works with exactly 2 levels!
#endif
	_fqsort(keypack, count, sizeof(*keypack), keycomp);
	uint16 n = 0;
	BtPage *Root = getPage(0, n++);
	BtPage *Leaf = getPage(1, n);
	Root->_hea._down = n;
	putPage(0, true);
	while (count--) {
		if (Leaf->_hea._count >= ArrayCount(Leaf->_lea)) {
			putPage(1, true);     // save filled page
			Leaf = getPage(1, ++n);   // take empty page
			memcpy(Root->_inn[Root->_hea._count]._key, keypack->_key, kBtKeySize);
			Root->_inn[Root->_hea._count++]._down = n;
			_buff[0]._updt = true;
		}
		Leaf->_lea[Leaf->_hea._count++] = *(keypack++);
		_buff[1]._updt = true;
	}
}

} // End of namespace CGE
